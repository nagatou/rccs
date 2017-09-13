/*
 * $Id: parse.c,v 1.5 2011/07/08 10:46:10 nagatou Exp $
 */
//static char rcsid[]="@(#)$Id: parse.c,v 1.5 2011/07/08 10:46:10 nagatou Exp $";
//static char copyright[]=
//   "Copyright (C) 1998 Naoyuki Nagatou";

/************************************************************************
   this file is a set of functions for parser.
                                 Copyright (C) 1998 Naoyuki Nagatou
 ************************************************************************/
#define EXTERN extern
#define PARSE
#include "comm.h"

/*****************************************************
 *  add attributes for token                         *
 *  its the first argment is token,the following     *
 * argments is attributes.                           *
 * echo attribute field is changed after the called  *
 * function.                                         *
 *---------------------------------------------------*
 *  ret = addattr(tk,...);                           *
 *                                                   *
 *  token ret --- pointer to symbol                  *
 *  token tk --- pointer to symbol                   *
 *****************************************************/
token addattr(token tk,...)
{
   va_list ap;
   int type;

#  ifdef DEBUG
   printf("addattr->");
#  endif
   if (tk==(token)NIL)
      error(FATAL|ETK,"Segmentation fault(parse.c:addattr37) %s\n", tk);
   va_start(ap,tk);
   switch(tk->token_name){
      case ID:
         switch(type=va_arg(ap,field_t)){
            case ACT:
               tk->attr.id.type = type;
               tk->attr.id.fld.act.action_type = va_arg(ap,int);
               tk->attr.id.fld.act.label_type = va_arg(ap,int);
               break;
            case A_CON:
               tk->attr.id.type = type;
               break;
            case A_VAR:
               tk->attr.id.type = type;
               break;
            case V_VAR:
               tk->attr.id.type = type;
               break;
            default:
               error(FATAL|ETK,"Invalid type(parse.c:addattr57) token=%s, type=%d\n", tk, type);
         }
         break;
     default:
         error(FATAL|ETK,"Invalid token name(addattr61) token=%s, token_name=%d\n", tk, tk->token_name);
   }
   va_end(ap);
   return(tk);
}

/*****************************************************
 *---------------------------------------------------*
 *  ret = cvtfa(FLOAT val);                          *
 *                                                   *
 *  splp ret --- pointer to strings                  *
 *  FLOAT val --- value to symbol                    *
 *****************************************************/
//static splp cvtfa(FTYPE val)
//{
//   splp buf=NIL;
//
//   if ((buf=(char *)malloc((sizeof(char))*FLT_PREC))==NIL)
//      return((splp)error(1,"allocation error(cvtfa71)\n"));
//   else
//      return(gcvt(val,FLT_PREC,buf));
//}

/*****************************************************
 *---------------------------------------------------*
 *  ret = cvtia(FLOAT val);                          *
 *                                                   *
 *  splp ret --- pointer to strings                  *
 *  FLOAT val --- value to symbol                    *
 *****************************************************/
static char *n_itoa(int value,char *buf)
{
   const char digits_for_itoa[]="0123456789";
   char *postp=buf;

   do
      *--postp = digits_for_itoa[value % 10];
   while((value /= 10)!=0);
   return(postp);
}

splp cvtia(int val)
{
   static buffer buf;

#  ifdef DEBUG
   printf("val=%d",val);
#  endif
   initbuf(&buf);
   if (val<0){
      char *str = n_itoa(val,buf.buf+(FLT_PREC-1));
      *(--str) = '-';
      return(str);
   }
   else
      return(n_itoa(val,buf.buf+(FLT_PREC-1)));
}

/*****************************************************
 * this function has only synthesized attributes.    *
 *---------------------------------------------------*
 *  ret = fact(void);                                *
 *                                                   *
 *  token ret --- attribute                          *
 *****************************************************/
static list_t fact(void)/*B028*/
{
   list_t ret=NIL;      /*B028*/

#  ifdef DEBUG_PARSE
   printf("fact->");
#  endif
   switch(lah->token_name){
      case VALUE:
         ret = cons(*makelet(TOKEN,lah),makenull(NIL)); /*B028*/
         lah = scanner(&lah);
         break;
      case PARENTHE:
         if (lah->attr.par.fr_or_af == FR){
            lah = scanner(&lah);
            ret = b_exp(); /*B028*/
            if ((lah->token_name==PARENTHE)&&(lah->attr.par.fr_or_af == AF))
               lah = scanner(&lah);
            else
               error(WARNING|ETK,"syntax error(parse.c:fact146) %s\n", lah);
         }
         else
            error(WARNING|ETK,"syntax error(fact149) %s\n", lah);
         break;
      case KEY_WORD:{ /*B041*/
         if ((strcmp(lah->attr.keywd.str,"true")==0)||
             (strcmp(lah->attr.keywd.str,"TRUE")==0)){
            ret = cons(*makelet(TOKEN,/*B028*/
                                makesym(VALUE,ICONST,cvtia((int)TRUE))),/*B028*/ /*B041*/
                       makenull(NIL));
            lah = scanner(&lah);
         }
         else{
            if ((strcmp(lah->attr.keywd.str,"false")==0)||
                (strcmp(lah->attr.keywd.str,"FALSE")==0)){
               ret = cons(*makelet(TOKEN,/*B028*/
                                   makesym(VALUE,ICONST,cvtia((int)FALSE))),/*B028*/ /*B041*/
                          makenull(NIL));
               lah = scanner(&lah);
            }
            else
               error(WARNING|ETK,"syntax error(fact168) %s\n", lah);
         }
         break;
      } /*B041*/
      case ID:                /* B010 */
         ret = cons(*makelet(TOKEN,lah),makenull(NIL));/* B010 */ /*B028*/
         lah = scanner(&lah); /* B010 */
         break;               /* B010 */
      default:
         error(WARNING|ETK,"syntax error(fact177) %s\n", lah);
   }
   return(ret); /*B028*/
}

