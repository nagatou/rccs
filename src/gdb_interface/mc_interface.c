#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "mc_interface.h"

struct state_data {
	char *state;
	char *location;
	char *value;
	char *var;
};
typedef struct state_data *StateData;

struct state_history {
	StateData data;
		
	struct state_history *next;
	struct state_history *prev;
};
typedef struct state_history *State;

/* Global variables*/
gdb_handle *g_h = (gdb_handle *)NULL;
State initState = NULL;
State lastState = NULL;

void
free_state_data(StateData data) {
	myfree(data->state);
	myfree(data->location);
	myfree(data->value);
	myfree(data->var);
}

void
free_states() {
	State cur = lastState;
	State tmp;
	while (cur) {
		tmp = cur;
		free_state_data(cur->data);
		cur = cur->prev;
		myfree(tmp);
	}
}

StateData
new_state_data() {
	StateData data = (StateData) mymalloc(sizeof(struct state_data));
	
	data->state = NULL;
	data->location = NULL;
	data->value = NULL;
	data->var = NULL;
	
	return data;
}

State
new_state() {
	State aState = (State) mymalloc(sizeof(struct state_history));
	
	aState->data = NULL;
	aState->next = NULL;
	aState->prev = NULL;
	return aState;
}

int
append_state(StateData data) {
	State temp = new_state();
	
	temp->data = data;
	temp->next = NULL;
	temp->prev = lastState;
	if (lastState) {
		lastState->next = temp;
	}
	lastState = temp;
	if (!initState) {
		initState = lastState;
	}
	
	return 1;
}

int
delete_last_state() {
	State tmp = NULL;
	if (lastState) {
		tmp = lastState;
		if (tmp->prev) {
			lastState = tmp->prev;
			lastState->next = NULL;
			free_state_data(tmp->data);
			myfree(tmp);
			return 1;
		}
		else {//initial state reached
#ifdef MC_DEBUG
			log_printf(LOG_WARNING, "(delete_last_state):cannot delete the initial state\n");
#endif
			return 0;
		}
	}
	else 
		return 0;
}

/*
 * Starts GDB as a separate process.
 * Loads the inferior program inside the GDB process.
 * Sets breakpoint at the beginning of the inferior program.
 * Runs the program. The programs stops at the main function.
 * Returns 1 on success, 0 on failure.
 * */
int
mc_init_debugger(char *prog, char *args) {
	gdbmi_result *result = (gdbmi_result *)NULL;
	int status = 0;
	StateData data = new_state_data();
	
	g_h = start_gdb(prog);
	if (!g_h) {
		return status;//failed to initialize GDB
	}
	status = 1;

#ifdef MC_LINUX
	//result = gdb_file(g_h, prog);
	//free_gdbmi_result(result);
		
	/*Set the program arguments, if any*/
	if (args) {
		result = gdb_set_args(g_h, args);
		free_gdbmi_result(result);
	}
	
	/*Set the breakpoint at the beginning of the program (main function)*/
	result = gdb_insert_breakpoint(g_h, "main", FALSE);
	free_gdbmi_result(result);
	
	/*Run the inferior program*/
	result = gdb_run(g_h);
	free_gdbmi_result(result);
#endif
	
	/*create the initState*/
	data->state = strdup("INIT");
	data->location = strdup("main");
	data->var = NULL;
	data->value = NULL;
	append_state(data);
	
	return status;
}

/*
 * Exit GDB
 * */
void
mc_exit_debugger() {
	free_states();
	stop_gdb(g_h);
}

/*
 * Set a temporary breakpoint at the location corresponding to label,
 * and execute upto that location.
 * Assign the 'value' to the variable at that location.
 * Returns 0 on error, 1 on success.
 * */
