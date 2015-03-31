/*
 * $Id: buchi-converter.c,v 1.5 2012/06/16 09:07:35 nagatou Exp $
 */

#define EXTERN extern
#include "comm.h"
#include <assert.h>
#include "buchi-node.h"
#include "ltl-token.h"


char *buchi2ccs(linked_list *nodes, buffer* buf) {
    list_elem *e;
    
    for (e = nodes->first; e != NULL; e = e->next) {
        list_elem *i;
        buchi_node *n = (buchi_node *)e->data;
        buchi_node *else_node = NULL;
        linked_list conditional_nodes;
        init_new_linked_list(&conditional_nodes);

        for (i = n->outgoing.first; i != NULL; i = i->next) {
            buchi_node *ni = (buchi_node *)i->data;
            if (is_empty_list(&ni->required_propositions)
                || (ni->required_propositions.count == 1 
                    && is_true((ltl_token*)ni->required_propositions.first->data))) {
                assert(else_node == NULL);
                else_node = ni;
            } else {
                add_to_list(&conditional_nodes, ni);
            }
        }
        
        ins_buf(buf, "(define ");
        ins_buf(buf, n->annotation);
        ins_buf(buf, " () (");

        if (is_empty_list(&conditional_nodes)) {
            if (else_node != NULL) {
                if (else_node->is_accept_state) {
                    ins_buf(buf, "accept: ");
                }
                ins_buf(buf, else_node->annotation);
            }
            else {
//                ins_buf(buf, "STOP");
                ins_buf(buf, "accept:Abort");
            }
        }
        else  {
            int depth = 0;

            for (i = conditional_nodes.first; i != NULL; i = i->next, depth++) {
                list_elem *p;
                buchi_node *ni = (buchi_node *)i->data;

                ins_buf(buf, "if (");

                for (p = ni->required_propositions.first; p != NULL; p = p->next) {
                    ltl_token *t = (ltl_token *)p->data;
                    ltl_token2css_syntax(t, buf);
                    if (p->next != NULL) {
                        ins_buf(buf, " & ");
                    }
                }

                ins_buf(buf, ") (");
                if (ni->is_accept_state) {
                    ins_buf(buf, "accept: ");
                }
                ins_buf(buf, ni->annotation);
                ins_buf(buf, ") (");
            }

            if (else_node != NULL) {
                if (else_node->is_accept_state) {
                    ins_buf(buf, "accept: ");
                }
                ins_buf(buf, else_node->annotation);
            }
            else {
//                ins_buf(buf, "STOP");
                ins_buf(buf, "Abort");
            }
        
            for (; depth > 0; depth--) {
                ins_buf(buf, ")");
            }
        }

        ins_buf(buf, "))  ");
        clear_linked_list(&conditional_nodes);
    }
    
//    ins_buf(buf, "(define _MC_START () (" INIT_NODE_NAME "))");
        
    return buf->buf;
}
