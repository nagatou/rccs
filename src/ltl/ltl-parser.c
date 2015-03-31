/*
 * $Id: ltl-parser.c,v 1.1.1.1 2011/05/18 06:25:23 nagatou Exp $
 */

#define EXTERN extern
#include "comm.h"
#include <assert.h>
#include "ltl-parser.h"


static ltl_token_type get_token_type(ltl_lexeme *lexeme) {
    switch (lexeme->type) {
        case ALL_PATH:
        case EXIST_PATH:
        case NOT_STATE_PROP:
        case NEXTTIME_PATH:
            return LTL_UNARY_OP;

        case UNTIL_PATH:
        case RELEASE_PATH:
        case AND_PATH:
        case OR_PATH:
        case AND_PROP:
        case OR_PROP:
        case IMPLIES_STATE_PROP:
        case FUNC_EQUALS:
        case FUNC_SMALLEREQUAL:
        case FUNC_SMALLER:
        case FUNC_LARGEREQUAL:
        case FUNC_LARGER:
        case FUNC_PLUS:
        case FUNC_MINUS:
            return LTL_BINARY_OP;

        case OPENBRACKET_PATH_TERM:
        case CLOSEBRACKET_PATH_TERM:
            return LTL_BRACKETS;

        case TERM_ID:
        case TERM_DIGITS:
        case TERM_BOOLEAN:
            return LTL_SYMBOL;

        default:
            return LTL_UNKNOWN_TYPE;
    }
}

static ltl_token_details get_token_details(ltl_lexeme *lexeme) {
    switch (lexeme->type) {
        case ALL_PATH:              return LTL_GLOBALLY;
        case EXIST_PATH:            return LTL_FUTURE;
        case NOT_STATE_PROP:        return LTL_NOT;
        case UNTIL_PATH:            return LTL_UNTIL;
        case RELEASE_PATH:          return LTL_RELEASE;
        case NEXTTIME_PATH:         return LTL_NEXTTIME;
        case AND_PATH:              return LTL_PATH_AND;
        case OR_PATH:               return LTL_PATH_OR;
        case AND_PROP:              return LTL_PROP_AND;
        case OR_PROP:               return LTL_PROP_OR;
        case IMPLIES_STATE_PROP:    return LTL_IMPLIES;
        case FUNC_EQUALS:           return LTL_EQUALS;
        case FUNC_SMALLEREQUAL:     return LTL_SMALLEREQUAL;
        case FUNC_SMALLER:          return LTL_SMALLER;
        case FUNC_LARGEREQUAL:      return LTL_LARGEREQUAL;
        case FUNC_LARGER:           return LTL_LARGER;
        case FUNC_PLUS:             return LTL_PLUS;
        case FUNC_MINUS:            return LTL_MINUS;
        case OPENBRACKET_PATH_TERM: return LTL_BRACKET;
        case TERM_ID:               return LTL_ID;
        case TERM_DIGITS:           return LTL_INTVALUE;
        case TERM_BOOLEAN:          return LTL_BOOLVALUE;

        case CLOSEBRACKET_PATH_TERM:
            printf("syntax error: unmatched closing bracket\n");
            return LTL_UNKNOWN_DETAIL;
        case EOT:
            printf("syntax error: reached unexpected end\n");
            return LTL_UNKNOWN_DETAIL;
        default: 
            printf("syntax error: invalid symbol: %s\n", lexeme->lit);
            return LTL_UNKNOWN_DETAIL;
    }
}

static ltl_token *new_ltl_token_from_lexeme(ltl_lexeme *lexeme, ltl_token *parent) {
    ltl_token *t = malloc(sizeof(ltl_token));
    // make sure everything is zero
    memset(t, '\0', sizeof(ltl_token));
    t->parent = parent;
    t->type = get_token_type(lexeme);
    t->details = get_token_details(lexeme);
    if (t->type == LTL_SYMBOL) {
        // fill the token with values from the lexeme
        if (t->details == LTL_ID) {
            t->info.id = strdup(lexeme->lit);
        } else if (t->details == LTL_INTVALUE) {
            t->info.intvalue = lexeme->info.termint;
        } else if (t->details == LTL_BOOLVALUE) {
            t->info.boolvalue = lexeme->info.termbool;
        }
    }
    return t;
}


static void climb_up(ltl_parser_state *state) {
    state->last = state->last->parent;
}


void ltl_parse(ltl_parser_state *state, ltl_lexeme *lexeme) {
    if (state->last != NULL) {
        switch (state->last->type) {
            case LTL_SYMBOL:
                // a symbol doesn't have any children, so go up in the tree
                climb_up(state);
                ltl_parse(state, lexeme);
                break;
                
            case LTL_UNARY_OP:
                // check if we already have an argument for this operation
                if (state->last->info.arg != NULL) {
                    // token is full, go up
                    climb_up(state);
                    ltl_parse(state, lexeme);
                } else {
                    // argument is not present, fill it
                    ltl_token *t = new_ltl_token_from_lexeme(lexeme, state->last);
                    state->last->info.arg = t;
                    // and set it as the last token processed
                    state->last = t;
                }
                break;

            case LTL_BINARY_OP:
                // in this case we need to check if right operand is present
                if (state->last->info.right != NULL) {
                    // token is full, go up.
                    climb_up(state);
                    ltl_parse(state, lexeme);
                } else {
                    // right operand is not present, fill it
                    ltl_token *t = new_ltl_token_from_lexeme(lexeme, state->last);
                    state->last->info.right = t;
                    // and set it as the last token processed
                    state->last = t;
                }
                break;

            case LTL_BRACKETS:
                // current state is a bracket, if it has an argument it means 
                // that we are climbing up the tree again
                if (state->last->info.arg != NULL) {
                    // yes, so we need a closing bracket now
                    if (get_token_type(lexeme) == LTL_BINARY_OP) {
                        // insert the binary operation
                        ltl_token *t = new_ltl_token_from_lexeme(lexeme, state->last);
                        t->info.left = state->last->info.arg;
                        state->last->info.arg = t;
                        state->last = t;
                    } else if (lexeme->type == CLOSEBRACKET_PATH_TERM) {
                        // ok, we found our closing bracket, climb up one more
                        climb_up(state);
                    } else {
                        printf("syntax error: unmatched opening bracket\n");
                        exit(1); // TODO check error handling
                    }
                } else {
                    // argument is not present, fill it
                    ltl_token *t = new_ltl_token_from_lexeme(lexeme, state->last);
                    state->last->info.arg = t;
                    // and set it as the last token processed
                    state->last = t;
                }
                break;
            
            case LTL_UNKNOWN_DETAIL:
                printf("unexpected token\n");
                exit(1); // TODO check error handling
        }
    }
    else { 
        // this is the top of the tree...
        if (lexeme->type != EOT) {   // are we done?
            ltl_token *t = new_ltl_token_from_lexeme(lexeme, NULL);
            if (state->top == NULL && 
                (t->type == LTL_UNARY_OP || 
                 t->type == LTL_SYMBOL || 
                 t->type == LTL_BRACKETS)) {
                // this is the first lexeme we read
                state->top = state->last = t;
            }
            else if ((state->top != NULL) && (t->type == LTL_BINARY_OP)) {
                t->info.left = state->top;
                state->top = state->last = t;
            }
            else {
                printf("syntax error: unexpected symbol: %s\n", lexeme->lit);
                exit(1); // TODO check error handling
            }
        }
    }
}