int
mc_send(element_t label, char value[]) {
	char *file = (char *)NULL;
	int line = 0;
	filename_lineno_pair_t *fl;
	//char *location = (char *)NULL;
	StateData data = new_state_data();
	
	assert(g_h);

#ifdef MC_DEBUG
	log_printf(LOG_INFO, " SEND->\n");
#endif
	
	data->state = strdup("SEND");;
	data->value = strdup(value);
	
	/*retrieve filename:linenumber pair from the
	 * hash table, using label as the key*/
	fl = retrieve_filename_lineno_pair(label);
	if (fl) {
		file = strdup(fl->file_or_host);
		line = fl->line_or_socket;
		//fprintf(stderr, "filename = %s\n linenum=%d\n", file, line);
	}
	else {
		log_printf(LOG_ERROR, "No filename:linenum corresponding to label.\n");
	}
	//asprintf(&location,"%s:%d", file, line);
	data->location = (char *) malloc(sizeof(char) * (strlen(file) + 10));
	sprintf(data->location, "%s:%d", file, line+1);
	
	if (mc_set_breakpoint(file, line + 1, FALSE)) {
		char *var = (char *)NULL;
		//gdbmi_result *res_tmp = (gdbmi_result *)NULL;
		gdbmi_result *result = (gdbmi_result *)NULL;
		
		result = gdb_continue(g_h);
		if (result) {
			free_gdbmi_result(result);
			var = gdb_get_varname(g_h, file, line);
			if (var) {
				result = gdb_set_var(g_h, var, value);
				data->var = strdup(var);
				myfree(var);
				free_gdbmi_result(result);
			}
		}
	}
	
	//myfree(location);
	append_state(data);
	
	return 1;
}

/*
 * Set a temporary breakpoint at the location corresponding to label,
 * and execute upto that location.
 * Return the value of the variable at the location.
 * */
char *
mc_recv(element_t label, buffer *buf){
	char *file = (char *)NULL;
	int line = 0;
	filename_lineno_pair_t *fl;
	//char *location = (char *)NULL;
	StateData data = new_state_data();
	
	assert(g_h);

#ifdef MC_DEBUG
	log_printf(LOG_INFO, " RECV->\n");
#endif	

	data->state = strdup("RECV");
	
	/*retrieve filename:linenumber pair from the
	 * hash table, using label as the key*/
	fl = retrieve_filename_lineno_pair(label);
	if (fl) {
		file = strdup(fl->file_or_host);
		line = fl->line_or_socket;
		//fprintf(stderr, "filename = %s\n linenum=%d\n", file, line);
	}
	else {
		log_printf(LOG_ERROR, "No filename:linenum corresponding to label.\n");
	}
	
	//asprintf(&location, "%s:%d", file, line);
	data->location = (char *) malloc(sizeof(char) * (strlen(file) + 10));
	sprintf(data->location, "%s:%d", file, line+1);
	if (mc_set_breakpoint(file, line + 1, FALSE)) {
		char *var = (char *)NULL;
		gdbmi_result *res_tmp = (gdbmi_result *)NULL;
		gdbmi_result *result = (gdbmi_result *)NULL;
		
		result = gdb_continue(g_h);
		if (result) {
			free_gdbmi_result(result);
			var = gdb_get_varname(g_h, file, line);
			if (var) {
				result = gdb_print(g_h, var);
				if (!result) {
					log_printf(LOG_ERROR, "cannot get the value of %s\n", var);
					return (char *)NULL;
				}
				
				res_tmp = mi_parser_get_tuple_value(result, "value");
				if (res_tmp && res_tmp->type == GDBMI_LITERAL) {
					initbuf(buf);
					ins_buf(buf, res_tmp->data.literal);
				}
				data->value = strdup(res_tmp->data.literal);
				data->var = strdup(var);
				myfree(var);
				free_gdbmi_result(result);
			}
		}
	}
	/*make list for returning the value*/
	append_state(data);
	//myfree(location);
	return buf->buf;
}

/*
 * Set breakpoint at the location corresponding to 'label'.
 * flag = TRUE => permanent breakpoint
 * flag = FALSE => temporary breakpoint
 * */
int
mc_set_br(element_t label, char flag) {
	int linenum = 0;
	char *file = (char *)NULL;
	int line = 0;
	filename_lineno_pair_t *fl;
	
	assert(g_h);
	
	/*retrieve filename:linenumber pair from the
	 * hash table, using label as the key*/
	fl = retrieve_filename_lineno_pair(label);
	if (fl) {
		file = strdup(fl->file_or_host);
		line = fl->line_or_socket;
	}
	else {
		log_printf(LOG_ERROR, "No filename:linenum corresponding to label.\n");
	}
	
	linenum = mc_set_breakpoint(file, line, flag);
	myfree(file);
	return linenum;
}

