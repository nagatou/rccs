#ifndef __MC_INTERFACE_H__
#define __MC_INTERFACE_H__

#include "gdb_control.h"

#ifndef EXTERN
#define EXTERN extern
#endif

#include "comm.h"

int mc_init_debugger(/*bindLSp hash_tbl[], */char *prog, char *args);
int mc_set_breakpoint(const char *file, int line, char flag);
int mc_set_br(element_t label, char flag);
char *mc_recv(element_t label, buffer *buf);
int mc_send(element_t label, char value[]);
char *mc_get_value(char *var, buffer *buf);
void mc_exit_debugger();
void mc_back(int state_num);

#endif /*__MC_INTERFACE_H__*/
