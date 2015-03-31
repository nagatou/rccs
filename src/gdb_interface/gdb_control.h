#ifndef __GDB_CONTROL_H__
#define __GDB_CONTROL_H__

#include "debugger.h"
#include "mi_parser.h"
#include "log_printf.h"
gdbmi_result *wait_for_async_stop(gdb_handle *h);
gdbmi_result *gdb_file(gdb_handle *h, const char *fn);
gdbmi_result *gdb_run(gdb_handle *h);
gdbmi_result *gdb_next(gdb_handle *h);
gdbmi_result *gdb_step(gdb_handle *h);
gdbmi_result *gdb_continue(gdb_handle *h);
gdbmi_result *gdb_finish(gdb_handle *h);
gdbmi_result *gdb_insert_breakpoint(gdb_handle *h, const char *location, char flag);
gdbmi_result *gdb_print(gdb_handle *h, const char *exp);
//char *gdb_get_varname(gdb_handle *h, char *location);
char *gdb_get_varname(gdb_handle *h, char *file, int line);
gdbmi_result *gdb_set_var(gdb_handle *h, const char *var, const char val[]);
gdbmi_result *gdb_set_args(gdb_handle *h, const char *args);

#endif //__GDB_CONTROL_H__
