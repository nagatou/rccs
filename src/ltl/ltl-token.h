/*
 * $Id: ltl-token.h,v 1.1.1.1 2011/05/18 06:25:24 nagatou Exp $
 */

#ifndef _LTLTOKEN_H_
#define _LTLTOKEN_H_

#include "linked-list.h"

typedef enum {
    LTL_SYMBOL = 0,
    LTL_UNARY_OP = 1,
    LTL_BINARY_OP = 2,
    LTL_BRACKETS = 3,
    LTL_UNKNOWN_TYPE = -1
} ltl_token_type;

typedef enum {
    LTL_ID              = 0,
    LTL_INTVALUE,
    LTL_BOOLVALUE,
    LTL_STRINGVALUE,
    LTL_GLOBALLY        = 100,  /* [] / A   */
    LTL_FUTURE,                 /* <> / F   */
    LTL_NOT,                    /* !        */
    LTL_NEXTTIME,               /* X        */
    LTL_UNTIL           = 200,  /* U        */
    LTL_RELEASE,                /* R (not from scanner) */
    LTL_PATH_AND,               /* /\\      */
    LTL_PROP_AND,               /* &        */
    LTL_PATH_OR,                /* \\/      */
    LTL_PROP_OR,                /* |        */
    LTL_IMPLIES,                /* ->       */
    LTL_EQUALS,                 /* =        */
    LTL_SMALLEREQUAL,           /* <=       */
    LTL_SMALLER,                /* <        */
    LTL_LARGEREQUAL,            /* >=       */
    LTL_LARGER,                 /* >        */
    LTL_PLUS,                   /* +        */
    LTL_MINUS,                  /* -        */
    LTL_BRACKET         = 300,  /* ()       */
    LTL_UNKNOWN_DETAIL  = -1
} ltl_token_details;

typedef struct _ltl_token _ltl_token;

typedef struct _ltl_token {
    ltl_token_type type;
    ltl_token_details details;
    _ltl_token *parent;
    union {
        _ltl_token *arg;    // unary operators
        struct {            // binary operators
            _ltl_token *left;
            _ltl_token *right;
        };
        char* id;           // literal ID
        int intvalue;       // integer value
        bool boolvalue;     // boolean value
        char* stringvalue;  // character sequence literal
    } info;
} ltl_token;

extern ltl_token *new_ltl_token(ltl_token *parent, ltl_token_details details);
extern void free_ltl_token(ltl_token *token);

typedef enum {
    DESCENDING = 1,  // 01
    ASCENDING = 2,   // 10
    LEAF_NODE = 3,   // 11
} visit_state;

extern bool visit_tokens_depth_first(ltl_token *token, bool (*visitor)(ltl_token *, visit_state, void *data), void *data);

extern bool is_proposition(ltl_token *token);
extern bool is_false(ltl_token *token);
extern bool is_true(ltl_token *token);

// returns true if the given list is either 
// - empty or 
// - all tokens in the list are TRUE
extern bool is_unconditional(linked_list* list);

// returns a new linked list containing only the propositions from the given list
extern linked_list propositions_from_ltl_token_list(linked_list *list);

// returns a new linked_list containing all until subformulas
extern linked_list untils_from_ltl_formula(ltl_token *token);

// negates a term (by returning a "not"-token which points to the given token)
extern ltl_token *negate_term(ltl_token *token);

// returns true iff token1 equals to token2
extern bool ltl_token_comparator(ltl_token *token1, ltl_token *token2);

// returns a string representation of the token and its sub-tokens
extern char *ltl_token2string(ltl_token *token, buffer *buf);

// returns a string representation of the token and its sub-tokens contained in the given list
extern char *ltl_token_list2string(linked_list *list, buffer *buf);

// returns a string representation of this token only (i.e. without any sub-tokens)
extern char *ltl_token_details2string(ltl_token *token, buffer *buf);

// returns the token and its sub-tokens in css syntax
extern char *ltl_token2css_syntax(ltl_token *token, buffer *buf);

#endif /*_LTLTOKEN_H_*/
