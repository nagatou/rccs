/*
 * $Id: verifier.c,v 1.56 2013/07/18 12:41:16 nagatou Exp $
 */
//static char rcsid[]="$Id: verifier.c,v 1.56 2013/07/18 12:41:16 nagatou Exp $";
//static char copyright[]=
//   "Copyright (C) 1998 Naoyuki Nagatou";

/*******************************************************
   this file defines a set of functions to model checking.
                 Copyright (C) 1998 Naoyuki Nagatou
 *******************************************************/
#ifdef DEBUG_MC
static void mc_back(int state_num)
{
   return;
}
#endif
#ifndef _MODEL
#  define _MODEL exp
#endif
/*****************************************************
 *  This function make a model checking.
 * Basically this look like eval(),
 * however it is differnt to track the all possible
 * execution.
 *---------------------------------------------------
 *  ret = verifier(exp,assertion,env,cont,history);
 *
 *  list_t ret -- a list of a whole history as one of exections.
 *  list_t exp -- a list of a model.
 *  list_t assertion -- a list of a property.
 *  list_t env -- is the same of eval.
 *  list_t cont -- is the same of eval.
 *  list_t history -- a list of a history on its way.
 *****************************************************/
static list_t make_current_state(list_t exp,list_t cont,list_t assertion, list_t ch,list_t env)
{
   if (exp==(list_t)NIL||cont==(list_t)NIL||assertion==(list_t)NIL||ch==(list_t)NIL)
      return((list_t)error(FATAL,"segmentation fault(make_current_state36)\n"));
   else
      return(cons(*makelet(LIST,resume(exp,cont)),
                   cons(*makelet(LIST,assertion),makenull(NIL))));
//                   cons(*makelet(LIST,ch),makenull(NIL))));
//                   cons(*makelet(LIST,cont),makenull(NIL))));
}
static list_t mark(list_t state,list_t stack,bool side_effect)
{
#  ifdef DEBUG_VERIFIER1
   printf("mark->");
#  endif
   if ((state==(list_t)NIL)||(stack==(list_t)NIL))
      return((list_t)error(FATAL,"Segmentation fault(mark33).\n"));
   else{
      if (side_effect==TRUE){
         if (ismember(getls(car(stack)),*makelet(LIST,state)))
            return(stack);
         else
            return(q_append(stack,*makelet(LIST,state)));
      }
      else{
         if (ismember(stack,*makelet(LIST,state)))
            return(stack);
         else
            return(cons(*makelet(LIST,state),stack));
      }
   }
}
static list_t on_stack(list_t state,list_t stack)
{
#  ifdef DEBUG_VERIFIER1
   printf("on_stack->");
#  endif
   if ((state==(list_t)NIL)||(stack==(list_t)NIL))
      return((list_t)error(FATAL,"Segmentation fault(on_stack49).\n"));
   else
      return(mark(state,stack,(bool)FALSE));
}
static bool isaccept_abort(list_t ltl_automaton)
{
#  ifdef DEBUG_VERIFIER1
   printf("isaccept_abort->");
#  endif
   if (isaccept(ltl_automaton)){
      if (isabort(getls(car(cdr(cdr(cdr(ltl_automaton)))))))
         return((bool)TRUE);
      else
         return((bool)FALSE);
   }
   else
      return((bool)FALSE);
}

static list_t initial_form=&epsilon;