/*****************************************************
 * this function has only synthesized attributes.    *
 *---------------------------------------------------*
 *  ret = v_unary_exp(void);                         *
 *                                                   *
 *  token ret --- attribute                          *
 *****************************************************/
static list_t v_unary_exp(void)
{
   list_t syp=(list_t)NIL;
   token op=(token)NIL;

#  ifdef DEBUG_PARSE
   printf("v_unary_exp->");
#  endif
   switch(lah->token_name){
      case BOOL_OP:
         if (lah->attr.op.type == NOT){
            op=lah;                                                    /*B028*/
            lah = scanner(&lah);
            syp = cons(*makelet(TOKEN,op),                             /*B028*/
                       cons(*makelet(LIST,cons(*makelet(LIST,v_unary_exp()),/*B028*/
                                                makenull(NIL))),            /*B028*/
                            makenull(NIL)));                           /*B028*/
         }
         break;
      default:
         syp = fact();
   }
   return(syp);
}

/*****************************************************
 *  this function has synthesized attributes and     *
 * inherited attributes.                             *
 *---------------------------------------------------*
 *  ret = v_term_left(inp);                          *
 *                                                   *
 *  token ret --- attribute                          *
 *  token inp --- pointer to inherited attribute     *
 *****************************************************/
static list_t v_term_left(list_t inp)
{
   list_t syp=(list_t)NIL;
   list_t ret=(list_t)NIL;
   token op=(token)NIL;

#  ifdef DEBUG_PARSE
   printf("v_term_left->");
#  endif
   switch(lah->token_name){
      case VALUE_OP:
         switch(lah->attr.op.type){
            case MULT:
            case DIV:
            case MOD:
               op = lah;                                                         /*B028*/
               lah = scanner(&lah);
               syp = v_unary_exp();
               ret = v_term_left(cons(*makelet(TOKEN,op),                        /*B028*/
                                      cons(*makelet(LIST,dotpair(*makelet(LIST,inp),  /*B028*/
                                                            *makelet(LIST,syp))),/*B028*/
                                            makenull(NIL))));                     /*B028*/
               break;
            default:
               ret = inp;
         }
         break;
      default:
         ret = inp;
   }
   return(ret);
}

/*****************************************************
 *  this function has synthesized attributes and     *
 * inherited attributes.                             *
 *---------------------------------------------------*
 *  ret = v_term(void);                              *
 *                                                   *
 *  token ret --- attribute                          *
 *****************************************************/
static list_t v_term(void)
{
   list_t inp=NIL;
   list_t ret=NIL;

#  ifdef DEBUG_PARSE
   printf("v_term->");
#  endif
   inp = v_unary_exp();
   ret = v_term_left(inp);
   return(ret);
}

/*****************************************************
 *  this function has synthesized attributes and     *
 * inherited attributes.                             *
 *---------------------------------------------------*
 *  ret = v_exp_left(inp);                          *
 *                                                   *
 *  token ret --- attribute                          *
 *  token inp --- pointer to inherited attribute     *
 *****************************************************/
static list_t v_exp_left(list_t inp)
{
   list_t syp=(list_t)NIL;
   list_t ret=(list_t)NIL;
   token op=(token)NIL;    /*B028*/

#  ifdef DEBUG_PARSE
   printf("v_exp_left->");
#  endif
   switch(lah->token_name){
      case VALUE_OP:
         switch(lah->attr.op.type){
            case PLUS:
            case MINS:
               op = lah;                                               /*B028*/
               lah = scanner(&lah);
               syp = v_term();
               ret = v_exp_left(cons(*makelet(TOKEN,op),                         /*B028*/
                                      cons(*makelet(LIST,dotpair(*makelet(LIST,inp),   /*B028*/
                                                                 *makelet(LIST,syp))),/*B028*/
                                            makenull(NIL))));                     /*B028*/
               break;
            default:
               ret = inp;
         }
         break;
      default:
         ret = inp;
   }
   return(ret);
}

/*****************************************************
 *  this function has synthesized attributes and     *
 * inherited attributes.                             *
 *---------------------------------------------------*
 *  ret = v_exp(void);                               *
 *                                                   *
 *  token ret --- attribute                          *
 *****************************************************/
static list_t v_exp(void)
{
   list_t syp=NIL;
   list_t ret=NIL;

#  ifdef DEBUG_PARSE
   printf("v_exp->");
#  endif
   syp = v_term();
   ret = v_exp_left(syp);
   return(ret);
}

/*****************************************************
 *  this function has synthesized attributes and     *
 * inherited attributes.                             *
 *---------------------------------------------------*
 *  ret = c_exp_left(inp);                          *
 *                                                   *
 *  token ret --- attribute                          *
 *  token inp --- pointer to inherited attribute     *
 *****************************************************/
static list_t c_exp_left(list_t inp)
{
   list_t syp=(list_t)NIL;
   list_t ret=(list_t)NIL;
   token op=(token)NIL;    /*B028*/

#  ifdef DEBUG_PARSE
   printf("c_exp_left->");
#  endif
   switch(lah->token_name){
      case COMP_OP:
         switch(lah->attr.op.type){
            case LE:
            case LT:
            case GR:
            case GT:
            case EQ:
               op = lah;                                               /*B028*/
               lah = scanner(&lah);
               syp = v_exp();
               ret = c_exp_left(cons(*makelet(TOKEN,op),                         /*B028*/
                                       cons(*makelet(LIST,dotpair(*makelet(LIST,inp),   /*B028*/
                                                                  *makelet(LIST,syp))),/*B028*/
                                             makenull(NIL))));                     /*B028*/
               break;
            default:
               ret = inp;
         }
         break;
      default:
         ret = inp;
   }
   return(ret);
}

/*****************************************************
 *  this function has synthesized attributes and     *
 * inherited attributes.                             *
 *---------------------------------------------------*
 *  ret = c_exp(void);                               *
 *                                                   *
 *  token ret --- attribute                          *
 *****************************************************/
