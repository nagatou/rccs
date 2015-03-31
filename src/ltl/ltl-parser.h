/*
 * $Id: ltl-parser.h,v 1.1.1.1 2011/05/18 06:25:24 nagatou Exp $
 */

#ifndef _LTLPARSER_H_
#define _LTLPARSER_H_

#include "ltl-scanner.h"
#include "ltl-token.h"

typedef struct _ltl_parser_state {
    ltl_token *top;
    ltl_token *last;
} ltl_parser_state;

extern void ltl_parse(ltl_parser_state *state, ltl_lexeme *lexeme);
    
#endif /*_LTLPARSER_H_*/
