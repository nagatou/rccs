/*
 * $Id: parse.h,v 1.1.1.1 2011/05/18 06:25:25 nagatou Exp $
 */

/***************************************************
 this file is header files for syntax analysis.
                   Copyright (C) 1998 Naoyuki Nagatou
 ***************************************************/
#ifndef RCCS_PARSE_H
#define RCCS_PARSE_H

/***************************************************
 *  class name:env                                 *
 *-------------------------------------------------*
 *-------------------------------------------------*
 *  methods:                                       *
 *      setjmp();                                  *
 *      longjmp();                                 *
 ***************************************************/
EXTERN jmp_buf env_par;

/***************************************************
 *  class name:lookahead                           *
 *-------------------------------------------------*
 *-------------------------------------------------*
 *  methods:                                       *
 *      parse();                                   *
 ***************************************************/
EXTERN token lah;

#ifdef PARSE
/*** function decroletar ***/
static list_t b_exp(void);
static list_t relabel_seq_left(list_t);
static list_t id_seq_left(list_t);
static list_t agent_exp(void);
static list_t a_unary_exp(void);
#endif
#endif
