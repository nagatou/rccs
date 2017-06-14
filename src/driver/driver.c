/*
 * $Id: driver.c,v 1.34 2015/02/16 07:08:13 nagatou Exp $
 */
//static char rcsid[]="$Id: driver.c,v 1.34 2015/02/16 07:08:13 nagatou Exp $";
//static char copyright[]=
//   "Copyright (C) 1998 Naoyuki Nagatou";

/************************************************************************
   this file is a set of functions for read-eval-print loop.
                 Copyright (C) 1998 Naoyuki Nagatou
 ************************************************************************/
#define EXTERN
#define EVAL
#include "comm.h"
#include "driver.h"
#include "primitive.h"
#undef EXTERN
#define EXTERN extern
#include "setupsignalstack.h"

static jmp_buf when_length_of_path_is_zero;

#ifdef DEBUG_MC
static char *ltl2ccs(char *ba,buffer *prop)
{
   ins_buf(prop,ba);
   return(prop->buf);
}
#endif
/*****************************************************
 *---------------------------------------------------*
 *  ret = read();                                    *
 *                                                   *
 *****************************************************/
static list_t n_read(void)
{
   list_t agent_exp=NIL;

#  ifdef DEBUG_EVAL
   printf("n_read->");
#  endif
   agent_exp = parser(&agent_exp,(char *)NIL);
   if (agent_exp == (list_t)NIL){
      printf("\nEOF\n");
      clear_in_buf(&seed);
      return(agent_exp);
   }
   else{
      gc(&memory_control_table);
      return(agent_exp);
   }
}

/*****************************************************
 *  utility function for nCCS                        *
 *---------------------------------------------------*
 *  ret = getop(tk);                                 *
 *  ret = isdef(tk);                                 *
 *  ret = isbind(tk);                                *
 *  ret = lookup(name,env,ch);                          *
 *  ret = initenv(tbl);                              *
 *  ret = bindenv(env);                              *
 *  ret = bindls(rls,lls,env);                       *
 *                                                   *
 *****************************************************/