static list_t c_exp(void)/*B028*/
{
   list_t syp=NIL;
   list_t ret=NIL;

#  ifdef DEBUG_PARSE
   printf("c_exp->");
#  endif
   syp = v_exp();
   ret = c_exp_left(syp);
   return(ret);
}

/*****************************************************
 *  this function has synthesized attributes and     *
 * inherited attributes.                             *
 *---------------------------------------------------*
 *  ret = b_exp_left(inp);                          *
 *                                                   *
 *  token ret --- attribute                          *
 *  token inp --- pointer to inherited attribute     *
 *****************************************************/
static list_t b_exp_left(list_t inp)/*B028*/
{
   list_t syp=(list_t)NIL;
   list_t ret=(list_t)NIL;
   token op=(token)NIL;           /*B028*/

#  ifdef DEBUG_PARSE
   printf("b_exp_left->");
#  endif
   switch(lah->token_name){
      case BOOL_OP:
         switch(lah->attr.op.type){
            case OR:
            case AND:
               op = lah;          /*B028*/
               lah = scanner(&lah);
               syp = c_exp();
               ret = b_exp_left(cons(*makelet(TOKEN,op),            /*B028*/
                                      cons(*makelet(LIST,dotpair(*makelet(LIST,inp),*makelet(LIST,syp))),/*B028*/
                                            makenull(NIL))));/*B028*/
               break;
            default:
               ret = inp;
         }
         break;
      default:
         ret = inp;
   }
   return(ret);
}

/*****************************************************
 *  this function has synthesized attributes and     *
 * inherited attributes.                             *
 *---------------------------------------------------*
 *  ret = b_exp(void);                               *
 *                                                   *
 *  token ret --- attribute                          *
 *****************************************************/
static list_t b_exp(void)/*B028*/
{
   list_t syp=NIL;
   list_t ret=NIL;

#  ifdef DEBUG_PARSE
   printf("b_exp->");
#  endif
   syp = c_exp();
   ret = b_exp_left(syp);
   return(ret);
}
/*****************************************************
 *  this function has synthesized attributes.        *
 *---------------------------------------------------*
 *  ret = v_fact(void);                              *
 *                                                   *
 *  elementp ret --- pointer to element              *
 *****************************************************/
static elementp v_fact(void)         /*B028*/
{

#  ifdef DEBUG_PARSE
   printf("v_fact->");
#  endif
   return(makelet(LIST,b_exp()));
}

/*****************************************************
 *  this function has synthesized attributes and     *
 * inherited attributes.                             *
 *---------------------------------------------------*
 *  ret = ex_relabel_seq_left(inp);                  *
 *                                                   *
 *  list_t inp --- pointer to list for inehrited      *
 *                attribute.                         *
 *  list_t ret --- pointer to list                    *
 *****************************************************/
static list_t ex_relabel_seq_left(list_t inp)
{
   elementp elds=NIL;
   elementp elsr=NIL;
   list_t inp1=NIL;
   list_t syp=NIL;

#  ifdef DEBUG_PARSE
   printf("ex_relabel_seq_left->");
#  endif
   if (lah->token_name == ID){
      elds = makelet(TOKEN,addattr(lah,ACT,LAB,NAME));
      lah = scanner(&lah);
      if ((lah->token_name == VALUE_OP)&&(lah->attr.op.type == DIV)){
         lah = scanner(&lah);
         if (lah->token_name == ID){
            elsr = makelet(TOKEN,addattr(lah,ACT,LAB,NAME));
            lah = scanner(&lah);
            inp1 = cons(*makelet(LIST,dotpair(*elds,*elsr)),inp);
            syp = relabel_seq_left(inp1);
         }
         else
            return((list_t)error(WARNING|ETK,"syntax error(relabel_seq526) %s\n", lah));
      }
      else
         return((list_t)error(WARNING|ETK,"syntax error(relabel_seq529) %s\n", lah));
   }
   else{
      if ((lah->token_name == AGENT_OP)&&(lah->attr.op.type == CO)){
         lah = scanner(&lah);
         if (lah->token_name == ID){
            elds = makelet(TOKEN,addattr(lah,ACT,LAB,CO_NAME));
            lah = scanner(&lah);
            if ((lah->token_name == VALUE_OP)&&(lah->attr.op.type == DIV)){
               lah = scanner(&lah);
               if ((lah->token_name == AGENT_OP)&&(lah->attr.op.type == CO)){
                  lah = scanner(&lah);
                  if (lah->token_name == ID){
                     elsr = makelet(TOKEN,addattr(lah,ACT,LAB,CO_NAME));
                     inp1 = cons(*makelet(LIST,dotpair(*elds,*elsr)),inp);
                     syp = relabel_seq_left(inp1);
                  }
                  else
                     error(WARNING|ETK,"syntax error(relabel_seq547) %s\n", lah);
               }
               else
                  error(WARNING|ETK,"syntax error(relabel_seq550) %s\n", lah);
            }
            else
               error(WARNING|ETK,"syntax error(relabel_seq553) %s\n", lah);
         }
         else
            error(WARNING|ETK,"syntax error(relabel_seq556) %s\n", lah);
      }
      else
         error(WARNING|ETK,"syntax error(relabel_seq559) %s\n", lah);
   }
   return(syp);
}

/*****************************************************
 *  this function has synthesized attributes and     *
 * inherited attributes.                             *
 *---------------------------------------------------*
 *  ret = relabel_seq_left(inp);                     *
 *                                                   *
 *  list_t inp --- pointer to list for inehrited      *
 *                attribute.                         *
 *  list_t ret --- pointer to list                    *
 *****************************************************/
static list_t relabel_seq_left(list_t inp)
{
   list_t syp=NIL;

#  ifdef DEBUG_PARSE
   printf("relabel_seq_left->");
#  endif
   if ((lah->token_name == VALUE_OP)&&(lah->attr.op.type == SEQ)){
      lah = scanner(&lah);
      syp = ex_relabel_seq_left(inp);
   }
   else
      syp = inp;
   return(syp);
}

