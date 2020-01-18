/*
 * $Id: list.c,v 1.14 2011/07/14 07:24:25 nagatou Exp $
 */
//static char rcsid[]="@(#)$Id: list.c,v 1.14 2011/07/14 07:24:25 nagatou Exp $";
//static char copyright[]=
//   "Copyright (C) 1998 Naoyuki Nagatou";

/***********************************************
  this file is functions for list(ADT)
              Copyright (C) 1998 Naoyuki Nagatou
 ***********************************************/
#define EXTERN extern
#define EPS
#include "comm.h"
#include "driver.h"

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

/**************************************************
 *  get the first element of list. if it is the   *
 * empty list, this function return ERROR.        *
 *------------------------------------------------*
 *  ret = car(list);                              *
 *                                                *
 *  elementp ret - succ:the first element         *
 *                 fail:ERROR                     *
 *  list_t list - pointer of header for            *
 *                       list.                    *
 **************************************************/
element_t car(list_t ls)
{
#  ifdef DEBUG_LIST
   printf("car-> ");
#  endif
   if (ls==NIL){
      error(FATAL,"segmentation fault(car56) %s\n", ls),exit(FATAL);
   }
   else{
      if (isempty(ls)){
         error(FATAL|ELS,"The argument in car is an empty list(car60) %s\n", ls),exit(FATAL);
      }
      else
         return(ls->element);
   }
}

/**************************************************
 *  get the rest of list. if it is the empty list,*
 * this function return ERROR.                    *
 *------------------------------------------------*
 *  ret = cdr(list);                              *
 *                                                *
 *  list_t ret - succ:the rest of lists            *
 *              fail:ERROR                        *
 *  list_t list - pointer of header for            *
 *                       list.                    *
 **************************************************/
static list_t cdr1(list_t ls) /** UNUSED **/
{
#  ifdef DEBUG_LIST
   printf("cdr1-> ");
#  endif
   if (ls == NIL)
      error(FATAL|ELS,"segmentation falt(cdr65) %s\n", ls);
   if (isempty(ls))
      return(ls);
   else{
      return(cons(car(ls),cdr1(ls->next)));
   }
   return((list_t)NIL);
}
list_t cdr(list_t ls)
{
#  ifdef DEBUG_LIST
   printf("cdr-> ");
#  endif
   if (ls == NIL)
      error(FATAL|ELS,"segmentation falt(cdr67) %s\n", ls);
   if (isempty(ls))
      error(FATAL|ELS,"Apply cdr to the empty list(cdr99) %s\n", ls);
   else
      return(ls->next);
   return((list_t)NIL);
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
elementp makelet(enum entry_t el_type,...)
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
         error(FATAL,"please contact me(makelet177)\n");
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
         error(FATAL,"invalid element type(makelet205) %d\n",el_type);
   }
   va_end(argp);
   return(new_elp);
}

/****************************************************
 *  list building function.                         *
 *  cons returns the list.the car part is its first *
 * argment, the cdr part is its second argment.     *
 * the first argment is any element and the second  *
 * argment is the list.                             *
 *--------------------------------------------------*
 *  ret=cons(car_part,cdr_part);                    *
 *                                                  *
 *  list_t ret - succ:the list. one longer            *
 *                      than the second argment.    *
 *             fail:ERROR. the second               *
 *                      argment is not any list.    *
 *  element car_part - any element                  *
 *  list cdr_part - any list                        *
 ****************************************************/
