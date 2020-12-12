/***************************************************
 this file is header files for RCCS.
                 Copyright (C) 2020 Naoyuki Nagatou
 ***************************************************/
#include <stdio.h>

#ifndef RCCS_OSENV_H
#  define RCCS_OSENV_H
#  define OSENV_MAX_DEPTH 32
   typedef struct{
      FILE *bkup[OSENV_MAX_DEPTH]; 
      int top;
      buffer buf;
   } os_env_t;
#endif
EXTERN os_env_t os_env_backup;

extern void backup_os_env(FILE *file_ptr, buffer);
extern FILE * reload_os_env(void);
extern void init_os_env(void);
