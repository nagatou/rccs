/*
 * $Id: list-improp.h,v 1.2 2011/07/09 15:11:08 nagatou Exp $
 */

/***************************************************
 this file is header files for list ADT
                   Copyright (C) 2009 Naoyuki Nagatou
 ***************************************************/
#ifndef RCCS_LIST_H
#  define RCCS_LIST_H
#  ifndef LIST_DEC
#    define LIST_DEC

/***************************************************
 * class name:env_ls                               *
 *-------------------------------------------------*
 *-------------------------------------------------*
 * methods:setjmp                                  *
 *        :longjmp                                 *
 ***************************************************/
EXTERN jmp_buf env_ls;

/***************************************************
 * class name:list                                 *
 *-------------------------------------------------*
 *-------------------------------------------------*
 * methods: cons                                   *
 *        : car                                    *
 *        : cdr                                    *
 *        : append                                 *
 *        : locate                                 *
 *        : dotpair                                *
 *        : makenull                               *
 *        : makelet                                *
 *        : makecell                               *
 *        : freelet                                *
 *        : freecell                               *
 ***************************************************/

typedef enum {EPSILON,TOKEN,GC} el_t;
typedef struct {
   el_t t;
   union {
      char epsilon[8];
      token tk;
      int iconst;
      char strconst[SPL_LEN];
      struct {
         list_t vacant;
      } gc;
   } entry;
} element_t;
typedef element_t *elementp;

typedef union se{
   struct cell *cp;
   element_t e;
} sexpr_t;

typedef struct cell {
   union se head;
   union se tail;
} cell_t;

typedef cell_t *list_t;

#    ifndef EPS
        extern cell_t epsilon;
#    else
        element_t epsilon ={
           EPSILON,
           {"epsilon"},
        };
#    endif
#  endif
#endif
