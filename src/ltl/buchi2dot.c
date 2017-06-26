/*
 * $Id: buchi2dot.c,v 1.1.1.1 2011/05/18 06:25:23 nagatou Exp $
 */

#define EXTERN extern
#include "comm.h"
#include <assert.h>
#include "ltl-token.h"
#include "buchi-node.h"


void buchi2dot(linked_list *nodes, const char *filename) {
    list_elem *e, *i;
    FILE *file = fopen(filename, "w");
    fprintf(file, "digraph ltl {\nnode [fontname=Helvetica,fontsize=10];\n");
    
    for (e = nodes->first; e != NULL; e = e->next) {
        buffer buf;
        buchi_node *n = (buchi_node *)e->data;
        linked_list prop_list;

        initbuf(&buf);
        fprintf(file, "\"%zi\" [label=\"%s\" style=filled,color=%s];\n",
            // (int)n, buchi_node2long_string(n, &buf),
            (intptr_t)n, buchi_node2string(n),
            (n->is_accept_state ? "green" : "red"));

        initbuf(&buf);
        prop_list = propositions_from_ltl_token_list(&n->old);
        ltl_token_list2string(&prop_list, &buf);
        clear_linked_list(&prop_list);
        
        for (i = n->incoming.first; i != NULL; i = i->next) {
            fprintf(file, "\"%zi\" -> \"%zi\" [label=\"%s\"];\n",
                (intptr_t)i->data, (intptr_t)n, buf.buf);
        }
    }
    
    fprintf(file, "}\n");
}
