#ifndef _list_h
#define _list_h

typedef struct List {
	int activeCount;
	int count;
	int capacity;
	void** data;
}
*List;

/*
 * Constructor for new list with initial capacity of 10;
 */
extern List new_List();

/*
 * Frees the list.
 */
extern void List_free(List list);

/*
 * Calls the provided free function on all the remaining items on the list, then frees the list.
 */
extern void List_freeAll(List list, void (*free_fct)(void* ptr));

/*
 * Gets item at index.
 */
extern void* List_get(List list, int index);

/*
 * Inserts new item at index. Runs in linear time.
 */
extern void List_insert(List list, void* data, int index);

/*
 * Removes the item at the index.
 */
extern void List_remove(List list, int index);

/*
 * Sets item at index.
 */
extern void List_set(List list, void* data, int index);

/*
 * Adds item to end of list.
 */
extern void List_add(List list, void* data);

/*
 * Gets the first item of the list.
 */
extern void* List_getfirst(List list);

/*
 * Gets the last item of the list.
 */
extern void* List_getlast(List list);

#endif