/*****************************************************
 *  this function has synthesized attributes and     *
 * inherited attributes.                             *
 *---------------------------------------------------*
 *  ret = relabel_seq(void);                         *
 *                                                   *
 *  list_t ret --- pointer to list                    *
 *****************************************************/
static list_t relabel_seq(void)
{
   elementp elds=NIL;
   elementp elsr=NIL;
   list_t inp=NIL;
   list_t syp=NIL;

#  ifdef DEBUG_PARSE
   printf("relabel_seq->");
#  endif
   if (lah->token_name == ID){
      elds = makelet(TOKEN,addattr(lah,ACT,LAB,NAME));
      lah = scanner(&lah);
      if ((lah->token_name == VALUE_OP)&&(lah->attr.op.type == DIV)){
         lah = scanner(&lah);
         if (lah->token_name == ID){
            elsr = makelet(TOKEN,addattr(lah,ACT,LAB,NAME));
            lah = scanner(&lah);
            inp = cons(*makelet(LIST,dotpair(*elds,*elsr)),makenull(NIL));
            syp = relabel_seq_left(inp);
         }
         else
            return((list_t)error(WARNING|ETK,"syntax error(relabel_seq621) %s\n", lah));
      }
      else
         return((list_t)error(WARNING|ETK,"syntax error(relabel_seq624) %s\n", lah));
   }
   else{
      if ((lah->token_name == AGENT_OP)&&(lah->attr.op.type == CO)){
         lah = scanner(&lah);
         if (lah->token_name == ID){
            elds = makelet(TOKEN,addattr(lah,ACT,LAB,CO_NAME));
            lah = scanner(&lah);
            if ((lah->token_name == VALUE_OP)&&(lah->attr.op.type == DIV)){
               lah = scanner(&lah);
               if ((lah->token_name == AGENT_OP)&&(lah->attr.op.type == CO)){
                  lah = scanner(&lah);
                  if (lah->token_name == ID){
                     elsr = makelet(TOKEN,addattr(lah,ACT,LAB,CO_NAME));
                     inp = cons(*makelet(LIST,dotpair(*elds,*elsr)),
                                makenull(NIL));
                     syp = relabel_seq_left(inp);
                  }
                  else
                     return((list_t)error(WARNING|ETK,"syntax error(relabel_seq643) %s\n", lah));
               }
               else
                  return((list_t)error(WARNING|ETK,"syntax error(relabel_seq646) %s\n", lah));
            }
            else
               return((list_t)error(WARNING|ETK,"syntax error(relabel_seq649) %s\n", lah));
         }
         else
            return((list_t)error(WARNING|ETK,"syntax error(relabel_seq652) %s\n", lah));
      }
      else
         return((list_t)error(WARNING|ETK,"syntax error(relabel_seq655) %s\n", lah));
   }
   return(syp);
}

/*****************************************************
 *  this function has synthesized attributes and     *
 * inherited attributes.                             *
 *---------------------------------------------------*
 *  ret = value_seq_left(inp);                       *
 *                                                   *
 *  list_t inp --- pointer to list for inehrited      *
 *                attribute.                         *
 *  list_t ret --- pointer to list                    *
 *****************************************************/
static list_t value_seq_left(list_t inp)
{
   elementp syp=NIL;
   list_t inp1=NIL;
   list_t syp1=NIL;

#  ifdef DEBUG_PARSE
   printf("value_seq_left->");
#  endif
   if ((lah->token_name == VALUE_OP)&&(lah->attr.op.type == SEQ)){
      lah = scanner(&lah);
      syp = v_fact();
      inp1 = cons(*syp,inp);
      syp1 = value_seq_left(inp1);
   }
   else
      syp1 = inp;
   return(syp1);
}

/*****************************************************
 *  this function has synthesized attributes and     *
 * inherited attributes.                             *
 *---------------------------------------------------*
 *  ret = value_seq(void);                           *
 *                                                   *
 *  list_t ret --- pointer to list                    *
 *****************************************************/
static list_t value_seq(void)
{
   list_t inp=NIL;
   list_t syp=NIL;

#  ifdef DEBUG_PARSE
   printf("value_seq->");
#  endif
   inp = cons(*v_fact(),makenull(NIL));
   syp = value_seq_left(inp);
   return(syp);
}

/*****************************************************
 *  this function has synthesized attributes and     *
 * inherited attributes.                             *
 *---------------------------------------------------*
 *  ret = ex_id_seq_left(inp);                       *
 *                                                   *
 *  list_t ret --- pointer to list                    *
 *  list_t inp --- pointer to inherited atrributes    *
 *****************************************************/
static list_t ex_id_seq_left(list_t inp)
{
   list_t inp1=NIL;
   list_t syp=NIL;
   token id=NIL;

#  ifdef DEBUG_PARSE
   printf("ex_id_seq_left->");
#  endif
   if (lah->token_name == ID){
      id = lah;
      lah = scanner(&lah);
      inp1 = cons(*makelet(TOKEN,addattr(id,ACT,LAB,NAME)),inp);
      syp = id_seq_left(inp1);
   }
   else{
      if ((lah->token_name == AGENT_OP)&&(lah->attr.op.type == CO)){
         lah = scanner(&lah);
         if (lah->token_name == ID){
            id = lah;
            lah = scanner(&lah);
            inp1 = cons(*makelet(TOKEN,addattr(id,ACT,LAB,CO_NAME)),inp);
            syp = id_seq_left(inp1);
         }
      }
   }
   return(syp);
}

/*****************************************************
 *  this function has synthesized attributes and     *
 * inherited attributes.                             *
 *---------------------------------------------------*
 *  ret = id_seq_left(inp);                          *
 *                                                   *
 *  list_t ret --- pointer to list                    *
 *  list_t inp --- pointer to inherited atrributes    *
 *****************************************************/
