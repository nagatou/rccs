/*
 * $Id: ltl-token.c,v 1.1.1.1 2011/05/18 06:25:24 nagatou Exp $
 */

#define EXTERN extern
#include "comm.h"
#include <assert.h>
#include "ltl-token.h"

static ltl_token_type get_token_type_from_details(ltl_token_details details) {
    if (details < 0) {
        printf("strange id for details: %i", details);
        return LTL_UNKNOWN_TYPE;
    } else if (details < 100) {
        return LTL_SYMBOL;
    } else if (details < 200) {
        return LTL_UNARY_OP;
    } else if (details < 300) {
        return LTL_BINARY_OP;
    } else if (details < 400) {
        return LTL_BRACKETS;
    } else {
        printf("strange id for details: %i", details);
        return LTL_UNKNOWN_TYPE;
    }
}

ltl_token *new_ltl_token(ltl_token *parent, ltl_token_details details) {
    ltl_token *t = malloc(sizeof(ltl_token));
    // make sure everything is zero
    memset(t, '\0', sizeof(ltl_token));
    t->parent = parent;
    t->type = get_token_type_from_details(details);
    t->details = details;
    return t;
}

bool visit_tokens_depth_first(ltl_token *token, bool (*visitor)(ltl_token *, visit_state, void *data), void *data) {
    if (token == NULL) {
        return FALSE;
    }
    
    switch (token->type) {
        case LTL_UNARY_OP:
        case LTL_BRACKETS:
            if (!visitor(token, DESCENDING, data))
                return FALSE;
            if (!visit_tokens_depth_first(token->info.arg, visitor, data))
                return FALSE;
            return visitor(token, ASCENDING, data);
        case LTL_BINARY_OP:
            if (!visitor(token, DESCENDING, data))
                return FALSE;
            if (!visit_tokens_depth_first(token->info.left, visitor, data))
                return FALSE;
            if (!visit_tokens_depth_first(token->info.right, visitor, data))
                return FALSE;
            return visitor(token, ASCENDING, data);
        default:
            return visitor(token, LEAF_NODE, data);
    }
}

static bool do_free_ltl_token(ltl_token *token, visit_state s, void *data) {
    if (s == DESCENDING) return TRUE; // only interested in leaf and ascending
    
    if (token->details == LTL_ID) {
        free(token->info.id);
    }
    free(token);
#ifdef LTL_TEST
    printf(".");
#endif
    return TRUE;
}

void free_ltl_token(ltl_token *token) {
    assert(visit_tokens_depth_first(token, &do_free_ltl_token, NULL));
}

bool ltl_token_comparator(ltl_token *token1, ltl_token *token2) {
    if (token1 == NULL || token2 == NULL) {
        return FALSE;
    }
    if (token1 == token2) {
        return TRUE;
    }
    
    if ((token1->type == token2->type) && (token1->details == token2->details)) {
        switch (token1->type) {
            case LTL_SYMBOL:
                if (token1->details == LTL_ID) {
                    return strcmp(token1->info.id, token2->info.id) == 0;
                } else if (token1->details == LTL_INTVALUE) {
                    return token1->info.intvalue == token2->info.intvalue;
                } else if (token1->details == LTL_BOOLVALUE) {
                    return token1->info.boolvalue == token2->info.boolvalue;
                } else {
                    assert(FALSE);
            }
            
        case LTL_UNARY_OP:
        case LTL_BRACKETS:
            return ltl_token_comparator(token1->info.arg, token2->info.arg);
            
        case LTL_BINARY_OP:
            return ltl_token_comparator(token1->info.left, token2->info.left) 
                && ltl_token_comparator(token1->info.right, token2->info.right);
        
        default:
            assert(FALSE);
        }
    }

    return FALSE;
}

