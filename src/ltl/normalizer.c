/*
 * $Id: normalizer.c,v 1.1.1.1 2011/05/18 06:25:23 nagatou Exp $
 */

#define EXTERN extern
#include "comm.h"
#include <assert.h>
#include "ltl-parser.h"


static ltl_token *rewrite_to_basic_ops(ltl_token *token) {
    switch (token->details) {
        case LTL_BRACKET: {     // we don't need no explicit brackets any more
            ltl_token *child = token->info.arg;
            free(token);
            return rewrite_to_basic_ops(child);
            }

        case LTL_GLOBALLY:      // G a  =>  ff R a
        case LTL_FUTURE: {      // F a  =>  tt U a
            ltl_token_details details = 
                (token->details == LTL_GLOBALLY ?  LTL_RELEASE : LTL_UNTIL);
            ltl_token *replacement = new_ltl_token(token->parent, details);
            ltl_token *rep_left = new_ltl_token(replacement, LTL_BOOLVALUE);
            // TRUE for LTL_FINALLY; FALSE for LTL_GLOBALLY
            rep_left->info.boolvalue = (token->details == LTL_FUTURE);
            replacement->info.left = rep_left;
            replacement->info.right = token->info.arg;
            free(token);
            return rewrite_to_basic_ops(replacement);
            }

        // case LTL_IMPLIES:       // a -> b  =>  !a \/ b

        default:
            break;
    }


    switch (token->type) {
        case LTL_UNARY_OP:
        case LTL_BRACKETS:
            token->info.arg = rewrite_to_basic_ops(token->info.arg);
            token->info.arg->parent = token;
            break;
        case LTL_BINARY_OP:
            token->info.left = rewrite_to_basic_ops(token->info.left);
            token->info.left->parent = token;
            token->info.right = rewrite_to_basic_ops(token->info.right);
            token->info.right->parent = token;
            break;
        default:
            break;
    }

    return token;
}


ltl_token *ltl_normalize(ltl_token *token) {
    token = rewrite_to_basic_ops(token);
    // pushdown_nots(token);
    return token;
}