static list_t id_seq_left(list_t inp)
{
   list_t syp=NIL;

#  ifdef DEBUG_PARSE
   printf("id_seq_left->");
#  endif
   if ((lah->token_name == VALUE_OP)&&(lah->attr.op.type == SEQ)){
      lah = scanner(&lah);
      syp = ex_id_seq_left(inp);
   }
   else
      syp = inp;
   return(syp);
}

/*****************************************************
 *  this function has synthesized attributes and     *
 * inherited attributes.                             *
 *---------------------------------------------------*
 *  ret = id_seq(void);                              *
 *                                                   *
 *  list_t ret --- pointer to list                    *
 *****************************************************/
static list_t id_seq(void)
{
   list_t inp=NIL;

#  ifdef DEBUG_PARSE
   printf("id_seq->");
#  endif
   if (lah->token_name == ID){
      inp = cons(*makelet(TOKEN,addattr(lah,ACT,LAB,NAME)),makenull(NIL));
      lah = scanner(&lah);
      return(id_seq_left(inp));
   }
   else{
      if ((lah->token_name == AGENT_OP)&&(lah->attr.op.type == CO)){
         lah = scanner(&lah);
         if (lah->token_name == ID){
            inp = cons(*makelet(TOKEN,addattr(lah,ACT,LAB,CO_NAME)),makenull(NIL));
            lah = scanner(&lah);
            return(id_seq_left(inp));
         }
         else
            return((list_t)error(WARNING|ETK,"syntax error(id_seq807) %s\n", lah));
      }
      else
         return((list_t)error(WARNING|ETK,"syntax error(id_seq810) %s\n", lah));
   }
}

/*****************************************************
 *  this function has synthesized attributes.        *
 *---------------------------------------------------*
 *  ret = ex_a_fact(void);                           *
 *                                                   *
 *  list_t ret --- pointer to list                    *
 *****************************************************/
static list_t ex_a_fact(void)
{
   list_t syp=NIL;

#  ifdef DEBUG_PARSE
   printf("ex_a_fact->");
#  endif
   if ((lah->token_name == PARENTHE)&&(lah->attr.par.fr_or_af == AF)){
      lah = scanner(&lah);
      return(makenull(NIL));
   }
   else{
      syp = agent_exp();
      if ((lah->token_name == PARENTHE)&&(lah->attr.par.fr_or_af == AF)){
         lah = scanner(&lah);
         return(syp);
      }
      else
         return((list_t)error(WARNING|ETK,"syntax error(ex_a_fact839) %s\n", lah));
   }
}

/*****************************************************
 *  this function has synthesized attributes.        *
 *---------------------------------------------------*
 *  ret = a_fact(void);                              *
 *                                                   *
 *  list_t ret --- pointer to list                    *
 *****************************************************/
static list_t a_fact(void)
{
#  ifdef DEBUG_PARSE
   printf("a_fact->");
#  endif
   if ((lah->token_name == PARENTHE)&&(lah->attr.par.fr_or_af == FR)){
      lah = scanner(&lah);
      return(ex_a_fact());
   }
   else
      return((list_t)error(WARNING|ETK,"syntax error %s(a_fact850)\n",lah));
}

/*****************************************************
 *  this function has synthesized attributes.        *
 *---------------------------------------------------*
 *  ret = a_fix_exp(void);                           *
 *                                                   *
 *  list_t ret --- pointer to list                    *
 *****************************************************/
static list_t a_fix_exp(void)
{
   list_t syp=NIL;
   list_t ret=NIL;
   token id =NIL;

#  ifdef DEBUG_PARSE
   printf("a_fix_exp->");
#  endif
   if ((lah->token_name == KEY_WORD)&&(strcmp(lah->attr.keywd.str,"fix") == 0)){
      lah = scanner(&lah);
      if ((lah->token_name == PARENTHE)&&(lah->attr.par.fr_or_af == FR)){
         lah = scanner(&lah);
         if (lah->token_name == ID){
            id = lah;
            lah = scanner(&lah);
            if ((lah->token_name == COMP_OP)&&(lah->attr.op.type == EQ)){
               lah = scanner(&lah);
               syp = a_fact();
               ret = (cons(*makelet(TOKEN,makesym(AGENT_OP,CLS)),
                           dotpair(*makelet(TOKEN,addattr(id,A_VAR)),
                                   *makelet(LIST,syp))));
               if (!((lah->token_name == PARENTHE)&&(lah->attr.par.fr_or_af == AF)))
                  return((list_t)error(WARNING|ETK,"syntax error(a_fix_exp896) %s\n", lah));
            }
            else
               return((list_t)error(WARNING|ETK,"syntax error(a_fix_exp899) %s\n", lah));
         }
         else
            return((list_t)error(WARNING|ETK,"syntax error(a_fix_exp902) %s\n", lah));
      }
      else
         return((list_t)error(WARNING|ETK,"syntax error(a_fix_exp905) %s\n", lah));
   }
   else
      ret = (a_fact());
   return(ret);
}

/*****************************************************
 *  this function has synthesized attributes.        *
 *---------------------------------------------------*
 *  ret = ex_a_unary_exp(void);                      *
 *                                                   *
 *  list_t ret --- pointer to list                    *
 *****************************************************/
