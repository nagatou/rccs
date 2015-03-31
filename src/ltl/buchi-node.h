#ifndef _BUCHINODE_H_
#define _BUCHINODE_H_

#include "linked-list.h"
#include "ltl.h"
//#define INIT_NODE_NAME "Init"

typedef struct _buchi_node {
    char *annotation;
    linked_list incoming;
    linked_list old;
    linked_list new;
    linked_list next;

    // the next fields must be explicitly computed using
    // postprocess_buchi_nodes
    linked_list required_propositions;
    linked_list outgoing;
    bool is_accept_state;
} buchi_node;

extern buchi_node *new_buchi_node();
extern buchi_node *clone_buchi_node(buchi_node *org);
extern void postprocess_buchi_nodes(linked_list *nodes);
extern void free_buchi_node(buchi_node *node);
extern void free_buchi_nodes(linked_list *nodes);
extern char *buchi_node2string(buchi_node *node);
extern char *buchi_node2long_string(buchi_node *node, buffer *buf);

#endif /*_BUCHINODE_H_*/
