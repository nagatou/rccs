/*
 * $Id: list-improp.c,v 1.1.1.1 2011/05/18 06:25:24 nagatou Exp $
 */
//static char rcsid[]="@(#)$Id";
//static char copyright[]="Copyright (C) 1998 Naoyuki Nagatou";

/***********************************************
  this file includes functions for list(ADT)
              Copyright (C) 1998 Naoyuki Nagatou
 ***********************************************/
#define EXTERN extern
#define EPS
#include "comm.h"

enum el_t {EPSILON,TOKEN,GC};
typedef struct {
   enum el_t t;
   union {
      char epsilon[8];
      token tk;
      int i;
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

/*****************************************************
 *  construct element.                               *
 *---------------------------------------------------*
 *  ret=makelet(type,ety);                           *
 *                                                   *
 *  elementp ret --- the returned value:succ non zero*
 *                                     :fail ERROR   *
 *  int type --- element type code(show "list.h")    *
 *  void *ety -- pointer to entry                    *
 *****************************************************/
static elementp makelet(entry_t el_type,...)
{
   va_list argp;
   elementp new_elp=(elementp)NIL;

#  ifdef DEBUG_LIST
   printf("makelement-> ");
#  endif
   if (memory_control_table.el_area.number >= ElementCapacity)
      memory_control_table.el_area.number = 0;
   new_elp = memory_control_table.el_area.area
              +memory_control_table.el_area.number;
   zeroclear(sizeof(element_t),new_elp);
   ++memory_control_table.el_area.number;
   va_start(argp,el_type);
   new_elp->type = el_type;
   switch(el_type){
      case EPSILON:
         return((elementp)error(FATAL,"please contact me(makelet177)\n"));
         break;
      case TOKEN:{
         token tk;

         tk = va_arg(argp,token);
         new_elp->entry.tk = tk;
         new_elp->type = el_type;
         break;
      }
      case LIST:{
         list_t lsp;

         lsp = va_arg(argp,list_t);
         new_elp->entry.list = lsp;
         new_elp->type = el_type;
         break;
      }
      case GC:{
         list_t lsp;

         lsp = va_arg(argp,list_t);
         new_elp->entry.gc.vacant = lsp;
         new_elp->type = el_type;
         break;
      }
      case DUMY:
      default:
         return((elementp)error(FATAL,"invalid element type(makelet205)\n"));
   }
   va_end(argp);
   return(new_elp);
}

/**************************************************
 *  Foundamental functions on list-ADT.           *
 *------------------------------------------------*
 *  sexpr_t ret = car(list);                      *
 *  sexpr_t ret = cdr(list);                      *
 *  list_t ret = cons(sexpr_t a, sexpr_t b);      *
 *                                                *
 **************************************************/
#define car(s) (s).cp->head
#define cdr(s) (s).cp->tail
//#define from_tk(z) ({sexpr_t tmp; tmp.e.entry.i=z; tmp;})
//#define to_i(s) (s).e.entry.i

static list_t vacant_cell(void)
{
   list_t cell=(list_t)NIL;

#  ifdef DEBUG_GC
   printf("vacant_cell->");
#  endif
   if (isempty(memory_control_table.ls_area.ls_reg[FREE_LIST]))
      gc(&memory_control_table);
   if (isempty(memory_control_table.ls_area.ls_reg[FREE_LIST]))
      return((cell_t *)error(FATAL,"Memory Full(vacant_cell240)\n"));
   cell = car(memory_control_table.ls_area.ls_reg[FREE_LIST]).entry.gc.vacant;
   if (cell->element.type != GC)
      return((cell_t *)error(FATAL|ELS,"Invalid element type(vacant_cell243) %s\n", cell));
   memory_control_table.ls_area.ls_reg[FREE_LIST]
      =cdr(memory_control_table.ls_area.ls_reg[FREE_LIST]);
   zeroclear(sizeof(cell_t),cell);
   cell->for_gc = WHITE;
   return(cell);
}
static list_t makecell(elementp elp)
{
   cell_t *new_cell=(cell_t *)NIL;

#  ifdef DEBUG_GC
   printf("makecell->");
#  endif
   if (elp == NIL)
      return((list_t)error(FATAL,"segmentation falt(makecell258)\n"));
   new_cell=vacant_cell();
   switch(elp->type){
      case EPSILON:
         new_cell = makenull(NIL);
         break;
      case TOKEN:{
            new_cell->element.entry.tk = elp->entry.tk;
            new_cell->element.type = elp->type;
         }
         break;
//      case LIST:{
//            new_cell->element.entry.list = elp->entry.list;
//            new_cell->element.type = elp->type;
//         }
//         break;
      case GC:{
            new_cell->element.entry.gc.vacant = elp->entry.gc.vacant;
            new_cell->element.type = elp->type;
         }
         break;
      case DUMY:
      default:
         return((list_t)error(FATAL,"invalid element type(makecell279)\n"));
   }
   return(new_cell);
}
sexpr_t cons(sexpr_t a, sexpr_t b){
#  ifdef DEBUG_LIST
   printf("cons-> ");
#  endif
   if ((cdr_part==(list_t)NIL)||(&car_part==(elementp)NIL))
      return((list_t)error(FATAL,"segmentation falt(cons309)\n"));
   else{
      new_cell = makecell();
      new_cell->next = cdr_part;
      new_cell->for_gc = WHITE;
      return(new_cell); /* for GC */
   }
   car(new_cell)=a;
   cdr(new_cell)=b;
   return(new_cell);
}

/**************************************************
 *  if it is the empty list, this function return *
 * FALSE.                                         *
 *------------------------------------------------*
 *  ret = isempty(list);                          *
 *                                                *
 *  bool ret --- if list is empty then return to  *
 *              TRUE, else return to FALSE        *
 *  list_t list - pointer of header for list.      *
 **************************************************/
bool isempty(list_t ls)
{
#  ifdef DEBUG_LIST
   printf("isempty-> ");
#  endif
   if (ls==(list_t)NIL)
      return((bool)error(FATAL,"segmentation falt(isempty27)\n"));
   if (ls==&epsilon)
      return((bool)TRUE);
   else
      return((bool)FALSE);
} 
/****************************************************
 *  create the empty list.                          *
 *  if the argment is not a empty list, it clear all*
 *  cell area of the list.                          *
 *--------------------------------------------------*
 *  ret=makenull(list);                             *
 *                                                  *
 *  list_t ret - succ:the empty list                 *
 *              fail:ERROR                          *
 *  list_t list - any list                           * 
 ****************************************************/
list_t freecell(list_t cell)
{
#  ifdef DEBUG_LIST
   printf("freecell-> ");
#  endif
   if (cell == NIL)
      error(FATAL|ELS,"segmentation falt(freecell189) %s\n", cell);
   if (isempty(cell))
      return(cell);
   else{
      zeroclear(sizeof(cell_t),cell);
      cell->for_gc=WHITE;
      cell->ref=(list_t)NIL;
      cell->element.type=GC;
      return(cell);
   }
}
list_t makenull(list_t ls)
{
#  ifdef DEBUG_LIST
   printf("makenull-> ");
#  endif
   if (ls == NIL)
      return (&epsilon);
   if (isempty(ls))
      return (&epsilon);
   else{
      list_t rest=cdr(ls);
      freecell(ls);
      return(makenull(rest));
   }
}

// a skelton to test
//int main(){
//   printf("%d\n",to_i(car(car(cons(cons(from_i(0),from_i(1)),from_i(2))))));
//   return(printf("%d\n",to_i(car(cons(from_i(1),from_i(2))))));
//}
