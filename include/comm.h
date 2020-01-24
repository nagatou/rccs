/*
 * $Id: comm.h,v 1.10 2015/02/16 07:07:31 nagatou Exp $
 */

/***************************************************
   this file is common header files for R-CCS
                   Copyright (C) 1998 Naoyuki Nagatou
 ***************************************************/
#ifndef RCCS_COMM_H
#  define RCCS_COMM_H
#  include <stdio.h>
#  include <stdlib.h>
#  include <string.h>
#  include <stdarg.h>
#  include <setjmp.h>
#  include <ctype.h>
#  include <unistd.h>
#  include <stdint.h>
#  include <errno.h>
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <sys/time.h>
#  include <sys/resource.h>
#  include <netinet/in.h>
#  include <netdb.h>
#  include <signal.h>
#  include <arpa/inet.h>

#  define NIL NULL
#  ifndef BOOL
#     define BOOL
      typedef enum {FALSE=0,TRUE=!0,OTHERWISE=2,ERROR=0,OFF=0,ON=!0} bool;
#  endif
/*** UNUSED
   typedef int bool;
#  ifndef FALSE
     #define FALSE ((int)0)
#  endif
#  ifndef TRUE
     #define TRUE !FALSE
#  endif
#  ifndef ERROR
     #define ERROR FALSE
#  endif
#  ifndef OTHERWISE
#     define OTHERWISE ((int)2)
#  endif
#  ifndef ON_OFF
#     define OFF FALSE
#     define ON TRUE
#     define ON_OFF
#  endif
******/
/*** the following is not modify,
    because the function "atof()" return double.  ***/
   typedef double FTYPE;
#  define FLT_PREC 15

#  define CR '\r'  /*0Dh*/
#  define LF '\n'  /*0Ah*/

/*** header files for R-CCS ***/
#  include "shand.h"
#  include "fhand.h"
#  include "scan.h"
#  include "list.h"
#  include "parse.h"
#  include "ehand.h"
#  include "error.h"
#  include "print.h"
#  include "gc.h"
#  include "channel.h"

/* header file for ltl */
#  ifndef DEBUG_MC_M
#     include "ltl.h"
#  endif

/*header file for mc_interface*/
#  ifndef DEBUG_MC_GDB
#     include "mc_interface.h"
#  endif


/*** global variables for R-CCS ***/
   EXTERN FILE *source_file;
   EXTERN FILE *bkup;
   EXTERN FILE *log_file;
   EXTERN int trace_on;
   EXTERN bool interactive_mode;
   EXTERN bool g_interactive_mode_backup;
   EXTERN buffer formula;
   EXTERN buffer target;
   typedef enum{C_QUEUE,C_STACK} channel_order_t;
   EXTERN channel_order_t channel_order;
   typedef enum{ACC_WEAKLY,ACC_STRONGLY} acceptance_condition_t;
   EXTERN acceptance_condition_t acceptance_condition;
   EXTERN bool dotfile_condition;

#endif
