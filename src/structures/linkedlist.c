#include <stdlib.h>
#include <stdio.h>
#include "linkedlist.h"

LinkedListNode new_LinkedListNode(void* data) {
	LinkedListNode node = (LinkedListNode)malloc(sizeof(struct LinkedListNode));
	if (!node) return NULL;
	node->data = data;
	node->next = node->prev = NULL;
	return node;
}

LinkedList new_LinkedList() {
	LinkedList this = (LinkedList)malloc(sizeof(struct LinkedList));
	if (!this) return NULL;
	this->count = 0;
	this->head = new_LinkedListNode(NULL);
	this->tail = new_LinkedListNode(NULL);
	this->head->next = this->tail;
	this->tail->prev = this->head;
	return this;
}

void LinkedList_free(LinkedList list) {
	LinkedListNode node = list->head;
	while (node) {
		LinkedListNode tmp = node->next;
		free(node);
		node = tmp;
	}
	free(list);
}

void LinkedList_freeAll(LinkedList list, void (*free_fct)(void* ptr)) {
	LinkedListNode node = list->head;
	while (node) {
		LinkedListNode tmp = node->next;
		if(node->data) free_fct(node->data);
		free(node);
		node = tmp;
	}
	free(list);
}

void LinkedList_append(LinkedList list, void* data) {
	LinkedListNode node = new_LinkedListNode(data);
	node->prev = list->tail->prev;
	node->next = list->tail;
	list->tail->prev->next = node;
	list->tail->prev = node;
	list->count++;
}

void LinkedList_prepend(LinkedList list, void* data) {
	LinkedListNode node = new_LinkedListNode(data);
	node->next = list->head->next;
	node->prev = list->head;
	list->head->next->prev = node;
	list->head->next = node;
	list->count++;
}

LinkedListNode LinkedList_getfirst(LinkedList list) {
	if (!list->count) return NULL;
	return list->head->next;
}

LinkedListNode LinkedList_getlast(LinkedList list) {
	if (!list->count) return NULL;
	return list->tail->prev;
}

LinkedListNode LinkedList_pollfirst(LinkedList list) {
	if (list->count == 0) return NULL;
	LinkedListNode node = list->head->next;
	node->next->prev = list->head;
	list->head->next = node->next;
	list->count--;
	return node;
}

LinkedListNode LinkedList_polllast(LinkedList list) {
	if (list->count == 0) return NULL;
	LinkedListNode node = list->tail->prev;
	node->prev->next = list->tail;
	list->tail->prev = node->prev;
	list->count--;
	return node;
}

void LinkedList_remove(LinkedList list, LinkedListNode node) {
	node->prev->next = node->next;
	node->next->prev = node->prev;
	list->count--;
}

//Integer Linked List

IntLinkedListNode new_IntLinkedListNode(int data) {
	IntLinkedListNode node = (IntLinkedListNode)malloc(sizeof(struct IntLinkedListNode));
	if (!node) return NULL;
	node->data = data;
	node->next = node->prev = NULL;
	return node;
}

IntLinkedList new_IntLinkedList() {
	IntLinkedList this = (IntLinkedList)malloc(sizeof(struct IntLinkedList));
	if (!this) return NULL;
	this->count = 0;
	this->head = new_IntLinkedListNode(0);
	this->tail = new_IntLinkedListNode(0);
	this->head->next = this->tail;
	this->tail->prev = this->head;
	return this;
}

void IntLinkedList_free(IntLinkedList list) {
	IntLinkedListNode node = list->head;
	while (node) {
		IntLinkedListNode tmp = node->next;
		free(node);
		node = tmp;
	}
	free(list);
}

void IntLinkedList_append(IntLinkedList list, int data) {
	IntLinkedListNode node = new_IntLinkedListNode(data);
	node->prev = list->tail->prev;
	node->next = list->tail;
	list->tail->prev->next = node;
	list->tail->prev = node;
	list->count++;
}

void IntLinkedList_prepend(IntLinkedList list, int data) {
	IntLinkedListNode node = new_IntLinkedListNode(data);
	node->next = list->head->next;
	node->prev = list->head;
	list->head->next->prev = node;
	list->head->next = node;
	list->count++;
}

int IntLinkedList_getfirst(IntLinkedList list) {
	return list->head->next->data;
}

int IntLinkedList_getlast(IntLinkedList list) {
	return list->tail->prev->data;
}

int IntLinkedList_pollfirst(IntLinkedList list) {
	if (list->count == 0) return -1;
	IntLinkedListNode node = list->head->next;
	node->next->prev = list->head;
	list->head->next = node->next;
	list->count--;
	int result = node->data;
	free(node);
	return result;
}

int IntLinkedList_polllast(IntLinkedList list) {
	if (list->count == 0) return -1;
	IntLinkedListNode node = list->tail->prev;
	node->prev->next = list->tail;
	list->tail->prev = node->prev;
	list->count--;
	int result = node->data;
	free(node);
	return result;
}