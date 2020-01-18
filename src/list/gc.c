/*
 * $Id: gc.c,v 1.21 2011/07/14 07:24:25 nagatou Exp $
 */
//static char rcsid[]="@(#)$Id: gc.c,v 1.21 2011/07/14 07:24:25 nagatou Exp $";
//static char copyright[]=
//   "Copyright (C) 1998 Naoyuki Nagatou";

/***********************************************
  this file is functions for GC
              Copyright (C) 1998 Naoyuki Nagatou
 ***********************************************/
#define EXTERN extern
#include "comm.h"
#include "driver.h"

/**************************************************
 *  Initializeing memory-block control table
 *------------------------------------------------*
 *  ret = init_MBCT(void);                          *
 *                                                *
 *  bool ret --- if list is empty then return to  *
 *              TRUE, else return to FALSE        *
 *  list_t list - pointer of header for list.      *
 **************************************************/
void zeroclear(size_t size,void *block)
{
   memset(block,0x00,size);
}
void *create_heap_area(size_t capacity)
{
   void *area=(void *)NIL;

   if ((area=malloc(capacity))==(void *)NIL)
      error(FATAL,"Cannot allocate. the available memory on your system is not enough.(create_heap_area34)\n");
   else
      return(area);
   return(area);
}
#define PID_MAX_LENGTH 5
//#pragma weak __libc_stack_end;
extern void *__libc_stack_end;
//#define GETSP () ({unsigned int *stack_ptr asm ("%esp"); stack_ptr;})
static void get_mem_map(MBCT *table)
{
   int cnt;
   FILE *maps;
   char procpath[PID_MAX_LENGTH+12+1];
   long pagesize=sysconf(_SC_PAGESIZE);
   register unsigned int *stack_ptr asm ("%esp");
   uintptr_t stack_base=(((uintptr_t)__libc_stack_end+pagesize)&~(pagesize-1));

#  ifdef DEBUG_GC
   printf("get_mem_map->");
#  endif
   sprintf(procpath,"/proc/%d/maps",getpid());
   if ((maps=fopen(procpath,"r"))==NULL)
      error(FATAL,"cannot open (/proc/%d/maps)\n",getpid());
   for(;!feof(maps);){
      char buf[PATH_MAX+100], perm[5], dev[6], mapname[PATH_MAX];
      unsigned long begin, end, inode, foo;

      if(fgets(buf, sizeof(buf), maps) == 0)
          break;
      mapname[0] = '\0';
      sscanf(buf,"%lx-%lx %4s %lx %5s %ld %s",&begin,&end,perm,&foo,dev,&inode,mapname);
#     ifdef DEBUG_GC
      if ((!(strstr(mapname,"stack")==NIL))||(!(strstr(mapname,"heap")==NIL)))
         printf("%lx-%lx %4s %lx %5s %ld %s\n",begin,end,perm,foo,dev,inode,mapname);
#     endif
      if (!(strstr(mapname,"stack")==NIL)){
         table->stack_seg.stack_bottom=stack_base;
         table->stack_seg.stack_top=(uintptr_t)stack_ptr;
         table->stack_seg.stack_limit=(uintptr_t)begin;
         break;
      }
      for(cnt=0;cnt<PATH_MAX+10;++cnt)
         buf[cnt]='\0';
   }
   fclose(maps);
}
void init_MBCT(MBCT *table)
{
   cell_t *current=(cell_t *)NIL;
   int i=0;

#  ifdef DEBUG_GC
   printf("init_MBCT->");
#  endif
   printf("The size of a cell is %lu b, memory limit %lu Mb\n",sizeof(cell_t),(sizeof(cell_t)*CellCapacity)/1000000);
   table->ls_area.area = (list_t)(create_heap_area(sizeof(cell_t)*(CellCapacity)));
   table->ls_area.area_top = (list_t)(table->ls_area.area+CellCapacity);
   table->ls_area.gc_counter = 0;
   for(current=table->ls_area.area;current<table->ls_area.area+(CellCapacity);current++){
      current->for_gc = WHITE;
      current->ref = makenull(NIL);
      current->element.type = GC;
      current->element.entry.gc.vacant = current;
      current->next = current+1;
   }
   (table->ls_area.area+(CellCapacity-1))->next = makenull(NIL);
   table->ls_area.ls_reg[FREE_LIST] = table->ls_area.area;
   for(i=1;i<NUMBEROFREG;i++)
      table->ls_area.ls_reg[i] = makenull(NIL);
   table->el_area.area = (element_t *)(create_heap_area(sizeof(element_t)*ElementCapacity));
   table->el_area.area_top = (element_t *)(table->ls_area.area+ElementCapacity);
   table->el_area.number = 0;
}

