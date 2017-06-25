/*
 * $Id: print.c,v 1.3 2015/02/16 07:08:26 nagatou Exp $ 
 */
//static char rcsid[]="$Id: print.c,v 1.3 2015/02/16 07:08:26 nagatou Exp $";
//static char copyright[]=
//   "Copyright (C) 1998 Naoyuki Nagatou";

/***********************************************
  this file is functions for a print phase.
              Copyright (C) 1998 Naoyuki Nagatou
 ***********************************************/
#define EXTERN
#include "comm.h"

/****************************************************
 *  unparse expressions.
 *--------------------------------------------------*
 *  ret=print(list);                             *
 *                                                 *
 *  listp ret - succ:length of list.                *
 *              fail:ERROR                          *
 *  listp list - any list                           * 
 ****************************************************/
static char *operators(buffer *buf,const element_t el)
{
#  ifdef DEBUG_PRINT
   printf("operators[");
   printf("%s",buf->buf);
   printf("]->");
#  endif
   if (buf==(buffer *)NIL)
      return((splp)error(FATAL,"Segmentation fault(operators32)\n"));
   else{
      prtsym(BUF,gettk(el),buf);
      return(buf->buf);
   }
}
static char *label(buffer *buf,const element_t el)
{
#  ifdef DEBUG_PRINT
   printf("label");
   printf("%s",buf->buf);
   printf("]->");
#  endif
   if (buf==(buffer *)NIL)
      return((splp)error(5,"Segmentation fault(label30)\n"));
   else{
      operators(buf,el);
      return(buf->buf);
   }
}
static splp bool_exp(buffer *buf,const element_t el);
static splp val_exp(buffer *buf,const element_t el)
{
   list_t ls=getls(el);

#  ifdef DEBUG_PRINT
   printf("val_exp[");
   printf("%s",buf->buf);
   printf("]->");
#  endif
   if ((buf==(buffer *)NIL)||(ls==(list_t)NIL))
      return((splp)error(FATAL,"Segmentation fault(val_exp63)\n"));
   else{
      if (car(ls).type==TOKEN){
         switch(gettk(car(ls))->token_name){
            case VALUE_OP:
               switch(gettk(car(ls))->attr.op.type){
                  case PLUS:
                  case MINS:
                  case MULT:
                  case DIV:
                  case MOD:
                     val_exp(buf,car(getls(car(cdr(ls)))));
                     label(buf,car(ls));
                     val_exp(buf,car(cdr(getls(car(cdr(ls))))));
                     break;
                  case SEQ:
                  default:
                     return((splp)error(FATAL,"invalid field type[%d](val_exp80)\n",car(ls).entry.tk->attr.op.type));
               }
               break;
            case BOOL_OP:
               bool_exp(buf,el);
               break;
            case VALUE:
            case ID:
               label(buf,car(ls));
               break;
            default:
               return((splp)error(FATAL,"invalid token[%d](val_exp91)\n",gettk(car(ls))->token_name));
         }
      }
      else
         return((splp)error(FATAL,"Invalid element type(val_exp95)\n"));
      return(buf->buf);
   }
}
static splp val_exp_ls2(buffer *buf,const list_t ls)
{
#  ifdef DEBUG_PRINT
   printf("val_exp_ls2[");
   printf("%s",buf->buf);
   printf("]->");
#  endif
   if ((buf==(buffer *)NIL)||(ls==(list_t)NIL))
      return((splp)error(FATAL,"Segmentation fault(val_exp_ls2101)\n"));
   else{
      if (isempty(ls))
         return(buf->buf);
      else{
         val_exp_ls2(buf,cdr(ls));
         val_exp(buf,car(ls));
         ins_buf(buf,",");
         return(buf->buf);
      }
   }
}
static splp val_exp_ls1(buffer *buf,const list_t ls)
{
#  ifdef DEBUG_PRINT
   printf("val_exp_ls1[");
   printf("%s",buf->buf);
   printf("]->");
#  endif
   if ((buf==(buffer *)NIL)||(ls==(list_t)NIL))
      return((splp)error(FATAL,"Segmentation fault(val_exp_ls1121)\n"));
   else{
      val_exp_ls2(buf,cdr(ls));
      val_exp(buf,car(ls));
      return(ins_buf(buf,")"));
      return(buf->buf);
   }
}
static splp val_exp_ls(buffer *buf,const element_t el)
{
   list_t ls = getls(el);

#  ifdef DEBUG_PRINT
   printf("val_exp_ls[");
   prtlst(ls);
   printf("]->");
#  endif
   if ((buf==(buffer *)NIL)||(ls==(list_t)NIL))
      return((splp)error(FATAL,"Segmentation fault(val_exp_ls139)\n"));
   else{
      if (isempty(ls))
         return(buf->buf);
      ins_buf(buf,"(");
      return(val_exp_ls1(buf,getls(el)));
   }
}
static splp val_var(buffer *buf,const element_t el)
{
   list_t ls=getls(el);

#  ifdef DEBUG_PRINT
   printf("val_var[");
   printf("%s",buf->buf);
   printf("]->");
#  endif
   if ((buf==(buffer *)NIL)||(ls==(list_t)NIL))
      return((splp)error(FATAL,"Segmentation fault(val_var157)\n"));
   else{
      return(operators(buf,car(ls)));
   }
}
static splp val_var_ls2(buffer *buf,const list_t ls)
{
#  ifdef DEBUG_PRINT
   printf("val_var_ls2[");
   printf("%s",buf->buf);
   printf("]->");
#  endif
   if ((buf==(buffer *)NIL)||(ls==(list_t)NIL))
      return((splp)error(FATAL,"Segmentation fault(val_var_ls170)\n"));
   else{
      if (isempty(ls))
         return(buf->buf);
      else{
         val_var_ls2(buf,cdr(ls));
         val_var(buf,car(ls));
         ins_buf(buf,",");
         return(buf->buf);
      }
   }
}
static splp val_var_ls1(buffer *buf,const list_t ls)
{
#  ifdef DEBUG_PRINT
   printf("val_var_ls1[");
   printf("%s",buf->buf);
   printf("]->");
#  endif
   if ((buf==(buffer *)NIL)||(ls==(list_t)NIL))
      return((splp)error(FATAL,"Segmentation fault(val_var_ls190)\n"));
   else{
      val_var_ls2(buf,cdr(ls));
      val_var(buf,car(ls));
      return(ins_buf(buf,")"));
      return(buf->buf);
   }
}
static splp val_var_ls(buffer *buf,const element_t el)
{
   list_t ls=getls(el);

#  ifdef DEBUG_PRINT
   printf("val_var_ls-> ");
#  endif
   if ((buf==(buffer *)NIL)||(ls==(list_t)NIL))
      return((splp)error(FATAL,"Segmentation fault(val_var_ls206)\n"));
   else{
      if (isempty(ls))
         return(buf->buf);
      ins_buf(buf,"(");
      val_var_ls1(buf, getls(el));
      return(buf->buf);
   }
}
static splp bool_exp(buffer *buf,const element_t el)
{
   list_t ls=getls(el);

#  ifdef DEBUG_PRINT
   printf("bool_exp[");
   printf("%s",buf->buf);
   printf("]->");
   fflush(stdout);
#  endif

   if (ls==(list_t)NIL)
      return((splp)error(FATAL,"Segmentation fault(bool_exp225)\n"));
   if (buf==(buffer *)NIL)
      return((splp)error(FATAL,"Segmentation fault(bool_exp227)\n"));
   else{
      switch(gettk(car(ls))->token_name){
         case KEY_WORD:
            label(buf,car(ls));
            break;
         case COMP_OP:
         case BOOL_OP:
            switch(gettk(car(ls))->attr.op.type){
               case EQ:
               case GR:
               case GT:
               case LE:
               case LT:
                  ins_buf(buf,"(");
                  val_exp(buf,car(getls(car(cdr(ls)))));
                  operators(buf,car(ls));
                  val_exp(buf,car(cdr(getls(car(cdr(ls))))));
                  ins_buf(buf,")");
                  break;
               case OR:
               case AND:
                  ins_buf(buf,"(");
                  bool_exp(buf,car(getls(car(cdr(ls)))));
                  operators(buf,car(ls));
                  bool_exp(buf,car(cdr(getls(car(cdr(ls))))));
                  ins_buf(buf,")");
                  break;
               case NOT:
                  operators(buf,car(ls));
                  bool_exp(buf,car(getls(car(cdr(ls)))));
                  break;
               default:
                  return((splp)error(FATAL,"invalid field type(bool_exp255) (%d)\n", car(ls).entry.tk->attr.op.type));
            }
            break;
         case VALUE:
         case ID:
            label(buf,car(ls));
            break;
         default:
            return((splp)error(FATAL,"Invalid token type(bool_exp263) (token_name=%d)\n", car(ls).entry.tk->token_name));
      }
      return(buf->buf);
   }
}
static splp operand(buffer *buf,const list_t ls)
{
#  ifdef DEBUG_PRINT
   printf("operand [");
   printf("%s",buf->buf);
   printf("]->");
#  endif
   if ((buf==(buffer *)NIL)||(ls==(list_t)NIL))
      return((splp)error(FATAL,"Segmentation fault(operand276)\n"));
   else{
      if (isempty(ls)){
         ins_buf(buf,"()");
         return(buf->buf);
      }
      ins_buf(buf,"(");
      if ((car(ls).type==TOKEN)
        &&(car(ls).entry.tk->token_name==AGENT_OP)){
         switch(car(ls).entry.tk->attr.op.type){
            case RECV:
               label(buf,car(cdr(ls)));
               val_var_ls(buf,car(cdr(cdr(ls))));
               ins_buf(buf,":");
               operand(buf,getls(car(cdr(cdr(cdr(ls))))));
               break;
            case SEND:
               operators(buf,car(ls));
               label(buf,car(cdr(ls)));
               val_exp_ls(buf,car(cdr(cdr(ls))));
               ins_buf(buf,":");
               operand(buf,getls(car(cdr(cdr(cdr(ls))))));
               break;
            case SUM:
            case COM:
               operand(buf,getls(car(cdr(ls))));
               operators(buf,car(ls));
               operand(buf,getls(car(cdr(cdr(ls)))));
               break;
            case DEF:
               operators(buf,car(ls));
               ins_buf(buf," ");
               operators(buf,car(cdr(ls)));
               ins_buf(buf," ");
               operand(buf,getls(car(cdr(cdr(ls)))));
               ins_buf(buf," ");
               operand(buf,getls(car(cdr(cdr(cdr(ls))))));
               break;
            case IF:
               operators(buf,car(ls));
               ins_buf(buf,"(");
               bool_exp(buf,car(cdr(ls)));
               ins_buf(buf,")");
               operand(buf,getls(car(cdr(cdr(ls)))));
               operand(buf,getls(car(cdr(cdr(cdr(ls))))));
               break;
            case CON:
               label(buf,car(cdr(ls)));
               val_exp_ls(buf,car(cdr(cdr(ls))));
               break;
            case BIND:
               operators(buf,car(cdr(ls)));
               ins_buf(buf," ");
               operators(buf,car(cdr(cdr(ls))));
               break;
            case REC:
               operand(buf,getls(car(cdr(ls))));
               break;
            case RES:
            case REL:
            default:
               return((splp)error(FATAL,"Please contact me(operand331). (%d)\n", car(ls).entry.tk->attr.op.type));
               break;
         }
      }
      else{
         if (car(ls).type==TOKEN)
            label(buf,car(ls));
      }
      ins_buf(buf,")");
      return(buf->buf);
   }
}
int n_print(const list_t ls)
{
#  ifdef DEBUG_PRINT_A
   printf("n_print-> ");
#  else
   buffer output;
   int ret;

   if (ls==(list_t)NIL)
      return((intptr_t)error(FATAL,"Segmentation fault(print350)\n"));
   else{
      initbuf(&output);
//      printf("\n");
      operand(&output,ls);
      ret = fprintf(stdout,"%s\n",output.buf);
      fflush(stdout);
      return(ret);
   }
#  endif
}
