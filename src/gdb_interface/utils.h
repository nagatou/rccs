#ifndef __UTILS_H__
#define __UTILS_H__
#include "mi_parser.h"

#define LIST_SIZE 25

struct _gdbmi_list {
   struct _gdbmi_result **element;
   int last;            /* position of last element */
   int count;           /* number of elements */
};

typedef struct _gdbmi_list GDBMI_List;

GDBMI_List *new_List();

void expand_List();

void free_List(GDBMI_List *list);

void myfree(void *ptr);

void *mymalloc(size_t size);

#endif