static bool isif(element_t el)
{
#  ifdef DEBUG_EVAL
   printf("getop->");
#  endif
   if (el.type==TOKEN){
      if ((el.entry.tk->token_name==AGENT_OP)&&(el.entry.tk->attr.op.type==IF))
         return((bool)TRUE);
      else
         return((bool)FALSE);
   }
   else
      return((bool)FALSE);
}
static field_t getop(element_t el)
{
#  ifdef DEBUG_EVAL
   printf("getop->");
#  endif
   if (el.type==TOKEN){
      if ((el.entry.tk->token_name==AGENT_OP)||
          (el.entry.tk->token_name==KEY_WORD))
         return(el.entry.tk->attr.op.type);
      else
         return((int)error(FATAL|EEL,"invalid type(getop89) element=%s.\n", el));
   }
   else
      return((int)error(FATAL|EEL,"invalid type(getop92) element=%s, type=%d\n", el, el.type));
}
static bool isdef(element_t el)
{
#  ifdef DEBUG_EVAL
   printf("isdef->");
#  endif
   if (getop(el)==DEF)
      return((bool)TRUE);
   else
      return((bool)FALSE);
}
static bool isdefinit(element_t el)
{
#  ifdef DEBUG_EVAL
   printf("isdefinit->");
#  endif
   if (getop(el)==DEFINIT)
      return((bool)TRUE);
   else
      return((bool)FALSE);
}
static bool isbind(element_t el)
{
#  ifdef DEBUG_EVAL
   printf("isbind->");
#  endif
   if (getop(el)==BIND)
      return((bool)TRUE);
   else
      return((bool)FALSE);
}
static list_t n_bound(element_t var,element_t val,list_t env)
{
   list_t pair = dotpair(var,val);
#  ifdef DEBUG_REG_Q
   printf("n_bound->");
#  endif
   if (pair==(list_t)NIL)
      return((list_t)error(FATAL,"Segmentation falt(n_bound115)\n"));
   return(cons(*makelet(LIST,pair),env));
}
static list_t n_bound_ch(element_t var,element_t val,queue_t ch)
{
#  ifdef DEBUG_REG_Q
   printf("n_bound_ch->");
#  endif
   if (ch==(queue_t)NIL)
      return((list_t)error(FATAL,"Segmentation error(n_bound_ch106)\n"));
   switch(channel_order){
      case C_QUEUE:
         return(q_append(ch,
                         *makelet(LIST,
                                  cons(var,
                                       cons(val,makenull(NIL))))));
         break;
      case C_STACK:
         return(n_bound(var,val,ch));
         break;
      default:
         return((list_t)error(FATAL,"Invalid channel type(n_bound_ch118)\n"));
   }
}
static list_t n_boundls_ch(list_t rls,list_t lls,queue_t que)
{
#  ifdef DEBUG_BIND
   printf("n_boundls_ch->");
#  endif
   if (isempty(rls)){
      if (isempty(lls))
         return(que);
      else
         return((list_t)error(FATAL|ELS,"unbound(n_boundls_ch131) %s\n", lls));
   }
   else{
      if (isempty(lls))
         return((list_t)error(FATAL|ELS,"unbound(n_boundls_ch135) %s\n", rls));
      else{
         list_t tmp2=n_bound_ch(car(rls),car(lls),que);
         return(n_boundls_ch(cdr(rls),cdr(lls),tmp2));
      }
   }
}
static list_t n_boundls(list_t rls,list_t lls,list_t env)
{
#  ifdef DEBUG_BIND
   printf("n_boundls->");
#  endif
   if (isempty(rls)){
      if (isempty(lls))
         return(env);
      else
         return((list_t)error(FATAL|ELS,"unbound(n_boundls1110) %s\n", lls));
   }
   else{
      if (isempty(lls))
         return((list_t)error(FATAL|ELS,"unbound(n_boundls1114) %s\n", rls));
      else{
         list_t tmp2=n_bound(car(rls),car(lls),env);
         return(n_boundls(cdr(rls),cdr(lls),tmp2));
      }
   }
}
list_t getls(element_t el)
{
#  ifdef DEBUG_
   printf("getls->");
#  endif
   if (el.type == LIST)
      return(el.entry.list);
   else
      return(error(FATAL,"invalid element_t(getls193) %d.\n",el.type));
}
token gettk(element_t el)
{
#  ifdef DEBUG_EVAL
   printf("gettk->");
#  endif
   if (el.type == TOKEN)
      return(el.entry.tk);
   else
      return((token)(error(FATAL|EEL,"invalid element_t(gettk195) element=%s, type=%d\n", el, el.type)));
}
int getval(element_t el)
{
   token tk=NIL;
   int ret=ERROR;

#  ifdef DEBUG_EVAL
   printf("getval->");
#  endif
   tk = gettk(el);
   if (tk->token_name==VALUE){
      switch(tk->attr.value.type){
         case ICONST:
            ret = tk->attr.value.fld.iconst.int_v;
            break;
         case STR:
            ret = (int)(tk->attr.value.fld.strings.str);
            break;
         default:
            return((int)error(FATAL|ETK,"invalid element(getval209) token=%s, token_name=%d\n", tk, tk->token_name));
      }
      return(ret);
   }
   else
      return((int)error(FATAL|ETK,"invalid element(getval158) token=%s, token_name=%d\n", tk, tk->token_name));
}
static list_t lookup21(const element_t name,list_t lvals,list_t rvals)
{
#  ifdef DEBUG_ENV
   printf("lookup21->");
#  endif
   if (isempty(lvals))
      return((list_t)NIL);
   else{
      element_t lval = car(lvals);
      switch(lval.type){
         case GC:
            return((list_t)error(FATAL|EEL,"%s already released by GC(lookup21187).\n", lval));
            break;
         case TOKEN:
            if ((name.entry.tk->token_name==ID)&&(lval.entry.tk->token_name==ID)){
               if (strcmp(name.entry.tk->attr.id.spl_ptr,lval.entry.tk->attr.id.spl_ptr)==0)
                  return(cons(car(rvals),makenull(NIL)));
               else
                  return(lookup21(name,cdr(lvals),cdr(rvals)));
            }
            else
               return(lookup21(name,cdr(lvals),cdr(rvals)));
            break;
         case LIST:
            return(lookup21(name,cdr(lvals),cdr(rvals)));
            break;
         default:
            return((list_t)error(FATAL|EEL,"Invalid token types(lookup21:246): %s\n", lval));
      
      }
   }
}
static list_t lookup2(const element_t name,list_t vals)
{
#  ifdef DEBUG_ENV
   printf("lookup2->");
#  endif
   if (vals==(list_t)NIL)
      return((list_t)error(FATAL,"Segmentation fault(lookup2201).\n"));
   if (isempty(vals))
      return((list_t)NIL);
   else
      return(lookup21(name,getls(car(vals)),getls(car(cdr(vals)))));
}
static list_t lookup1(const element_t name,list_t env)
{
   list_t value=NIL;
   element_t lval;

#  ifdef DEBUG_ENV
   printf("lookup1->");
#  endif
   if (env==(list_t)NIL)
      return((list_t)error(FATAL,"Segmentation fault(lookup1238).\n"));
   if (isempty(env))
      return((list_t)NIL);
   else{
      value = getls(car(env));
      lval = car(value);
//      if (eqel(name,lval))
//         return(getls(car(cdr(value))));
//      else
//         return(lookup1(name,cdr(env)));
      switch(name.type){
         case TOKEN:
            switch(lval.type){
               case GC:
                  return((list_t)error(FATAL|EEL,"%s already released by GC(lookup1234).\n", lval));
                  break;
               case TOKEN:
//                  if ((name.entry.tk->token_name==ID)&&(lval.entry.tk->token_name==ID)){
//                     if (strcmp(name.entry.tk->attr.id.spl_ptr,lval.entry.tk->attr.id.spl_ptr)==0)
                  if (eqel(name,lval)){
                     return(getls(car(cdr(value))));
//                     else
//                        return(lookup1(name,cdr(env)));
                  }
                  else
                     return(lookup1(name,cdr(env)));
                  break;
               case LIST:{
                  list_t ret = lookup2(name,value); //                  list_t ret = lookup2(name,getls(car(env)));
                  if (ret==(list_t)NIL)
                     return(lookup1(name,cdr(env)));
                  else
                     return(ret);
                  break;
               }
               default:
                  return((list_t)error(FATAL|EEL,"Invalid token types(lookup1206): %s\n", lval));
            }
            break;
         case LIST:
            switch(lval.type){
               case GC:
                  return((list_t)error(FATAL|EEL,"%s already released by GC(lookup1212).\n", lval));
                  break;
               case TOKEN:
                  return(lookup1(name,cdr(env)));
                  break;
               case LIST:
                  if (eqls(getls(name),getls(lval)))
                     return(getls(car(cdr(value))));
                  else
                     return(lookup1(name,cdr(env)));
                  break;
               default:
                  return((list_t)error(FATAL|EEL,"Invalid token types(lookup1224): %s\n", lval));
            }
            break;
         case GC:
            return((list_t)error(FATAL|EEL,"%s already released by GC(lookup1228).\n", name));
            break;
         default:
            return((list_t)error(FATAL|EEL,"Invalid token types(lookup1231): %s\n", name));
      }
   }
}
static list_t lookup_ch1(queue_t ch, element_t name)
{
   buffer tmp;
#  ifdef DEBUG_CHANNEL
   printf("lookup_ch1->");
#  endif
   if (ch==(queue_t)NIL)
      return((list_t)error(FATAL,"Segmentation falt(lookup_ch1304)\n"));
   switch(channel_order){
      case C_QUEUE:
         if (isempty_que(ch))
            return((list_t)NIL);
         break;
      case C_STACK:
         if (isempty(ch))
            return((list_t)NIL);
         break;
      default:
         return((list_t)error(FATAL,"Invalid channel type(lookup_ch1359)\n"));
   }
   if ((name.type==TOKEN)&&(name.entry.tk->token_name==ID)){
      if (!isempty_buf(&target)){
         if (mc_get_value(name.entry.tk->attr.id.spl_ptr,&tmp)==(char *)NIL)
            return((list_t)error(WARNING|EEL,"unbound(driver.c:lookup_ch1318): %s\n",name));
         else
            return(cons(*makelet(TOKEN,regsym(tmp.buf,VALUE,ICONST)),makenull(NIL)));
      }
      else{
         switch(channel_order){
            case(C_QUEUE):{
               list_t tmp=q_first(ch,*makelet(LIST,cons(name,makenull(NIL))));
               if (isempty(tmp))
                  return((list_t)NIL);
               else
                  return(getls(car(cdr(tmp))));
            }
            case(C_STACK):{
               list_t ret=lookup1(*makelet(LIST,cons(name,makenull(NIL))),ch);
               if (ret==(list_t)NIL)
                  return((list_t)NIL);
               else
                  return(ret);
            }
            default:
               return((list_t)error(FATAL,"Invalid channel type(lookup_ch1359)\n"));
         }
      }
   }
   else
      return((list_t)error(FATAL|EEL,"Invalid token(lookup_ch1390): %s\n",name));
}
static list_t lookup_ch(queue_t ch, element_t name)
{
   list_t ret=lookup_ch1(ch,name);
#  ifdef DEBUG_CHANNEL
   printf("lookup_ch->");
#  endif
   if (ret==(list_t)NIL)
      return((list_t)error(WARNING|EEL,"unbound(driver.c:lookup_ch395): %s\n",name));
   else
      return(ret);
}
#ifdef DEBUG_MC
static char *mc_get_value(char *tk,buffer *dmy)
{
   return((char *)NIL);
}
#endif
static list_t lookup(element_t name,list_t env,queue_t ch)
{
   list_t ret=lookup1(name,env);
#  ifdef DEBUG_ENV
   printf("lookup->");
#  endif
   if (ret == (list_t)NIL){
      if ((ret=lookup_ch1(ch,name))==(list_t)NIL){
         if (!(g_step_exec==FALSE)&&(!isempty_buf(&formula)))
            return(ret);
         else
            return((list_t)error(WARNING|EEL,"unbound(driver.c:lookup406): %s\n",name));
      }
      else
         return(ret);
   }
   else
      return(ret);
}
static list_t lookup_env(element_t name,list_t env)
{
   list_t ret=lookup1(name,env);
#  ifdef DEBUG_ENV
   printf("lookup[");
   prtlst(ret);
   printf("]->");
#  endif
   if (ret == (list_t)NIL)
      return((list_t)error(WARNING|EEL,"unbound(driver.c:lookup_env430): %s\n",name));
   else
      return(ret);
}
static bool istrans_in_act(element_t name,list_t env,queue_t ch)
{
   list_t ret=lookup_ch1(ch,name);

#  ifdef DEBUG_ENV
   printf("istrans_in_act->");
#  endif
   if (ret == (list_t)NIL)
      return((bool)FALSE);
   else
      return((bool)TRUE);
}
static bool isabort1(element_t label)
{
   buffer tmp;
#  ifdef DEBUG_EVAL
   printf("isabort1->");
#  endif
   if (label.type==TOKEN){
      initbuf(&tmp);
      if (strncmp(n_tolower(label.entry.tk->attr.id.spl_ptr,&tmp),"abort",strlen("abort"))==0)
         return(TRUE);
      else
         return(FALSE);
   }
   else
      return(FALSE);
}
static bool isabort(list_t exp)
{
#  ifdef DEBUG_EVAL
   printf("isabort->");
#  endif
   return(isabort1(car(cdr(exp))));
}
static bool isothers(element_t label)
{
   buffer tmp;
#  ifdef DEBUG_EVAL
   printf("isothers->");
   fflush(stdout);
#  endif
   initbuf(&tmp);
   if (strncmp(n_tolower(label.entry.tk->attr.id.spl_ptr,&tmp),
       primnametbl[OTHERS],strlen(primnametbl[OTHERS]))==0)
      return(TRUE);
   else
      return(FALSE);
}
static bool isstop(element_t el)                  /* B022 */
{                                               /* B022 */
   buffer low;

#  ifdef DEBUG_EVAL                             /* B022 */
   printf("isstop->");                          /* B022 */
#  endif                                        /* B022 */
   initbuf(&low);
   return(isin(stopacttbl,                      /* B022 */
               n_tolower(el.entry.tk->attr.id.spl_ptr,&low), /* B022 */
               STOP_ACT_SIZE));                 /* B022 */
}                                               /* B022 */
static bool eqstop1(list_t exp) /* B035 */
{
#  ifdef DEBUG_EVAL
   printf("eqstop1->");
#  endif
   if (isstop(car(cdr(exp))))
      return((bool)TRUE);
   else
      return((bool)FALSE);
}
static bool eqstop(list_t exp) /* B035 */
{
#  ifdef DEBUG_EVAL
   printf("eqstop->");
#  endif
   switch(getop(car(exp))){
      case CON:
         if (eqstop1(exp))
            return((bool)TRUE);
         break;
      case SUM:
      case COM:
         if (eqstop(getls(car(cdr(exp))))){
            if (eqstop(getls(car(cdr(cdr(exp))))))
               return((bool)TRUE);
            else
               return((bool)FALSE);
         }
         else
            return((bool)FALSE);
         break;
      case RES:
         return((bool)error(FATAL,"sorry. operater RES has not been implemented yet.\n"));
         break;
      default:
         return((bool)FALSE);
   }
   return((bool)FALSE);
}
static bool isprimagnt1(element_t el)               /* B022 */
{
#  ifdef DEBUG_EVAL
   printf("isprimagnt1->");
#  endif
   return(isin(primoptbl,
               el.entry.tk->attr.id.spl_ptr,
               PRIMOP_SIZE));
}
static bool isprimagnt(element_t el) /* B022 */
{                                /* B022 */
#  ifdef DEBUG_EVAL              /* B022 */
   printf("isprimagnt->");         /* B022 */
#  endif                         /* B022 */
   if (isstop(el))               /* B022 */
      return((bool)TRUE);        /* B022 */
   else                          /* B022 */
      return(isprimagnt1(el));     /* B022 */
}

