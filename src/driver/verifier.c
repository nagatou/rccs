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
static list_t n_abort(list_t history) /*** does not use ***/
{
#  ifdef DEBUG_EVAL
   printf("n_abort->");
#  endif
   if (history==(list_t)NIL)
      error(FATAL,"Segmentation fault(n_abort370).\n");
   else{
      if (isempty_buf(&formula))
         error(FATAL,"I detect one sequence aginst the policy.\n");
      else
         error(WARNING|ELS,"I detect one sequence aginst the property:\n %s",history);
   }
   return((list_t)NIL);
}
static list_t make_current_state(list_t exp,list_t cont,list_t assertion, list_t ch,list_t env)
{
   if (exp==(list_t)NIL||cont==(list_t)NIL||assertion==(list_t)NIL||ch==(list_t)NIL){
      error(FATAL,"segmentation fault(make_current_state36)\n");
      return((list_t)NIL);
   }
   else
      return(cons(*makelet(LIST,resume(exp,cont)),
                   cons(*makelet(LIST,assertion),
                        cons(*makelet(LIST,ch),makenull(NIL)))));
}
static list_t mark(list_t state,list_t stack,bool side_effect)
{
#  ifdef DEBUG_VERIFIER1
   printf("mark->");
#  endif
   if ((state==(list_t)NIL)||(stack==(list_t)NIL)){
      error(FATAL,"Segmentation fault(mark33).\n");
      return((list_t)NIL);
   }
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
   if ((state==(list_t)NIL)||(stack==(list_t)NIL)){
      error(FATAL,"Segmentation fault(on_stack49).\n");
      return((list_t)NIL);
   }
   else
      return(mark(state,stack,(bool)TRUE));
}
static bool ismember_on_queue(queue_t queue,list_t state)
{
#  ifdef DEBUG_VERIFIER1
   printf("ismember_on_queue->");
#  endif
   if (ismember(getls(car(queue)),*makelet(LIST,state)))
      return((bool)TRUE);
   else
      return((bool)FALSE);
}
static bool ismember_on_stack(list_t stack,list_t state)
{
#  ifdef DEBUG_VERIFIER1
   printf("ismember_on_stack->");
#  endif
   if (ismember(stack,*makelet(LIST,state)))
      return((bool)TRUE);
   else
      return((bool)FALSE);
}
static bool ismember_of_stack(list_t stack,list_t state)
{
#  ifdef DEBUG_VERIFIER1
   printf("ismember_of_stack->");
#  endif
   return(ismember_on_queue(stack,state));
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

static list_t secondDFS(list_t,list_t,list_t,list_t,list_t,list_t,list_t,queue_t,list_t,int);
static list_t secondDFS2(list_t exp,
                         list_t assertion,
                         list_t env,
                         list_t cont,
                         list_t history,
                         list_t for_firstDFS,
                         list_t for_secondDFS,
                         bool sw,
                         queue_t ch,
                         list_t procedures,
                         int depth_counter)
{
#  ifdef DEBUG_VERIFIER2
   printf("secondDFS2->");
#  endif
   if (sw == FALSE){
      if (depth_counter>=DEPTH_LIMIT){
         if (acceptance_condition==ACC_WEAKLY){
            g_emptyness=TRUE;
            error(SUCCESS|EEL,"Emptyness(truncation;2ndDFS): TRUE, %s",*makelet(LIST,history)),fflush(stdout);
            if ((++g_counter_for_counterexamples) < MAX_COUNTEREXAMPLES){
               return(for_secondDFS);
            }
            else{
               g_counter_for_counterexamples=0;
               error(WARNING,"Terminate by the limit on the number of counterexample");
               return(for_secondDFS);
            }
         }
         else
            return(for_secondDFS); /*** is not accepted ***/
      }
      else{
         if (ismember_on_queue(for_secondDFS,make_current_state(exp,cont,assertion,ch,env)))
            return(for_secondDFS);
         else{
            if (isabort(assertion))
               return(for_secondDFS); /*** is not accepted ***/
            else{
               if (isaccept(assertion))
                  return(secondDFS(exp,getls(car(cdr(cdr(cdr(assertion))))),env,cont,history,for_firstDFS,for_secondDFS,ch,procedures,depth_counter));
               else
                  return(secondDFS(exp,assertion,env,cont,history,for_firstDFS,for_secondDFS,ch,procedures,depth_counter));
            }
         }
      }
   }
   else{ /*** push([m,f]) step ***/
      list_t tmp = eval(assertion,env,makenull(NIL),make_ch(),procedures);
      gc(&memory_control_table);
      if (ismember_of_stack(for_firstDFS,make_current_state(exp,cont,tmp,ch,env))){
         g_emptyness=TRUE;
         error(SUCCESS|EEL,"Emptyness: TRUE, %s",*makelet(LIST,history)),fflush(stdout);
         if ((++g_counter_for_counterexamples) < MAX_COUNTEREXAMPLES){
            return(for_secondDFS);
         }
         else{
            g_counter_for_counterexamples=0;
            error(WARNING,"Terminate by the limit on the number of counterexample");
            return(for_secondDFS);
         }
      }
      else
         return(secondDFS2(exp,tmp,env,cont,history,for_firstDFS,on_stack(make_current_state(exp,cont,tmp,ch,env),for_secondDFS),g_step_exec=FALSE,ch,procedures,depth_counter));
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
                         list_t procedures,
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
                  error(FATAL,"Invalid channel type(secondDFS1:181)\n");
            }
         }
         if (ismember_of_stack(for_secondDFS,make_current_state(_MODEL,cont,assertion,ch,env))) /* don't explore same states */
            return(for_secondDFS);
         ret=secondDFS2(resume(make_cls(body,n_boundls(val_ls,data,env)),cont),
                        assertion,
                        env,
                        makenull(NIL),
                        cons(*makelet(LIST,dotpair(rate,*makelet(LIST,at_once))),history),
                        for_firstDFS,
                        for_secondDFS,
                        g_step_exec=TRUE,
                        new_ch,
                        procedures,
                        ++depth_counter);
         if (isouter_action(label)){
            if (retrieval1(label)!=(bindLSp)NIL){
               if (!isempty_buf(&target))
                  mc_back(1);
            }
         }
         return(ret);
         break;
      }
