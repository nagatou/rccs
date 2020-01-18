/*
 * $Id: channel.h,v 1.1.1.1 2011/05/18 06:25:25 nagatou Exp $
 */

/***************************************************
 this file is header files for QUEUE
                   Copyright (C) 2007 Naoyuki Nagatou
 ***************************************************/
#ifndef RCCS_QUEUE_H
   #define RCCS_QUEUE_H
   typedef list_t queue_t;

   /*** function decroletar ***/
   extern bool isempty_que(queue_t);
   extern list_t make_ch(void);
   extern queue_t make_que(void);
   extern queue_t q_append(queue_t, element_t);
   extern list_t q_remove(queue_t, element_t);
   extern list_t q_first(queue_t que, element_t el);
#endif
