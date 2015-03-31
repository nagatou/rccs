/*
 * $Id: primitive.c,v 1.1.1.1 2011/05/18 06:25:25 nagatou Exp $
 */
//static char rcsid[]="@(#)$Id: primitive.c,v 1.1.1.1 2011/05/18 06:25:25 nagatou Exp $";
static char copyright[]=
   "A programming language based on CCS.\nCopyright (C) 2000 Naoyuki NAGATOU";

/************************************************************************
   this file is a set of functions for primitive action.
                 Copyright (C) 1998 Naoyuki Nagatou
 ************************************************************************/
#define EXTERN
#include "comm.h"
#define PRIM
#include "primitive.h"

/*****************************************************
 *  checking a primitive action.                     *
 *---------------------------------------------------*
 *  ret = isouter_action(label);                     *
 *                                                   *
 *  bool ret --- if outer-action TRUE                *
 *                  otherwise    FALSE               *
 *  element label --- an action                      *
 *****************************************************/
static bool isprimconame(splp el)
{
#  ifdef DEBUG_PRIM
   printf("isprimconame->");
#  endif
   return(isin(primconametbl,
//               el.el_entry.tk->attr.id.spl_ptr,
               el,
               PRIMCONAME_SIZE));
}
static bool isprimname(splp el)
{
#  ifdef DEBUG_PRIM
   printf("isprimname->");
#  endif
   return(isin(primnametbl,
//               el.el_entry.tk->attr.id.spl_ptr,
               el,
               PRIMNAME_SIZE));
}
bool isouter_action(element_t label)
{
#  ifdef DEBUG_PRIM
   printf("isouter_action->");
#  endif
   if (isprimname(label.entry.tk->attr.id.spl_ptr))
      return((bool)TRUE);
   else{
      if (isprimconame(label.entry.tk->attr.id.spl_ptr))
         return((bool)TRUE);
      else{
         if (((bindLSp)(NIL))==retrieval1(label))
            return((bool)FALSE);
         else
            return((bool)TRUE);
      }
   }
}
/*****************************************************
 *  checking a primitive action.                     *
 *---------------------------------------------------*
 *  ret = istrans_out_act(label);                    *
 *                                                   *
 *  bool ret --- if outer-action TRUE                *
 *                  otherwise    FALSE               *
 *  element label --- an action                      *
 *****************************************************/
bool istrans_out_act(element_t label)
{
#  ifdef DEBUG_PRIM
   printf("istrans_out_act->");
#  endif
   return(n_select(label));
}
/*****************************************************
 *  An initialze function.                           * 
 *  First, the function clear spell-table with 0,    *
 * and initialize a file buffer.                     *
 * Final, the function registory primitive actions to*
 * hash-table.                                       *
 *---------------------------------------------------*
 *  void prologue(char * log);                       *
 *                                                   *
 *****************************************************/
static int cvrtname(char * name)
{
#  ifdef DEBUG_PRIM
   printf("cvrtname->");
#  endif
   if (!strcmp(name,"display"))
      return(1);
   else{
      if (!strcmp(name,"key"))
         return(2);
      else{
         if ((!strcmp(name,primnametbl[ACCEPT]))||
             (!strcmp(name,primnametbl[OTHERS])))
            return(DUMMY_SOCKET);
         else
            return((int)error(FATAL,"Invalid primitive name and co-name(cvrtname105): %s\n",name));
      }
   }
}
static void regprimact(char *coname[],char *name[])
{
   int i=0;

#  ifdef DEBUG_PRIM
   printf("regprimact->");
#  endif
   for(i=0;i<PRIMCONAME_SIZE;i++)
      registry1(primconametbl[i],cvrtname(primconametbl[i]),"",FILE_SYS);
   for(i=0;i<PRIMNAME_SIZE;i++)
      registry1(primnametbl[i],cvrtname(primconametbl[i]),"",FILE_SYS);
}
void prologue(char * log)
{
#  ifdef DEBUG_PRIM
   printf("prologue->");
#  endif
   printf("%s\n",copyright);
   /*------ initialize spell table -----*/
   initspltbl(&spl_tbl); /***/
   /*------ initialize heap area -----*/
   init_MBCT(&memory_control_table);
   /*------ initialize heap area -----*/
   init_sym_tbl(&symbol_table);
   /*------ initialize file system -----*/
   initsf(NIL,log);/***/
   /*------ register primitive action -----*/
   regprimact(primconametbl,primnametbl);
//   /*------ install siganal handler -----*/
//   set_signal();
}
/*****************************************************
 *  this function close sockets.                     * 
 *---------------------------------------------------*
 *  void epilogue(void);                     *
 *                                                   *
 *****************************************************/
static void epilogue1(bindLSp member)
{
#  ifdef DEBUG_PRIM
   printf("epilogue1->");
#  endif
   if (member==(bindLSp)NIL)
      return;
   else{
      if ((isprimconame(member->label))||(isprimname(member->label)))
         epilogue1(member->next);
      else{
         shutdown(member->pair.line_or_socket,2);
         close(member->pair.line_or_socket);
         epilogue1(member->next);
      }
   }
}
void epilogue(void)
{
   int i=0;

#  ifdef DEBUG_PRIM
   printf("epilogue->");
#  endif
   for(i=0;i<HASH_TBL_SIZE;i++)
      epilogue1(hash_tbl_LS[i]);
   free(memory_control_table.ls_area.area);
   free(memory_control_table.el_area.area);
   mc_exit_debugger();
}