/*   (SEND label    val-exp-ls rand)          */
      case SEND:
#        ifdef DEBUG_VERIFIER2
         printf("send->");
         fflush(stdout);
#        endif
         if (ismember_of_stack(for_secondDFS,make_current_state(_MODEL,cont,assertion,ch,env))) /* don't explore same states */
            return(for_firstDFS);
         if (isouter_action(label)){
            list_t at_once=evalval_ls(val_ls,env,ch); /* B043 */
            n_send(label,at_once);
            if (!isempty_buf(&target)){
               ret = secondDFS(resume(make_cls(body,env),cont),
                               assertion,
                               env,
                               makenull(NIL),
                               cons(*makelet(LIST,cons(rate,dotpair(label,*makelet(LIST,at_once)))),history),
                               for_firstDFS,
                               on_stack(make_current_state(_MODEL,cont,assertion,ch,env),for_secondDFS),
                               ch,
                               procedures,
                               depth_counter);
               if (retrieval1(label)!=(bindLSp)NIL)
                  mc_back(1);
               return(ret);
            }
            else
               return(secondDFS(resume(make_cls(body,env),cont),
                                assertion,
                                env,
                                makenull(NIL),
                                cons(*makelet(LIST,cons(rate,dotpair(label,*makelet(LIST,at_once)))),history),
                                for_firstDFS,
//                                on_stack(make_current_state(_MODEL,cont,assertion,ch,env),for_secondDFS),
                                for_secondDFS,
                                ch,
                                procedures,
                                depth_counter));
         }
         else{ /* is an inner-action */
            list_t at_once=evalval_ls(val_ls,env,ch);
            return(secondDFS(resume(make_cls(body,env),cont),
                             assertion,
                             env,
                             makenull(NIL),
                             cons(*makelet(LIST,cons(rate,dotpair(label,*makelet(LIST,at_once)))),history),
                             for_firstDFS,
//                             on_stack(make_current_state(_MODEL,cont,assertion,ch,env),for_secondDFS),
                             for_secondDFS,
                             n_bound_ch(*makelet(LIST,cons(label,makenull(NIL))),
                                        *makelet(LIST,at_once),
                                        ch),
                             procedures,
                             depth_counter));
         }
         break;
      default:
         error(FATAL|EEL,"invalid primitive agent expression(secondDFS263) (%s)\n", rate);
   }
   return((list_t)NIL);
}
static list_t secondDFS(list_t exp,
                       list_t assertion,
                       list_t env,
                       list_t cont,
                       list_t history,
                       list_t for_firstDFS,
                       list_t for_secondDFS,
                       queue_t ch,
                       list_t procedures,
                       int depth_counter)
{
   list_t marked_states=makenull(NIL);
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
   if (!istrans(resume(exp,cont),env,ch,procedures)) /*** When a model cannot produce a transition, then set FALSE to g_step_exec and LIMIT to depth_counter. ***/
      return(secondDFS2(resume(exp,cont),assertion,env,makenull(NIL),history,for_firstDFS,for_secondDFS,g_step_exec=FALSE,ch,procedures,DEPTH_LIMIT));
   g_state_counter += 1;
   switch(getop(car(exp))){
/*   (RECV label    val-var-ls rand)          */
/*   (SEND label    val-exp-ls rand)          */
      case RECV:
      case SEND:
         if (istrans(exp,env,ch,procedures)){
            if (ismember_of_stack(for_secondDFS,make_current_state(_MODEL,cont,assertion,ch,env)))
               return(for_secondDFS);
            else
               return(secondDFS1(car(exp),car(cdr(exp)),getls(car(cdr(cdr(exp)))),getls(car(cdr(cdr(cdr(exp))))),assertion,env,cont,history,for_firstDFS,for_secondDFS,ch,procedures,exp,depth_counter));
         }
         else
            return(for_secondDFS);
         break;
/*   (SUM  rand     rand           )          */
      case SUM:
#        ifdef DEBUG_VERIFIER
         printf("sum->");
         fflush(stdout);
#        endif
         marked_states=secondDFS(getls(car(cdr(exp))),assertion,env,cont,history,for_firstDFS,for_secondDFS,ch,procedures,depth_counter);
         if (trace_on)
            printf("-- back track ----------\n");
         gc(&memory_control_table);
         return(secondDFS(getls(car(cdr(cdr(exp)))),assertion,env,cont,history,for_firstDFS,marked_states,ch,procedures,depth_counter));
         break;
/*   (COM  rand     rand           )          */
      case COM:
#        ifdef DEBUG_VERIFIER
         printf("com->");
         fflush(stdout);
#        endif
         marked_states=secondDFS(getls(car(cdr(exp))),assertion,env,make_cont(LEFT,exp,cont,env),history,for_firstDFS,for_secondDFS,ch,procedures,depth_counter);
         if (trace_on)
            printf("-- back track ----------\n");
         gc(&memory_control_table);
         return(secondDFS(getls(car(cdr(cdr(exp)))),assertion,env,make_cont(RIGHT,exp,cont,env),history,for_firstDFS,marked_states,ch,procedures,depth_counter));
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
                            procedures,
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
                            procedures,
                            depth_counter));
         break;
