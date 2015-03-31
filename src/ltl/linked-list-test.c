/*
 * $Id: linked-list-test.c,v 1.1.1.1 2011/05/18 06:25:23 nagatou Exp $
 */

#define EXTERN extern
#include "comm.h"
#include <assert.h>
#include "linked-list.h"

#ifdef LINKED_LIST_TEST
# define main_linked_list_test main
#endif

static char *c1 = "A";
static char *c2 = "B";
static char *c3 = "C";

static bool test_comparator(void *a1, void *a2) {
    return (a1 == a2);
}

int main_linked_list_test(int argi,char *argv[]) {
    linked_list lst1;
    linked_list lst2;
    init_new_linked_list(&lst1);
    init_new_linked_list(&lst2);
    
    assert(is_empty_list(&lst1) && is_empty_list(&lst2));
    add_to_list(&lst1, c1);
    add_to_list(&lst1, c2);
    add_to_list(&lst1, c3);
    assert(!is_empty_list(&lst1));
    assert(pop_first_element(&lst1) == c1);
    assert(pop_first_element(&lst1) == c2);
    assert(pop_first_element(&lst1) == c3);
    assert(pop_first_element(&lst1) == NULL);
    assert(is_empty_list(&lst1));

    add_to_list(&lst1, c1);
    add_to_list(&lst1, c2);
    add_to_list(&lst1, c3);
    add_all_to_list(&lst2, &lst1);
    assert(!is_empty_list(&lst1) && !is_empty_list(&lst2));
    assert(lists_have_equal_content(&lst2, &lst1, &test_comparator));
    assert(pop_first_element(&lst2) == c1);
    add_to_list(&lst2, c1);
    assert(lists_have_equal_content(&lst2, &lst1, &test_comparator));
    assert(pop_first_element(&lst2) == c2);
    add_to_list(&lst2, c1);
    assert(!lists_have_equal_content(&lst2, &lst1, &test_comparator));
    assert(pop_first_element(&lst2) == c3);
    assert(pop_first_element(&lst2) == c1);
    assert(pop_first_element(&lst2) == c1);
    assert(is_empty_list(&lst2));
    
    assert(search_in_list(&lst1, c3, &test_comparator) == c3);
    assert(search_in_list(&lst1, c2, &test_comparator) == c2);
    assert(search_in_list(&lst1, c1, &test_comparator) == c1);
    
    return 0;
}
