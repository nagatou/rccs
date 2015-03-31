/*
 * $Id: ehand.h,v 1.1.1.1 2011/05/18 06:25:25 nagatou Exp $
 */

/***************************************************
   this file is header files for syntax analysis.
                 Copyright (C) 1998 Naoyuki Nagatou
 ***************************************************/
#ifndef RCCS_EHAND_H
#define RCCS_EHAND_H

#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif
#define LINELEN 128
#define SOCKET_ERROR -1
#define BACKLOG 5
#define DELAY_TIME 0
#define PORTBASE  0

//EXTERN int errno;
//EXTERN char *sys_errlist[LINELEN];
//EXTERN int sckt;

/***************************************************
 *  class name: finename_lineno_pair               *
 *-------------------------------------------------*
 *-------------------------------------------------*
 *  methods:                                       *
 ***************************************************/
enum entity_qualifier_t {
   FILE_SYS=700,
   BREAK_POINT,
   SYS_CALL,
};
typedef int line_or_socket_t;
typedef struct{
   int qualifier;
   char file_or_host[STR_LEN];
   line_or_socket_t line_or_socket;
} filename_lineno_pair_t;
/***************************************************
 *  class name: host_serv                          *
 *-------------------------------------------------*
 *-------------------------------------------------*
 *  methods:                                       *
 ***************************************************/
#define CS_LEN 7
typedef struct {
   char host[STR_LEN];
   char service[STR_LEN];
   char cs[CS_LEN];
} hs_t;
typedef hs_t *hsp;

/***************************************************
 *  class name:bindLS                              *
 *-------------------------------------------------*
 *-------------------------------------------------*
 *  methods:                                       *
 ***************************************************/
typedef struct bindLS_tt *bindLSp;
typedef struct bindLS_tt{
   splp label;
   filename_lineno_pair_t pair;
   bindLSp next;
} bindLS_t;

/***************************************************
 *  class name:hash table                          *
 *-------------------------------------------------*
 *-------------------------------------------------*
 *  methods:                                       *
 *     registory();                                *
 *     ritrieve();                                 *
 ***************************************************/
#ifndef HASH_TBL_SIZE
  #define HASH_TBL_SIZE 17
#endif
EXTERN bindLSp hash_tbl_LS[HASH_TBL_SIZE];

/*** function decroletar ***/
#ifndef EHAND
extern bindLSp retrieval1(element_t label);
extern bindLSp registry1(splp ,int ,splp ,int);
extern bool n_select(element_t label);
#endif
#endif
