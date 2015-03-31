/*
 * $Id: ltl-converter.c,v 1.4 2012/06/16 09:07:35 nagatou Exp $
 */

#define EXTERN extern
#include "comm.h"
#include <assert.h>
#include "ltl-converter.h"


/* helper functions */

static bool void_ltl_token_comparator(void *t1, void *t2) {
    return ltl_token_comparator((ltl_token *)t1, (ltl_token *)t2);
}

static bool void_pointer_comparator(void *p1, void *p2) {
    return p1 == p2;
}

static bool completed_nodes_comparator(void *n1, void *n2) {
    buchi_node *node1 = (buchi_node *)n1;
    buchi_node *node2 = (buchi_node *)n2;
    if (lists_have_equal_content(&node1->old, &node2->old, &void_ltl_token_comparator) 
        && lists_have_equal_content(&node1->next, &node2->next, &void_ltl_token_comparator))
        return TRUE;
    else
        return FALSE;    
}

/* buch automaton creation functions */

static void expand_node(buchi_node *n, linked_list *completed_nodes) {
#ifdef LTL_CONVERTER_DEBUG
    printf("expand_node 0x%X\n", (int)n);
#endif
    if (is_empty_list(&n->new)) {
#ifdef LTL_CONVERTER_DEBUG
        printf(" - new is empty...\n");
#endif
        buchi_node *ex = search_in_list(completed_nodes, n, &completed_nodes_comparator);
        if (ex != NULL) {
            add_all_to_list(&ex->incoming, &n->incoming);
#ifdef LTL_CONVERTER_DEBUG
    printf("  - free_buchi_node(0x%X)\n", (int)n);
#endif
            free_buchi_node(n);
        } else {
            buchi_node *n2 = new_buchi_node();
            add_to_list(&n2->incoming, n);
            add_all_to_list(&n2->new, &n->next);
            add_to_list(completed_nodes, n);
            expand_node(n2, completed_nodes);
        }
    } 
    else {
        ltl_token *t = (ltl_token *)pop_first_element(&n->new);
#ifdef LTL_CONVERTER_DEBUG
        buffer buf;
        initbuf(&buf);
        printf(" - processing token: %s\n", ltl_token2string(t, &buf));
#endif
        
        if (search_in_list(&n->old, t, &void_ltl_token_comparator) != NULL) {
#ifdef LTL_CONVERTER_DEBUG
            printf("  - token is in old\n");
#endif
            expand_node(n, completed_nodes);
        
        } else if (is_proposition(t)) {
#ifdef LTL_CONVERTER_DEBUG
            printf("  - token is a proposition\n");
#endif
            if (is_false(t)) {
#ifdef LTL_CONVERTER_DEBUG
            printf("   - token is false\n");
#endif
                return;
            }

            {   ltl_token *neg = negate_term(t);
                if (search_in_list(&n->old, neg, &void_ltl_token_comparator) != NULL) {
                    // TODO free_ltl_token(neg);
                    free(neg);
#ifdef LTL_CONVERTER_DEBUG
            printf("   - negative token is in old\n");
#endif
                    return;
                }
                // TODO free_ltl_token(neg);
                free(neg); }
            
            add_to_list(&n->old, t);
            expand_node(n, completed_nodes);
            
        } else if (t->details == LTL_UNTIL) {
            buchi_node *n2 = clone_buchi_node(n);

#ifdef LTL_CONVERTER_DEBUG
            printf("  - token is UNTIL statement\n");
#endif
            add_to_list(&n->old, t);
            add_to_list(&n->new, t->info.left);
            add_to_list(&n->next, t);

            add_to_list(&n2->old, t);
            add_to_list(&n2->new, t->info.right);

            expand_node(n, completed_nodes);
            expand_node(n2, completed_nodes);

        } else if (t->details == LTL_RELEASE) {
            buchi_node *n2 = clone_buchi_node(n);

#ifdef LTL_CONVERTER_DEBUG
            printf("  - token is RELEASE statement\n");
#endif
            add_to_list(&n->old, t);  /*** q1 ***/
            add_to_list(&n->new, t->info.right);
            add_to_list(&n->next, t);

            add_to_list(&n2->old, t);  /*** q2 ***/
            add_to_list(&n2->new, t->info.right);
            add_to_list(&n2->new, t->info.left);

            expand_node(n, completed_nodes);
            expand_node(n2, completed_nodes);

        } else if (t->details == LTL_PATH_OR) {
            buchi_node *n2 = clone_buchi_node(n);

#ifdef LTL_CONVERTER_DEBUG
            printf("  - token is PATH_OR statement\n");
#endif            
            add_to_list(&n->old, t);
            add_to_list(&n->new, t->info.left);

            add_to_list(&n2->old, t);
            add_to_list(&n2->new, t->info.right);

            expand_node(n, completed_nodes);
            expand_node(n2, completed_nodes);

        } else if (t->details == LTL_PATH_AND) {
#ifdef LTL_CONVERTER_DEBUG
            printf("  - token is PATH_AND statement\n");
#endif
            add_to_list(&n->old, t);
            add_to_list(&n->new, t->info.left);
            add_to_list(&n->new, t->info.right);
            expand_node(n, completed_nodes);                

        } else if (t->details == LTL_NEXTTIME) {
#ifdef LTL_CONVERTER_DEBUG
            printf("  - token is NEXTTIME statement\n");
#endif
            add_to_list(&n->old, t);
            add_to_list(&n->next, t->info.arg);
            expand_node(n, completed_nodes);                
            
        } else {
#ifdef LTL_CONVERTER_DEBUG
            printf("  - ERROR: unkown converter case\n");
#endif
//            assert(FALSE);
            error(FATAL,"unkown converter case(expand_node167)\n");
        }
    }
}

