/*
 * $Id: ltl2dot.c,v 1.1.1.1 2011/05/18 06:25:23 nagatou Exp $
 */

#define EXTERN extern
#include "comm.h"
#include <assert.h>
#include "ltl-token.h"

static FILE *file;

static bool ltl2dotfile(ltl_token *t, visit_state s, void *data) {
    if (s == ASCENDING) return TRUE;
    
    buffer buf;
    initbuf(&buf);
    
    switch (t->type) {
        case LTL_SYMBOL:
            fprintf(file, "\"%i\" [label=\"%s\" style=filled,color=yellow];\n", (int)t, ltl_token_details2string(t, &buf));
            break;
        case LTL_UNARY_OP:
        case LTL_BRACKETS:
            fprintf(file, "\"%i\" [label=\"%s\" style=filled,color=green];\n", (int)t, ltl_token_details2string(t, &buf));
            fprintf(file, "\"%i\" -> \"%i\";\n", (int)t, (int)t->info.arg);
            break;
        case LTL_BINARY_OP:
            fprintf(file, "\"%i\" [label=\"%s\" style=filled,color=green];\n", (int)t, ltl_token_details2string(t, &buf));
            fprintf(file, "\"%i\" -> \"%i\";\n", (int)t, (int)t->info.left);
            fprintf(file, "\"%i\" -> \"%i\";\n", (int)t, (int)t->info.right);
            break;
        case LTL_UNKNOWN_TYPE:
            assert(FALSE);
    }
    
    return TRUE;
}

void ltl2dot(ltl_token *top, const char *filename) {
    file = fopen(filename, "w");
    fprintf(file, "digraph ltl {\nnode [fontname=Helvetica,fontsize=10];\n");
    visit_tokens_depth_first(top, &ltl2dotfile, NULL);
    fprintf(file, "}\n");
}
