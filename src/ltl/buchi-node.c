/*
 * $Id: buchi-node.c,v 1.2 2012/06/16 09:07:35 nagatou Exp $
 */

#define EXTERN extern
#include "comm.h"
#include <assert.h>
#include "buchi-node.h"
#include "ltl-token.h"

buchi_node *new_buchi_node() {
    buchi_node *n = malloc(sizeof(buchi_node));
    // make sure everything is zero
    memset(n, '\0', sizeof(buchi_node));
    return n;
}

buchi_node *clone_buchi_node(buchi_node *org) {
    buchi_node *new = new_buchi_node();
    add_all_to_list(&new->incoming, &org->incoming);
    add_all_to_list(&new->old, &org->old);
    add_all_to_list(&new->new, &org->new);
    add_all_to_list(&new->next, &org->next);
    return new;
}

static int buchi_node_sort(void *d1, void *d2) {
    buchi_node *n1 = (buchi_node *)d1;
    buchi_node *n2 = (buchi_node *)d2;
    return n1->required_propositions.count - n2->required_propositions.count;
}

void postprocess_buchi_nodes(linked_list *nodes) {
    list_elem *e, *i;
    int counter = 1;
    
    for (e = nodes->first; e != NULL; e = e->next) {
        buchi_node *n = (buchi_node *)e->data;
        clear_linked_list(&n->outgoing);
        n->required_propositions = propositions_from_ltl_token_list(&n->old);
        if (n->annotation == NULL) {
            n->annotation = malloc(10*sizeof(char));
            snprintf(n->annotation, 10, "Node%04d", counter++);
        }
    }

    for (e = nodes->first; e != NULL; e = e->next) {
        buchi_node *n = (buchi_node *)e->data;
        for (i = n->incoming.first; i != NULL; i = i->next) {
            buchi_node *ni = (buchi_node *)i->data;
            // add_to_sorted_list(&ni->outgoing, n, &buchi_node_sort);
            if (!is_empty_list(&n->required_propositions))
               add_to_list(&ni->outgoing, n);
        }
    }
}

void free_buchi_node(buchi_node *node) {
    free(node->annotation);
    clear_linked_list(&node->incoming);
    clear_linked_list(&node->new);
    clear_linked_list(&node->next);
    clear_linked_list(&node->old);
    clear_linked_list(&node->outgoing);
    clear_linked_list(&node->required_propositions);
    free(node);
}

void free_buchi_nodes(linked_list *nodes) {
    buchi_node *n;
    while ((n = (buchi_node *)pop_first_element(nodes)) != NULL) {
        free_buchi_node(n);
    }
}

char *buchi_node2long_string(buchi_node *node, buffer *buf) {
    ins_buf(buf, (node->annotation == NULL ? "" : node->annotation));
    ins_buf(buf, "\\nnext={");
    ltl_token_list2string(&node->next, buf);
    ins_buf(buf, "}\\nold={");
    ltl_token_list2string(&node->old, buf);
    ins_buf(buf, "}");
    return buf->buf;
}

char *buchi_node2string(buchi_node *node) {
    return node->annotation;
}