static void calc_accept_states(linked_list *nodes, ltl_token *top) {
    list_elem *e, *i;
    linked_list us = untils_from_ltl_formula(top);

    for (e = nodes->first; e != NULL; e = e->next) {
        buchi_node *n = (buchi_node *)e->data;
        n->is_accept_state = TRUE;
    }

    for (e = nodes->first; e != NULL; e = e->next) {
        buchi_node *n = (buchi_node *)e->data;
        if (n->is_accept_state) {
            for (i = us.first; i != NULL; i = i->next) {
                ltl_token *t = (ltl_token *)i->data;

#ifdef LTL_CONVERTER_DEBUG
                buffer buf;
                initbuf(&buf);
                printf("processing token for accept: %s\n", ltl_token2string(t, &buf));
#endif
                
                if (search_in_list(&n->old, t->info.right, &void_ltl_token_comparator) == NULL) {
                    if (search_in_list(&n->old, t, &void_ltl_token_comparator) != NULL) {
                        n->is_accept_state = FALSE;
                    }
                }
            }
        }
    }
    
    clear_linked_list(&us);
}

static void merge_nodes(buchi_node *from, buchi_node *into) {
    list_elem *o;
    
    for (o = from->outgoing.first; o != NULL; o = o->next) {
        buchi_node *aon = (buchi_node *)o->data;
        if (aon == from)
            aon = into;  // add self reference
        
        if (search_in_list(&into->outgoing, aon, &void_pointer_comparator) == NULL) {
            add_to_list(&into->outgoing, aon);
            add_to_list(&aon->incoming, into);
        }
    }
}


static void compress_automaton(linked_list *nodes) {
    list_elem *e = nodes->first;
    
    while(e != NULL) {
        buchi_node *n = (buchi_node *)e->data;
        list_elem *cur = e; // e is free'd in remove_element_from_list
        e = e->next;

        if (is_unconditional(&n->required_propositions)) {
            list_elem *i;
            printf("unconditional node: %s\n", n->annotation);
            
            for (i = n->incoming.first; i != NULL; i = i->next) {
                buchi_node *in = (buchi_node *)i->data;
                if (n != in) {
                    assert(remove_from_list(&in->outgoing, n, &void_pointer_comparator));
                    merge_nodes(n, in);
                }
            }

            for (i = n->outgoing.first; i != NULL; i = i->next) {
                buchi_node *out = (buchi_node *)i->data;
                if (n != out) {
                    assert(remove_from_list(&out->incoming, n, &void_pointer_comparator));
                }
            }
            
            remove_element_from_list(nodes, cur);
            free_buchi_node(n);
        }
    }
}

/* exported function */

linked_list ltl2buchi(ltl_token *top) {
    buchi_node *init = new_buchi_node();
    init->annotation = strdup(INIT_NODE_NAME);

    buchi_node *n = new_buchi_node();
    add_to_list(&n->incoming, init);
    add_to_list(&n->new, top);

    linked_list completed_nodes;
    init_new_linked_list(&completed_nodes);
    expand_node(n, &completed_nodes);
    postprocess_buchi_nodes(&completed_nodes);
    calc_accept_states(&completed_nodes, top);
//    compress_automaton(&completed_nodes);

    // init has to be added last, it has no incoming nodes
    add_to_list(&completed_nodes, init);

    return completed_nodes;
}
