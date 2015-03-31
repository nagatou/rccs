/*** MODE: Gnu C ***/
#include "stdio.h"
#include "stdlib.h"
#include <signal.h>
//#include "pthread.h"
#include "setjmp.h"
//#include "error.h"
#define EXTERN
#include "setupsignalstack.h"


//static sigjmp_buf return_point;

static void signal_handler(int sig, siginfo_t * sig_info, void * sig_data){
   if (sig==SIGSEGV){
//      siglongjmp(return_point, 2);
      siglongjmp(signal_driver_env, 2);
   }
}
//static void meaningless_recursive_function(int i){
//   fprintf(stderr,"%d->",i);
//   meaningless_recursive_function(i+1);
//}
static void register_sigaltstack(){
   stack_t newSS, oldSS;

   newSS.ss_sp = malloc(ALT_STACK_SIZE);
   newSS.ss_size = ALT_STACK_SIZE;
   newSS.ss_flags = 0;
   sigaltstack(&newSS,&oldSS);
}
void setup_signal_stack(){
   struct sigaction newAct, oldAct;

   register_sigaltstack();
   sigemptyset(&newAct.sa_mask);
   sigaddset(&newAct.sa_mask, SIGSEGV);
   newAct.sa_sigaction = signal_handler;
   newAct.sa_flags = SA_SIGINFO|SA_RESTART|SA_ONSTACK;
   sigaction(SIGSEGV, &newAct, &oldAct);
   return;
}
