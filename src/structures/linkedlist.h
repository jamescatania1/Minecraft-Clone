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


typedef struct IntLinkedListNode {
	struct LinkedListNode* prev;
	struct LinkedListNode* next;
	int data;
} *IntLinkedListNode;

typedef struct IntLinkedList {
	int count;
	IntLinkedListNode head;
	IntLinkedListNode tail;
} *IntLinkedList;

/*
 * Constructor for new linked list
 */
extern IntLinkedList new_IntLinkedList();

/*
 * Frees the list.
 */
extern void IntLinkedList_free(IntLinkedList list);

/*
 * Appends item to end. Runs in constant time.
 */
extern void IntLinkedList_append(IntLinkedList list, int data);

/*
 * Prepends item to start. Runs in constant time.
 */
extern void IntLinkedList_prepend(IntLinkedList list, int data);

/*
 * Gets the first item of the list.
 */
extern int IntLinkedList_getfirst(IntLinkedList list);

/*
 * Gets the last item of the list.
 */
extern int IntLinkedList_getlast(IntLinkedList list);

/*
 * Removes and returns the first item of the list.
 */
extern int IntLinkedList_pollfirst(IntLinkedList list);

/*
 * Removes and returns the last item of the list.
 */
extern int IntLinkedList_polllast(IntLinkedList list);
#endif