static list_t ex_a_unary_exp(void)
{
   list_t syp=NIL;
   list_t syp1=NIL;
   token id=NIL;

#  ifdef DEBUG_PARSE
   printf("ex_a_unary_exp->");
#  endif
   if (lah->token_name == ID){
      id = lah;
      lah = scanner(&lah);
      if ((lah->token_name == PARENTHE)&&(lah->attr.par.fr_or_af == FR)){
         lah = scanner(&lah);
         syp = value_seq();
         if ((lah->token_name == PARENTHE)&&(lah->attr.par.fr_or_af == AF)){
            lah = scanner(&lah);
            if ((lah->token_name == AGENT_OP)&&(lah->attr.op.type == PRE)){
               lah = scanner(&lah);
               syp1 = a_unary_exp();
               return(cons(*makelet(TOKEN,addattr(id,ACT,LAB,CO_NAME)),
                           dotpair(*makelet(LIST,syp),
                                   *makelet(LIST,syp1))));
            }
            else
               return((list_t)error(WARNING|ETK,"syntax error(ex_a_unary_exp945) %s\n", lah));
         }
         else
            return((list_t)error(WARNING|ETK,"syntax error(ex_a_unary_exp948) %s\n", lah));
      }
      else{
         if ((lah->token_name == AGENT_OP)&&(lah->attr.op.type == PRE)){
            syp = makenull(NIL);
            lah = scanner(&lah);
            syp1 = a_unary_exp();
            return(cons(*makelet(TOKEN,addattr(id,ACT,LAB,CO_NAME)),
                        dotpair(*makelet(LIST,syp),
                                *makelet(LIST,syp1))));
         }
         else
            return((list_t)error(WARNING|ETK,"syntax error(ex_a_unary_exp951) %s\n", lah));
      }
   }
   else
      return((list_t)error(WARNING|ETK,"syntax error %s(ex_a_unary_exp954) %s\n", lah));
}

/*****************************************************
 *  this function has synthesized attributes.        *
 *---------------------------------------------------*
 *  ret = ex_a_unary_exp_id_par(void);               *
 *                                                   *
 *  list_t ret --- pointer to list                    *
 *****************************************************/
static list_t ex_a_unary_exp_id_val_seq2(token id,list_t syp)  /* B010 */
{                                                     /* B010 */
   list_t syp1=NIL;                                    /* B010 */
#  ifdef DEBUG_PARSE                                  /* B010 */
   printf("ex_a_unary_exp_id_par_val_seq2->");          /* B010 */
#  endif                                              /* B010 */
   if ((lah->token_name == AGENT_OP)&&(lah->attr.op.type == PRE)){ /* B010 */
      lah = scanner(&lah);                            /* B010 */
      syp1 = a_unary_exp();                           /* B010 */
      return(cons(*makelet(TOKEN,makesym(AGENT_OP,RECV)),   /* B010 */
                  cons(*makelet(TOKEN,addattr(id,ACT,LAB,NAME)), /* B010 */
                        dotpair(*makelet(LIST,syp), /* B010 */
                                *makelet(LIST,syp1))))); /* B010 */
   }                                                  /* B010 */
   else{                                              /* B010 */
      return(cons(*makelet(TOKEN,makesym(AGENT_OP,CON)),    /* B010 */
                  dotpair(*makelet(TOKEN,addattr(id,A_CON)), /* B010 */
                          *makelet(LIST,syp)))); /* B010 */
   }                                                  /* B010 */
}                                                     /* B010 */
static list_t ex_a_unary_exp_id_val_seq(token id)
{
   list_t syp=NIL;

#  ifdef DEBUG_PARSE
   printf("ex_a_unary_exp_id_par->");
#  endif
   syp = value_seq();
   if ((lah->token_name == PARENTHE)&&(lah->attr.par.fr_or_af == AF)){
      lah = scanner(&lah);
      return(ex_a_unary_exp_id_val_seq2(id,syp)); /* B010 */
   }
   else
      return((list_t)error(WARNING|ETK,"syntax error(ex_a_unary_exp_id_val_seq1002) %s\n", lah));
}

/*****************************************************
 *  this function has synthesized attributes.        *
 *---------------------------------------------------*
 *  ret = ex_a_unary_exp_id(void);                   *
 *                                                   *
 *  list_t ret --- pointer to list                    *
 *****************************************************/
static list_t ex_a_unary_exp_id(token id)
{
   list_t syp1=NIL;

#  ifdef DEBUG_PARSE
   printf("ex_a_unary_exp_id->");
#  endif
   if ((lah->token_name == PARENTHE)&&(lah->attr.par.fr_or_af == FR)){
      lah = scanner(&lah);
      return(ex_a_unary_exp_id_val_seq(id));   /* B010 */
   }
   else{
      if ((lah->token_name == AGENT_OP)&&(lah->attr.op.type == PRE)){
         lah = scanner(&lah);
         syp1 = a_unary_exp();
         return(cons(*makelet(TOKEN,makesym(AGENT_OP,RECV)),
                     cons(*makelet(TOKEN,addattr(id,ACT,LAB,NAME)),
                          dotpair(*makelet(LIST,makenull(NIL)),
                                  *makelet(LIST,syp1)))));
      }
      else
         return(cons(*makelet(TOKEN,makesym(AGENT_OP,CON)),
                     dotpair(*makelet(TOKEN,addattr(id,A_CON)),
                             *makelet(LIST,makenull(NIL)))));
   }
}

/*****************************************************
 *  this function has synthesized attributes.        *
 *---------------------------------------------------*
 *  ret = a_unary_exp(void);                         *
 *                                                   *
 *  list_t ret --- pointer to list                    *
 *****************************************************/
static list_t a_unary_exp(void)
{
   token id=NIL;

#  ifdef DEBUG_PARSE
   printf("a_unary_exp->");
#  endif
   switch(lah->token_name){
      case ID:
         id = lah;
         lah = scanner(&lah);
         return(ex_a_unary_exp_id(id));
         break;
      case AGENT_OP:
         if (lah->attr.op.type == CO){
            lah = scanner(&lah);
            return(cons(*makelet(TOKEN,makesym(AGENT_OP,SEND)),
                        ex_a_unary_exp()));
         }
         break;
      default:
         return(a_fix_exp());
   }
   return(a_fix_exp());
}

/*****************************************************
 *  this function has synthesized attributes and     *
 * inherited attributes.                             *
 *---------------------------------------------------*
 *  ret = a_label_exp_left(inp);                     *
 *                                                   *
 *  list_t ret --- pointer to list                    *
 *  list_t inp --- pointer to list,inherited attribute*
 *****************************************************/