bool is_proposition(ltl_token *token) {
    if (token->details == LTL_GLOBALLY
        || token->details == LTL_FUTURE
        || token->details == LTL_NEXTTIME
        || token->details == LTL_UNTIL
        || token->details == LTL_RELEASE
        || token->details == LTL_PATH_AND
        || token->details == LTL_PATH_OR
        || token->details == LTL_UNKNOWN_DETAIL)
        return FALSE;

    switch (token->type) {
        case LTL_SYMBOL:
            return TRUE;

        case LTL_UNARY_OP:
        case LTL_BRACKETS:
            return is_proposition(token->info.arg);

        case LTL_BINARY_OP:
            return is_proposition(token->info.left)
                && is_proposition(token->info.right);

        default:
            return FALSE;
    }
}

linked_list propositions_from_ltl_token_list(linked_list *list) {
    linked_list rl;
    list_elem *e;
    init_new_linked_list(&rl);
    for (e = list->first; e != NULL; e = e->next) {
        if (is_proposition((ltl_token *)e->data))
            add_to_list(&rl, e->data);
    }
    return rl;
}

static bool _untils_from_ltl_formula_visitor(ltl_token *token, visit_state s, void *data) {
    if (s != DESCENDING) return TRUE; // only interested in descending
    if (token->details == LTL_UNTIL) {
        add_to_list((linked_list *)data, token);
    }
    return TRUE;
}

linked_list untils_from_ltl_formula(ltl_token *token) {
    linked_list rl;
    init_new_linked_list(&rl);
    visit_tokens_depth_first(token, _untils_from_ltl_formula_visitor, &rl);
    return rl;
}

bool is_true(ltl_token *token) {
    if (token->details == LTL_BRACKETS)         // ( tt )
        return is_true(token->info.arg);
    if (token->details == LTL_NOT)              // ! ff
        return is_false(token->info.arg);
    return (token->details == LTL_BOOLVALUE)
        && (token->info.boolvalue == TRUE);     // tt
}

bool is_false(ltl_token *token) {
    if (token->details == LTL_BRACKETS)         // ( ff )
        return is_false(token->info.arg);
    if (token->details == LTL_NOT)              // ! tt
        return is_true(token->info.arg);
    return (token->details == LTL_BOOLVALUE)
        && (token->info.boolvalue == FALSE);    // ff
}

bool is_unconditional(linked_list* list) {
    list_elem *e;
    for (e = list->first; e != NULL; e = e->next) {
        ltl_token *t = (ltl_token *)e->data;
        if (!is_true(t))
            return FALSE;
    }
    return TRUE;
}

ltl_token *negate_term(ltl_token *token) {
    ltl_token *neg = new_ltl_token(NULL, LTL_NOT);
    neg->info.arg = token;
    return neg;
}

// to string

static char *_ltl_token2string(ltl_token *token, buffer *buf, char *(*ltl_token_details2string_func)(ltl_token *, buffer *)) {
    if (token == NULL) {
        ins_buf(buf, "null");
        return buf->buf;
    }
    
    switch (token->type) {
        case LTL_SYMBOL:
            ltl_token_details2string_func(token, buf);
            break;

        case LTL_UNARY_OP: {
            ltl_token_details2string_func(token, buf);
            ins_buf(buf, " (");
            _ltl_token2string(token->info.arg, buf, ltl_token_details2string_func);
            ins_buf(buf, ")");
            break; }

        case LTL_BINARY_OP: {
            ins_buf(buf, "(");
            _ltl_token2string(token->info.left, buf, ltl_token_details2string_func);
            ins_buf(buf, ") ");
            ltl_token_details2string_func(token, buf);
            ins_buf(buf, " (");
            _ltl_token2string(token->info.right, buf, ltl_token_details2string_func);
            ins_buf(buf, ")");
            break; }

        case LTL_BRACKETS: {
            ins_buf(buf, "(");
            _ltl_token2string(token->info.arg, buf, ltl_token_details2string_func);
            ins_buf(buf, ")");
            break; }

        default:
            ins_buf(buf, "?");
            break;
    }
 
    return buf->buf;
}