/*****************************************************
 *  evaluate primitive agent expressions.            *
 *  its the first argment is agent expression,       *
 * the second argments is enviroment, the 3rd argment*
 * is sort that is a set of action,the 4th argment   *
 * is side-effect for print function.                *
 *---------------------------------------------------*
 *  ret = primagnteval(exp,env,ch);                    *
 *                                                   *
 *  token ret --- pointer to list                    *
 *****************************************************/
static list_t n_load(list_t args,list_t env,queue_t ch)
{
#  ifdef DEBUG_EVAL
   printf("load->");
#  endif
   bkup = source_file;
   source_file = fopen(gettk(car(args))->attr.value.fld.strings.str,"r");
   if (source_file == NIL){
      printf("cann't open (%s)(n_load231)\n",
             gettk(car(args))->attr.value.fld.strings.str);
      source_file = bkup;
   }
   return(driver_loop(env));
}
static list_t primagnteval(element_t agent,list_t args,list_t env,queue_t ch)
{
   token lval=NIL;

#  ifdef DEBUG_EVAL
   printf("primagnteval->");
#  endif
   lval = gettk(agent);
   if (isstop(agent))       /* B022 */
      return(cons(agent,makenull(NIL)));
   if ((lval->token_name==ID)&&
       (lval->attr.id.type==A_CON)&&
       ((strcmp(lval->attr.id.spl_ptr,"load")==0)||
        (strcmp(lval->attr.id.spl_ptr,"LOAD")==0))){
      return(n_load(args,env,ch));
   }
   if ((lval->token_name==ID)&&
       (lval->attr.id.type==A_CON)&&
       ((strcmp(lval->attr.id.spl_ptr,"quit")==0)||
        (strcmp(lval->attr.id.spl_ptr,"QUIT")==0))){
      printf("\nBye bye!!\n"), /* B007 */
      longjmp(terminate_driver_env,1);
//      epilogue(); /* B021 */
//      exit(0);                 /* B007 */
   }
   return((list_t)error(FATAL|EEL,"invalid primitive agent expression(primagnteval349) (%s)\n", lval));
}

/*****************************************************
 *  evaluate agent expressions.                      *
 *  its the first argment is agent expression,       *
 * the second argments is enviroment, the 3rd argment*
 * is sort that is a set of action,the 4th argment   *
 * is side-effect for print function.                *
 * the internal representation(abstract syntax) is   *
 * as follows.                                       *
 *  rand := (RECV label    val-var-ls rand)          *
 *        | (SEND label    val-exp-ls rand)          *
 *        | (SUM  rand     rand           )          *
 *        | (COM  rand     rand           )          *
 *        | (DEF  a-cons   val-var-ls rand)          *
 *        | (IF   bool-exp rand       rand)          *
 *        | (CON  a-cons   val-exp-ls     )          *
 *        | (REC  a-var    rand           )          *
 *        | (BIND label    port           )          *
 *        | (REL  rand    (rel-ls)        )          *
 *        | (RES  rand     label-ls       )          *
 *---------------------------------------------------*
 *  ret = eval(exp,env,hunoz);                       *
 *                                                   *
 *  token ret --- pointer to list                    *
 *****************************************************/