static list_t secondDFS(list_t,list_t,list_t,list_t,list_t,list_t,list_t,queue_t,int);
static list_t secondDFS2(list_t exp,
                         list_t assertion,
                         list_t env,
                         list_t cont,
                         list_t history,
                         list_t for_firstDFS,
                         list_t for_secondDFS,
                         bool sw,
                         queue_t ch,
                         int depth_counter)
{
#  ifdef DEBUG_VERIFIER2
   printf("secondDFS2->");
#  endif
   if (sw == FALSE){
      if (depth_counter>DEPTH_LIMIT){
         depth_counter=0;
//         g_emptyness=TRUE;
//         return((list_t)(error(SUCCESS|EEL,"Emptyness(2): TRUE, %s",*makelet(LIST,history))));
         return(history);
      }
      else
         return(secondDFS(exp,assertion,env,cont,history,for_firstDFS,for_secondDFS,ch,++depth_counter));
   }
   else{
      list_t tmp = eval(assertion,env,makenull(NIL),make_ch());
      gc(&memory_control_table);
      if ((isaccept_abort(tmp)) || (ismember(for_firstDFS,*makelet(LIST,make_current_state(exp,cont,tmp,ch,env))))){
            g_emptyness=TRUE;
            return((list_t)(error(SUCCESS|EEL,"Emptyness: TRUE, %s",*makelet(LIST,history))));
      }
      else{
         if (isaccept(tmp))
            return(secondDFS2(exp,
                              getls(car(cdr(cdr(cdr(tmp))))),
                              env,
                              cont,
                              history,
                              for_firstDFS,
//                              on_stack(make_current_state(exp,cont,tmp,ch,env),for_firstDFS),
                              mark(make_current_state(exp,cont,tmp,ch,env),for_secondDFS,TRUE),
                              g_step_exec=FALSE,
                              ch,
                              depth_counter));
         else
            return(secondDFS2(exp,
                              tmp,
                              env,
                              cont,
                              history,
                              for_firstDFS,
                              mark(make_current_state(exp,cont,tmp,ch,env),for_secondDFS,TRUE),
                              g_step_exec=FALSE,
                              ch,
                              depth_counter));
      }
   }
}
static list_t secondDFS1(element_t rate,
                         element_t label,
                         list_t val_ls,
                         list_t body,
                         list_t assertion,
                         list_t env,
                         list_t cont,
                         list_t history,
                         list_t for_firstDFS,
                         list_t for_secondDFS,
                         queue_t ch,
                         list_t exp,
                         int depth_counter)
{
   list_t data=(list_t)NIL;
   list_t ret=(list_t)NIL;

#  ifdef DEBUG_VERIFIER2
   printf("secondDFS1->");
#  endif
   switch(getop(rate)){
/*   (RECV label    val-var-ls rand)          */
      case RECV:{
         list_t at_once=makenull(NIL);
         list_t new_ch=(list_t)NIL;
#        ifdef DEBUG_VERIFIER2
         printf("recv->");
         fflush(stdout);
#        endif
         if (istrans(exp,env,ch)){
            if (isouter_action(label))
               data = n_recv(label);
            else{
               data=lookup_ch(ch,label); /* B043 */
               at_once= cons(*makelet(LIST,cons(label,makenull(NIL))),
                              cons(*makelet(LIST,data),makenull(NIL)));
               switch(channel_order){
                  case(C_QUEUE):
                     new_ch = q_remove(ch,*makelet(LIST,at_once));
                     break;
                  case(C_STACK):
                     new_ch = delete(*makelet(LIST,at_once),ch);
                     break;
                  default:
                     return((list_t)error(FATAL,"Invalid channel type(secondDFS1:181)\n"));
               }
            }
            if (ismember(getls(car(for_secondDFS)),*makelet(LIST,make_current_state(_MODEL,cont,assertion,ch,env)))) /* don't explore same state */
               return(history);
            ret=secondDFS2(resume(body,cont),
                           assertion,
                           n_boundls(val_ls,data,env),
                           makenull(NIL),
                           cons(*makelet(LIST,dotpair(rate,*makelet(LIST,at_once))),history),
                           for_firstDFS,
                           for_secondDFS,
                           g_step_exec=TRUE,
                           new_ch,
                           depth_counter);
            if (isouter_action(label)){
               if (retrieval1(label)!=(bindLSp)NIL){
                  if (!isempty_buf(&target))
                     mc_back(1);
               }
            }
            return(ret);
         }
         else{
//            if (!istrans(resume(exp,cont),env,ch)){
//               return(secondDFS2(resume(exp,cont),
//                                 assertion,
//                                 env,
//                                 makenull(NIL),
//                                 history,
//                                 for_firstDFS,
//                                 for_secondDFS,
//                                 g_step_exec=TRUE,
//                                 ch,
//                                 depth_counter));
//            }
//            else
               return(history);
         }
         break;
      }
/*   (SEND label    val-exp-ls rand)          */
      case SEND:
#        ifdef DEBUG_VERIFIER2
         printf("send->");
         fflush(stdout);
#        endif
         if (isouter_action(label)){
            list_t at_once=evalval_ls(val_ls,env,ch); /* B043 */
            n_send(label,at_once);
            if (!isempty_buf(&target)){
               ret = secondDFS(resume(body,cont),
                               assertion,
                               env,
                               makenull(NIL),
                               cons(*makelet(LIST,cons(rate,dotpair(label,*makelet(LIST,at_once)))),history),
                               for_firstDFS,
                               mark(make_current_state(_MODEL,cont,assertion,ch,env),for_secondDFS,TRUE),
                               ch,
                               depth_counter);
               if (retrieval1(label)!=(bindLSp)NIL)
                  mc_back(1);
               return(ret);
            }
            else
               return(secondDFS(resume(body,cont),
                                assertion,
                                env,
                                makenull(NIL),
                                cons(*makelet(LIST,cons(rate,dotpair(label,*makelet(LIST,at_once)))),history),
                                for_firstDFS,
                                mark(make_current_state(_MODEL,cont,assertion,ch,env),for_secondDFS,TRUE),
                                ch,
                                depth_counter));
         }
         else{ /* is an inner-action */
            list_t at_once=evalval_ls(val_ls,env,ch);
            return(secondDFS(resume(body,cont),
                             assertion,
                             env,
                             makenull(NIL),
                             cons(*makelet(LIST,cons(rate,dotpair(label,*makelet(LIST,at_once)))),history),
                             for_firstDFS,
                             mark(make_current_state(_MODEL,cont,assertion,ch,env),for_secondDFS,TRUE),
                             n_bound_ch(*makelet(LIST,cons(label,makenull(NIL))),
                                        *makelet(LIST,at_once),
                                        ch),
                             depth_counter));
         }
         break;
      default:
         return((list_t)error(FATAL|EEL,"invalid primitive agent expression(secondDFS263) (%s)\n", rate));
   }
}
static list_t secondDFS(list_t exp,
                       list_t assertion,
                       list_t env,
                       list_t cont,
                       list_t history,
                       list_t for_firstDFS,
                       queue_t for_secondDFS,
                       queue_t ch,
                       int depth_counter)
{
#  ifdef DEBUG_VERIFIER2
   printf("secondDFS->");
#  endif
   if (trace_on){
      printf("\n=== trace on (secondDFS)==================\n");
      n_print(resume(exp,cont)); 
      fprintf(stdout,"depth: %d\n",depth_counter);
      n_print(assertion); 
      prtlst(ch);
      printf("================== (channel) =====\n");
      fflush(stdout);
   }
   if (!istrans(resume(exp,cont),env,ch)){
      return(secondDFS2(resume(exp,cont),
                        assertion,
                        env,
                        makenull(NIL),
                        history,
                        for_firstDFS,
                        for_secondDFS,
                        g_step_exec=TRUE,
                        ch,
                        depth_counter));
      
   }
   switch(getop(car(exp))){
/*   (RECV label    val-var-ls rand)          */
/*   (SEND label    val-exp-ls rand)          */
      case RECV:
      case SEND:
         return(secondDFS1(car(exp),car(cdr(exp)),getls(car(cdr(cdr(exp)))),getls(car(cdr(cdr(cdr(exp))))),assertion,env,cont,history,for_firstDFS,for_secondDFS,ch,exp,depth_counter));
         break;
/*   (REC  rand     env        cont)          */
      case REC:
#        ifdef DEBUG_VERIFIER
         printf("rec->");
         fflush(stdout);
#        endif
         return(secondDFS(getls(car(cdr(exp))),assertion,cons(*makelet(LIST,getls(car(cdr(cdr(exp))))),env),cont,history,for_firstDFS,for_secondDFS,ch,depth_counter));
         break;
/*   (SUM  rand     rand           )          */
      case SUM:
#        ifdef DEBUG_VERIFIER
         printf("sum->");
         fflush(stdout);
#        endif
         secondDFS(getls(car(cdr(exp))),assertion,env,cont,history,for_firstDFS,for_secondDFS,ch,depth_counter);
         if (trace_on)
            printf("-- back track ----------\n");
         gc(&memory_control_table);
         secondDFS(getls(car(cdr(cdr(exp)))),assertion,env,cont,history,for_firstDFS,for_secondDFS,ch,depth_counter);
         return(makenull(NIL));
         break;
/*   (COM  rand     rand           )          */
      case COM:
#        ifdef DEBUG_VERIFIER
         printf("com->");
         fflush(stdout);
#        endif
         secondDFS(getls(car(cdr(exp))),assertion,env,make_cont(LEFT,exp,cont),history,for_firstDFS,for_secondDFS,ch,depth_counter);
         if (trace_on)
            printf("-- back track ----------\n");
         gc(&memory_control_table);
         secondDFS(getls(car(cdr(cdr(exp)))),assertion,env,make_cont(RIGHT,exp,cont),history,for_firstDFS,for_secondDFS,ch,depth_counter);
         return(makenull(NIL));
         break;
/*   (IF   bool-exp rand rand      )          */
      case IF:
#        ifdef DEBUG_VERIFIER
         printf("if->");
         fflush(stdout);
#        endif
         if (getval(car(evalval(car(cdr(exp)),env,ch))))
            return(secondDFS(getls(car(cdr(cdr(exp)))),
                            assertion,
                            env,
                            cont,
                            history,
                            for_firstDFS,
                            for_secondDFS,
                            ch,
                            depth_counter));
         else
            return(secondDFS(getls(car(cdr(cdr(cdr(exp))))),
                            assertion,
                            env,
                            cont,
                            history,
                            for_firstDFS,
                            for_secondDFS,
                            ch,
                            depth_counter));
         break;
/*   (CON  a-cons   val-exp-ls     )          */
      case CON:
#        ifdef DEBUG_VERIFIER
         printf("con->");
         fflush(stdout);
#        endif
         if (isabort(exp)){
            return(n_abort(history));
         }
         else{
            if (eqstop(exp))
               return(history);
            else{
               if (isprimagnt(car(cdr(exp)))){
                  if (isempty(getls(car(cdr(cdr(exp))))))
                     return(primagnteval(car(cdr(exp)),(getls(car(cdr(cdr(exp))))),env,ch));
                  else
                     return(primagnteval(car(cdr(exp)),getls(car(evalval_ls(getls(car(cdr(cdr(exp)))),env,ch))),env,ch));
               }
               else{
                  if (isempty(getls(car(cdr(cdr(exp))))))
                     return(secondDFS(getls(car(cdr(lookup(car(cdr(exp)),env,ch)))), /* B043 */
                                      assertion,
                                      env,
                                      cont,
                                      history,
                                      for_firstDFS,
                                      for_secondDFS,
                                      ch,
                                      depth_counter));
                  else
                     return(secondDFS(getls(car(cdr(lookup(car(cdr(exp)),env,ch)))), /* B043 */
                                     assertion,
                                     n_boundls(getls(car(lookup(car(cdr(exp)),env,ch))), /* B043 */
                                               evalval_ls(getls(car(cdr(cdr(exp)))),env,ch),
                                               env),
                                     cont,
                                     history,
                                     for_firstDFS,
                                     for_secondDFS,
                                     ch,
                                     depth_counter));
               }
            }
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
/*   (BIND label    port           )          */
      case BIND:{
         n_bind(car(cdr(exp)),car(cdr(cdr(exp))));
         if (!isempty(cont))
            return(secondDFS(resume(cons(*makelet(TOKEN,makesym(AGENT_OP,CON)),
                                         dotpair(*makelet(TOKEN,addattr(regsym("stop",ID),A_CON)),
                                                  *makelet(LIST,makenull(NIL)))),
                                   cont),
                            assertion,
                            env,
                            makenull(NIL),
                            history,
                            for_firstDFS,
                            for_secondDFS,
                            ch,
                            depth_counter));
         else
            return(makenull(NIL));
         break;
      }
      default:
         return((list_t)error(FATAL|ELS,"invalid primitive agent expression(secondDFS1597) (%s)\n", exp));
   }
}
static list_t n_accept(list_t exp,
                       list_t assertion,
                       list_t env,
                       list_t cont,
                       list_t history,
                       list_t for_firstDFS,
                       list_t for_secondDFS,
                       queue_t ch,
                       int depth_counter)
{
#  ifdef DEBUG_VERIFIER
   printf("n_accept->");
#  endif
//   return(secondDFS(exp,getls(car(cdr(cdr(cdr(assertion))))),env,cont,history,on_stack(make_current_state(exp,cont,assertion,ch,env),for_firstDFS),for_secondDFS,ch, depth_counter));
   return(secondDFS(exp,getls(car(cdr(cdr(cdr(assertion))))),env,cont,history,on_stack(make_current_state(exp,cont,assertion,ch,env),for_firstDFS),make_que(),ch, depth_counter));
}
static list_t firstDFS(list_t,list_t,list_t,list_t,list_t,list_t,list_t,queue_t,int);
static list_t firstDFS2(list_t exp,
                        list_t assertion,
                        list_t env,
                        list_t cont,
                        list_t history,
                        list_t for_firstDFS,
                        list_t for_secondDFS,
                        bool sw,
                        queue_t ch,
                        int depth_counter)
{
#  ifdef DEBUG_VERIFIER1
   printf("firstDFS2->");
#  endif
   if (sw == FALSE){
      if (depth_counter>DEPTH_LIMIT){
         depth_counter=0;
         return(makenull(NIL));
      }
      else
         return(firstDFS(exp,assertion,env,cont,history,on_stack(make_current_state(exp,cont,assertion,ch,env),for_firstDFS),for_secondDFS,ch,++depth_counter));
   }
   else{
      list_t tmp = eval(assertion,env,makenull(NIL),make_ch());
      gc(&memory_control_table);
      if (isaccept(tmp))
         return(n_accept(exp,tmp,env,cont,history,for_firstDFS,for_secondDFS,ch,++depth_counter));
      else
         return(firstDFS2(exp,tmp,env,cont,history,for_firstDFS,for_secondDFS,g_step_exec=FALSE,ch,depth_counter));
   }
}
static list_t firstDFS1(element_t rate,
                        element_t label,
                        list_t val_ls,
                        list_t body,
                        list_t assertion,
                        list_t env,
                        list_t cont,
                        list_t history,
                        list_t for_firstDFS,
                        list_t for_secondDFS,
                        queue_t ch,
                        list_t exp,
                        int depth_counter)
{
   list_t data=(list_t)NIL;
   list_t ret=(list_t)NIL;

#  ifdef DEBUG_VERIFIER
   printf("firstDFS1->");
#  endif
   switch(getop(rate)){
/*   (RECV label    val-var-ls rand)          */
      case RECV:{
         list_t at_once=makenull(NIL);
         list_t new_ch=(list_t)NIL;
#        ifdef DEBUG_VERIFIER
         printf("recv->");
         fflush(stdout);
#        endif
         if (istrans(exp,env,ch)){
            if (isouter_action(label))
               data = n_recv(label);
            else{
               data=lookup_ch(ch,label); /* B043 */
               at_once= cons(*makelet(LIST,cons(label,makenull(NIL))),
                              cons(*makelet(LIST,data),makenull(NIL)));
               switch(channel_order){
                  case(C_QUEUE):
                     new_ch = q_remove(ch,*makelet(LIST,at_once));
                     break;
                  case(C_STACK):
                     new_ch = delete(*makelet(LIST,at_once),ch);
                     break;
                  default:
                     return((list_t)error(FATAL,"Invalid channel type(firstDFS1:687)\n"));
               }
            }
            if (ismember(for_firstDFS,*makelet(LIST,make_current_state(_MODEL,cont,assertion,ch,env)))) /* don't explore same state */
               return(history);
            ret=firstDFS2(resume(body,cont),
                          assertion,
                          n_boundls(val_ls,data,env),
                          makenull(NIL),
                          cons(*makelet(LIST,dotpair(rate,*makelet(LIST,at_once))),history),
                          for_firstDFS,
                          for_secondDFS,
                          g_step_exec=TRUE,
                          new_ch,
                          depth_counter);
            if (isouter_action(label)){
               if (retrieval1(label)!=(bindLSp)NIL){
                  if (!isempty_buf(&target))
                     mc_back(1);
               }
            }
            return(ret);
         }
         else
            return(history);
         break;
      }
/*   (SEND label    val-exp-ls rand)          */
      case SEND:
#        ifdef DEBUG_VERIFIER
         printf("send->");
         fflush(stdout);
#        endif
         if (ismember(for_firstDFS,*makelet(LIST,make_current_state(_MODEL,cont,assertion,ch,env)))) /* don't explore same state. When the action is external this if-statment is needed */
            return(history);
         if (isouter_action(label)){
            list_t at_once=evalval_ls(val_ls,env,ch); /* B043 */
            n_send(label,at_once);
            if (!isempty_buf(&target)){
               ret = firstDFS(resume(body,cont),
                              assertion,
                              env,
                              makenull(NIL),
                              cons(*makelet(LIST,cons(rate,dotpair(label,*makelet(LIST,at_once)))),history),
                              on_stack(make_current_state(_MODEL,cont,assertion,ch,env),for_firstDFS),
                              for_secondDFS,
                              ch,
                              depth_counter);
               if (retrieval1(label)!=(bindLSp)NIL)
                  mc_back(1);
               return(ret);
            }
            else
               return(firstDFS(resume(body,cont),
                               assertion,
                               env,
                               makenull(NIL),
                               cons(*makelet(LIST,cons(rate,dotpair(label,*makelet(LIST,at_once)))),history),
                               on_stack(make_current_state(_MODEL,cont,assertion,ch,env),for_firstDFS),
                               for_secondDFS,
                               ch,
                               depth_counter));
         }
         else{ /* is an inner-action */
            list_t at_once=evalval_ls(val_ls,env,ch);
            return(firstDFS(resume(body,cont),
                            assertion,
                            env,
                            makenull(NIL),
                            cons(*makelet(LIST,cons(rate,dotpair(label,*makelet(LIST,at_once)))),history),
                            for_firstDFS,
//                            on_stack(make_current_state(_MODEL,cont,assertion,ch,env),for_firstDFS),
                            for_secondDFS,
                            n_bound_ch(*makelet(LIST,cons(label,makenull(NIL))),
                                       *makelet(LIST,at_once),
                                        ch),
                            depth_counter));
         }
         break;
      default:
         return((list_t)error(FATAL|EEL,"invalid primitive agent expression(firstDFS1608) (%s)\n", rate));
   }
}
static list_t firstDFS(list_t exp,
                       list_t assertion,
                       list_t env,
                       list_t cont,
                       list_t history,
                       queue_t for_firstDFS,
                       queue_t for_secondDFS,
                       queue_t ch,
                       int depth_counter)
{
   list_t marked_states=makenull(NIL);
#  ifdef DEBUG_VERIFIER1
   printf("firstDFS->");
   fflush(stdout);
#  endif
   if (trace_on){
      printf("\n=== trace on (firstDFS)==================\n");
      n_print(resume(exp,cont)); 
      fprintf(stdout,"depth: %d\n",depth_counter);
      n_print(assertion); 
      prtlst(ch);
//      printf("-------------------(firstDFS)-\n");
//      prtlst(for_firstDFS);
      printf("================== (channel) =====\n");
      fflush(stdout);
   }
   if (!istrans(resume(exp,cont),env,ch)){
      return(firstDFS2(resume(exp,cont),
                       assertion,
                       env,
                       makenull(NIL),
                       history,
                       for_firstDFS,
                       for_secondDFS,
                       g_step_exec=TRUE,
                       ch,
                       depth_counter));
   }
   switch(getop(car(exp))){
/*   (RECV label    val-var-ls rand)          */
/*   (SEND label    val-exp-ls rand)          */
      case RECV:
      case SEND:
         firstDFS1(car(exp),car(cdr(exp)),getls(car(cdr(cdr(exp)))),getls(car(cdr(cdr(cdr(exp))))),assertion,env,cont,history,for_firstDFS,for_secondDFS,ch,exp,depth_counter);
         break;
/*   (REC  rand     env        cont)          */
      case REC:
#        ifdef DEBUG_VERIFIER
         printf("rec->");
         fflush(stdout);
#        endif
         firstDFS(getls(car(cdr(exp))),assertion,cons(*makelet(LIST,getls(car(cdr(cdr(exp))))),env),cont,history,for_firstDFS,for_secondDFS,ch,depth_counter);
         break;
/*   (SUM  rand     rand           )          */
      case SUM:
#        ifdef DEBUG_VERIFIER
         printf("sum->");
         fflush(stdout);
#        endif
         gc(&memory_control_table);
         firstDFS(getls(car(cdr(exp))),assertion,env,cont,history,for_firstDFS,for_secondDFS,ch,depth_counter);
         if (trace_on)
            printf("-- back track ----------\n");
         firstDFS(getls(car(cdr(cdr(exp)))),assertion,env,cont,history,for_firstDFS,for_secondDFS,ch,depth_counter);
         break;
/*   (COM  rand     rand           )          */
      case COM:
#        ifdef DEBUG_VERIFIER
         printf("com->");
         fflush(stdout);
#        endif
         gc(&memory_control_table);
         firstDFS(getls(car(cdr(exp))),assertion,env,make_cont(LEFT,exp,cont),history,for_firstDFS,for_secondDFS,ch,depth_counter);
         if (trace_on)
            printf("-- back track ----------\n");
         firstDFS(getls(car(cdr(cdr(exp)))),assertion,env,make_cont(RIGHT,exp,cont),history,for_firstDFS,for_secondDFS,ch,depth_counter);
         break;
/*   (IF   bool-exp rand rand      )          */
      case IF:
#        ifdef DEBUG_VERIFIER
         printf("if->");
         fflush(stdout);
#        endif
         if (getval(car(evalval(car(cdr(exp)),env,ch)))) /* B043 */
            firstDFS(getls(car(cdr(cdr(exp)))), assertion, env, cont, history, for_firstDFS, for_secondDFS, ch,depth_counter);
         else
            firstDFS(getls(car(cdr(cdr(cdr(exp))))), assertion, env, cont, history, for_firstDFS, for_secondDFS, ch,depth_counter);
         break;
/*   (CON  a-cons   val-exp-ls     )          */
      case CON:
#        ifdef DEBUG_VERIFIER
         printf("con->");
         fflush(stdout);
#        endif
         if (isabort(exp)){
            return(n_abort(history));
         }
         else{
            if (eqstop(exp))
               return(history);
            else{
               if (isprimagnt(car(cdr(exp)))){
                  if (isempty(getls(car(cdr(cdr(exp))))))
                     primagnteval(car(cdr(exp)),getls(car(cdr(cdr(exp)))),env,ch);
                  else
                     primagnteval(car(cdr(exp)),getls(car(evalval_ls(getls(car(cdr(cdr(exp)))),env,ch))),env,ch);
               }
               else{
                  if (isempty(getls(car(cdr(cdr(exp))))))
                     firstDFS(getls(car(cdr(lookup(car(cdr(exp)),env,ch)))), assertion, env, cont, history, for_firstDFS, for_secondDFS, ch,depth_counter);
                  else{
                     list_t args = evalval_ls(getls(car(cdr(cdr(exp)))),env,ch); /* B043 */
                     firstDFS(getls(car(cdr(lookup(car(cdr(exp)),env,ch)))), assertion, n_boundls(getls(car(lookup(car(cdr(exp)),env,ch))), args, env), cont, history, for_firstDFS, for_secondDFS, ch,depth_counter);
                  }
               }
            }
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
/*   (BIND label    port           )          */
      case BIND:{
         n_bind(car(cdr(exp)),car(cdr(cdr(exp))));
         if (!isempty(cont))
            firstDFS(resume(cons(*makelet(TOKEN,makesym(AGENT_OP,CON)),
                                  dotpair(*makelet(TOKEN,addattr(regsym("stop",ID),A_CON)),
                                          *makelet(LIST,makenull(NIL)))),
                            cont),
                     assertion,
                     env,
                     makenull(NIL),
                     history,
                     for_firstDFS,
                     for_secondDFS,
                     ch,
                     depth_counter);
         else
            return(makenull(NIL));
         break;
      }
      default:
         return((list_t)error(FATAL|ELS,"invalid primitive agent expression(firstDFS1597) (%s)\n", exp));
   }
   return(marked_states); /** Not reach a final state **/
}
list_t verifier(list_t exp,
                list_t top_env,
                list_t cont,
                list_t assertion,
                queue_t ch)
{
#  ifdef DEBUG_VERIFIER
   printf("verifier->");
   fflush(stdout);
#  endif
   if ((exp==(list_t)NIL)||
       (top_env==(list_t)NIL)||
       (ch==(queue_t)NIL)||
       (cont==(list_t)NIL))
      return((list_t)error(FATAL,"Segmentation fault(verifier832).\n"));
   else{
      g_emptyness=FALSE;
//      firstDFS(exp,assertion,top_env,cont,makenull(NIL),on_stack(make_current_state(exp,cont,assertion,ch,top_env),makenull(NIL)),make_que(),ch,0);
      firstDFS(exp,assertion,top_env,cont,makenull(NIL),makenull(NIL),make_que(),ch,0);
      if (g_emptyness==TRUE)
         return(makenull(NIL));
      else
         return((list_t)error(SUCCESS,"Emptyness: FALSE\n"));
   }
}