char *ltl_token_details2string(ltl_token *token, buffer *buf) {
    if (token->details == LTL_ID) {
        ins_buf(buf, token->info.id);
    } else if (token->details == LTL_INTVALUE) {
        char tmp[20];
        snprintf(tmp, 20, "%i", token->info.intvalue);
        ins_buf(buf, tmp);
    } else if (token->details == LTL_BOOLVALUE) {
        ins_buf(buf, (token->info.boolvalue == TRUE ? "tt" : "ff"));
    } else if (token->details == LTL_STRINGVALUE) {
        ins_buf(buf, token->info.id);
    } else {
        char *s;
        switch (token->details) {
            case LTL_GLOBALLY:      s = "A";   break;
            case LTL_FUTURE:        s = "F";   break;
            case LTL_NOT:           s = "!";   break;
            case LTL_NEXTTIME:      s = "X";   break;
            case LTL_UNTIL:         s = "U";   break;
            case LTL_RELEASE:       s = "R";   break;
            case LTL_PATH_AND:      s = "AND"; break;
            case LTL_PATH_OR:       s = "OR";  break;
            case LTL_PROP_AND:      s = "&";   break;
            case LTL_PROP_OR:       s = "|";   break;
            case LTL_IMPLIES:       s = "->";  break;
            case LTL_EQUALS:        s = "=";   break;
            case LTL_SMALLEREQUAL:  s = "<=";  break;
            case LTL_SMALLER:       s = "<";   break;
            case LTL_LARGEREQUAL:   s = ">=";  break;
            case LTL_LARGER:        s = ">";   break;
            case LTL_PLUS:          s = "+";   break;
            case LTL_MINUS:         s = "-";   break;
            case LTL_BRACKET:       s = "()";  break;
            default: assert(FALSE);
        }
        ins_buf(buf, s);
    }
    return buf->buf;
}

static char *ltl_token_details2css(ltl_token *token, buffer *buf) {
    if (token->details == LTL_ID) {
        ins_buf(buf, token->info.id);
    } else if (token->details == LTL_INTVALUE) {
        char tmp[20];
        snprintf(tmp, 20, "%i", token->info.intvalue);
        ins_buf(buf, tmp);
    } else if (token->details == LTL_BOOLVALUE) {
        ins_buf(buf, (token->info.boolvalue == TRUE ? "TRUE" : "FALSE"));
    } else if (token->details == LTL_STRINGVALUE) {
        ins_buf(buf, token->info.id);
    } else {
        char *s;
        switch (token->details) {
            case LTL_ID:            s = "%s";  break;
            case LTL_INTVALUE:      s = "%i";  break;
            case LTL_BOOLVALUE:     s = "%s";  break;
            case LTL_NOT:           s = "!";   break;
            case LTL_PROP_AND:      s = "&";   break;
            case LTL_PROP_OR:       s = "|";   break;
            case LTL_EQUALS:        s = "=";   break;
            case LTL_SMALLEREQUAL:  s = "<=";  break;
            case LTL_SMALLER:       s = "<";   break;
            case LTL_LARGEREQUAL:   s = ">=";  break;
            case LTL_LARGER:        s = ">";   break;
            case LTL_PLUS:          s = "+";   break;
            case LTL_MINUS:         s = "-";   break;
            default: assert(FALSE);
        }
        ins_buf(buf, s);
    }
    return buf->buf;
}

char *ltl_token2string(ltl_token *token, buffer *buf) {
    return _ltl_token2string(token, buf, &ltl_token_details2string);
}

char *ltl_token2css_syntax(ltl_token *token, buffer *buf) {
    return _ltl_token2string(token, buf, &ltl_token_details2css);
}

char *ltl_token_list2string(linked_list *list, buffer *buf) {
    list_elem *e;
    for (e = list->first; e != NULL; e = e->next) {
        ltl_token2string((ltl_token *)e->data, buf);
        if (e->next != NULL)
            ins_buf(buf, ", ");
    }
    return buf->buf;
}
