/*
 * $Id: channel.c,v 1.7 2011/07/14 07:24:25 nagatou Exp $
 */
/*************************************************************
   The following functions are a set of functions to a queue.
 The queue is used to implement channels as actions in CCS.
 You should notice that all has side-effects.
                 Copyright (C) 2007 Naoyuki Nagatou
 *************************************************************/
#define EXTERN
#define CHANNEL
#include "comm.h"
#include "channel.h"
#include "driver.h"

/*******************
*  generates a header of a queue.
*-------------------
*  ret = isempty_que(void);
*
*  list_t ret -- pointer to a queue header.
***********************************/
bool isempty_que(queue_t que)
{
#  ifdef DEBUG_CHANNEL
   printf("isempty_que->");
#  endif
   return(isempty(getls(car(que))));
}
/*******************
*  generates a header of a queue.
*-------------------
*  ret = make_ch(void);
*
*  list_t ret -- pointer to a queue header.
***********************************/
queue_t make_que(void)
{
#  ifdef DEBUG_CHANNEL
   printf("make_que->");
#  endif
   return(cons(*makelet(LIST,makenull(NIL)),makenull(NIL)));
}
list_t make_ch(void)
{
#  ifdef DEBUG_CHANNEL
   printf("make_ch->");
#  endif
   switch(channel_order){
      case C_QUEUE:
         return(make_que());
      case C_STACK:
         return(makenull(NIL));
      default:
         return((list_t)error(FATAL,"cannot genelate a channel buffer(make_cht54)\n"));
   }
}
/*******************
*  insert an element to the queue.
* this function has side-effect.
*-------------------
*  ret = enque(que,el);
*
*  list_t ret -- pointer to the queue header.
***********************************/
static list_t set_cdr(list_t ls,list_t cdr_part)
{
#  ifdef DEBUG_CHANNEL
   printf("set_cdr->");
#  endif
   if ((cdr_part==(list_t)NIL)||(ls == (list_t)NIL))
      return((list_t)error(FATAL,"Segmentation falt(set_cdr51)\n"));
   ls->next=cdr_part;
   return(maintain_reg(CONS,ls));
}
static list_t set_car(list_t ls, element_t el)
{
#  ifdef DEBUG_CHANNEL
   printf("set_car->");
#  endif
   if (ls == (list_t)NIL)
      return((list_t)error(FATAL,"Segmentation falt(set_car57)\n"));
   switch(el.type){
      case EPSILON:
         return((list_t)error(FATAL,"Cannot set into ()(set_car58)\n"));
      case TOKEN:{
         elementp car_part = &(ls->element);
         car_part->type = el.type;
         car_part->entry.tk = el.entry.tk;
         break;
      }
      case LIST:{
         elementp car_part = &(ls->element);
         car_part->type = el.type;
         car_part->entry.list = maintain_reg(AX,getls(el));
         break;
      }
      case GC:
      case DUMY:
      default:
         return((list_t)error(FATAL,"invalid element type(set_car72)\n"));
   }
   return(ls);
}
static queue_t enque1(list_t buf,list_t qel)
{
#  ifdef DEBUG_CHANNEL
   printf("enque1->");
#  endif
   if ((buf==(queue_t)NIL)||(qel==(list_t)NIL))
      return((list_t)error(FATAL,"Segmentation falt(enque1:96)\n"));
   if (isempty(buf))
      return((list_t)error(FATAL,"Q-buffer is empty(enque1:98)\n"));
   else{
      if (isempty(cdr(buf)))
         return(set_cdr(buf,qel));
      else
         return(enque1(cdr(buf),qel));
   }
}
static queue_t enque(queue_t que,element_t el)
{
   list_t ls=cons(el,makenull(NIL));

#  ifdef DEBUG_CHANNEL
   printf("enque->");
#  endif
   if (que==(list_t)NIL)
      return((list_t)error(FATAL,"Segmentation falt(enque79)\n"));
   if (isempty_que(que)){
      set_car(que,*makelet(LIST,ls));
      set_cdr(que,ls);
   }
   else{
      enque1(getls(car(que)),ls);
      set_cdr(que,ls);
   }
   return(que);
}
queue_t q_append(queue_t que,element_t el)
{
#  ifdef DEBUG_CHANNEL
   printf("q_append->");
#  endif
   if (que==(queue_t)NIL)
      return((queue_t)error(FATAL,"Segmentation falt(q_append115)\n"));
   else
      return(enque(que,el));
}
/*******************
*  gets and deletes the front element within the queue.
* this has a side-effect.
*-------------------
*  ret = deque(que);
*
*  queue_t que -- pointer to a queue header.
*  element_t ret -- the top element of the queue.
***********************************/
static list_t deque(queue_t que)
{
#  ifdef DEBUG_CHANNEL
   printf("deque->");
#  endif
   if (que==(queue_t)NIL)
      return((list_t)error(FATAL,"Segmentation falt(deque107)\n"));
   if (isempty(que))
      return((list_t)error(FATAL,"Invalid an empty list(deque109)\n"));
   if (isempty_que(que))
      return(que);
   {
//      list_t tmp = getls(car(getls(car(que))));
      set_car(que,*makelet(LIST,cdr(getls(car(que)))));
      if (isempty_que(que))
         set_cdr(que,makenull(NIL));
      return(que);
   }
}
/*******************
*  gets a front element in a queue
*-------------------
*  ret = q_front();
*
*  list_t ret -- pointer to a queue.
***********************************/
static element_t q_front(queue_t que)
{
#  ifdef DEBUG_CHANNEL
   printf("q_front->");
#  endif
   if (que==(list_t)NIL)
      error(FATAL,"Segmentation falt(q_front132)\n");
   if (isempty_que(que)){
      error(FATAL,"Try to get a front for an empty queue(q_front161)\n");
      exit(FATAL);
   }
   else
      return(car(getls(car(que))));
}
/*******************
*  gets the first element corresponding to el in a queue.
*-------------------
*  ret = q_first(que,el);
*
*  list_t que -- pointer to the queue.
*  element_t el -- an element you want to remove from the queue.
*  list_t ret -- pointer to a new queue which is removed the element.
***********************************/
static list_t q_first1(list_t buf, element_t el)
{
#  ifdef DEBUG_CHANNEL
   printf("q_first1->");
#  endif
   if ((buf==(list_t)NIL))
      error(FATAL|EEL,"Segmentation falt(q_first1:182)\n",el);
   if (isempty(buf))
      return(makenull(NIL));
   if (eqel(car(getls(car(buf))),el))
      return(getls(car(buf)));
   else
      return(q_first1(cdr(buf),el));
}
list_t q_first(queue_t que, element_t el)
{
#  ifdef DEBUG_CHANNEL
   printf("q_first->");
#  endif
   if (que==(queue_t)NIL)
      error(FATAL|EEL,"Segmentation falt(q_first:196:%s)\n",el);
   if (isempty(que))
      error(FATAL|EEL,"Segmentation falt(q_first:198:%s)\n",el);
   if (isempty_que(que))
      error(WARNING|EEL,"A queue is empty(q_first:200:%s)\n",el);
   if (eqel(car(getls(q_front(que))),el))
      return(getls(q_front(que)));
   else
      return(q_first1(cdr(getls(car(que))),el));
}
/*******************
*  gets and remove the first element in a queue.
* NOTICE: this causes side-effect.
*-------------------
*  ret = q_remove(que,el);
*
*  list_t que -- pointer to the queue.
*  element_t el -- an element you want to remove from the queue.
*  list_t ret -- pointer to a new queue which is removed the element.
***********************************/
static list_t q_remove1(queue_t que,list_t buf, element_t el, list_t previous)
{
#  ifdef DEBUG_CHANNEL
   printf("q_remove1->");
#  endif
   if ((que==(queue_t)NIL)||(buf==(list_t)NIL)||(previous==(list_t)NIL))
      error(FATAL|EEL,"Segmentation falt(q_remove1:224)\n",el);
   if (isempty_que(que))
      error(FATAL|EEL,"Queue is empty(q_remove1:226)\n",el);
   if (isempty(buf))
      return(que);
   else{
      if (eqel(car(buf),el)){
         if (isempty(cdr(buf))){
            set_cdr(previous,makenull(NIL));
            set_cdr(que,previous);
            return(que);
         }
         else{
            set_cdr(previous,cdr(buf));
            return(que);
         }
      }
      else
         return(q_remove1(que,cdr(buf),el,cdr(previous)));
   }
}
list_t q_remove(queue_t que, element_t el)
{
#  ifdef DEBUG_CHANNEL
   printf("q_remove->");
#  endif
   if (que==(queue_t)NIL)
      error(FATAL|EEL,"Segmentation falt(q_remove:242:%s)\n",el);
   if (isempty(que))
      error(FATAL|EEL,"Segmentation falt(q_remove:244:%s)\n",el);
   if (isempty_que(que))
      return(que);
   else{
      if (eqel(q_front(que),el))
         return(deque(que));
      else
         return(q_remove1(que,cdr(getls(car(que))),el,getls(car(que))));
   }
}