/*
 * Sets breakpoint at location file:line.
 * flag = TRUE => permanent breakpoint
 * flag = FALSE => temporary breakpoint
 * Returns the linenum of breakpoint's location on success, 0 on error.
 * */
int
mc_set_breakpoint(const char *file, int line, char flag) {
	gdbmi_result *result = (gdbmi_result *)NULL;
	gdbmi_result *res_tmp = (gdbmi_result *)NULL;
	char *location = (char *) NULL;
	int linenum = 0;
	
	assert(g_h);
	
	if (file == (char *) NULL || !line) {
		return 0;
	}

	location = (char *) malloc(sizeof(char) * (strlen(file) + 10));	
	sprintf(location, "%s:%d", file, line);
	result = gdb_insert_breakpoint(g_h, location, flag);
	if (!result) {
		log_printf(LOG_ERROR, "Cannot set the breakpoint at %s", location);
	}
	else {
		res_tmp = mi_parser_get_tuple_value(result, "bkpt");
		res_tmp = mi_parser_get_tuple_value(res_tmp, "line");
		if (res_tmp && res_tmp->type == GDBMI_LITERAL) {
			linenum = atoi(res_tmp->data.literal);
		}
		else
			linenum = 0;
	}
	free_gdbmi_result(result);
	myfree(location);
	
	return linenum;
}

/*
 * Get the value of variable 'var'.
 * */
char *
mc_get_value(char *var, buffer *buf){
	gdbmi_result *result = (gdbmi_result *)NULL;
	gdbmi_result *res_tmp = (gdbmi_result *)NULL;
	
	assert(g_h);
	
	result = gdb_print(g_h, var);
	if (!result) {
		log_printf(LOG_ERROR, "cannot get the value of %s\n", var);
		return (char *)NULL;
	}
	
	res_tmp = mi_parser_get_tuple_value(result, "value");
	if (res_tmp && res_tmp->type == GDBMI_LITERAL) {
		initbuf(buf);
		ins_buf(buf, res_tmp->data.literal);
	}
	free_gdbmi_result(result);
	return buf->buf;
}

void
re_send(StateData data) {
	gdbmi_result *result = NULL;
	
	assert(g_h);
#ifdef MC_DEBUG
	log_printf(LOG_INFO, " Re-sending->\n");
#endif	
	result = gdb_insert_breakpoint(g_h, data->location, FALSE);
	free_gdbmi_result(result);
	
	result = gdb_continue(g_h);
	free_gdbmi_result(result);
	
	result = gdb_set_var(g_h, data->var, data->value);
	free_gdbmi_result(result);
}

/*
 * Go back by #state_num states, and
 * re-execute the program from the beginning
 * */
void
mc_back(int state_num) {
	int i = 0;
	State curState = initState;
	gdbmi_result *result = NULL;
	
	assert(g_h);
	
#ifdef MC_DEBUG
	log_printf(LOG_INFO, " Re-executing [upto %d last state(s)]\n", state_num);
#endif
	
	if(!initState) {
		log_printf(LOG_ERROR, "backtrace error: NULL initState");
		return;
	}
	
	/*Delete states.*/
	for (i = 0; i < state_num; i++) {
		if (!delete_last_state()) {
#ifdef MC_DEBUG
			log_printf(LOG_WARNING, "Attempting to backtrack from the initial state.\n");
#endif
			return;
		}
	}
	
	/*Set the breakpoint at the beginning of the program (main function)*/
	result = gdb_insert_breakpoint(g_h, "main", FALSE);
	free_gdbmi_result(result);
	
	/*Run the inferior program*/
	result = gdb_run(g_h);
	free_gdbmi_result(result);
	
	while (curState) {
		if (!strcmp(curState->data->state, "SEND")) {
			re_send(curState->data);
		}
		curState = curState->next;
	}
}
