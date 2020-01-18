/*
 * $Id: error.h,v 1.2 2011/07/27 23:33:50 nagatou Exp $
 */
/***************************************************
   this file is header files for syntax analysis.
                 Copyright (C) 1998 Naoyuki Nagatou
 ***************************************************/

#ifndef RCCS_ERROR_H
#  define RCCS_ERROR_H
#  ifndef ERROR_DEC
#     define ERROR_DEC
      typedef unsigned int error_code_t;
/*
 * Error codes:
 *   15              7             0
 *  +-------------------------------+
 *  | | | | | | | | | | | | | | | | |
 *  +-------------------------------+
 *   ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^
 *   | | | | | | | | | | | | | | | |
 *   | | | | | | | | | | | | | | | +- ESYM
 *   | | | | | | | | | | | | | | +--- ETK
 *   | | | | | | | | | | | | | +----- ELS
 *   | | | | | | | | | | | | +------- ECHAR
 *   | | | | | | | | | | | +--------- EEL
 *   | | | | | | | | | | +----------- RESERVED
 *   | | | | | | | | | +------------- RESERVED
 *   | | | | | | | | +--------------- RESERVED
 *   | | | | | | | +----------------- RESERVED
 *   | | | | | | +------------------- RESERVED
 *   | | | | | +--------------------- RESERVED
 *   | | | | +----------------------- RESERVED
 *   | | | +------------------------- SUCCESS
 *   | | +--------------------------- SYNTAX
 *   | +----------------------------- WARNING
 *   +------------------------------- FATAL
 */
#     define SUCCESS 0x1000 /*** Continue rccs. */
#     define FATAL   0x8000 /*** Terminate rccs. */
#     define WARNING 0x4000 /*** Terminate evaluation of exp but continue rccs. */
#     define SYNTAX  0x2000 /*** For any syntax errors, terminate to evaluate exp, continue rccs. */
#     define ESYM    0x0001
#     define ETK     0x0002
#     define ELS     0x0004
#     define ECHAR   0x0008
#     define EEL     0x0010
#  endif

   EXTERN jmp_buf driver_env;
   EXTERN jmp_buf terminate_driver_env;
   EXTERN jmp_buf jumptoeval_env;

/*** function declaretions ***/
   extern error_code_t error(error_code_t code,const char *format,...);
   extern void setup_signal_stack(void);
#endif
