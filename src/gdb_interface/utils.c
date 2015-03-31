#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "utils.h"
#include "mi_parser.h"

GDBMI_List *
new_List() {
   int i = 0;
   GDBMI_List *new_list = (GDBMI_List *) mymalloc(sizeof(GDBMI_List));
   new_list->count = LIST_SIZE;
   new_list->last = 0;
   new_list->element = (gdbmi_result **) mymalloc(sizeof(gdbmi_result *) * LIST_SIZE);
   for (i = 0; i < LIST_SIZE; i++) {
      new_list->element[i] = (gdbmi_result *) mymalloc(sizeof(gdbmi_result));
   }   
   return new_list;
}

void
expand_List(GDBMI_List **list) {
   int i;
   GDBMI_List *l = *list;
   gdbmi_result **item = l->element;
   int old_count = l->count;
   l->count += 10;
   
   item = (gdbmi_result **) realloc(item, sizeof(gdbmi_result *) * l->count);
   for (i = old_count; i < l->count; i++) {
      item[i] = (gdbmi_result *) mymalloc(sizeof(gdbmi_result));
   }      
}

void
free_List(GDBMI_List *list) {
   int i;
   assert(list);
   for (i = 0; i < list->count; i++) {
      myfree(list->element[i]);
   }
   myfree(list);
}

void
myfree(void *ptr) {
	if (ptr != NULL) {
		//memset(ptr, 0, sizeof(ptr));
		free(ptr);
		ptr = NULL;
	}
}

void *
mymalloc(size_t size) {
	void *r = NULL;
	r = malloc(size);
	//TODO
	//if (!r)....
	memset(r, 0, size);
	return r;
}