/*   (CLS  rand     env        cont)   Used as a representation of closure */
      case CLS:
#        ifdef DEBUG_VERIFIER
         printf("cls->");
         fflush(stdout);
#        endif
//         if (istrans(getls(car(cdr(exp))),cdr(cdr(exp)),ch,procedures))
            return(secondDFS(getls(car(cdr(exp))),assertion,cdr(cdr(exp)),cont,history,for_firstDFS,for_secondDFS,ch,procedures,depth_counter));
//         else{
//            return(for_secondDFS);
//         }
         break;
/*   (CON  a-cons   val-exp-ls     )          */
      case CON:
#        ifdef DEBUG_VERIFIER
         printf("con->");
         fflush(stdout);
#        endif
         if (eqstop(exp))
            return(for_firstDFS);
         else{
            if (isprimagnt(car(cdr(exp)))){
               if (isempty(getls(car(cdr(cdr(exp))))))
                  return(primagnteval(car(cdr(exp)),(getls(car(cdr(cdr(exp))))),env,ch,procedures));
               else
                  return(primagnteval(car(cdr(exp)),getls(car(evalval_ls(getls(car(cdr(cdr(exp)))),env,ch))),env,ch,procedures));
            }
            else{
               return(secondDFS(resume(make_cls(getls(car(cdr(lookup_env(car(cdr(exp)),procedures)))),
                                                n_boundls(getls(car(lookup_env(car(cdr(exp)),procedures))),
                                                          evalval_ls(getls(car(cdr(cdr(exp)))),env,ch),
                                                          env)),
                                       cont),
                                assertion,
                                env,
                                makenull(NIL),
                                history,
                                for_firstDFS,
                                for_secondDFS,
                                ch,
                                procedures,
                                depth_counter));
            }
         }
         break;