static cell_t *vacant_cell(void)
{
   cell_t *cell=(cell_t *)NIL;

#  ifdef DEBUG_GC
   printf("\nvacant_cell[");
//   printf("free=");
//   prtlst(memory_control_table.ls_area.ls_reg[FREE_LIST]);
   printf("]->");
#  endif
   if (isempty(memory_control_table.ls_area.ls_reg[FREE_LIST]))
      gc(&memory_control_table);
   if (isempty(memory_control_table.ls_area.ls_reg[FREE_LIST]))
      error(FATAL,"Memory Full(vacant_cell240)\n");
   cell = car(memory_control_table.ls_area.ls_reg[FREE_LIST]).entry.gc.vacant;
   if (cell->element.type != GC)
      error(FATAL|ELS,"Invalid element type(vacant_cell243) %s\n", cell);
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
      error(FATAL,"segmentation falt(makecell258)\n");
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
      case LIST:{
            new_cell->element.entry.list = elp->entry.list;
            new_cell->element.type = elp->type;
         }
         break;
      case GC:{
            new_cell->element.entry.gc.vacant = elp->entry.gc.vacant;
            new_cell->element.type = elp->type;
         }
         break;
      case DUMY:
      default:
         error(FATAL,"invalid element type(makecell279) %d\n",elp->type);
   }
   return(new_cell);
}
static list_t cons1(list_t ls) /* UNUSED */
{
   list_t new_cell=(list_t)NIL;

#  ifdef DEBUG_LIST
   printf("cons1-> ");
#  endif
   if (ls==(list_t)NIL)
      error(FATAL|ELS,"segmentation falt(cons97) %s\n", ls);
   if (isempty(ls))
      return(makenull(NIL));
   else{
      new_cell = makecell(&(ls->element));
      new_cell->next = cons1(ls->next);
      new_cell->for_gc = WHITE;
      return(new_cell);
   }
   return(new_cell);
}
list_t cons(element_t car_part,list_t cdr_part)
{
   list_t new_cell=NIL;

#  ifdef DEBUG_LIST
   printf("cons-> ");
#  endif
   if ((cdr_part==(list_t)NIL)||(&car_part==(elementp)NIL))
      error(FATAL,"segmentation falt(cons309)\n");
   else{
      if (car_part.type==LIST)
         maintain_reg(AX,getls(car_part)); /* for GC */
      maintain_reg(AX,cdr_part); /* for GC */
      new_cell = makecell(&car_part);
      new_cell->next = cdr_part;
      new_cell->for_gc = WHITE;
      return(maintain_reg(CONS,new_cell)); /* for GC */
   }
   return(new_cell);
}

static bool iseqls(list_t lsa,list_t lsb); /* B036 */

/****************************************************
 *  if element is in list then it returns TRUE,     *
 * else nil pointer. the list is the empty list then*
 * return to nil pointer.                           *
 * the calling function do not use a return value of*
 * this function.                                   *
 *--------------------------------------------------*
 *  ret=member(list,elt);                           *
 *                                                  *
 *  bool ret -- TRUE: it is in                      *
 *                FALSE: others                     *
 *  list_t list -- pointer to list                   *
 *  element elt -- any element                      *
 ****************************************************/
