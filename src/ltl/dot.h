#ifndef _DOT_H_
#define _DOT_H_

#include "ltl-token.h"
#include "linked-list.h"

extern void ltl2dot(ltl_token *top, const char *filename);
extern void buchi2dot(linked_list *nodes, const char *filename);

#endif /*_DOT_H_*/
