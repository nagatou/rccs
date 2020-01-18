/******************************************************
   this file is a set of functions for event handler.
                  Copyright (C) 1998 Naoyuki Nagatou
 ******************************************************/
#define EXTERN extern
#include "comm.h"
#include "error.h"
#include "primitive.h"

#ifndef ERR_CODE_MASK
#  define ERR_CODE_MASK 0xF000
#endif
/**************************************************
 * print an error message and exit                *
 *------------------------------------------------*
 * void error(code,format,va_alist);              *
 *                                                *
 *  err_t code -- error code                      *
 *  char *format -- output format                 *
 *  va_dcl va_alist -- argments                   *
 *************************************************/
void * error(error_code_t code,const char *format,...)/*B028*/
{
   va_list args;
   char *p=(char *)format;

   va_start(args,format);
   fprintf(stdout,"\n");
   for(;*p;p++){
      if (*p != '%'){
         printf("%c",*p);
         continue;
      }
      switch(*++p){
         case 'c':
            printf("%c",va_arg(args,int));
            break;
         case 'd':
            printf("%d",va_arg(args,int));
            break;
         case 's':{
            switch(code&0x0fff){
               case EEL:
                 prtelt(va_arg(args,element_t));
                 break;
               case ESYM:
                 prtelt(*makelet(TOKEN,
                                va_arg(args,token)));
                 break;
               case ELS:
                 prtlst(va_arg(args,list_t));
                 break;
               case ETK:
                 prtelt(*makelet(TOKEN,
                                va_arg(args,token)));
                 break;
               case ECHAR:
                 printf("%s", va_arg(args,char*));
                 break;
               default:
                 printf("%s",va_arg(args,char*));
            }
            break;
         }
         default:
            printf("%c",*p);
      }
   }
   va_end(args);
   fflush(stdin);
   switch(code&ERR_CODE_MASK){
      case SYNTAX:
         fflush(source_file);
         initsf(NIL,NIL);
      case WARNING:
         break;
      case FATAL:{
         epilogue();
         exit(code);
         break;
      }
      case SUCCESS:
         return(makenull(NIL));
         break;
      case 0xc000:
      default:
         printf("Undefined error code: %x.\n",code);
         epilogue();
         exit(code);
   }
   fflush(stdout);
   fflush(stderr);
   longjmp(driver_env,(code&ERR_CODE_MASK));
   return(makenull(NIL));
}