static bool iseqop(int op_a,int op_b) /* B036 */
{
#  ifdef DEBUG_LIST
   printf("iseqop-> ");
#  endif
   if (op_a==op_b)
      return((bool)TRUE);
   else
      return((bool)FALSE);
}
static bool iscmp_PBB(token token_a,token token_b) /* B036 */
{
#  ifdef DEBUG_LIST
   printf("iscmp_PBB-> ");
#  endif
   if (token_a->token_name==token_b->token_name){
      if (token_a->attr.par.fr_or_af
        ==token_b->attr.par.fr_or_af)
         return((bool)TRUE);
      else
         return((bool)FALSE);
   }
   else
      return((bool)FALSE);
}
static bool iseqtk(token token_a,token token_b) /* B036 */
{
#  ifdef DEBUG_LIST
   printf("iseqtk-> ");
#  endif
   switch(token_a->token_name){
      case(ID):
         if (token_b->token_name==ID){
            if (!strcmp(token_a->attr.id.spl_ptr,
                        token_b->attr.id.spl_ptr))
               return((bool)TRUE);
            else
               return((bool)FALSE);
         }
         else
            return((bool)FALSE);
         break;
      case(VALUE):{
         field_t fld_type=(token_a->attr.value.type);
         if (token_b->token_name==VALUE){
            switch(fld_type){
               case(ICONST):
                  if (token_a->attr.value.type==ICONST){
                     if (token_a->attr.value.fld.iconst.int_v
                       ==token_b->attr.value.fld.iconst.int_v)
                        return((bool)TRUE);
                     else
                        return((bool)FALSE);
                  }
                  else
                     return((bool)FALSE);
                  break;
               case(STR):
                  if (token_a->attr.value.type==STR){
                     if (!strcmp(token_a->attr.value.fld.strings.str,
                                 token_b->attr.value.fld.strings.str))
                        return((bool)TRUE);
                     else
                        return((bool)FALSE);
                  }
                  else
                     return((bool)FALSE);
                  break;
               default:
                  return((bool)FALSE);
            }
         }
         else
            return((bool)FALSE);
         break;
      }
      case(PARENTHE):
      case(BRACE):
      case(BRACKET):
         return(iscmp_PBB(token_a,token_b));
         break;
      case(AGENT_OP):
      case(COMP_OP):
      case(BOOL_OP):
      case(VALUE_OP):
         if (iseqop(token_a->attr.op.type,
                     token_b->attr.op.type))
            return((bool)TRUE);
         else
            return((bool)FALSE);
         break;
      case(KEY_WORD):
         if (!strcmp(token_a->attr.keywd.str,
                     token_b->attr.keywd.str))
            return((bool)TRUE);
         else
            return((bool)FALSE);
         break;
      default:
         return((bool)FALSE);
   }
}
static bool iseqel(element_t ea,element_t eb) /* B036 */
{
   bool truth=FALSE;

#  ifdef DEBUG_LIST
   printf("iseqel-> ");
#  endif
   if (ea.type == eb.type){
      switch(ea.type){
         case TOKEN:
            if (iseqtk(ea.entry.tk,eb.entry.tk))
               truth=(bool)TRUE;
            break;
         case LIST:
            if (iseqls(ea.entry.list,eb.entry.list))
               truth=(bool)TRUE;
            break;
         case GC:
            if (ea.entry.gc.vacant==eb.entry.gc.vacant)
               truth=(bool)TRUE;
            break;
         default:
            error(FATAL|EEL,"invalid element type(iseqel519) %s\n", ea);
      }
   }
   return(truth);
}
static bool iseqls(list_t lsa,list_t lsb) /* B036 */
{
#  ifdef DEBUG_LIST
   printf("iseqls-> ");
#  endif
   if ((lsa == NIL)||(lsb ==NIL))
      error(FATAL,"segmentation falt(iseqls528)\n");
   else{
      if (isempty(lsa) && isempty(lsb))
         return((bool)TRUE);
      else{
         if (isempty(lsa) || isempty(lsb))
            return((bool)FALSE);
         else{
            if (lsa==lsb)
               return((bool)TRUE);
            else{
               if (iseqel(car(lsa),car(lsb)))
                  return(iseqls(cdr(lsa),cdr(lsb)));
               else
                  return((bool)FALSE);
            }
         }
      }
   }
   return((bool)FALSE);
}
elementp member(list_t ls,element_t elt)
{
   static element_t a;

#  ifdef DEBUG_LIST
   printf("member->");
#  endif
   if (ls == (list_t)NIL)
      error(FATAL,"segmentation falt(member559)\n");
   else{
      if (isempty(ls)){
#        ifdef DEBUG_LIST
         printf("FALSE(member)\n");
#        endif
         return((elementp)NIL);
      }
      else{
         if (iseqel(car(ls),elt)){
            a = car(ls);
#           ifdef DEBUG_LIST
            printf("TRUE(%04x)\n",&a);
#           endif
            return(&a);
         }
         else{
            return(member(cdr(ls),elt));
         }
      }
   }
   return((elementp)NIL);
}
bool ismember(list_t ls,element_t elt)
{
#  ifdef DEBUG_LIST
   printf("ismember->");
#  endif
   if (ls == (list_t)NIL)
      return((bool)error(FATAL,"segmentation falt(ismember581)\n"));
   else{
      if (member(ls,elt)==(elementp)NIL)
         return(FALSE);
      else
         return(TRUE);
   }
}
/***********************
 *  if the list is eqaule to anothor.
 *------------------------------------
 *  ret=eqls(lsa,lsb);
 *
 *  list_t lsa -- pointer to a list.
 *  list_t lsb -- pointer to a list.
 *  bool ret -- true if lsa==lsb
 *              false otherwise.
 ***********************/
