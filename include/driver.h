/*
 * $Id: driver.h,v 1.15 2015/02/16 07:07:31 nagatou Exp $
 */

/***************************************************
 this file is header files for evaluator.
                 Copyright (C) 1998 Naoyuki Nagatou
 ***************************************************/

#ifndef RCCS_DRIVER_H
#  define RCCS_DRIVER_H
#  ifdef EVAL
/***************************************************
 *  class name:g_step_exec 
 *-------------------------------------------------*
 *  this is global and static, and
 * in verification mode, eval() becomes single-step
 * execution mode.
 *-------------------------------------------------*
 *  methods:              
 ***************************************************/
      static bool g_step_exec=FALSE;
      static bool g_emptyness=FALSE;
      static bool g_state_counter=0;
      static int g_counter_for_counterexamples=0;
#     ifndef DEPTH_LIMIT
#        define DEPTH_LIMIT 128
#     endif
#     ifndef MAX_COUNTEREXAMPLES
#        define MAX_COUNTEREXAMPLES 3
#     endif

/***************************************************
 *  class name:sort                                *
 *-------------------------------------------------*
 *-------------------------------------------------*
 *  methods:                                       *
 ***************************************************/
      EXTERN list_t sort;

/***************************************************
 *  class name:primitive agents                    *
 *-------------------------------------------------*
 *-------------------------------------------------*
 *  methods:                                       *
 ***************************************************/
#     define STOP_ACT_SIZE 6                  /* B022 */
      EXTERN char *stopacttbl[STOP_ACT_SIZE]={ /* B022 */
         "stop",                               /* B022 */
         "zero",                               /* B022 */ /* E001 */
         "Stop",                               /* B022 */
         "abort",                              /* B022 */
         "ABORT",                              /* B022 */
         "Abort",                              /* B022 */
      };                                       /* B022 */
                                         /* B022 */
#     define PRIMOP_SIZE 4                    /* B022 */
      EXTERN char *primoptbl[PRIMOP_SIZE]={
         "load","LOAD",
         "quit","QUIT",
      };

///***************************************************
// *  class name:primitive operator representation set *
// *-------------------------------------------------*
// *-------------------------------------------------*
// *  methods:                                       *
// ***************************************************/
//EXTERN listp primreptbl[PRIMOP_SIZE]={ /*** do not used ***/
//   &epsilon, &epsilon,
//   &epsilon, &epsilon,
//   &epsilon, &epsilon,
//   &epsilon, &epsilon,
//};
///***************************************************
// *  class name:primitive operator argment list.    *
// *-------------------------------------------------*
// *-------------------------------------------------*
// *  methods:                                       *
// ***************************************************/
//EXTERN char *primargtbl[PRIMOP_SIZE]={ /*** do not used ***/
//   "","",
//   "","",
//   "","",
//   "","",
//};
      static list_t driver_loop(list_t,list_t);
      static list_t eval(list_t,list_t,list_t,queue_t,list_t);
      static list_t evalval(element_t ,list_t,queue_t);
      static list_t evalval_ls(list_t,list_t,queue_t);
#  endif

/*** function decroletar ***/
   extern void driver(void);
   extern int getval(element_t);
   extern char *getstr(element_t);
   extern token gettk(element_t);
   extern list_t getls(element_t);
#endif
