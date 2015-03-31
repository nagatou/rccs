/*
 * $Id: ccs.c,v 1.8 2015/03/31 10:14:10 nagatou Exp $
 */
//static char rcsid[]="@(#)$Id: ccs.c,v 1.8 2015/03/31 10:14:10 nagatou Exp $
//static char copyright[]=
//   "Copyright(C) 1998 Naoyuki Nagatou";

/*****************************************************
   this file is a set of functions for main loop
                   Copyright (C) 1998 Naoyuki Nagatou
 *****************************************************/
#define EXTERN
#include "comm.h"

#ifdef DEBUG_MC
static int mc_init_debugger(char *target,char *args)
{
   return(TRUE);
}
#endif
#ifdef DEBUG_DRIVE
listp buf=NIL;

listp n_bind(element label,element port)
{
   return(makenull(NIL));
}
listp n_send(element label,listp data)
{
   buf = data;
   return(makenull(NIL));
}
listp n_recv(element label)
{
   prtlst(buf);
   return(buf);
}
#endif

static void help(void)
{
   printf("\n\trccs [-tisq] [-f FORMULA] [-d TARGET [ARGUMENTS]] [-m MODEL]\n");
   printf("\n\tWhen you give both -d and -f, then I examine the target in the verification mode and \
give only -f then I make a modelchecking of the model. \
when you don't give both -f and -d, I will run the model in the emulation mode.\n");
   printf("\n\tOPTIONS::\n");
   printf("\t-t\t\t:turns the trace flag on.\n");
   printf("\t-f \"formula\"\t:specifies a formula.\n");
   printf("\t-d \"target\" [\"arguments\"]\t:specifies a target program.\n");
   printf("\t-m \"model\"\tNot yet: specifies a source file written in RCCS.\n");
   printf("\t-q \t\t:changes channels in RCCS to queue buffer(Default).\n");
   printf("\t-s \t\t:changes channels in RCCS to stack buffer.\n");
   printf("\t-i \t\t:provids the interactive execution.\n");
}
static void analyse_options1(int count, int number, char *options[]);
static void analyse_f_option(int count,int number,char *options[],buffer *formula)
{
   if (options[count]==(char *)NIL){
      help();
      error(FATAL,"invalid argments(analyse_f_options60)\n");
   }
   else{
      if (!strncmp("-",options[count],1)){
         help();
         error(FATAL,"%s invalid argments(analyse_f_options65)\n",options[count]);
      }
      else{
         ins_buf(formula,options[count]);
         analyse_options1(count+1,number,options);
      }
   }
}
static void analyse_d_option(int count,int number,char *options[],buffer *target)
{
   if (options[count]==(char *)NIL){
      help();
      error(FATAL,"invalid argments(analyse_d_options62)\n");
   }
   else{
      if (!strncmp("-",options[count],1)){
         help();
         error(FATAL,"%s invalid argments(analyse_d_options67)\n",options[count]);
      }
      else{
         ins_buf(target,options[count]);
         analyse_options1(count+1,number,options);
      }
   }
}
static void analyse_options1(int count, int number, char *options[])
{
   if (count>=number)
      return;
   else{
      if (!strncmp("-",options[count],1)){
         switch(*(options[count]+1)){
            case 'i':
               interactive_mode = ON;
               analyse_options1(count+1,number,options);
               break;
            case 't':
               trace_on = ON;
               analyse_options1(count+1,number,options);
               break;
            case 'd':
               analyse_d_option(count+1,number,options,&target);
               break;
            case 'f':
               interactive_mode = OFF;
               channel_order=C_STACK;
               analyse_f_option(count+1,number,options,&formula);
               break;
            case 'h':
               help();
               break;
            case 'q':
               channel_order=C_QUEUE;
               analyse_options1(count+1,number,options);
               break;
            case 's':
               channel_order=C_STACK;
               analyse_options1(count+1,number,options);
               break;
            case 'm':
            default:
               help();
               error(FATAL,"Invalid options(analyse_options111):%s\n",options[count]);
         }
      }
      else{
         help();
         error(FATAL,"Invalid options(analyse_options198):%s\n",options[count]);
      }
   }
}
static void analyse_options(int number, char *options[])
{
   if (options==(char **)NIL)
      error(FATAL,"invalid argments(analyse_options109)\n");
   else{
      trace_on = OFF;        /* Set default */
      channel_order=C_STACK;
      initbuf(&target);
      initbuf(&formula);
      analyse_options1(1,number,options);
   }
}
int main(int argi,char *argv[])
{
   char *log=(char *)NIL;

   switch(argi){
      case 1:
         log = (char *)NIL;
         trace_on = OFF;
         interactive_mode = OFF;
         initbuf(&target);
         initbuf(&formula);
         break;
      case 2:
         if (!strcmp("-h",argv[1])){
            help();
            return(0);
         }
         else
            ;
      default:
         analyse_options(argi,argv);
   }
/*--Body----------------------------------------*/
   prologue(log);
   if (!isempty_buf(&target)){
      int ret=mc_init_debugger(target.buf,NIL);
      if (ret==ERROR)
         return((int)error(FATAL,"Cannot run a debbuger.\n"));
   }
   if (!isempty_buf(&formula)) /*** fix channel_order at C_STACK in verifier mode. ***/
      channel_order=C_STACK;
   driver();
   epilogue();
   return(0);
}
