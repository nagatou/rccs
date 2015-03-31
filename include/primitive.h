/*
 * $Id: primitive.h,v 1.2 2011/06/05 22:26:19 nagatou Exp $
 */

/***************************************************
 this file is header files for primitive action.
                 Copyright (C) 1998 Naoyuki Nagatou
 ***************************************************/
#ifndef RCCS_PRIMITIVE_H
   #define RCCS_PRIMITIVE_H
   #define DUMMY_SOCKET -2
   #define PRIMCONAME_SIZE 1
   #define PRIMNAME_SIZE 2
   #define ACCEPT 1
//   #define OTHERS 2
      #ifdef PRIM
/***************************************************
 *  class name:primitive actions                   *
 *-------------------------------------------------*
 *-------------------------------------------------*
 *  methods:                                       *
 ***************************************************/
      EXTERN char *primconametbl[PRIMCONAME_SIZE]={
         "display",
      };
      EXTERN char *primnametbl[PRIMNAME_SIZE]={
         "key",
         "accept",
//         "others",
      };
   #else
   extern char *primconametbl[PRIMCONAME_SIZE];
   extern char *primnametbl[PRIMNAME_SIZE];
   #endif
/*** function decroletar ***/
   extern bool isouter_action(element_t);
   extern bool istrans_out_act(element_t);
#endif
