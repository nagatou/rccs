/*
 * $Id: comm.h,v 1.10 2015/02/16 07:07:31 nagatou Exp $
 */

/***************************************************
   this file is common header files for R-CCS
                   Copyright (C) 1998 Naoyuki Nagatou
 ***************************************************/
#ifndef RCCS_COMM_H
#define RCCS_COMM_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <setjmp.h>
#include <ctype.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <arpa/inet.h>

#define NIL NULL
//enum boolean {FALSE=NIL,TRUE=!NIL};
//enum boolean {FALSE=NIL,TRUE=!NIL,OTHERWISE=2,ERROR=NIL,OFF=NIL,ON=!NIL};
//typedef enum boolean bool;
typedef int bool;
   #ifndef FALSE
      #define FALSE (int)NIL
   #endif
   #ifndef TRUE
      #define TRUE !FALSE
   #endif
   #ifndef ERROR
      #define ERROR FALSE
   #endif
   #ifndef OTHERWISE
      #define OTHERWISE 2
   #endif
   #ifndef ON_OFF
      #define OFF FALSE
      #define ON TRUE
      #define ON_OFF
   #endif
/*** the following is not modify,
    because the function "atof()" return double.  ***/
typedef double FTYPE;
#define FLT_PREC 15

#define CR '\r'  /*0Dh*/
#define LF '\n'  /*0Ah*/

/*****************************************************
 * class name:Buffer                                 *
 *---------------------------------------------------*
 * attribute:                                        *
 *---------------------------------------------------*
 * methods:                                          *
 *    initbuf();                                     *
 *****************************************************/
   #ifdef DEBUG
      #define BUF_SIZE 16
   #else
      #define BUF_SIZE 2048
   #endif
typedef struct{
   int cnt;
   char buf[BUF_SIZE];
} buffer;

/*** header files for R-CCS ***/
#include "shand.h"
#include "scan.h"
#include "list.h"
#include "parse.h"
#include "ehand.h"
#include "error.h"
#include "print.h"
#include "gc.h"
#include "channel.h"

/* header file for ltl */
#ifndef DEBUG_MC_M
#  include "ltl.h"
#endif

/*header file for mc_interface*/
#ifndef DEBUG_MC_GDB
#include "mc_interface.h"
#endif

/*** global variables for R-CCS ***/
EXTERN FILE *source_file;
EXTERN FILE *bkup;
EXTERN FILE *log_file;
EXTERN int trace_on;
EXTERN int interactive_mode;
EXTERN buffer formula;
EXTERN buffer target;
typedef struct{
   int ptr;
   buffer buf;
} seed_t;
EXTERN seed_t seed;
typedef enum{C_QUEUE,C_STACK} channel_order_t;
EXTERN channel_order_t channel_order;
typedef enum{ACC_WEAKLY,ACC_STRONGLY} acceptance_condition_t;
EXTERN acceptance_condition_t acceptance_condition;
EXTERN bool dotfile_condition;

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
/*--- symbol handler ---*/
extern bool isletters(char);
extern bool isdigits(char);
extern bool n_iswspace(char);
extern bool iskeyword(char *);
extern int hash(splp);
extern token makesym(token_t,...);
extern token regsym(splp,token_t,...);
extern void initspltbl(spell_table *);
extern void init_sym_tbl(SYMTBL *);
extern void prtsym(int ,token ,...);
extern bool isin(char *[],char *,size_t);
/*--- scanner ---*/
extern token scanner(token *);/***/
/*--- gc ---*/
extern void zeroclear(size_t ,void *);
extern void init_MBCT(MBCT *);
extern void *create_heap_area(size_t );
extern void gc(MBCT *);
extern list_t maintain_reg(register_num_t number,list_t val);
extern void prtreg(void);
/*--- list ---*/
extern list_t freecell(list_t );
extern bool isempty(list_t);
extern void prtelt(const element_t );
extern elementp makelet(enum entry_t el_type,...);
extern list_t cons(element_t ,list_t);
extern void prtlst(const list_t ls);
extern bool eqls(list_t,list_t);
extern element_t car(list_t);
extern list_t makenull(list_t);
extern list_t cdr(list_t);
extern bool eqel(element_t ea,element_t eb);
extern elementp member(list_t,element_t);
extern bool ismember(list_t,element_t);
extern list_t append(list_t,list_t);
extern list_t dotpair(element_t,element_t);
extern list_t delete(element_t, list_t);
/*--- parser ---*/
extern list_t parser(list_t *,char *);
extern splp cvtia(int);
extern token addattr(token tk,...);
/*--- driver ---*/
extern void driver(void);
extern int getval(element_t);
extern token gettk(element_t);
extern list_t getls(element_t);
/*--- primitive action ---*/
extern void epilogue(void);
extern void prologue(char *);
/*--- event handler ---*/
extern filename_lineno_pair_t *retrieve_filename_lineno_pair(element_t label);
extern char *n_tolower(char *lah,buffer *buf);
extern list_t n_bind(element_t,element_t);
extern list_t n_send(element_t,list_t);
extern list_t n_recv(element_t);
/*--- print ---*/
extern int n_print(list_t);
/*--- channel ---*/
extern bool isempty_que(queue_t);
extern list_t make_ch(void);
extern queue_t make_que(void);
extern queue_t q_append(queue_t, element_t);
extern list_t q_remove(queue_t, element_t);
extern list_t q_first(queue_t que, element_t el);

#endif
