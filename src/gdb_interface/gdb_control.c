#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "gdb_control.h"

gdbmi_result *
wait_for_async_stop(gdb_handle *h) {
	gdbmi_result *result = (gdbmi_result *)NULL;
	
	get_gdb_output(&h);
	result = parse_gdbmi_response(h);
	if (!result) {
		log_printf(LOG_ERROR, "ERROR while waiting\n");
	}
	return result;
}

gdbmi_result *
gdb_file(gdb_handle *h, const char *fn) {
	gdbmi_result *result;
 	
 	send_to_gdb(h, "-file-exec-and-symbols %s\n", fn);
 	get_gdb_output(&h);
	result = parse_gdbmi_response(h);
	
	return result;
}

gdbmi_result *
gdb_run(gdb_handle *h) {
	gdbmi_result *result;
	
	send_to_gdb(h, "-exec-run\n");
	get_gdb_output(&h);	
	//parse_gdbmi_response(h);
	
	result = wait_for_async_stop(h);
	return result;
}

gdbmi_result *
gdb_next(gdb_handle *h) {
	gdbmi_result *result;
	
	send_to_gdb(h, "-exec-next\n");
	get_gdb_output(&h);	
	//result = parse_gdbmi_response(h);
	result = wait_for_async_stop(h);
	return result;	
}

gdbmi_result *
gdb_step(gdb_handle *h) {
	gdbmi_result *result;
	
	send_to_gdb(h, "-exec-step\n");
	get_gdb_output(&h);	
	//result = parse_gdbmi_response(h);
	result = wait_for_async_stop(h);
	return result;	
}

gdbmi_result *
gdb_continue(gdb_handle *h) {
	gdbmi_result *result;
	
	send_to_gdb(h, "-exec-continue\n");
	get_gdb_output(&h);	
	//result = parse_gdbmi_response(h);
	
	result = wait_for_async_stop(h);
	return result;	
}

gdbmi_result *
gdb_finish(gdb_handle *h) {
	gdbmi_result *result;
	
	send_to_gdb(h, "-exec-finish\n");
	get_gdb_output(&h);	
	//result = parse_gdbmi_response(h);
	result = wait_for_async_stop(h);
	return result;	
}

/*if flag==FALSE, a temporary bkpt is set.*/
gdbmi_result *
gdb_insert_breakpoint(gdb_handle *h, const char *location, char flag) {
	gdbmi_result *result;
	assert(location != (char *)NULL);
	
	if (flag)
		send_to_gdb(h, "-break-insert %s\n", location);
	else
		send_to_gdb(h, "-break-insert -t %s\n", location);
	get_gdb_output(&h);
	result = parse_gdbmi_response(h);
	return result;
}

gdbmi_result *
gdb_print(gdb_handle *h, const char *exp) {
	gdbmi_result *result;
	
	assert (exp != (const char *)NULL);
	
	send_to_gdb(h, "-data-evaluate-expression %s\n", exp);
	get_gdb_output(&h);
	result = parse_gdbmi_response(h);
	return result;
}

gdbmi_result *
gdb_set_var(gdb_handle *h, const char *var, const char val[]) {
	gdbmi_result *result = NULL;
	
	assert(var && val);
	
	send_to_gdb(h, "-gdb-set var %s=%s\n", var, val);
	get_gdb_output(&h);
	result = parse_gdbmi_response(h);
	return result;
}

gdbmi_result *
gdb_set_args(gdb_handle *h, const char *args) {
	gdbmi_result *result = NULL;
	
	send_to_gdb(h, "-exec-argument %s\n", args);
	get_gdb_output(&h);
	result = parse_gdbmi_response(h);
	return result;
}

/*
 * Parse a line of source code listed by gdb in the format <linenum> <var = value;>+
 * Return value is the varname.
 * Needs to be free()'d.
 * */
char *
parse_line(char *line) {
	char *var = (char *)NULL;
	char *tmpR = (char *)NULL;
/*	char *tmpL = (char *)NULL;  Not use */
	char *tmpvar = (char *)NULL;
	char *tmp = (char *)NULL;
	
	if (!line) {
		return (char *)NULL;
	}

	tmpvar = strdup(line);
	
	tmpR = strchr(tmpvar, '=');
	if (!tmpR) {
		log_printf(LOG_ERROR, "cannot parse the varname.\n");
		return (char *)NULL;
	}
	else
		--tmpR;
	//eliminate whitespace characters and operators between the var and '='
	while (isspace(*tmpR) || \
				*tmpR == '+' || *tmpR == '-' || \
				*tmpR == '*' || *tmpR == '/')
		--tmpR;
	
	
	*(tmpR+1) = 0;
	
	//discard the line number
	while (isdigit(*tmpvar))
		++tmpvar;
		
	//reject all the preceding whitespace characters.
	while ((tmp = strpbrk(tmpvar, " \t")))
		tmpvar = tmp + 1;

	var = strdup(tmpvar);
	
//	myfree(tmpvar);
	return var;	
}

char *getaline(FILE *f) {
	char *buf;              /* buffer for line */
   size_t size;            /* size of buffer */
   size_t inc;             /* how much to enlarge buffer */
   size_t len;             /* # of chars stored into buf before '\0' */
   char *p;
   const size_t thres = 128; /* initial buffer size*/
   const size_t mucho = 128; /* if there is at least this much wasted
                                space when the whole buffer has been
                                read, try to reclaim it.*/

   len = 0;
   size = thres;
   buf = mymalloc(size);
   if (buf == NULL)
   	return NULL;

   while (fgets(buf+len, size-len, f) != NULL) {
      len += strlen(buf+len);
      if (len > 0 && buf[len-1] == '\n')
         break;          /* the whole line has been read */

      for (inc = size, p = NULL; p == NULL && inc > 0; inc /= 2)
         p = realloc(buf, size + inc);

      if (inc <= 0 || p == NULL) {
         free(buf);
         return NULL;    /* couldn't get more memory */
      }

      size += inc;
      buf = p;
   }

   if (len == 0) {
   	free(buf);
      return NULL;    /* nothing read (eof or error) */
   }

   if (buf[len-1] == '\n') /* go back on top of the newline */
   	--len;
   buf[len] = '\0';        /* unconditionally terminate string,
                              possibly overwriting newline */

   if (size - len > mucho) { /* a plenitude of unused memory? */
   	p = realloc(buf, len+1);
      if (p != NULL) {
      	buf = p;
         size = len+1;
      }
   }
   
   return buf;
}

char *
gdb_get_varname(gdb_handle *h, char *file, int line) {
	int count = 0;
	FILE *fp = NULL;
	char *aline = NULL;
	char *var = NULL;

	//send_to_gdb(h, "list %s, %s\n", location, location);
	//get_gdb_output(&h);	
	//return parse_line(h->response);
	//printf("here %s we go\n", aline);
	
	fp = fopen(file, "r");	
	if(!fp) {
		log_printf(LOG_ERROR, "Cannot open source file %s\n", file);
		return (char *)NULL;
	}
		 
	for (count = 0; count < line; count++) {
		aline = getaline(fp);
		if (!aline)
			break;
	}

	if (!aline) {
		log_printf(LOG_ERROR, "Cannot read line number %d of file %s.\n", line, file);
		return NULL;
	}
	
	var = parse_line(aline);
	myfree(aline);
	
	fclose(fp);
	return var;
}