static list_t a_label_exp_left(list_t inp)
{
   list_t syp=NIL;
   list_t inp1=NIL;

#  ifdef DEBUG_PARSE
   printf("a_label_exp_left->");
#  endif
   switch(lah->token_name){
      case BRACKET:
         if (lah->attr.par.fr_or_af == FR){
            lah = scanner(&lah);
            syp = id_seq();
            if ((lah->token_name == BRACKET)&&(lah->attr.par.fr_or_af == AF)){
               lah = scanner(&lah);
               inp1 = cons(*makelet(TOKEN,makesym(AGENT_OP,RES)),
                          dotpair(*makelet(LIST,inp),
                                  *makelet(LIST,syp)));
               return(a_label_exp_left(inp1));
            }
            else
               return((list_t)error(WARNING|ETK,"syntax error(a_label_exp_left1096) %s\n", lah));
         }
         else
            return((list_t)error(WARNING|ETK,"syntax error(a_label_exp_left1099) %s\n", lah));
         break;
      case BRACE:
         if (lah->attr.par.fr_or_af == FR){
            lah = scanner(&lah);
            syp = relabel_seq();
            if ((lah->token_name == BRACE)&&(lah->attr.par.fr_or_af == AF)){
               lah = scanner(&lah);
               inp1 = cons(*makelet(TOKEN,makesym(AGENT_OP,REL)),
                          dotpair(*makelet(LIST,inp),*makelet(LIST,syp)));
               return(a_label_exp_left(inp1));
            }
            else
               return((list_t)error(WARNING|ETK,"syntax error(a_label_exp_left1115) %s\n", lah));
         }
         else
            return((list_t)error(WARNING|ETK,"syntax error(a_label_exp_left1118) %s\n", lah));
         break;
      default:
         return(inp);
   }
}

/*****************************************************
 *  this function has synthesized attributes and     *
 * inherited attributes.                             *
 *---------------------------------------------------*
 *  ret = a_label_exp(void);                         *
 *                                                   *
 *  list_t ret --- pointer to list                    *
 *****************************************************/
static list_t a_label_exp(void)
{
#  ifdef DEBUG_PARSE
   printf("a_label_exp->");
#  endif
   return(a_label_exp_left(a_unary_exp()));
}

/*****************************************************
 *  this function has synthesized attributes and     *
 * inherited attributes.                             *
 *---------------------------------------------------*
 *  ret = a_binary_exp_left(inp);                    *
 *                                                   *
 *  list_t ret --- pointer to list                    *
 *  list_t inp --- pointer to list,inherited attribute*
 *****************************************************/
static list_t a_binary_exp_left(list_t inp)
{
   token id=NIL;
   list_t syp=NIL;
   list_t inp1=NIL;

#  ifdef DEBUG_PARSE
   printf("a_binary_exp_left->");
#  endif
   switch(lah->token_name){
      case AGENT_OP:
         switch(lah->attr.op.type){
            case SUM:
            case COM:
               id = lah;
               lah = scanner(&lah);
               syp = a_label_exp();
               inp1 = cons(*makelet(TOKEN,id),dotpair(*makelet(LIST,inp),
                                                      *makelet(LIST,syp)));
               return(a_binary_exp_left(inp1));
               break;
               default:
               return((list_t)error(WARNING|ETK,"syntax error(a_binary_exp_left1174) %s\n", lah));
         }
         break;
      default:
         return(inp);
   }
}

/*****************************************************
 *  this function has synthesized attributes and     *
 * inherited attributes.                             *
 *---------------------------------------------------*
 *  ret = a_binary_exp(void);                        *
 *                                                   *
 *  list_t ret --- pointer to list                    *
 *****************************************************/
static list_t a_binary_exp(void)
{
   list_t inp=NIL;

#  ifdef DEBUG_PARSE
   printf("a_binary_exp->");
#  endif
   inp = a_label_exp();
   return(a_binary_exp_left(inp));
}

/*****************************************************
 *  this function has synthesized attributes.        *
 *---------------------------------------------------*
 *  ret = agent_exp(void);                           *
 *                                                   *
 *  list_t ret --- pointer to list                    *
 *****************************************************/
