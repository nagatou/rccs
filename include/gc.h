/*
 * $Id: gc.h,v 1.24 2011/07/20 23:57:41 nagatou Exp $
 */

/***************************************************
 this file is header files for GC
                 Copyright (C) 2002 Naoyuki Nagatou
 ***************************************************/
#ifndef RCCS_GC_H
#  define RCCS_GC_H
#  ifndef GC_DEC

#     define BaseCapacity 24

/***************************************************
 * class name: MBCT
 *                                                *
 * WARRNING:                                           *
 *  You must use a generatted element quickly at
 * a function cons() that we will provide.
 *-------------------------------------------------*
 *-------------------------------------------------*
 * methods:
 *   makelet();
 *   makecell();
 *   init_MBCT();
 ***************************************************/
#     define Cell_LIMIT 40000
#     define CellCapacity BaseCapacity*Cell_LIMIT
#     define ElementCapacity BaseCapacity
#     define NUMBEROFREG 4
      typedef enum {
         FREE_LIST=0,/* Use in only GC */
         STACK, /* in only GC, in order to modify a value of register in mutators. Don't use in usual functions.*/
         CONS, /* for only cons() */
         AX, /* for only cons() */
      } register_num_t;
      typedef struct{
         struct{
            uintptr_t stack_bottom;
            uintptr_t stack_top;
            uintptr_t stack_limit;
         } stack_seg;
         struct{
            int number;
            element_t *area;
            element_t *area_top;
         }el_area;
         struct{
            list_t area;
            list_t area_top;
            list_t ls_reg[NUMBEROFREG];
            int gc_counter;
         }ls_area;
      } MBCT;
      EXTERN MBCT memory_control_table;

#     define GC_DEC
#  endif

/*** function declaretions ***/
   extern void zeroclear(size_t ,void *);
   extern void init_MBCT(MBCT *);
   extern void *create_heap_area(size_t );
   extern void gc(MBCT *);
   extern list_t maintain_reg(register_num_t number,list_t val);
   extern void prtreg(void);
#endif
