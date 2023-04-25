#ifndef _linkedlist_h
#define _linkedlist_h

typedef struct LinkedListNode {
	struct LinkedListNode* prev;
	struct LinkedListNode* next;
	void* data;
} *LinkedListNode;

typedef struct LinkedList {
	int count;
	LinkedListNode head;
	LinkedListNode tail;
} *LinkedList;

/*
 * Constructor for new linked list
 */
extern LinkedList new_LinkedList();

/*
 * Frees the list.
 */
extern void LinkedList_free(LinkedList list);

/*
 * Calls the provided free function on all the remaining items on the list, then frees the list.
 */
extern void LinkedList_freeAll(LinkedList list, void (*free_fct)(void* ptr));

/*
 * Appends item to end. Runs in constant time.
 */
extern void LinkedList_append(LinkedList list, void* data);

/*
 * Prepends item to start. Runs in constant time.
 */
extern void LinkedList_prepend(LinkedList list, void* data);

/*
 * Gets the first item of the list.
 */
extern LinkedListNode LinkedList_getfirst(LinkedList list);

/*
 * Gets the last item of the list.
 */
extern LinkedListNode LinkedList_getlast(LinkedList list);

/*
 * Removes and returns the first item of the list.
 */
extern LinkedListNode LinkedList_pollfirst(LinkedList list);

/*
 * Removes and returns the last item of the list.
 */
extern LinkedListNode LinkedList_polllast(LinkedList list);

#endif