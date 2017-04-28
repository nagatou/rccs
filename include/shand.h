/*
 * $Id: shand.h,v 1.10 2012/04/04 08:08:20 nagatou Exp $
 */

/***************************************************
 this file is header files for lexical analysis.
                   Copyright (C) 1998 Naoyuki Nagatou
 ***************************************************/

#ifndef RCCS_SHAND_H
#define RCCS_SHAND_H
/***************************************************
 *  class name:spell table                         *
 *-------------------------------------------------*
 *-------------------------------------------------*
 *  methods:                                       *
 *      regspl();                                  *
 *      rtrvspl();                                 *
 *      initspltbl();                              *
 ***************************************************/
#define SPL_TBL_SIZE 4152
#define EOS 0xff
typedef struct{
   char *top;
   char *bottom;
   char *point;
   int  cunt;
   char spell[SPL_TBL_SIZE];
} spell_table;
EXTERN spell_table spl_tbl; /***/
typedef char *splp;

/***************************************************
 *  class name:symbol table                        *
 *-------------------------------------------------*
 *-------------------------------------------------*
 *  methods:                                       *
 *    regsym();                                    *
 *    rtrvsym();                                   *
 *    initsymtbl();                                *
 ***************************************************/
#define STR_LEN BUF_SIZE
#define SYM_TBL_SIZE 30000
enum{  /*** enumrating functions of prtsym ***/
   PRN=0,
   BUF,
};
typedef enum {  /*** enumrating token names ***/
   ID=0,
   VALUE,
   PARENTHE=10,BRACE,BRACKET,
   AGENT_OP=21,COMP_OP,BOOL_OP,VALUE_OP,
   KEY_WORD=99,
} token_t;
typedef enum {
/*** enumrating field type for ID ***/
   ACT=10,A_VAR,A_CON,V_VAR,
   NAME=20,CO_NAME,OTHERS,
   LAB=30,
/*** enumrating field type for VALUE ***/
   ICONST=40,STR,
/*** enumrating field type for parenthsis ***/
   FR=50,AF,
/*** enumrating field type for operators ***/
   DEF=70,DEFINIT,IF,LET,FIX,BIND,
/*** enumrating field type for AGENT_OP ***/
   PRE=80,SUM,COM,RES,REL,CON,REC,CO,SEND,RECV,
/*** enumrating field type for COMP_OP ***/
   EQ=90,GR,GT,LE,LT,
/*** enumrating field type for BOOL_OP ***/
   OR=100,AND,NOT,
/*** enumrating field type for VALUE_OP ***/
   PLUS=110,MINS,MULT,DIV,MOD,SEQ,
} field_t;
typedef enum { /*** enumrating field type for operands ***/
   UNARY=120,BINARY,
} operand_t;
typedef union{
   struct{
      field_t type;
      splp spl_ptr;
      union{
         struct{ /* is not used currently */
            int action_type;
            int label_type;
         }act;
      }fld;
   }id;
   struct{
      char *str;
   }keywd;
   struct{
      field_t type;
      union{
         struct{
            int int_v;
        }iconst;
        struct{
           FTYPE flt_v;
        }fconst;
        struct{
           char str[STR_LEN];
        }strings;
      } fld;
   }value;
   struct{
      field_t fr_or_af;
   } par;
   struct{
      field_t type;
      operand_t number;
   }op;
   struct{
      int undefined;
   }undefined;
}attribute;
typedef struct{
   token_t token_name;
   attribute attr;
}symbol;

typedef struct{
   int next;
   symbol *area;
} SYMTBL;

EXTERN SYMTBL symbol_table;

/***************************************************
 *  class name:token                               *
 *-------------------------------------------------*
 *-------------------------------------------------*
 *  methods:                                       *
 *     scan();                                     *
 ***************************************************/
typedef symbol *token;

/***************************************************
 *  class name:hash table                          *
 *-------------------------------------------------*
 *-------------------------------------------------*
 *  methods:                                       *
 *     happend();                                  *
 *     hash();                                     *
 ***************************************************/
#ifndef HASH_TBL_SIZE
  #define HASH_TBL_SIZE 17
#endif
typedef struct entry_of_symbol * hash_entry_p;
typedef struct entry_of_symbol{
   token sym_p;
   hash_entry_p next;
} hash_entry_t ;
EXTERN hash_entry_t *hash_table[HASH_TBL_SIZE];

#ifdef SHAND
/***************************************************
 *  class name:keyword set                         *
 *-------------------------------------------------*
 *-------------------------------------------------*
 *  methods:                                       *
 *    isin();                                      *
 ***************************************************/
#define SYM_SIZE 11
static char *nccskwd[SYM_SIZE]={
   "define","if","definit","let","fix","res",
   "TRUE","FALSE","true","false",
   "bind",
};
#endif
#endif