bool eqls(list_t lsa,list_t lsb)
{
   return((bool)iseqls(lsa,lsb));
}
/***********************
 *  if the element is eqaule to anothor.
 *------------------------------------
 *  ret=eqel(ela,elb);
 *
 *  list_t ela -- an element.
 *  list_t elb -- an element.
 *  bool ret -- true if ela==elb
 *              false otherwise.
 ***********************/
bool eqel(element_t ela,element_t elb)
{
   return((bool)iseqel(ela,elb));
}

/****************************************************
 *  if a element is in list then, it returns list,  *
 * that the specified element isn't in list.        *
 *--------------------------------------------------*
 *  ret=delete(elt,list);                           *
 *                                                  *
 *  bool ret -- TRUE: it is in                      *
 *                FALSE: others                     *
 *  listp list -- pointer to list                   *
 *  element elt -- any element                      *
 ****************************************************/
list_t delete(element_t elt,list_t ls)
{
#  ifdef DEBUG_LIST
   printf("delete->");
#  endif
   if (ls == NIL)
      error(FATAL,"segmentation falt(delete593)\n");
   else{
      if (isempty(ls))
         return(makenull(NIL));
      else{
         if (iseqel(elt,car(ls)))
            return(cdr(ls));
         else
            return(cons(car(ls),delete(elt,cdr(ls))));
      }
   }
   return((list_t)NIL);
}

/****************************************************
 *  dotted pair                                     *
 *  this function create new list with cdr field and*
 * car filed. it initialieze value of first argment *
 * and second argment. the same cons                *
 *--------------------------------------------------*
 *  ret=dotpair(ea,eb);                             *
 *                                                  *
 *  list *ret - the returned value                  *
 *  element ea --- the first element                *
 *  element eb --- the second element               *
 ****************************************************/
list_t dotpair(element_t ea,element_t eb)
{
#  ifdef DEBUG_REGa
   printf("dotpair->");
#  endif
   return(cons(ea,cons(eb,makenull(NIL))));
}

/****************************************************
 *  add all elements of a lists to the fornt of     *
 * another list in the same order.                  *
 *--------------------------------------------------*
 *  ret=append(lsa,lsb);                            *
 *                                                  *
 *  list_t ret - a lists                             *
 *  list_t lsa -- the first list                     *
 *  list_t lsb -- the second list                    *
 ****************************************************/
list_t append(list_t lsa,list_t lsb)
{
#  ifdef DEBUG_LIST
   printf("append-> ");
#  endif
   if ((isempty(lsa))||(isempty(lsb)))
      return(makenull(NIL));
   else{
      if (isempty(lsa))
         return(lsb);
      else
         return(cons(car(lsa),append(cdr(lsa),lsb)));
   }
}

/****************************************************
 *  printf list
 *--------------------------------------------------*
 *  void prtlst(list);                              *
 *                                                  *
 *  list  --- header of list                        *
 ****************************************************/
static void prtls1(const list_t ls);
void prtelt(const element_t el)
{
#  ifdef DEBUG_LIST
   printf("prtelt-> ");
#  endif
   switch(el.type){
      case EPSILON:
         printf(")");
         break;
      case LIST:
         printf("(");
         prtls1(el.entry.list);
         break;
      case TOKEN:{
         prtsym(PRN,el.entry.tk);
         break;
      }
      case GC:{
         printf("%p",el.entry.gc.vacant);
         break;
      }
      case DUMY:
      default:
         error(FATAL,"Invalid element type(prtelt683) %d\n",el.type);
   }
}
static void prtls1(const list_t ls)
{
#  ifdef DEBUG_LIST
   printf("prtls1-> ");
#  endif
   if (isempty(ls)){
      prtelt(epsilon.element);
      return;
   }
   else{
      prtelt(car(ls));
      if (!isempty(cdr(ls)))
         printf(",");
      prtls1(cdr(ls));
      return;
   }
}
void prtlst(const list_t ls)
{
#  ifdef DEBUG_LIST
   printf("prtls-> ");
#  endif
   if (ls==(list_t)NIL)
      error(FATAL,"Null pointer(prtlst712).\n");
   else{
      printf("(");
      prtls1(ls);
      printf("\n");
      fflush(stdout);
   }
}
