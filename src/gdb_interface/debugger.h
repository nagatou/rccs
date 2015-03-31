#ifndef __DEBUGGER_H__
#define __DEBUGGER_H__

#include <stdlib.h>
#include <stdio.h>

#include "mi_parser.h"

//typedef char bool;

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

typedef struct {
   /*Pipes connected to GDB*/
	int to_gdb[2];
	int from_gdb[2];
	pid_t gdb_pid;
	char *gdb_bin;
	
	char *response;
	int len_response;
	
   /*Streams*/
	FILE *gdbin, *gdbout;
} gdb_handle;

void stop_gdb(gdb_handle *g_h);
int send_to_gdb(gdb_handle *g_h, const char *cmd, ...);
int get_gdb_output(gdb_handle **g_h);
gdb_handle *start_gdb(const char *prog);
gdbmi_result *gdb_tty(gdb_handle *h, const char *tty_name);
gdbmi_result *gdb_set(gdb_handle *h, const char *var, const char *val);

#endif