/**************************************************
 *  gabarage collecter.
 *------------------------------------------------*
 *  ret = gc(table);                          *
 *                                                *
 **************************************************/
void prtreg(void)
{
   fflush(stdout);
   printf("[");
   printf("FREE=>");
   prtlst(memory_control_table.ls_area.ls_reg[FREE_LIST]);
//   printf("CONS =>");
//   fflush(stdout);
//   prtlst(memory_control_table.ls_area.ls_reg[CONS]);
//   printf("STACK=>");
//   fflush(stdout);
//   prtlst(memory_control_table.ls_area.ls_reg[STACK]);
   fflush(stdout);
   printf("==========================================================\n");
}
static bool isbottom(list_t ls)
{
   return(isempty(ls));
}
static list_t gc_push(cell_t *top,list_t stack)
{
#  ifdef DEBUG_GC
   printf("gc_push->");
#  endif
   if ((stack==(list_t)NIL)||(top==(list_t)NIL))
      error(FATAL,"Segmentation fault(gc_push111)\n");
   if ((memory_control_table.ls_area.area > top)&&(top > memory_control_table.ls_area.area_top))
      error(FATAL,"cannot access over the attached heap area(gc_push139)\n");
   else{
      if (top==&epsilon)
         return(stack);
      else{
         switch(top->for_gc){
            case WHITE:
               if (top->element.type!=GC){
                  top->for_gc=GLAY;
                  return((top->ref=stack,stack=top));
               }
               else
                  return(stack);
               break;
            case BLACK:
            case GLAY:
            case NON_GC:
               return(stack);
            default:
               error(FATAL,"invalid cell(gc_push153) %d",top->for_gc);
         }
      }
   }
   return((list_t)NIL);
}
static list_t gc_pop(list_t stack)
{
#  ifdef DEBUG_GC
   printf("gc_pop->");
#  endif
   if ((stack==(list_t)NIL)||(stack->ref==(list_t)NIL))
      error(FATAL,"Segmentation fault(gc_pop125)\n");
   else
      return(stack->ref);
   return((list_t)NIL);
}
static list_t gc_init(list_t reg[],MBCT *table)
{
#  ifdef DEBUG_GC
   printf("gc_init->");
#  endif
   get_mem_map(table);
   if (reg==(list_t *)NIL)
      error(FATAL,"segmentation fault(gc_init115)\n");
   else{
      int i=0;
      list_t top=memory_control_table.ls_area.ls_reg[STACK];
      list_t *ptr=(list_t *)table->stack_seg.stack_top;
      if ((ptr==(list_t *)NIL)||(table==(MBCT *)NIL))
         error(FATAL,"Segmentation fault(gc_init182)\n");
      for(;ptr<(list_t *)table->stack_seg.stack_bottom;ptr++){
         if ((table->ls_area.area <= (*ptr))&&((*ptr) <= table->ls_area.area_top)){
            if ((*ptr)!=&epsilon){
               if ((*ptr)->for_gc==WHITE){
                  if ((*ptr)->element.type!=GC)
                     table->ls_area.ls_reg[STACK]=top=gc_push((*ptr),top);
               }
            }
         }
      }
      for(i=2;i<NUMBEROFREG;i++){
         if (reg[i]!=(list_t)NIL){
            if (reg[i]!=&epsilon){
               table->ls_area.ls_reg[STACK] = (top = gc_push(reg[i],top));
            }
         }
         else
            error(FATAL,"segmentation fault(gc_init132)\n");
      }
      return(top);
   }
   return((list_t)NIL);
}
list_t maintain_reg(register_num_t number,list_t val)
{
#  ifdef DEBUG_REG_A
   printf("maintain_reg->");
   fflush(stdout);
#  endif
   if ((memory_control_table.ls_area.area > val)&&(val > memory_control_table.ls_area.area_top))
      error(FATAL,"cannot access over the attached heap area(maintain_reg220)\n");
   if (memory_control_table.ls_area.ls_reg[number]==(list_t)NIL)
      error(FATAL,"segmentation fault(maintain_reg187)\n");
   if (val==(list_t)NIL)
      return(val);
   else{
      if ((0>=number)||(number>=NUMBEROFREG))
         error(FATAL,"segmentation fault(maintain_reg191)\n");
      else{
         if (!isempty(memory_control_table.ls_area.ls_reg[number])){
            if (!isempty(val)){
               memory_control_table.ls_area.ls_reg[STACK]=gc_push(memory_control_table.ls_area.ls_reg[number],memory_control_table.ls_area.ls_reg[STACK]);
               memory_control_table.ls_area.ls_reg[number]=val;
               return(val);
            }
            else
               return(val);
         }
         else{
            if (!isempty(val)){
               memory_control_table.ls_area.ls_reg[number]=val;
               return(val);
            }
            else
               return(val);
         }
      }
   }
   return((list_t)NIL);
}
static MBCT *gc_mark(MBCT *,cell_t *);
static MBCT *gc_mark11(MBCT *table,cell_t *P,list_t top);
static MBCT *gc_mark2(MBCT *table,cell_t *P,cell_t *Q,list_t top)
{
#  ifdef DEBUG_GC
   printf("gc_mark2->");
#  endif
   if ((!isempty(Q))){
      switch(Q->for_gc){
         case WHITE:
            return(gc_mark11(table,cdr(P),gc_push(Q,top)));
            break;
         case GLAY:
         case BLACK:
            return(gc_mark11(table,cdr(P),top));
            break;
         default:
            error(FATAL,"Contact us. Invalid color for GC(gc_mark2205).\n");
            return((MBCT *)NIL);
      }
   }
   else
      return(gc_mark(table,top));
}
//static MBCT *gc_mark21(MBCT *table,cell_t *P,list_t top)
//{
//#  ifdef DEBUG_GC
//   printf("gc_mark21->");
//#  endif
//   return(gc_mark2(table,P,getls(car(P)),top));
//}
static MBCT *gc_mark11(MBCT *table,cell_t *P,list_t top)
{
#  ifdef DEBUG_GC
   printf("gc_mark11->");
#  endif
   if (isempty(P))
      return(gc_mark(table,top));
   else{
      switch(P->for_gc){
         case GLAY:
            P->for_gc=BLACK;
            if (P->element.type==LIST)
               return(gc_mark2(table,P,getls(car(P)),top));
            else
               return(gc_mark11(table,cdr(P),top));
         case WHITE:
         case BLACK:
            return(gc_mark11(table,cdr(P),top));
            break;
         default:
            error(FATAL,"Contact us. Invalid color for GC(gc_mark2237).\n");
            return((MBCT *)NIL);
      }
   }
}
//static MBCT *gc_mark12(MBCT *table,cell_t *P,list_t top)
//{
//#  ifdef DEBUG_GC
//   printf("gc_mark12->");
//#  endif
//   return(gc_mark11(table,cdr(P),top));
//}
//static MBCT *gc_mark1(MBCT *table,list_t top)
//{
//#  ifdef DEBUG_GC
//   printf("gc_mark1->");
//#  endif
//   return(gc_mark11(table,top,gc_pop(top)));
//}
static MBCT *gc_mark(MBCT *table,cell_t *top)
{
#  ifdef DEBUG_GC_M
   printf("gc_mark(%p)->\n",top);
#  endif
   if (isbottom(top))
      return(table);
   else{
      return(gc_mark11(table,top,gc_pop(top)));
   }
}
static MBCT *gc_mark_tail(MBCT *table,cell_t *stack_top)
{
   cell_t *top=stack_top;
   cell_t *P=(cell_t *)NIL;

   for(P=top,top=gc_pop(top);P!=&epsilon;P=top,top=gc_pop(top)){
#     ifdef DEBUG_GC
      printf("gc_mark_tail(%p)->\n",top);
#     endif
      if (P==&epsilon)
         continue;
      else{
         switch(P->for_gc){
            case GLAY:
               P->for_gc=BLACK;
               if (((cdr(P))!=&epsilon)&&(cdr(P)!=(cell_t *)NIL)){
if (P->element.type==3)
printf("\nMark: cell=%p,type=%d ->\n",P,P->element.type);
                  switch(P->next->for_gc){
                     case WHITE:
                        top = gc_push(P->next,top);
                        break;
                     case GLAY:
                     case BLACK:
                     case NON_GC:
                        break;
                     default:
                        error(FATAL,"Contact us. Invalid color for GC(gc_mark_tail307).\n");
                  }
               }
               if (P->element.type==LIST){
                  cell_t *Q = getls(car(P));
                  if ((Q!=&epsilon)&&(Q!=(cell_t *)NIL)){
                     switch(Q->for_gc){
                        case WHITE:
                           top = gc_push(Q,top);
                           break;
                        case GLAY:
                        case BLACK:
                        case NON_GC:
                           break;
                        default:
                           error(FATAL,"Contact us. Invalid color for GC(gc_mark_tail326): %d.\n",Q->for_gc);
                     }
                  }
               }
               break;
            case WHITE:
               error(FATAL,"Why its color is WHITE (gc_mark_tail336).\n");
            case BLACK:
            case NON_GC:
               break;
            default:
               error(FATAL,"Contact us. Invalid color for GC(gc_mark_tail341): %d.\n",P->for_gc);
         }
      }
   }
   return(table);
}
static void gc_epilogue(MBCT *table)
{
   int i=0;
   for(i=1;i<NUMBEROFREG;i++)
      table->ls_area.ls_reg[i] = makenull(NIL);
}
/***
* NOTE:
*  Don't start sweeping before all of the mark phase.
* It's phase must complete to start sweeping.
***/
static void sweep(MBCT *table)
{
   cell_t *current=(cell_t *)NIL;
   list_t heap=table->ls_area.area;

   for(current=heap;current<heap+(CellCapacity);current++){
#     ifdef DEBUG_GC_F
      printf("\nsweep(color=%d,cell=%p,type=%d)->\n",current->for_gc,current,current->element.type);
      fflush(stdout);
#     endif
if (current->element.type==3)
printf("\nsweep(Sweep: color=%d,cell=%p,type=%d)->\n",current->for_gc,current,current->element.type);
      switch(current->for_gc){
         case NON_GC:
            break;
         case GLAY:
            error(FATAL|ELS,"please, contact me(sweep374): %s\n",current);
            break;
         case BLACK:
            current->for_gc = WHITE;
            current->ref = (list_t)NIL;
            break;
         case WHITE:
            if (car(current).type==GC)
               continue;
            else{
               freecell(current);
               current->element.entry.gc.vacant=current;
               current->next=memory_control_table.ls_area.ls_reg[FREE_LIST];
               table->ls_area.ls_reg[FREE_LIST]=current;
               continue;
            }
            break;
         default:
            error(FATAL,"please, contact me.(sweep380) (%d)\n", current->for_gc);
      }
   }
   if (isempty(table->ls_area.ls_reg[FREE_LIST]))
      error(FATAL,"Use up the attached heap area.(sweep382)\n");
}
void gc(MBCT *table)
{
#  ifdef DEBUG_GC_D
   printf("gc[counter=%d,",(table->ls_area.gc_counter)+1);
   printf("]->");
   fflush(stdout);
#  endif
   memory_control_table.ls_area.gc_counter++;
//   sweep(gc_mark(table,gc_init(table->ls_area.ls_reg,table)));
   sweep(gc_mark_tail(table,gc_init(table->ls_area.ls_reg,table)));
   gc_epilogue(table);
}
