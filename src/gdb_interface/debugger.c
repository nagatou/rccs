#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "debugger.h"
#include "errors.h"
#include "log_printf.h"
#include "mi_parser.h"

gdb_handle *
alloc_handle() {
	gdb_handle *handle = (gdb_handle *) calloc(1, sizeof(gdb_handle));
	if (handle == (gdb_handle *)NULL) {
		m_error = ERR_GDB_HANDLE_ALLOC;
		return (gdb_handle *) NULL;
	}
	handle->to_gdb[0] = handle->to_gdb[1] = handle->from_gdb[0] = handle->from_gdb[1] = -1;
	handle->gdb_pid = -1;
	
	handle->len_response = 0;
	handle->response = (char *)NULL;
	
	return handle;	
}

void
free_handle(gdb_handle **h) {
	/*TODO: Close all the open streams/pipes*/
	if (*h) {
		fclose((*h)->gdbin);
		fclose((*h)->gdbout);
		
		myfree((*h)->gdb_bin);
		myfree(*h);
		*h = NULL;
	}
	return;
}

char *
get_gdb_bin() {
	char *colon_pos_ptr;
	char *path;
	char *gdb_path = (char *) mymalloc (256 * sizeof(char));
	
	gdb_path[0] = '\0';

#ifdef MC_LINUX //compiling for target LINUX
	path = getenv("PATH");
	if (path ==(char *)NULL) {
   	m_error = ERR_PATH_NOT_SET;
   	log_msg();
   	path = strdup("/usr/bin");
	}
	
	/*Search for the GDB binary in the $PATH*/
	colon_pos_ptr = strstr(path, ":");
	if (!colon_pos_ptr) colon_pos_ptr = path + strlen(path);
   do {
    	strncpy(gdb_path, path, colon_pos_ptr - path);
      gdb_path[colon_pos_ptr - path] = '\0';
      strcat(gdb_path, "/gdb");
      if (access(gdb_path, X_OK) == 0) {
         return gdb_path;
      }
      else 
      	path = colon_pos_ptr + 1;
   }
   while ((colon_pos_ptr = strstr(path, ":")) != NULL);
	
#else //compiling for target T-Engine
	path = getenv("GNU_BD");
	if (path == (char *)NULL) {
   	m_error = ERR_PATH_NOT_SET;
   	log_msg();

#if defined(__CYGWIN__) 
   	   path = strdup("/usr/local/te/tool/Cygwin-i686");
#else
   	   path = strdup("/usr/local/te/tool/Linux-i686");	
#endif	
	}
	gdb_path = strcat(gdb_path, path);
	gdb_path = strcat(gdb_path, "/bin/sh-unknown-tmonitor-gdb");
	
	//gdb_path = strdup("/usr/bin/gdb");//debug
	
	if (access(gdb_path, X_OK) == 0)
	   return gdb_path;
#endif //of #ifdef MC_LINUX
	
	m_error = ERR_GDB_NOT_FOUND;
	return (char *) NULL;
}

void
set_pipes_nonblocking(int p) {
	int f;
	f = fcntl(p, F_GETFL);
	f |= O_NONBLOCK;
	fcntl(p, F_SETFL, f);
}

/*
 * Initialize the handler for GDB.
 * Returns the pointer to the handler on success, NULL on error.
 * */
gdb_handle * 
gdb_init(const char *prog){
   pid_t p;
	char *gdb_bin;
	gdb_handle *h;
	
	/*Get the PATH from the environment, and find the GDB binary.*/	
	gdb_bin = get_gdb_bin();	
   if (gdb_bin == (char *)NULL) {
		log_msg();
		return (gdb_handle *)NULL;
	}
	/* Initialise GDB if everything is okay*/	
    else {
		log_printf(LOG_INFO, "GDB FOUND at [%s]\n", gdb_bin);
		
		h = alloc_handle();
		if (h == (gdb_handle *)NULL) {
			log_msg();
			return (gdb_handle *)NULL;
		}
		h->gdb_bin = gdb_bin;
		
		/* Create pipe for communicating with GDB.*/
		/* NOTE: Pipes have to be created before fork(), 
		 * otherwise the child process won't inherit them.*/
		if (pipe(h->to_gdb) || pipe(h->from_gdb)){
			free_handle(&h);
			m_error = STD_ERR_NO;
			log_msg();
			return (gdb_handle *)NULL;
		}
		
		/* Set the pipes to non-blocking mode*/
		set_pipes_nonblocking(h->to_gdb[1]);
		set_pipes_nonblocking(h->from_gdb[0]);
		
		/* Open streams*/
		h->gdbin = fdopen(h->to_gdb[1], "w");
		h->gdbout = fdopen(h->from_gdb[0], "r");
		
		if (!h->gdbin || !h->gdbout) {
			free_handle(&h);
			m_error = STD_ERR_NO;
			log_msg();
			return (gdb_handle *)NULL;
		}
		
		p = fork();
		h->gdb_pid = p;
		/* fork() fails*/
		if (p == -1) {
			free_handle(&h);
			m_error = STD_ERR_NO;
			log_msg();
			return (gdb_handle *)NULL;
		}
		/* Child process*/
		if (p == 0) {
			char *gdb_args[4];
			
			/* Connect the stdin/stdout of GDB to the pipe.*/
			dup2(h->to_gdb[0], STDIN_FILENO);
			dup2(h->from_gdb[1], STDOUT_FILENO);
			dup2(h->from_gdb[1], STDERR_FILENO);
			
			/* Execute GDB*/
			gdb_args[0] = gdb_bin;
			gdb_args[1] = "--quiet";
			gdb_args[2] = "--interpreter=mi";
			gdb_args[3] = strdup(prog);
			gdb_args[4] = (char *) NULL;
			
			execvp(gdb_args[0], gdb_args);
			
			/* exec fails*/
			free_handle(&h);
			m_error = STD_ERR_NO;
			log_msg();
			return (gdb_handle *)NULL;
		}
				
   	get_gdb_output(&h);//get the initial gdb response.
		/*parent*/
		return h;
	}
}

