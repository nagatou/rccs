/*
 * $Id: list.h,v 1.4 2011/07/14 07:24:24 nagatou Exp $
 */

/***************************************************
 this file is header files for list ADT
                   Copyright (C) 1998 Naoyuki Nagatou
 ***************************************************/
#ifndef RCCS_LIST_H
   #define RCCS_LIST_H
   #ifndef LIST_DEC
   #define LIST_DEC

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
enum entry_t {EPSILON=0,  /* type of entry */
     LIST,
     TOKEN,
     DUMY,
     GC=98,
} ;
typedef struct cell *list_t;
typedef struct{
   enum entry_t type;
   union{
      char epsilon[8];
      token tk;
      list_t list;
      struct{
         list_t vacant;
      }gc;
   }entry;
} element_t;
typedef element_t *elementp;

/*****************************************
 * class:list
 *---------------------------------------
 * attr:
 *---------------------------------------
 *metohs:
 *        car()
 *        cdr()
 *        cons()
 *        set_car()
 *        set_cdr()
 *****************************************/
typedef enum{WHITE=1000,GLAY,BLACK,NON_GC} gc_color_t; /* Don't use same number with element type. */
typedef struct cell{
   gc_color_t for_gc;
   list_t ref; /* next cell which is processed in GC */
   element_t element;
   struct cell *next;
//  struct cell *previous; /* Don't use this field else Queue */
} cell_t;

/*****************************************
 * class:epsilon                         *
 *---------------------------------------*
 * attr:                                 *
 *---------------------------------------*
 *metohs:                                *
 *****************************************/
   #ifndef EPS
      extern cell_t epsilon;
   #else
      cell_t epsilon={
         NON_GC,
         &epsilon,
         {EPSILON,{"epsilon"}},
         &epsilon,
      };
   #endif
#endif

/*** function declaretions ***/
#endif