static bool isaccept_for_eval(element_t label)
{
   buffer tmp;
#  ifdef DEBUG_EVAL
   printf("isaccept_for_eval->");
   fflush(stdout);
#  endif
   initbuf(&tmp);
   if (strncmp(n_tolower(label.entry.tk->attr.id.spl_ptr,&tmp),
               primnametbl[ACCEPT],
               strlen(primnametbl[ACCEPT]))==0)
      return(TRUE);
   else
      return(FALSE);
}
static bool isaccept(list_t p)
{
#  ifdef DEBUG_VERIFIER1
   printf("isaccept->");
   fflush(stdout);
#  endif
   if (getop(car(p))==RECV)
      return(isaccept_for_eval(car(cdr(p))));
   else
      return(FALSE);
}
static elementp derivatives1(element_t rate,
                             element_t label,
                             list_t env,
                             queue_t ch)
{
#  ifdef DEBUG_ENV
   printf("derivatives1->");
   fflush(stdout);
#  endif
   switch(getop(rate)){
      case RECV:
         if (isouter_action(label)){
            if (isaccept_for_eval(label))
               return(&label);
            if (isothers(label))
               return(&label);
            if (istrans_out_act(label))
               return(&label);
            else
               return((elementp)NIL);
         }
         else{
            if (istrans_in_act(label,env,ch))
               return(&label);
            else
               return((elementp)NIL);
         }
         break;
      case SEND:
         if (isouter_action(label))
            return(&label);
         else{
            if (istrans_in_act(label,env,ch))
               return((elementp)NIL);
            else
               return(&label);
         }
         break;
      default:
         return((elementp)error(FATAL|EEL,"invalid primitive agent expression(derivatives1402) (%s)\n", rate));
   }
}
static elementp derivatives(list_t exp,list_t env,queue_t ch)
{
#  ifdef DEBUG_EXP2
   printf("derivatives->");
#  endif
   switch(getop(car(exp))){
      case RECV:
      case SEND:
         return(derivatives1(car(exp),
                             car(cdr(exp)),
                             env,
                             ch));
         break;
      case SUM:{
         elementp ret=(elementp)NIL;
         if ((ret=derivatives(getls(car(cdr(exp))),env,ch))!=(elementp)NIL)
            return(ret);
         else{
            if ((elementp)NIL!=(ret=derivatives(getls(car(cdr(cdr(exp)))),env,ch)))
               return(ret);
            else
               return((elementp)NIL);
         }
         break;
      }
      case COM:{
         elementp ret=(elementp)NIL;
         if ((ret=derivatives(getls(car(cdr(exp))),env,ch))!=(elementp)NIL)
            return(ret);
         else{
            if ((elementp)NIL!=(ret=derivatives(getls(car(cdr(cdr(exp)))),env,ch)))
               return(ret);
            else
               return((elementp)NIL);
         }
         break;
      }
      case IF:
         return((elementp)!NIL);
         break;
      case CON:{
         if (isabort(exp))
            return((elementp)!NIL);
         if (eqstop(exp))         /* B035 */
            return((elementp)NIL);/* B035 */
         if (isprimagnt(car(cdr(exp))))
            return((elementp)!NIL);
         else{
            if (isempty(getls(car(cdr(cdr(exp))))))     /* B027 */
               return(derivatives(getls(car(cdr(lookup_env(car(cdr(exp)),env)))),/* B043*/
                                  env,                /* B027 */
                                  ch)); /* B043 */
            else                                        /* B027 */
               return(derivatives(getls(car(cdr(lookup_env(car(cdr(exp)),env)))), /* B043 */
                                  n_boundls(getls(car(lookup_env(car(cdr(exp)),env))), /* B043 */
                                            getls(car(cdr(cdr(exp)))),
                                            env),
                                   ch)); /* B043 */
         }
         break;
      }
      case REC:
         return(derivatives(getls(car(cdr(exp))),
                            cdr(cdr(exp)),
                            ch));
         break;
      case REL:
      case RES:
         return((elementp)NIL);
         break;
      default:
         return((elementp)error(FATAL|ELS,"invalid agent expression(derivatives483) (%s)\n", exp));
   }
}
static bool istrans(list_t exp,list_t env,queue_t ch)
{
#  ifdef DEBUG_EVAL_A
   printf("istrans->");
#  endif
   if (derivatives(exp,env,ch)==(elementp)NIL)
      return((bool)FALSE);
   else
      return((bool)TRUE);
}
static list_t resume2(list_t continuation)
{
#  ifdef DEBUG_CONT
   printf("resume2->");
#  endif
   switch(getop(car(continuation))){
      case SUM:
      case COM:
         if (isempty(getls(car(cdr(continuation)))))
            return(getls(car(cdr(cdr(continuation)))));
         else{
            if (isempty(getls(car(cdr(cdr(continuation))))))
               return(getls(car(cdr(continuation))));
            else
               return(resume2(getls(car(cdr(continuation)))));
         }
         break;
      default:
         return(continuation);
   }
}
static list_t resume1(list_t continuation)
{
   list_t ret;
#  ifdef DEBUG_CONT
   printf("resume1->");
#  endif
   switch(getop(car(continuation))){
      case SUM:
      case COM:
         if (isempty(getls(car(cdr(continuation)))))
            return(getls(car(cdr(cdr(continuation)))));
         else{
            if (isempty(getls(car(cdr(cdr(continuation))))))
               return(getls(car(cdr(continuation))));
            else{
               if (isempty(ret=resume1(getls(car(cdr(continuation))))))
                  return(resume2(getls(car(cdr(cdr(continuation))))));
               else
                  return(ret);
            }
         }
         break;
      default:
         return(continuation);
   }
}
static list_t resume_out_most(list_t continuation)
{
#  ifdef DEBUG_CONT
   printf("resume_out_most->");
#  endif
   if (isempty(continuation))
      return(makenull(NIL));
   switch(getop(car(continuation))){
      case SUM:
      case COM:
         if (isempty(getls(car(cdr(continuation)))))
            return(getls(car(cdr(continuation))));
         else{
            if (isempty(getls(car(cdr(cdr(continuation))))))
               return(getls(car(cdr(cdr(continuation)))));
            else{
               if (isempty(resume_out_most(getls(car(cdr(continuation))))))
                  return(makenull(NIL));
               else{
                  if (isempty(resume_out_most(getls(car(cdr(cdr(continuation)))))))
                     return(makenull(NIL));
                  else
                     return(continuation);
               }
            }
         }
         break;
      default:
         return(continuation);
   }
}
static bool ishole(list_t continuation)
{
#  ifdef DEBUG_CONT
   printf("ishole->");
#  endif
   if (isempty(resume_out_most(continuation)))
      return(TRUE);
   else
      return(FALSE);
}
#define OUT_MOST
static list_t resume(const list_t exp,list_t continuation)
{
#  ifdef DEBUG_CONT
   printf("resume->");
#  endif
   if (isempty(exp)){
#     ifdef OUT_MOST
      if (isempty(continuation))
         return(exp);
      else{
          switch(getop(car(continuation))){
             case SUM:
             case COM:
                if (ishole(getls(car(cdr(continuation)))))
                   return(getls(car(cdr(cdr(continuation)))));
                else{
                   if (ishole(getls(car(cdr(cdr(continuation))))))
                      return(getls(car(cdr(continuation))));
                   else
                      return(continuation);
                }
                break;
         default:
            return(continuation);
         }
      }
#     else
      {
         list_t ret = resume1(continuation);
         if (isempty(ret))
            return((list_t)error(FATAL,"please contact me(resume676).\n"));
         else
            return(ret);
      }
#     endif
   }
   else{
      if (isempty(continuation))
         return(exp);
      else{
         switch(getop(car(continuation))){
            case SUM:
            case COM:
               return(cons(car(continuation),
                           cons(*makelet(LIST,
                                         resume(exp,
                                                getls(car(cdr(continuation))))),
                                cons(*makelet(LIST,
                                              resume(exp,
                                                     getls(car(cdr(cdr(continuation)))))),
                                     makenull(NIL)))));
               break;
            default:
               return(continuation);
         }
      }
   }
}
typedef enum {LEFT,RIGHT,SWITCH} mkcont_t;
#ifdef OUT_MOST
static list_t make_cont_out_most1(mkcont_t op,const list_t exp,list_t continuation)
{
#  ifdef DEBUG_CONT
   printf("make_cont_out_most1->");
#  endif
   switch(getop(car(exp))){
      case SUM:
      case COM:
         switch(op){
            case RIGHT:
               return(cons(car(exp),
                           cons(*makelet(LIST,getls(car(cdr(exp)))),
                                cons(*makelet(LIST,makenull(NIL)),
                                     makenull(NIL)))));
               break;
            case LEFT:
               return(cons(car(exp),
                           cons(*makelet(LIST,makenull(NIL)),
                                cons(*makelet(LIST,getls(car(cdr(cdr(exp))))),
                                     makenull(NIL)))));
               break;
            case SWITCH:
            default:
               return((list_t)error(FATAL,"Please contact me(make_cont_out_most1885): %d\n",op));
         }
         break;
      default:
         return(exp);
   }
}
static list_t make_cont_out_most(mkcont_t op,const list_t exp,list_t continuation)
{
#  ifdef DEBUG_CONT
   printf("make_cont_out_most->");
#  endif
   switch(op){
      case SWITCH:
         switch(getop(car(continuation))){
            case SUM:
            case COM:
               if (ishole(getls(car(cdr(continuation))))){
                  list_t tmp=resume(exp,continuation);
                  return(cons(car(tmp),
                              cons(*makelet(LIST,getls(car(cdr(tmp)))),
                                   cons(*makelet(LIST,makenull(NIL)),
                                        makenull(NIL)))));
               }
               else{
                  if (ishole(getls(car(cdr(cdr(continuation)))))){
                     list_t tmp=resume(exp,continuation);
                     return(cons(car(tmp),
                                 cons(*makelet(LIST,makenull(NIL)),
                                      cons(*makelet(LIST,getls(car(cdr(cdr(tmp))))),
                                           makenull(NIL)))));
                  }
                  else
                     return(continuation);
               }
               break;
            default:
               return(continuation);
         }
         break;
      case RIGHT:{
         list_t inner_most = make_cont_out_most1(op,exp,continuation);
         return(resume(inner_most,continuation));
         break;
      }
      case LEFT:{
         list_t inner_most = make_cont_out_most1(op,exp,continuation);
         return(resume(inner_most,continuation));
         break;
      }
      default:
         return((list_t)error(FATAL,"Please contact me(make_cont_out_most957): %d\n",op));
   }
}
#else
static list_t make_cont_switch1(const list_t exp,list_t continuation)
{
#  ifdef DEBUG_CONT
   printf("make_cont_switch1->");
#  endif
   switch(getop(car(continuation))){
      case SUM:
      case COM:
         if (isempty(getls(car(cdr(continuation)))))
            return(cons(car(continuation),
                        cons(*makelet(LIST,exp),
                             cons(*makelet(LIST,makenull(NIL)),
                                  makenull(NIL)))));
         else{
            if (isempty(getls(car(cdr(cdr(continuation))))))
               return(cons(car(continuation),
                           cons(*makelet(LIST,makenull(NIL)),
                                cons(*makelet(LIST,exp),
                                     makenull(NIL)))));
            else
               return(cons(car(continuation),
                           cons(*makelet(LIST,make_cont_switch1(exp,getls(car(cdr(continuation))))),
                                cons(*makelet(LIST,
                                              make_cont_switch1(exp,getls(car(cdr(cdr(continuation)))))),
                                     makenull(NIL))))));
         }
         break;
      default:
         return(continuation);
   }
}
static list_t make_cont_switch(const list_t exp,list_t continuation)
{
   return(make_cont_switch1(exp,continuation,makenull(NIL)));
}
static list_t make_cont1(enum mkcont_t op,list_t exp,list_t continuation)
{
#  ifdef DEBUG_CONT
   printf("make_cont->");
#  endif
   switch(op){
      case SWITCH:
         return(make_cont_switch(exp,continuation));
         break;
      case RIGHT:
         return(resume(cons(car(exp),
                            (cons(car(cdr(exp)),
                                  (cons(*makelet(LIST,makenull(NIL)),
                                        makenull(NIL)))))),
                       continuation));
         break;
      case LEFT:
         return(resume(cons(car(exp),
                            (cons(*makelet(LIST,makenull(NIL)),
                                  (cons(car(cdr(cdr(exp))),
                                        makenull(NIL)))))),
                       continuation));
         break;
      default:
         return((list_t)error(FATAL,"please contact me(make_cont564) (%d)\n", op));
   }
}
#endif
static list_t make_cont(mkcont_t op,list_t exp,list_t continuation)
{
#  ifdef OUT_MOST
   return(make_cont_out_most(op,exp,continuation));
#  else
   return(make_cont1(op,exp,continuation));
#  endif
}
static int gettype(element_t args) /* B028 */
{
#  ifdef DEBUG_EVAL
   printf("gettype->");
#  endif
   return(gettk(car(getls(args)))->attr.value.type);
}
static list_t applytomins(list_t args) /* B028 */
{
#  ifdef DEBUG_EVAL
   printf("applytomins->");
#  endif
   switch(gettype(car(args))){
      case ICONST:
         switch(gettype(car(cdr(args)))){
            case ICONST:
               return(cons(*makelet(TOKEN,
                                    regsym(cvtia(gettk(car(getls(car(args))))->attr.value.fld.iconst.int_v-gettk(car(getls(car(cdr(args)))))->attr.value.fld.iconst.int_v),VALUE,ICONST)),
                           makenull(NIL)));
            default:
               return((list_t)error(FATAL|ELS,"operands are imcompatible type(applytomins592) (%s)\n", args));
         }
         break;
      case STR:
         switch(gettype(car(cdr(args)))){
            case ICONST:{
               buffer dumy;
               initbuf(&dumy);
               return(cons(*makelet(TOKEN,
                                    regsym(((char *)memcpy(dumy.buf,gettk(car(getls(car(args))))->attr.value.fld.strings.str,gettk(car(getls(car(cdr(args)))))->attr.value.fld.iconst.int_v)),
                                           VALUE,
                                           STR)),
                            makenull(NIL)));
            }
            default:
               return((list_t)error(FATAL|ELS,"operands are imcompatible type(applytomins611) (%s)\n", args));
         }
         break;
      default:
         return((list_t)error(FATAL|ELS,"is not implimented value types in(applytomins615) (%s)\n", args));
   }
}
static list_t applytoplus(list_t args) /* B028 */
{
#  ifdef DEBUG_EQ
   printf("applytoplus->");
#  endif
   switch(gettype(car(args))){
      case ICONST:
         switch(gettype(car(cdr(args)))){
            case ICONST:
               return(cons(*makelet(TOKEN,
                                    regsym(cvtia(gettk(car(getls(car(args))))->attr.value.fld.iconst.int_v+gettk(car(getls(car(cdr(args)))))->attr.value.fld.iconst.int_v),VALUE,ICONST)),
                            makenull(NIL)));
            default:
               return((list_t)error(FATAL|ELS,"operands are imcompatible type(applytoplus636) (%s)\n", args));
         }
         break;
      case STR:
         switch(gettype(car(cdr(args)))){
            case STR:{
               buffer dumy;
               initbuf(&dumy);
               return(cons(*makelet(TOKEN,
                                    regsym(strcat(strcat(dumy.buf,gettk(car(getls(car(args))))->attr.value.fld.strings.str),gettk(car(getls(car(cdr(args)))))->attr.value.fld.strings.str),
                                           VALUE,
                                           STR)),
                           makenull(NIL)));
            }
            default:
               return((list_t)error(FATAL|ELS,"operands are imcompatible type(applytoplus655) (%s)\n", args));
         }
         break;
      default:
         return((list_t)error(FATAL|ELS,"isn't implimented value types in(applytoplus659) (%s)\n", args));
   }
}
static list_t applytonot(list_t args) /* B028 */
{
#  ifdef DEBUG_EVAL
   printf("applytonot->");
#  endif
   switch(gettype(car(args))){
      case ICONST:
         return(cons(*makelet(TOKEN,regsym(cvtia(!(gettk(car(getls(car(args))))->attr.value.fld.iconst.int_v)),VALUE,ICONST)),
                      makenull(NIL)));
         break;
      case STR:
      default:
         return((list_t)error(FATAL|ELS,"isn't implimented value types in(applytonot678) (%s)\n", args));
   }
}
static list_t applytoor(list_t args) /* B028 */
{
#  ifdef DEBUG_EVAL
   printf("applytoor->");
#  endif
   switch(gettype(car(args))){
      case ICONST:
         switch(gettype(car(cdr(args)))){
            case ICONST:
               return(cons(*makelet(TOKEN,
                                    regsym(cvtia(gettk(car(getls(car(args))))->attr.value.fld.iconst.int_v||gettk(car(getls(car(cdr(args)))))->attr.value.fld.iconst.int_v),
                                           VALUE,
                                           ICONST)),
                           makenull(NIL)));
            default:
               return((list_t)error(FATAL|ELS,"operands are imcompatible type(applytoor699) (%s)\n", args));
         }
         break;
      case STR:
      default:
         return((list_t)error(FATAL|ELS,"isn't implimented value types in(applytoor704) (%s)\n", args));
   }
}
static list_t applytodiv(list_t args) /* B028 */
{
#  ifdef DEBUG_EVAL
   printf("applytodiv->");
#  endif
   switch(gettype(car(args))){
      case ICONST:
         switch(gettype(car(cdr(args)))){
            case ICONST:
               return(cons(*makelet(TOKEN,
                                    regsym(cvtia(gettk(car(getls(car(args))))->attr.value.fld.iconst.int_v/gettk(car(getls(car(cdr(args)))))->attr.value.fld.iconst.int_v),
                                           VALUE,
                                           ICONST)),
                           makenull(NIL)));
            default:
               return((list_t)error(FATAL|ELS,"operands are imcompatible type(applytodiv725) (%s)\n", args));
         }
         break;
      case STR:
         switch(gettype(car(cdr(args)))){
            case ICONST:{
               buffer dumy;
               initbuf(&dumy);
               return(cons(*makelet(TOKEN,
                                    regsym((char *)strcpy(dumy.buf,gettk(car(getls(car(args))))->attr.value.fld.strings.str+gettk(car(getls(car(cdr(args)))))->attr.value.fld.iconst.int_v),
                                           VALUE,
                                           STR)),
                           makenull(NIL)));
            }
            default:
               return((list_t)error(FATAL|ELS,"operands are imcompatible type(applytodiv744) (%s)\n", args));
         }
         break;
      default:
         return((list_t)error(FATAL|ELS,"isn't implimented value types in(applytodiv748)\n", args));
   }
}
static list_t applytomult(list_t args) /* B028 */
{
#  ifdef DEBUG_EVAL
   printf("applytomult->");
#  endif
   switch(gettype(car(args))){
      case ICONST:
         switch(gettype(car(cdr(args)))){
            case ICONST:
               return(cons(*makelet(TOKEN,
                                    regsym(cvtia(gettk(car(getls(car(args))))->attr.value.fld.iconst.int_v*gettk(car(getls(car(cdr(args)))))->attr.value.fld.iconst.int_v),
                                           VALUE,
                                           ICONST)),
                           makenull(NIL)));
            default:
               return((list_t)error(FATAL|ELS,"operands are imcompatible type(applytomult769) (%s)\n", args));
         }
         break;
      case STR:
      default:
         return((list_t)error(FATAL|ELS,"isn't implimented value types in(applytomult774) (%s)\n", args));
   }
}
static list_t applytomod(list_t args) /* B028 */
{
#  ifdef DEBUG_EVAL
   printf("applytomod->");
#  endif
   switch(gettype(car(args))){
      case ICONST:
         switch(gettype(car(cdr(args)))){
            case ICONST:
               return(cons(*makelet(TOKEN,
                                    regsym(cvtia(gettk(car(getls(car(args))))->attr.value.fld.iconst.int_v%gettk(car(getls(car(cdr(args)))))->attr.value.fld.iconst.int_v),
                                                  VALUE,
                                                  ICONST)),
                           makenull(NIL)));
            default:
               return((list_t)error(FATAL|ELS,"operands are imcompatible type(applytodiv795) (%s)\n", args));
         }
         break;
      case STR:
      default:
         return((list_t)error(FATAL|ELS,"isn't implimented value types in(applytomod800) (%s)\n", args));
   }
}
static list_t applytoand(list_t args) /* B028 */
{
#  ifdef DEBUG_EVAL
   printf("applytoand->");
#  endif
   switch(gettype(car(args))){
      case ICONST:
         switch(gettype(car(cdr(args)))){
            case ICONST:
               return(cons(*makelet(TOKEN,
                                    regsym(cvtia(gettk(car(getls(car(args))))->attr.value.fld.iconst.int_v&&gettk(car(getls(car(cdr(args)))))->attr.value.fld.iconst.int_v),
                                            VALUE,
                                            ICONST)),
                           makenull(NIL)));
            default:
               return((list_t)error(FATAL|ELS,"operands are imcompatible type(applytoand821) (%s)\n", args));
         }
         break;
      case STR:
      default:
         return((list_t)error(FATAL|ELS,"isn't implimented value types in(applytoand826) (%s)\n", args));
   }
}
static list_t applytoeq(list_t args) /* B028 */
{
#  ifdef DEBUG_EQ
   printf("applytoeq->");
#  endif
   switch(gettype(car(args))){
      case ICONST:
         switch(gettype(car(cdr(args)))){
            case ICONST:
               return(cons(*makelet(TOKEN,regsym(cvtia(gettk(car(getls(car(args))))->attr.value.fld.iconst.int_v==gettk(car(getls(car(cdr(args)))))->attr.value.fld.iconst.int_v),VALUE,ICONST)),makenull(NIL)));
            default:
               return((list_t)error(FATAL|ELS,"operands are imcompatible type(applyto847) (%s)\n", args));
         }
         break;
      case STR:
         switch(gettype(car(cdr(args)))){
            case STR:{
               int result = !TRUE;
               if (!strcmp(gettk(car(getls(car(args))))
                            ->attr.value.fld.strings.str,
                           gettk(car(getls(car(cdr(args)))))
                            ->attr.value.fld.strings.str))
                  result = (bool)TRUE;
               else
                  result = (bool)FALSE;
               return(cons(*makelet(TOKEN,regsym(cvtia(result),VALUE,ICONST)),makenull(NIL)));
            }
            default:
               return((list_t)error(FATAL|ELS,"operands are imcompatible(applytoeq868) (%s)\n", args));
         }
         break;
      default:
         return((list_t)error(FATAL|ELS,"isn't implimented value types in(applytoeq872) (%s)\n", args));
   }
}
static list_t applytole(list_t args) /* B028 */
{
#  ifdef DEBUG_EVAL
   printf("applytole->");
#  endif
   switch(gettype(car(args))){
      case ICONST:
         switch(gettype(car(cdr(args)))){
            case ICONST:
               return(cons(*makelet(TOKEN,
                                    regsym(cvtia(gettk(car(getls(car(args))))->attr.value.fld.iconst.int_v<gettk(car(getls(car(cdr(args)))))->attr.value.fld.iconst.int_v),
                                           VALUE,
                                           ICONST)),
                           makenull(NIL)));
            default:
               return((list_t)error(FATAL|ELS,"operands are imcompatible type(applytole893) (%s)\n", args));
         }
         break;
      case STR:
      default:
         return((list_t)error(FATAL|ELS,"isn't implimented value types in(applytole898) (%s)\n", args));
   }
}
static list_t applytolt(list_t args) /* B028 */
{
#  ifdef DEBUG_EVAL
   printf("applytolt->");
#  endif
   switch(gettype(car(args))){
      case ICONST:
         switch(gettype(car(cdr(args)))){
            case ICONST:
               return(cons(*makelet(TOKEN,
                                    regsym(cvtia(gettk(car(getls(car(args))))->attr.value.fld.iconst.int_v<=gettk(car(getls(car(cdr(args)))))->attr.value.fld.iconst.int_v),
                                           VALUE,
                                           ICONST)),
                           makenull(NIL)));
            default:
               return((list_t)error(FATAL|ELS,"operands are imcompatible type(applytolt919) (%s)\n", args));
         }
         break;
      case STR:
      default:
         return((list_t)error(FATAL|ELS,"isn't implimented value types in(applytolt924) (%s)\n", args));
   }
}
static list_t applytogr(list_t args) /* B028 */
{
#  ifdef DEBUG_EVAL
   printf("applytogr->");
#  endif
   switch(gettype(car(args))){
      case ICONST:
         switch(gettype(car(cdr(args)))){
            case ICONST:
               return(cons(*makelet(TOKEN,
                                    regsym(cvtia(gettk(car(getls(car(args))))->attr.value.fld.iconst.int_v>gettk(car(getls(car(cdr(args)))))->attr.value.fld.iconst.int_v),
                                           VALUE,
                                           ICONST)),
                           makenull(NIL)));
            default:
               return((list_t)error(FATAL|ELS,"operands are imcompatible type(applytogr945) (%s)\n", args));
         }
         break;
      case STR:
      default:
         return((list_t)error(FATAL|ELS,"isn't implimented value types in(applytogr950) (%s)\n", args));
   }
}
static list_t applytogt(list_t args) /* B028 */
{
#  ifdef DEBUG_EVAL
   printf("applytogt->");
#  endif
   switch(gettype(car(args))){
      case ICONST:
         switch(gettype(car(cdr(args)))){
            case ICONST:
               return(cons(*makelet(TOKEN,
                                    regsym(cvtia(gettk(car(getls(car(args))))->attr.value.fld.iconst.int_v>=gettk(car(getls(car(cdr(args)))))->attr.value.fld.iconst.int_v),
                                           VALUE,
                                           ICONST)),
                           makenull(NIL)));
            default:
               return((list_t)error(FATAL|ELS,"operands are imcompatible type(applytogt971) (%s)\n", args));
         }
         break;
      case STR:
      default:
         return((list_t)error(FATAL|ELS,"isn't implimented value types in(applytogt976) (%s)\n", args));
   }
}
static list_t primvalop_apply(element_t valop,list_t args) /* B028 */
{
#  ifdef DEBUG_EQ
   printf("primvalop_apply->");
#  endif
   switch(gettk(valop)->attr.op.type){
      case PLUS:
         return(applytoplus(args));
      case MINS:
         return(applytomins(args));
      case DIV:
         return(applytodiv(args));
      case MULT:
         return(applytomult(args));
      case MOD:
         return(applytomod(args));
      case OR:
         return(applytoor(args));
      case AND:
         return(applytoand(args));
      case NOT:
         return(applytonot(args));
      case EQ:
         return(applytoeq(args));
      case LE:
         return(applytole(args));
      case LT:
         return(applytolt(args));
      case GR:
         return(applytogr(args));
      case GT:
         return(applytogt(args));
      case DEF:
      default:
         return((list_t)error(FATAL|EEL,"please contact me(primvalop_apply1013). (%s)\n", valop));
   }
}
static bool isprimvalop(element_t valop) /* B028 */
{
#  ifdef DEBUG_EVAL
   printf("isprimvalop->");
#  endif
   switch(gettk(valop)->attr.op.type){
      case PLUS:
      case MINS:
      case DIV:
      case MULT:
      case MOD:
      case OR:
      case AND:
      case NOT:
      case EQ:
      case GR:
      case GT:
      case LE:
      case LT:
         return((bool)TRUE);
         break;
      case CON:
      default:
         return((bool)FALSE);
   }
}
static list_t valapply(element_t valop,list_t args,list_t env) /* B028 */
{
#  ifdef DEBUG_EQ
   printf("valapply->");
#  endif
   if (isprimvalop(valop))
      return(primvalop_apply(valop,args));
   else
      return((list_t)error(FATAL|EEL,"sorry. value operaters has not been implemented yet. (%s)\n", valop));
}
static list_t evalval(element_t val,list_t env,queue_t ch)                /* B010 */
{                                                          /* B010 */
#  ifdef DEBUG_VERIFIER1                                          /* B010 */
   printf("evalval->");                                    /* B010 */
#  endif                                                   /* B010 */
   switch(gettk(car(getls(val)))->token_name){             /* B010 */
      case VALUE:                                          /* B010 */
         return(cons(car(getls(val)),makenull(NIL)));      /* B010 */
         break;                                            /* B010 */
      case ID:                                           /* B010 */
         if (!(g_step_exec==FALSE)&&(!isempty_buf(&formula))){
            list_t enforce = lookup1(car(getls(val)),env);
            if (enforce == (list_t)NIL){
               longjmp(when_length_of_path_is_zero,1);
               return((list_t)NIL);
            }
            else
               return(evalval(*makelet(LIST,enforce),env,ch));
         }
         else
            return(evalval(*makelet(LIST,lookup_env(car(getls(val)),env)),env,ch));
         break;                                            /* B007 */
      default:                                             /* B010 */
         return(valapply(car(getls(val)),         /* B028 */
                         evalval_ls(getls(car(cdr(getls(val)))),env,ch),/* B028 */
                         env));                   /* B028 */
   }                                                       /* B010 */
}                                                          /* B010 */
static list_t evalval_ls(list_t val_ls,list_t env,queue_t ch)/* B010 */
{                                                          /* B010 */
#  ifdef DEBUG_EVAL_Q                                         /* B010 */
   printf("evalval_ls->");                                 /* B010 */
#  endif                                                   /* B010 */
   if (isempty(val_ls))                                    /* B010 */
      return(makenull(NIL));                               /* B010 */
   else{                                                   /* B010 */
      return(cons(*makelet(LIST,evalval(car(val_ls),env,ch)),
                   evalval_ls(cdr(val_ls),env,ch)));           /* B010 */
   }                                                       /* B010 */
}                                                          /* B010 */
static list_t make_cls(list_t exp, list_t env)
{
   return(cons(*makelet(TOKEN,regsym("dummy_REC",AGENT_OP,REC)),
               cons(*makelet(LIST,exp),env)));
}
static list_t evalprefix(element_t rate,
                        element_t label,
                        list_t val_ls,
                        list_t body,
                        list_t env,
                        list_t cont,
                        queue_t ch)
{
   list_t data=NIL;
   list_t at_once=makenull(NIL);

#  ifdef DEBUG_CONT
   printf("evalprefix->");
#  endif
   switch(getop(rate)){
/*   (RECV label    val-var-ls rand)          */
      case RECV:{
         list_t new_ch=(list_t)NIL;
#        ifdef DEBUG_EVAL
         printf("recv->");
         fflush(stdout);
#        endif
         if (isouter_action(label)){
            if (isaccept_for_eval(label)||isothers(label))
               return(eval(resume(body,cont),env,makenull(NIL),ch)); /* Ignore accept and others */
            else
               data = n_recv(label);
         }
         else{
            data = lookup_ch(ch,label); /* B043 */
            at_once=cons(*makelet(LIST,cons(label,makenull(NIL))),
                         cons(*makelet(LIST,data),
                              makenull(NIL)));
            switch(channel_order){
               case(C_QUEUE):
                  new_ch = q_remove(ch,*makelet(LIST,at_once));
                  break;
               case(C_STACK):
                  new_ch = delete(*makelet(LIST,at_once),ch);
                  break;
               default:
                  return((list_t)error(FATAL,"Invalid channel type(evalprefix1721)\n"));
            }
         }
         return(eval(resume(make_cls(body,n_boundls(val_ls,data,env)),cont),
                     env,
                     makenull(NIL),
                     new_ch));
         break;
      }
/*   (SEND label    val-exp-ls rand)          */
      case SEND:
#        ifdef DEBUG_EVAL
         printf("send->");
         fflush(stdout);
#        endif
         if (isouter_action(label)){
            n_send(label,evalval_ls(val_ls,env,ch)); /* B010 *//*B029*/
            if (!isempty(cont))
               return(eval(resume(makenull(NIL),cont),
                           env,
                           make_cont(SWITCH,make_cls(body,env),cont),
                           ch));
            else
               return(eval(resume(make_cls(body,env),cont),
                           env,
                           makenull(NIL),
                           ch));
         }
         else{ /* is a inner-action */
            if (!isempty(cont))
               return(eval(resume(makenull(NIL),cont),
                           env,
                           make_cont(SWITCH,make_cls(body,env),cont),
                           n_bound_ch(*makelet(LIST,cons(label,makenull(NIL))),
                                      *makelet(LIST,evalval_ls(val_ls,env,ch)),
                                      ch)));
            else
               return(eval(resume(make_cls(body,env),cont),
                           env,
                           makenull(NIL),
                           n_bound_ch(*makelet(LIST,cons(label,makenull(NIL))),
                                      *makelet(LIST,evalval_ls(val_ls,env,ch)),
                                      ch)));
         }
         break;
      default:
         return((list_t)error(FATAL|EEL,"invalid primitive agent expression(evalprefix301) (%s)\n", rate));
   }
}
static list_t eval(list_t exp,list_t env,list_t cont,queue_t ch)
{
#  ifdef DEBUG_EXP
   printf("eval->");
   fflush(stdout);
#  endif
//EVAL_LOOP:
   if ((trace_on)&&isempty_buf(&formula)){
      printf("\n=== trace on ==================\n");
      fflush(stdout);
      n_print(exp); 
      fflush(stdout);
      prtlst(ch); 
      printf("================== (eval) =====\n");
      fflush(stdout);
   }
   switch(getop(car(exp))){
/*   (RECV label    val-var-ls rand)          */
/*   (SEND label    val-exp-ls rand)          */
      case RECV:
      case SEND:
         if (isaccept(exp))
            if (!(g_step_exec==FALSE)&&(!isempty_buf(&formula)))
               return(exp);
         if (istrans(exp,env,ch))
            return(evalprefix(car(exp),
                              car(cdr(exp)),
                              getls(car(cdr(cdr(exp)))),
                              getls(car(cdr(cdr(cdr(exp))))),
                              env,
                              cont,
                              ch));
         else{
            if (isempty(cont))
               return(exp);
            else
               return(eval(resume(make_cls(exp,env),cont),
                           env,
                           makenull(NIL),
                           ch));
         }
         break;
/*   (SUM  rand     rand           )          */
      case SUM:{
#        ifdef DEBUG_EVAL
         printf("sum->");
         fflush(stdout);
#        endif
         if (interactive_mode == ON){
            n_print(exp);
            printf("Choose L(eft), R(ight) operand or C(ontinue):");
            fflush(stdout);
            switch(toupper(n_getc_stdin())){
               case 'C':{
                  interactive_mode=OFF;
                  return(eval(exp,env,cont,ch));
                  break;
               }
               case 'L':{
                  return(eval(getls(car(cdr(exp))),env,cont,ch));
                  break;
               }
               case 'R':
               default:
                  return(eval(getls(car(cdr(cdr(exp)))),env,cont,ch));
                  break;
            }
         }
         else{
            if (istrans(getls(car(cdr(exp))),env,ch))
               return(eval(getls(car(cdr(exp))),env,cont,ch));
            else{
               if (istrans(getls(car(cdr(cdr(exp)))),env,ch))
                  return(eval(getls(car(cdr(cdr(exp)))),env,cont,ch));
               else{                           /* B024 */
                  if (isempty(cont))  /* B042 */
                     return(exp);     /* B042 */
                  else                /* B042 */
                     return(eval(resume(exp,cont),/* B024 *//* B035 */
                                 env,             /* B024 *//* B035 */
                                 makenull(NIL), /* B024 *//* B035 */
                                 ch));
               }
            }
         }
         break;
      }
/*   (COM  rand     rand           )          */
      case COM:{
#        ifdef DEBUG_EVAL
         printf("com->");
         fflush(stdout);
#        endif
         if (interactive_mode == ON){
            n_print(exp);
            printf("Choose L(eft), R(ight) operand or C(ontinue):");
            fflush(stdout);
            switch(toupper(n_getc_stdin())){
               case 'C':{
                  interactive_mode=OFF;
                  return(eval(exp,env,cont,ch));
                  break;
               }
               case 'L':{
                  return(eval(getls(car(cdr(exp))),
                              env,
                              make_cont(LEFT,exp,cont),
                              ch));
                  break;
               }
               case 'R':
               default:{
                  return(eval(getls(car(cdr(cdr(exp)))),
                              env,
                              make_cont(RIGHT,exp,cont),
                              ch));
                  break;
               }
            }
         }
         else{
            if (istrans(getls(car(cdr(exp))),env,ch)){
               return(eval(getls(car(cdr(exp))),
                           env,
                           make_cont(LEFT,exp,cont),
                           ch));
            }
            else{
               if (istrans(getls(car(cdr(cdr(exp)))),env,ch)){
                  return(eval(getls(car(cdr(cdr(exp)))),
                              env,
                              make_cont(RIGHT,exp,cont),
                              ch));
               }
               else{
                  if (isempty(cont))
                     return(exp);
                  else
                     return(eval(resume(exp,cont),
                                 env,
                                 makenull(NIL),
                                 ch));
               }
            }
         }
         break;
      }
/*   (IF   bool-exp rand rand      )          */
      case IF:{
         list_t cond = (list_t)NIL;
         int ret=0;

#        ifdef DEBUG_EVAL
         printf("if->");
         fflush(stdout);
#        endif
         if ((g_step_exec==TRUE)&&(!isempty_buf(&formula))){
            if ((ret=setjmp(when_length_of_path_is_zero))==0){
               cond=evalval(car(cdr(exp)),env,ch);
               if (getval(car(cond))){
                  list_t then_part=getls(car(cdr(cdr(exp))));
                  if (isif(car(then_part))){
                     cond=evalval(car(cdr(then_part)),env,ch);
                     if (getval(car(cond)))
                        return(resume(getls(car(cdr(cdr(then_part)))),cont));
                     else
                        return(resume(getls(car(cdr(cdr(cdr(then_part))))),cont));
                  }
                  else
                     return(resume(then_part,cont));
               }
               else{
                  list_t else_part=getls(car(cdr(cdr(cdr(exp)))));
                  if (isif(car(else_part))){
                     cond=evalval(car(cdr(else_part)),env,ch);
                     if (getval(car(cond)))
                        return(resume(getls(car(cdr(cdr(else_part)))),cont));
                     else
                        return(resume(getls(car(cdr(cdr(cdr(else_part))))),cont));
                  }
                  else
                     return(resume(else_part,cont));
               }
            }
            else{
               list_t branch=(list_t)NIL;
               if (acceptance_condition==ACC_WEAKLY)
                  branch=getls(car(cdr(cdr(exp)))); /* undefined variables weakly hold */
               else
                  branch=getls(car(cdr(cdr(cdr(exp))))); /* undefined variables strongly hold */
               if (isif(car(branch))){
                  cond=evalval(car(cdr(branch)),env,ch);
                  if (getval(car(cond)))
                     return(resume(getls(car(cdr(cdr(branch)))),cont));
                  else
                     return(resume(getls(car(cdr(cdr(cdr(branch))))),cont));
               }
               else
                  return(resume(branch,cont));
            }
         }
         else{ /*** simulatin mode***/
            if ((cond=evalval(car(cdr(exp)),env,ch))== (list_t)NIL)
               return(resume(getls(car(cdr(cdr(cdr(exp))))),cont));
            if (getval(car(cond))) /* B023 */
               return(eval(getls(car(cdr(cdr(exp)))),env,cont,ch));/* B043 */
            else
               return(eval(getls(car(cdr(cdr(cdr(exp))))),env,cont,ch));/* B043 */
         }
         break;
      }
/*   (REC  body env)          */
      case REC:  /*** Notice that REC is used as a closure ***/
#        ifdef DEBUG_EVAL
         printf("rec->");
         fflush(stdout);
#        endif
         if (istrans(getls(car(cdr(exp))),cdr(cdr(exp)),ch))
            return(eval(getls(car(cdr(exp))),
                        cdr(cdr(exp)),
                        cont,
                        ch));
         else{
            if (isempty(cont))
               return(exp);
            else
               return(eval(resume(exp,cont),env,makenull(NIL),ch));
         }
         break;
/*   (CON  a-cons   val-exp-ls     )          */
      case CON:
#        ifdef DEBUG_EVAL
         printf("con->");
         fflush(stdout);
#        endif
         if (isabort(exp))
            return(exp);
         if (isstop(car(cdr(exp)))){
            if (!isempty(cont))
               return(eval(resume(exp,cont),
                           env,
                           makenull(NIL),
                           ch));
            else
               return(primagnteval(car(cdr(exp)),
                                 getls(car(cdr(cdr(exp)))),
                                 env,
                                 ch));
         }
         if (isprimagnt(car(cdr(exp)))){
            if (isempty(getls(car(cdr(cdr(exp))))))
               return(primagnteval(car(cdr(exp)),
                                 getls(car(cdr(cdr(exp)))),
                                 env,
                                 ch));
            else
               return(primagnteval(car(cdr(exp)),
                                 getls(car(evalval_ls(getls(car(cdr(cdr(exp)))),
                                                      env,
                                                      ch))),
                                 env,
                                 ch));
         }
         else{
            return(eval(resume(make_cls(getls(car(cdr(lookup_env(car(cdr(exp)),env)))),
                                        n_boundls(getls(car(lookup_env(car(cdr(exp)),env))),
                                                  evalval_ls(getls(car(cdr(cdr(exp)))),env,ch),
                                                  env)),
                               cont),
                        env,
                        makenull(NIL),
                        ch));
         }
         break;
/*   (REL  rand    (rel-ls)        )          */
      case REL:
         return((list_t)error(FATAL|ELS,"sorry. operater REL has not been implemented yet. (%s)\n", exp));
         break;
/*   (RES  rand    label-ls  co-env)          */
      case RES:
         return((list_t)error(FATAL|ELS,"sorry. operater RES has not been implemented yet. (%s)\n", exp));
         break;
      default:
         return((list_t)error(FATAL|ELS,"invalid primitive agent expression(eval1352) (%s)\n", exp));
   }
}
#include "verifier.c"
/*****************************************************
 *  the following interpreter is top level dirver    *
 * loop.                                             *
 *  its the first argment is agent expression,       *
 * the second argments is enviroment, the 3rd argment*
 * is sort that is a set of action,the 4th argment   *
 * is side-effect for print function.                *
 *  SORT is a global variable, becuse when this      *
 * this system import reflection, SORT is common     *
 * along with each meta-level.                       *
 *---------------------------------------------------*
 *                                                   *
 *****************************************************/