static list_t agent_exp(void)
{
   token id=(token)NIL;
   token id1=(token)NIL;
   list_t cond=(list_t)NIL;
   list_t syp=(list_t)NIL;
   list_t syp1=(list_t)NIL;

#  ifdef DEBUG_PARSE
   printf("agent_exp->");
#  endif
   if ((lah->token_name == KEY_WORD)&&(strcmp(lah->attr.keywd.str,"define")==0)){
      lah = scanner(&lah);
      if (lah->token_name == ID){
         id = lah;
         lah = scanner(&lah);
         if ((lah->token_name == PARENTHE)&&(lah->attr.par.fr_or_af == FR)){
            lah = scanner(&lah);
            if ((lah->token_name == PARENTHE)&&(lah->attr.par.fr_or_af == AF)){
                  lah = scanner(&lah);
                  syp1 = agent_exp();
                  return(cons(*makelet(TOKEN,makesym(AGENT_OP,DEF)),
                              cons(*makelet(TOKEN,addattr(id,A_CON)),
                                   dotpair(*makelet(LIST,makenull(NIL)),
                                           *makelet(LIST,syp1)))));
            }
            else{
               syp = id_seq();
               if ((lah->token_name == PARENTHE)&&(lah->attr.par.fr_or_af == AF)){
                  lah = scanner(&lah);
                  syp1 = agent_exp();
                  return(cons(*makelet(TOKEN,makesym(AGENT_OP,DEF)),
                              cons(*makelet(TOKEN,addattr(id,A_CON)),
                                   dotpair(*makelet(LIST,syp),
                                           *makelet(LIST,syp1)))));
               }
               else
                  return((list_t)error(WARNING|ETK,"syntax error(agent_exp1253) %s\n", lah));
            }
         }
         else
            return((list_t)error(WARNING|ETK,"syntax error(agent_exp1257) %s\n", lah));
      }
      else
         return((list_t)error(WARNING|ETK,"syntax error(agent_exp1260) %s\n", lah));
   }
   else{
      if ((lah->token_name == KEY_WORD)&&
          (strcmp(lah->attr.keywd.str,"bind")==0)){
         lah = scanner(&lah);
         if (lah->token_name == ID){
            id = lah;
            lah = scanner(&lah);
            if ((lah->token_name == VALUE)&&
                (lah->attr.value.type == STR)){
               id1 = lah;
               lah = scanner(&lah);
               if ((lah->token_name == PARENTHE)&&
                   (lah->attr.par.fr_or_af == AF))
                  return(cons(*makelet(TOKEN,makesym(AGENT_OP,BIND)),
                              dotpair(*makelet(TOKEN,addattr(id,ACT,LAB,NAME)),
                                      *makelet(TOKEN,id1))));
               else
                  return((list_t)error(WARNING|ETK,"syntax error(agent_exp1286) %s\n", lah));
            }
            else
               return((list_t)error(WARNING|ETK,"syntax error(agent_exp1289) %s\n", lah));
         }
         else
            return((list_t)error(WARNING|ETK,"syntax error(agent_exp1292) %s\n", lah));
      }
      else{
         if ((lah->token_name==KEY_WORD)&&(strcmp(lah->attr.keywd.str,"if")==0)){
            lah = scanner(&lah);
            if ((lah->token_name == PARENTHE)&&
                (lah->attr.par.fr_or_af == FR)){
               lah = scanner(&lah);
               cond = b_exp();                             /*B028*/
               if ((lah->token_name == PARENTHE)&&
                   (lah->attr.par.fr_or_af == AF)){
                  lah = scanner(&lah);
                  syp = agent_exp();
                  syp1 = agent_exp();
                  return(cons(*makelet(TOKEN,makesym(AGENT_OP,IF)),
                               cons(*makelet(LIST,cond),/*B028*/
                                    dotpair(*makelet(LIST,syp),
                                            *makelet(LIST,syp1)))));
               } /*** end of ) ***/
               else
                  return((list_t)error(WARNING|ETK,"syntax error(agent_exp1315) %s\n", lah));
            } /*** end of ( ***/
            else
               return((list_t)error(WARNING|ETK,"syntax error(agent_exp1318) %s\n", lah));
         } /*** end of if ***/
         else{
            if ((lah->token_name==KEY_WORD)&&(strcmp(lah->attr.keywd.str,"definit")==0)){
               lah = scanner(&lah);
               if ((lah->token_name == PARENTHE)&&
                   (lah->attr.par.fr_or_af == FR)){
                  lah = scanner(&lah);
                  syp = id_seq();
                  if ((lah->token_name == PARENTHE)&&
                      (lah->attr.par.fr_or_af == AF)){
                     lah = scanner(&lah);
                     if ((lah->token_name == PARENTHE)&&
                         (lah->attr.par.fr_or_af == FR)){
                        lah = scanner(&lah);
                        syp1 = value_seq();
                        if ((lah->token_name == PARENTHE)&&
                            (lah->attr.par.fr_or_af == AF)){
                           lah = scanner(&lah);
                           if ((lah->token_name == PARENTHE)&&
                               (lah->attr.par.fr_or_af == AF)){
                                return(cons(*makelet(TOKEN,makesym(AGENT_OP,DEFINIT)),
                                             dotpair(*makelet(LIST,syp),
                                                     *makelet(LIST,syp1))));
                           }
                           else
                              return((list_t)error(WARNING|ETK,"syntax error(agent_exp1307) %s\n", lah));
                        }
                        else
                           return((list_t)error(WARNING|ETK,"syntax error(agent_exp1310) %s\n", lah));
                     }
                     else
                        return((list_t)error(WARNING|ETK,"syntax error(agent_exp1313) %s\n", lah));
                  }
                  else
                     return((list_t)error(WARNING|ETK,"syntax error(agent_exp1316) %s\n", lah));
               }
               else
                  return((list_t)error(WARNING|ETK,"syntax error(agent_exp1319) %s\n", lah));
            }
         } /*** end of definit ***/
      } /*** end of bind ***/
   } /*** end of define ***/
   return(a_binary_exp());
}

/*****************************************************
 *  this function has synthesized attributes.        *
 *---------------------------------------------------*
 *  ret = start(void);                               *
 *                                                   *
 *  list_t ret --- pointer to list                    *
 *****************************************************/
static list_t start(void)
{
   list_t syp=NIL;

#  ifdef DEBUG_PARSE
   printf("start->");
#  endif
   if ((lah->token_name == PARENTHE)&&(lah->attr.par.fr_or_af == AF)){
      return(makenull(NIL));
   }
   else{
      syp = agent_exp();
      if ((lah->token_name == PARENTHE)&&(lah->attr.par.fr_or_af == AF)){
         return(syp);
      }
      else
         return((list_t)error(WARNING|ETK,"syntax error %s(start1348)\n",lah));
   }
}

/*****************************************************
 *  this function has synthesized attributes.        *
 *---------------------------------------------------*
 *  ret = ex_start(void);                            *
 *                                                   *
 *  list_t ret --- pointer to list                    *
 *****************************************************/
static list_t ex_start(void)
{
#  ifdef DEBUG_PARSE
   printf("ex_start->");
#  endif
   if ((lah->token_name == PARENTHE)&&(lah->attr.par.fr_or_af == FR)){
      lah = scanner(&lah);
      return(start());
   }
   else
      return((list_t)error(WARNING|ETK,"syntax error(ex_start1369) %s\n", lah));
}

/*****************************************************
 * return to abstract syntax tree represented by     *
 * list.                                     *
 *---------------------------------------------------*
 *  ret = parse(lspp);                               *
 *                                                   *
 *  list_t ret --- pointer to S-expression            *
 *****************************************************/
list_t parser(list_t *lspp,char *text)
{
#  ifdef DEBUG_PARSE
   printf("parser->");
#  endif
   if (text != (char *)NIL)
      set_in_buf(&(seed.buf),text);
   lah = scanner(&lah);
   if (lah == (token)NIL)
      return((list_t)NIL);
   else{
      *lspp = ex_start();
      return(*lspp);
   }
}