int
send_to_gdb(gdb_handle *g_h, const char *cmd, ...) {
	int ret;
	char *cmd_str;
	va_list va_arglist;
	
	va_start(va_arglist, cmd);
	cmd_str = (char *) malloc(sizeof(char) * (strlen(cmd) + 100));
	ret = vsprintf(cmd_str, cmd, va_arglist);
	va_end(va_arglist);
	
	if (ret) {
		ret = fputs(cmd_str, g_h->gdbin);
#ifdef MC_DEBUG
		printf("TO > %s\n\n", cmd_str);//debug
#endif
		fflush(g_h->gdbin);
	}
	myfree(cmd_str);
	/*int n;	
	n = write (gdb_handle->to_gdb[1], cmd, sizeof(cmd));
	if (n == -1) {
		m_error = errno;
		log_msg();
	}
	*/
	return ret;
}

int
get_gdb_response(gdb_handle **h) {
   gdb_handle *g_h = *h;
	char c;
	int len_total_res = 0;
	int n;
	char end_of_response = FALSE;
	
	n = read(g_h->from_gdb[0], &c, 1);
	while (n == 1) {
		if (g_h->len_response <= len_total_res){
			g_h->len_response = len_total_res + 256;
			g_h->response = (char *) realloc(g_h->response, g_h->len_response);
			//TODO: check for realloc error
		}
		g_h->response[len_total_res] = c;
		n = read(g_h->from_gdb[0], &c, 1);
		len_total_res++;
		/* Check if it's the end of GDB response.*/
		if ((len_total_res > 5 && strncmp(g_h->response+len_total_res-6, "(gdb)", 5) == 0) || 
			 (len_total_res > 5 && strncmp(g_h->response+len_total_res-6, "^exit", 5) == 0) ||
			 (len_total_res > 8 && strncmp(g_h->response+len_total_res-9, "^running", 8) == 0)) {
				int count = 0;
   			end_of_response = TRUE;
   			/* Discard the remaining of the response*/
   			while ((0 < read(g_h->from_gdb[0], &c, 1)) &&
   						count < 5) {
   				count++;
   			}
   			break;
			}
		/**/
	}
	
	if (n == -1 && !end_of_response)
		return -1;   
	else {
		g_h->response[len_total_res-6] = '\0';//reject the gdb prompt "(gdb)"
		g_h->len_response = 0;
		return len_total_res;
	}
}

int
get_gdb_output(gdb_handle **h) {
   gdb_handle *g_h = *h;
	int n = 0;
	
	if (g_h->response != (char *)NULL) {
      g_h->len_response = 0;
      myfree(g_h->response);
      g_h->response = (char *)NULL;
	}
	
	while (n ==-1 || n == 0) {
		n = get_gdb_response(&g_h);
		if (n == -1)
			/*TODO: Use select() so that things become faster.*/
			usleep(100);
		//else {
		//}
	}
	return n;
}

void
stop_gdb(gdb_handle *g_h) {
	//TODO: Check if GDB is really running.
	if (g_h) {
		log_printf(LOG_INFO, "\nStopping GDB (process ID %d)\n", g_h->gdb_pid);
		//kill(g_h->gdb_pid, SIGKILL);
		send_to_gdb(g_h, "-gdb-exit\n");
		get_gdb_output(&g_h);
		free_handle(&g_h);
	}
}

gdb_handle *
start_gdb(const char *prog) {
   gdb_handle *h = (gdb_handle *) NULL;
   gdbmi_result *result = (gdbmi_result *)NULL;
   
   h = gdb_init(prog);
   if (!h) {//the memory for GDB handler could not be allocated.
   	log_msg();
      return ((gdb_handle *)NULL);
   }
   
   result = gdb_tty(h, ttyname(STDIN_FILENO));
   free_gdbmi_result(result);
   
   result = gdb_set(h, "confirm", "off");
   free_gdbmi_result(result);
   
   return h;   
}

gdbmi_result *
gdb_tty(gdb_handle *h, const char *tty_name) {	
	gdbmi_result *result;
	
   send_to_gdb(h, "tty %s\n", tty_name);
   get_gdb_output(&h);
   result = parse_gdbmi_response(h);
   return result;
}

gdbmi_result *
gdb_set(gdb_handle *h, const char *var, const char *val) {
	gdbmi_result *result;
	
	send_to_gdb(h, "set %s %s\n",var, val);
	get_gdb_output(&h);
	result = parse_gdbmi_response(h);
	return result;
}