/*   (REL  rand    (rel-ls)        )          */
      case REL:
         error(FATAL|ELS,"sorry. operater REL has not been implemented yet. (%s)\n", exp);
         break;
/*   (RES  rand    label-ls  co-env)          */
      case RES:
         error(FATAL|ELS,"sorry. operater RES has not been implemented yet. (%s)\n", exp);
         break;
      default:
         error(FATAL|ELS,"invalid primitive agent expression(secondDFS1597) (%s)\n", exp);
   }
   return((list_t)NIL);
}
static list_t n_accept(list_t exp,
                       list_t assertion,
                       list_t env,
                       list_t cont,
                       list_t history,
                       list_t for_firstDFS,
                       list_t for_secondDFS,
                       queue_t ch,
                       list_t procedures,
                       int depth_counter)
{
#  ifdef DEBUG_VERIFIER
   printf("n_accept->");
#  endif
   return(secondDFS(exp,getls(car(cdr(cdr(cdr(assertion))))),env,cont,history,on_stack(make_current_state(exp,cont,assertion,ch,env),for_firstDFS),makenull(NIL),ch,procedures,depth_counter));
}
static list_t firstDFS(list_t,list_t,list_t,list_t,list_t,list_t,list_t,queue_t,list_t,int);
static list_t firstDFS2(list_t exp,
                        list_t assertion,
                        list_t env,
                        list_t cont,
                        list_t history,
                        list_t for_firstDFS,
                        list_t for_secondDFS,
                        bool sw,
                        queue_t ch,
                        list_t procedures,
                        int depth_counter)
{
#  ifdef DEBUG_VERIFIER1
   printf("firstDFS2->");
#  endif
   if (sw == FALSE){
      if (depth_counter>=DEPTH_LIMIT){
         if (acceptance_condition==ACC_WEAKLY){
            g_emptyness=TRUE;
            error(SUCCESS|EEL,"Emptyness(truncation;1stDFS): TRUE, %s",*makelet(LIST,history)),fflush(stdout);
            if ((++g_counter_for_counterexamples) < MAX_COUNTEREXAMPLES){
               return(for_firstDFS);
            }
            else{
               g_counter_for_counterexamples=0;
               error(WARNING,"Terminate by the limit on the number of counterexample");
               return(for_secondDFS);
            }
         }
         else
            return(for_firstDFS); /*** is not accepted ***/
      }
      else{
         if (isabort(assertion))
            return(for_firstDFS); /*** is not accepted ***/
         else{
            if (isaccept(assertion)){
               list_t marked_states=firstDFS(exp,getls(car(cdr(cdr(cdr(assertion))))),env,cont,history,for_firstDFS,for_secondDFS,ch,procedures,depth_counter);
               secondDFS(exp,getls(car(cdr(cdr(cdr(assertion))))),env,cont,history,marked_states,on_stack(make_current_state(exp,cont,assertion,ch,env),for_secondDFS),ch,procedures, depth_counter);
               return(for_firstDFS);
            }
            else
               return(firstDFS(exp,assertion,env,cont,history,for_firstDFS,for_secondDFS,ch,procedures,depth_counter));
         }
      }
   }
   else{ /*** push([m,f]) step ***/
      list_t tmp = eval(assertion,env,makenull(NIL),make_ch(),procedures);
      gc(&memory_control_table);
      if (ismember_of_stack(for_firstDFS,make_current_state(exp,cont,tmp,ch,env)))
         return(for_firstDFS);
      else
         return(firstDFS2(exp,tmp,env,cont,history,on_stack(make_current_state(exp,cont,tmp,ch,env),for_firstDFS),for_secondDFS,g_step_exec=FALSE,ch,procedures,depth_counter));
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
                        list_t procedures,
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
                  error(FATAL,"Invalid channel type(firstDFS1:687)\n");
            }
         }
         ret=firstDFS2(resume(make_cls(body,n_boundls(val_ls,data,env)),cont),
                       assertion,
                       env,
                       makenull(NIL),
                       cons(*makelet(LIST,dotpair(rate,*makelet(LIST,at_once))),history),
                       for_firstDFS,
                       for_secondDFS,
                       g_step_exec=TRUE,
                       new_ch,
                       procedures,
                       ++depth_counter);
         if (isouter_action(label)){
            if (retrieval1(label)!=(bindLSp)NIL){
               if (!isempty_buf(&target))
                  mc_back(1);
            }
         }
         return(ret);
         break;
      }