#include "time.h"
//static list_t driver_loop1(list_t form,list_t env)
//{
//#  ifdef DEBUG_EVAL
//   printf("driver_loop1->");
//#  endif
//   if (isempty(form)){
//      n_print(form);
//      return(driver_loop(env));
//   }
//   else{
//      if (isdef(car(form)))
//         return(driver_loop(n_bound(car(cdr(form)),
//                                    *makelet(LIST,
//                                             dotpair(car(cdr(cdr(form))),
//                                                     car(cdr(cdr(cdr(form)))))),
//                                    env)));
//      else{
//         if (isbind(car(form))){
//            prtlst(n_bind(car(cdr(form)),car(cdr(cdr(form)))));
//            return(driver_loop(env));
//         }
//         else{
//            list_t ret=(list_t)NIL;
//            if (isempty_buf(&formula))
//               n_print(eval(form,env,makenull(NIL),make_ch()));/*B043*/
//            else{
//               verifier(form,env,makenull(NIL),makenull(NIL),make_ch());
//            }
//            ret=driver_loop(env);
//            return(ret);
//         }
//      }
//   }
//}
static list_t driver_loop(list_t env)
{
   list_t form=NIL;
   int ret=0;
   clock_t begin_tt;
   clock_t end_tt;

#  ifdef DEBUG_EVAL
   printf("driver_loop->");
#  endif
 LOOP:
   if (source_file == stdin)
      printf("\nRCCS>>"), fflush(stdout);
   begin_tt = clock();
   g_state_counter = 0;
   if ((ret=setjmp(driver_env))==0){
      if ((form = n_read()) == (list_t)NIL){
         n_end();
         goto LOOP;
      }
      else{
         if (isempty(form)){
            n_print(form);
            goto LOOP;
         }
         else{
            if (isdef(car(form))){
               env=n_bound(car(cdr(form)),*makelet(LIST,dotpair(car(cdr(cdr(form))),car(cdr(cdr(cdr(form)))))),env);
               goto LOOP;
            }
            else{
               if (isbind(car(form))){
                  prtlst(n_bind(car(cdr(form)),car(cdr(cdr(form)))));
                  goto LOOP;
               }
               else{
                  if (isdefinit(car(form))){
                     env=n_boundls(getls(car(cdr(form))),getls(car(cdr(cdr(form)))),env);
                     goto LOOP;
                  }
                  else{
                     if (isempty_buf(&formula))
                        n_print(eval(form,env,makenull(NIL),make_ch()));
                     else{
                        list_t ASSERTION = cons(*makelet(TOKEN,regsym("dummy_CON",AGENT_OP,CON)),
                                                 dotpair(*makelet(TOKEN,addattr(regsym(INIT_NODE_NAME,ID),A_CON)),
                                                         *makelet(LIST,makenull(NIL))));
                        verifier((initial_form=form),env,makenull(NIL),ASSERTION,make_ch());
                        gc(&memory_control_table);
                        if (!isempty_buf(&formula)){
                           end_tt = clock();
                           if (!(end_tt==(clock_t)-1))
                              printf("\n\n%d states explored in %10f seconds(2).\n",g_state_counter,((double)(end_tt-begin_tt)/CLOCKS_PER_SEC));
                        }
                     }
                     goto LOOP;
                  }
               }
            }
         }
      }
   }
   else{
      if (ret!=1){
         gc(&memory_control_table);
         if (!isempty_buf(&formula)){
            end_tt = clock();
            if (!(end_tt==(clock_t)-1))
               printf("\n\n%d states explored in %10f seconds(1).\n",g_state_counter,((double)(end_tt-begin_tt)/CLOCKS_PER_SEC));
         }
         goto LOOP;
      }
      else
         return(makenull(NIL));
   }
}
void driver1(void)
{
   int ret=0;
#  ifdef DEBUG_EVAL
   printf("driver1->");
#  endif
   if ((ret=setjmp(terminate_driver_env))==0){
      if (!isempty_buf(&formula)){
         if (ltl2ccs(formula.buf,&(seed.buf)) ==(char *)NIL)
            error(FATAL,"ltl2ccs: cannot translate the LTL formula: %s.\n",formula.buf);
         else{
            seed.ptr = 0; /* initialize just the pointer to the top of seed buffer, not the clean of an input buffer.*/
            driver_loop(makenull(NIL));
         }
      }
      else
         driver_loop(makenull(NIL));
   }
   else
      return;
}
void driver(void)
{
   int ret=0;
#  ifdef DEBUG_EVAL
   printf("driver->");
#  endif
   setup_signal_stack();
   if ((ret=sigsetjmp(signal_driver_env,2))==0)
      driver1();
   else{
      error(SUCCESS,"RCCS: terminated by stack overflow\n");
      driver1();
      return;
   }
}
