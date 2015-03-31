#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

#ifndef EXTERN
#define EXTERN extern
#endif
#include "comm.h"

typedef struct _list_elem _list_elem;

typedef struct _list_elem {
    void *data;
    _list_elem *prev;
    _list_elem *next;
} list_elem;

typedef struct _linked_list {
    list_elem *first;
    list_elem *last;
    int count;
} linked_list;

extern void init_new_linked_list(linked_list *list);
extern void add_to_list(linked_list *list, void *data);
extern void add_to_sorted_list(linked_list *list, void *data, int (*compare_function)(void *, void *));
extern void add_all_to_list(linked_list *target, linked_list *source);
extern bool is_empty_list(linked_list *list);

// returns the first element in the list which matches some search creteria
// which is defined by the search_function. This function will be called for
// every element in the list (args: list-element, refdata) and if this 
// function returns true, the element will be returned.
extern void *search_in_list(linked_list *list, void *refdata, bool (*search_function)(void *, void *));

// returns true iff both lists contain the same elements (not necessarily in 
// the same order). To compare the elements in the list, compare_function is 
// called. It should return true if both elements are the same.
extern bool lists_have_equal_content(linked_list *list1, linked_list *list2, bool (*compare_function)(void *, void *));

// pops the first element from the list
extern void *pop_first_element(linked_list *list);

// removes an element from the list
extern bool remove_from_list(linked_list *list, void *data, bool (*search_function)(void *, void *));
extern void remove_element_from_list(linked_list *list, list_elem *elem);

// removes all elements from the given list
extern void clear_linked_list(linked_list *list);

#endif /*_LINKEDLIST_H_*/