/*   (SEND label    val-exp-ls rand)          */
      case SEND:
#        ifdef DEBUG_VERIFIER
         printf("send->");
         fflush(stdout);
#        endif
         if (isouter_action(label)){
            list_t at_once=evalval_ls(val_ls,env,ch); /* B043 */
            n_send(label,at_once);
            if (!isempty_buf(&target)){
               ret = firstDFS(resume(make_cls(body,env),cont),
                              assertion,
                              env,
                              makenull(NIL),
                              cons(*makelet(LIST,cons(rate,dotpair(label,*makelet(LIST,at_once)))),history),
                              on_stack(make_current_state(_MODEL,cont,assertion,ch,env),for_firstDFS),
                              for_secondDFS,
                              ch,
                              procedures,
                              depth_counter);
               if (retrieval1(label)!=(bindLSp)NIL)
                  mc_back(1);
               return(ret);
            }
            else
               return(firstDFS(resume(make_cls(body,env),cont),
                               assertion,
                               env,
                               makenull(NIL),
                               cons(*makelet(LIST,cons(rate,dotpair(label,*makelet(LIST,at_once)))),history),
                               on_stack(make_current_state(_MODEL,cont,assertion,ch,env),for_firstDFS),
                               for_secondDFS,
                               ch,
                               procedures,
                               depth_counter));
         }
         else{ /* is an inner-action */
            list_t at_once=evalval_ls(val_ls,env,ch);
            return(firstDFS(resume(make_cls(body,env),cont),
                            assertion,
                            env,
                            makenull(NIL),
                            cons(*makelet(LIST,cons(rate,dotpair(label,*makelet(LIST,at_once)))),history),
                            for_firstDFS,
                            for_secondDFS,
                            n_bound_ch(*makelet(LIST,cons(label,makenull(NIL))),
                                       *makelet(LIST,at_once),
                                        ch),
                            procedures,
                            depth_counter));
         }
         break;
      default:
         error(FATAL|EEL,"invalid primitive agent expression(firstDFS1608) (%s)\n", rate);
   }
   return((list_t)NIL);
}
static list_t firstDFS(list_t exp,
                       list_t assertion,
                       list_t env,
                       list_t cont,
                       list_t history,
                       list_t for_firstDFS,
                       list_t for_secondDFS,
                       queue_t ch,
                       list_t procedures,
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
   if (!istrans(resume(exp,cont),env,ch,procedures)) /*** When a model cannot produce transition, then set FALSE to g_step_exec and LIMIT to depth_counter. ***/
      return(firstDFS2(resume(exp,cont),assertion,env,makenull(NIL),history,for_firstDFS,for_secondDFS,g_step_exec=FALSE,ch,procedures,DEPTH_LIMIT));
   g_state_counter += 1;
   switch(getop(car(exp))){
/*   (RECV label    val-var-ls rand)          */
/*   (SEND label    val-exp-ls rand)          */
      case RECV:
      case SEND:
         if (istrans(exp,env,ch,procedures)){
            if (ismember_of_stack(for_firstDFS,make_current_state(_MODEL,cont,assertion,ch,env))) /* don't explore same state */
               return(for_firstDFS);
            else
               return(firstDFS1(car(exp),car(cdr(exp)),getls(car(cdr(cdr(exp)))),getls(car(cdr(cdr(cdr(exp))))),assertion,env,cont,history,for_firstDFS,for_secondDFS,ch,procedures,exp,depth_counter));
         }
         else
            return(for_firstDFS);
         break;
/*   (SUM  rand     rand           )          */
      case SUM:
#        ifdef DEBUG_VERIFIER
         printf("sum->");
         fflush(stdout);
#        endif
         gc(&memory_control_table);
         marked_states=firstDFS(getls(car(cdr(exp))),assertion,env,cont,history,for_firstDFS,for_secondDFS,ch,procedures,depth_counter);
         if (trace_on)
            printf("-- back track ----------\n");
         return(firstDFS(getls(car(cdr(cdr(exp)))),assertion,env,cont,history,marked_states,for_secondDFS,ch,procedures,depth_counter));
         break;
/*   (COM  rand     rand           )          */
      case COM:
#        ifdef DEBUG_VERIFIER
         printf("com->");
         fflush(stdout);
#        endif
         gc(&memory_control_table);
         marked_states=firstDFS(getls(car(cdr(exp))),assertion,env,make_cont(LEFT,exp,cont,env),history,for_firstDFS,for_secondDFS,ch,procedures,depth_counter);
         if (trace_on)
            printf("-- back track ----------\n");
         return(firstDFS(getls(car(cdr(cdr(exp)))),assertion,env,make_cont(RIGHT,exp,cont,env),history,marked_states,for_secondDFS,ch,procedures,depth_counter));
         break;
/*   (IF   bool-exp rand rand      )          */
      case IF:
#        ifdef DEBUG_VERIFIER
         printf("if->");
         fflush(stdout);
#        endif
         if (getval(car(evalval(car(cdr(exp)),env,ch)))) /* B043 */
            return(firstDFS(getls(car(cdr(cdr(exp)))),assertion,env,cont,history,for_firstDFS,for_secondDFS,ch,procedures,depth_counter));
         else
            return(firstDFS(getls(car(cdr(cdr(cdr(exp))))),assertion,env,cont,history,for_firstDFS,for_secondDFS,ch,procedures,depth_counter));
         break;
/*   (CLS  rand     env        cont)          */
      case CLS:
#        ifdef DEBUG_VERIFIER
         printf("cls->");
         fflush(stdout);
#        endif
//         if (istrans(getls(car(cdr(exp))),cdr(cdr(exp)),ch,procedures))
            return(firstDFS(getls(car(cdr(exp))),assertion,cdr(cdr(exp)),cont,history,for_firstDFS,for_secondDFS,ch,procedures,depth_counter));
//         else{
//            return(for_firstDFS);
//         }
         break;
/*   (CON  a-cons   val-exp-ls     )          */
      case CON:
#        ifdef DEBUG_VERIFIER
         printf("con->");
         fflush(stdout);
#        endif
         if (eqstop(exp))
            return(for_firstDFS);
         else{
            if (isprimagnt(car(cdr(exp)))){
               if (isempty(getls(car(cdr(cdr(exp))))))
                  primagnteval(car(cdr(exp)),getls(car(cdr(cdr(exp)))),env,ch,procedures);
               else
                  primagnteval(car(cdr(exp)),getls(car(evalval_ls(getls(car(cdr(cdr(exp)))),env,ch))),env,ch,procedures);
            }
            else{
               return(firstDFS(resume(make_cls(getls(car(cdr(lookup_env(car(cdr(exp)),procedures)))),
                                               n_boundls(getls(car(lookup_env(car(cdr(exp)),procedures))),
                                                         evalval_ls(getls(car(cdr(cdr(exp)))),env,ch),
                                                         env)),
                                      cont),
                               assertion,
                               env,
                               makenull(NIL),
                               history,
                               for_firstDFS,
                               for_secondDFS,
                               ch,
                               procedures,
                               depth_counter));
            }
         }
         break;
/*   (REL  rand    (rel-ls)        )          */
      case REL:
         error(FATAL|ELS,"sorry. operater REL has not been implemented yet. (%s)\n", exp);
         break;
/*   (RES  rand    label-ls  co-env)          */
      case RES:
         error(FATAL|ELS,"sorry. operater RES has not been implemented yet. (%s)\n", exp);
         break;
      default:
         error(FATAL|ELS,"invalid primitive agent expression(firstDFS1597) (%s)\n", exp);
   }
   return(for_firstDFS);
}
list_t verifier(list_t exp,
                list_t env,
                list_t cont,
                list_t assertion,
                queue_t ch,
                list_t procedures)
{
#  ifdef DEBUG_VERIFIER
   printf("verifier->");
   fflush(stdout);
#  endif
   if ((exp==(list_t)NIL)||
       (env==(list_t)NIL)||
       (ch==(queue_t)NIL)||
       (cont==(list_t)NIL)){
      error(FATAL,"Segmentation fault(verifier832).\n");
      return((list_t)NIL);
   }
   else{
      g_emptyness=FALSE;
      if (isempty(firstDFS(exp,assertion,env,cont,makenull(NIL),make_que(),make_que(),ch,procedures,0))){
         if (g_emptyness==TRUE)
            return(makenull(NIL));
         else{
            error(SUCCESS,"Emptyness: FALSE\n");
            return((list_t)NIL);
         }
      }
      else{
         if (g_emptyness==TRUE)
            return(makenull(NIL));
         else{
            error(SUCCESS,"Emptyness: FALSE\n");
            return((list_t)NIL);
         }
      }
   }
}
