/*
 * $Id: linked-list.c,v 1.1.1.1 2011/05/18 06:25:23 nagatou Exp $
 */

#define EXTERN extern
#include "comm.h"
#include <assert.h>
#include "linked-list.h"

// helper functions

static list_elem *new_list_elem(void *data) {
    list_elem *e = malloc(sizeof(list_elem));
    e->data = data;
    e->prev = NULL;
    e->next = NULL;
    return e;
}

void remove_element_from_list(linked_list *list, list_elem *elem) {
    assert(elem != NULL);
    
    if (elem->prev == NULL) {
        assert(list->first == elem);
        list->first = elem->next;
    } else {
        assert(elem->prev->next == elem);
        elem->prev->next = elem->next;
    }
    if (elem->next == NULL) {
        assert(list->last == elem);
        list->last = elem->prev;
    } else {
        assert(elem->next->prev == elem);
        elem->next->prev = elem->prev;
    }

    list->count--;
    free(elem);
}

static void insert_element_in_list(linked_list *list, list_elem *prev_elem, list_elem *new_elem) {
    if ((list->first == NULL) || (list->last == NULL)) {  // first element in the list
        assert((list->first == NULL) 
            && (list->last == NULL) 
            && (list->count == 0) 
            && prev_elem == NULL);
        list->first = new_elem;
        list->last = new_elem;
    } else {
        if (prev_elem == NULL) {
            assert(list->last->next == NULL);
            prev_elem = list->last; // append at the end
        }

        new_elem->next = prev_elem->next;
        new_elem->prev = prev_elem;
        
        if (new_elem->next != NULL) {
            new_elem->next->prev = new_elem;
        } else {
            assert(list->last == new_elem->prev);   // add at the end
            list->last = new_elem;
        }
        
        if (new_elem->prev != NULL) {
            new_elem->prev->next = new_elem;
        } else {
            assert(list->first == new_elem->next);  // append at the beginning
            list->first = new_elem;
        }
    }

    list->count++;
}

// exported functions

void init_new_linked_list(linked_list *list) {
    list->first = NULL;
    list->last = NULL;
    list->count = 0;
}

bool is_empty_list(linked_list *list) {
    return list->count == 0;
}

void add_to_list(linked_list *list, void *data) {
    list_elem *elem = new_list_elem(data);
    insert_element_in_list(list, list->last, elem);
}

void add_to_sorted_list(linked_list *list, void *data, int (*compare_function)(void *, void *)) {
    list_elem *new_elem = new_list_elem(data);
    list_elem *e;
    for (e = list->first; (e != NULL) && (compare_function(e->data, data) < 0); e = e->next);
    if (e != NULL) e = e->prev;
    insert_element_in_list(list, e, new_elem);
}

void add_all_to_list(linked_list *target, linked_list *source) {
    list_elem *e;
    for (e = source->first; e != NULL; e = e->next) {
        add_to_list(target, e->data);
    }
}

void *pop_first_element(linked_list *list) {
    list_elem *elem = list->first;
    void *data = NULL;
    if (elem != NULL) {
        data = elem->data;
        remove_element_from_list(list, elem);
    }
    return data;
}

void *search_in_list(linked_list *list, void *refdata, bool (*search_function)(void *, void *)) {
    list_elem *e;
    for (e = list->first; e != NULL; e = e->next) {
        if (search_function(e->data, refdata)) {
            return e->data;
        }
    }
    return NULL;
}

bool lists_have_equal_content(linked_list *list1, linked_list *list2, bool (*compare_function)(void *, void *)) {
    list_elem *e1;
    list_elem *e2;
    linked_list tmp_lst;

    if (list1->count != list2->count)
        return FALSE;
    
    init_new_linked_list(&tmp_lst);
    add_all_to_list(&tmp_lst, list2);
    
    for (e1 = list1->first; e1 != NULL; e1 = e1->next) {
        bool found = FALSE;
        for (e2 = tmp_lst.first; e2 != NULL; e2 = e2->next) {
            if (compare_function(e2->data, e1->data)) {
                found = TRUE;
                remove_element_from_list(&tmp_lst, e2);
                break;
            }
        }
        if (!found) {
            clear_linked_list(&tmp_lst);
            return FALSE;
        }
    }
    
    // if we reach this point, we know that the lists have the same size and 
    // that each element in list1 also has a corresponding entry in list2.
    clear_linked_list(&tmp_lst);
    return TRUE;
}

bool remove_from_list(linked_list *list, void *data, bool (*search_function)(void *, void *)) {
    list_elem *e;
    for (e = list->first; e != NULL; e = e->next) {
        if (search_function(e->data, data)) {
            remove_element_from_list(list, e);
            return TRUE;
        }
    }
    return FALSE;
}

void clear_linked_list(linked_list *list) {
    while (list->first != NULL) {
        remove_element_from_list(list, list->first);
    }
    assert(list->count == 0);
}
