/*
 * $Id: scan.h,v 1.1.1.1 2011/05/18 06:25:25 nagatou Exp $
 */

/***************************************************
 this file is header files for lexical analysis.
                   Copyright (C) 1998 Naoyuki Nagatou
 ***************************************************/
#ifndef RCCS_SCAN_H
#define RCCS_SCAN_H

/***************************************************
 *  class name:env                                 *
 *-------------------------------------------------*
 *-------------------------------------------------*
 *  methods:                                       *
 *      setjmp();                                  *
 *      longjmp();                                 *
 ***************************************************/
EXTERN jmp_buf env;

#ifdef SCAN
/*** function decroletar ***/
static void start_state(buffer *,token *);
#endif
#endif
