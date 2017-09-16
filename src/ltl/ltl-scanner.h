/*
 * $Id: ltl-scanner.h,v 1.1.1.1 2011/05/18 06:25:23 nagatou Exp $
 */
 
#ifndef _LTLSCANNER_H_
#define _LTLSCANNER_H_

#ifndef EXTERN
#define EXTERN extern
#endif
#include "comm.h"

typedef enum {
    UNKNOWN = 0,
    NOT_STATE_PROP,         /* !    */
    UNTIL_PATH,             /* U    */
    RELEASE_PATH,           /* R    */
    NEXTTIME_PATH,          /* X    */
    AND_PATH,               /* /\\  */
    OR_PATH,                /* \\/  */
    AND_PROP,               /* &    */
    OR_PROP,                /* |    */
    IMPLIES_STATE_PROP,     /* ->   */
    ALL_PATH,               /* []   */
    EXIST_PATH,             /* <>   */
    FUNC_EQUALS,            /* =    */
    FUNC_SMALLEREQUAL,      /* <=   */
    FUNC_SMALLER,           /* <    */
    FUNC_LARGEREQUAL,       /* >=   */
    FUNC_LARGER,            /* >    */
    FUNC_PLUS,              /* +    */
    FUNC_MINUS,             /* -    */
    OPENBRACKET_PATH_TERM,  /* (    */
    CLOSEBRACKET_PATH_TERM, /* )    */
    TERM_ID,
    TERM_DIGITS,
    TERM_BOOLEAN,
    TERM_STRING,
    EOT = -1,
} ltl_lexeme_type;

typedef struct _ltl_lexeme {
    ltl_lexeme_type type;
    char *lit;
    union {
        int termint;
        bool termbool;
    } info;
} ltl_lexeme;

typedef struct _ltl_scanner_info {
    const char *src;
} ltl_scanner_info;


extern ltl_lexeme get_next_lexeme(ltl_scanner_info *info);

#endif /*_LTLSCANNER_H_*/
