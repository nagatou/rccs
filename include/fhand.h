/*
 * $Id: fhand.h,v 1.1.1.1 2011/05/18 06:25:25 nagatou Exp $
 */

/*****************************************************
  this file is functions for file handlers.
                   Copyright (C) 1998 Naoyuki Nagatou
 *****************************************************/
#ifndef RCCS_FHAND_H
#define RCCS_FHAND_H

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

/*** global variable decroletar ***/
#ifdef FHAND
static queue que={
   0,0,
   {0,},
};
static buffer in_buf;
static buffer out_buf;
#endif
#endif
