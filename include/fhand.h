/*
 * $Id: fhand.h,v 1.1.1.1 2011/05/18 06:25:25 nagatou Exp $
 */

/*****************************************************
  this file is functions for file handlers.
                   Copyright (C) 1998 Naoyuki Nagatou
 *****************************************************/
#ifndef RCCS_FHAND_H
#   define RCCS_FHAND_H
/*****************************************************
 * class name:Buffer                                 *
 *---------------------------------------------------*
 * attribute:                                        *
 *---------------------------------------------------*
 * methods:                                          *
 *    initbuf();                                     *
 *****************************************************/
#  define BUF_SIZE STR_LEN*2
   typedef struct{
      int cnt;
      char buf[BUF_SIZE];
   } buffer;

/*****************************************************
 * class name:queue                                  *
 *---------------------------------------------------*
 *---------------------------------------------------*
 * methods:                                          *
 *   enque();                                        *
 *   deque();                                        *
 *****************************************************/
   typedef struct{
      int top;
      int bottom;
      buffer que_buf;
   }queue;
   typedef struct{
      int ptr;
      buffer buf;
   } seed_t;
   EXTERN seed_t seed;


/*** global variable decroletar ***/
#  ifdef FHAND
   static queue que={
      0,0,
      {0,},
    };
   static buffer in_buf;
   static buffer out_buf;
#  endif

/*** function decroletar ***/
/*--- file handler ---*/
   extern void clear_in_buf(seed_t *seed);
   extern char getnch(void);
   extern void ungetnch(char);
   extern int n_getc_stdin(void);
   extern void initsf(char *,char *);
   extern void initbuf(volatile buffer *);
   extern void n_end(void);
   extern splp ins_buf(buffer *,char *);
   extern splp append_buf(buffer *,char );
   extern splp append_str_buf(buffer *,char *);
   extern bool isempty_buf(buffer *buf);
   extern void set_in_buf(buffer *buf,char *str);
#endif
