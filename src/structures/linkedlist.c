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
	return list->head->next;
}

LinkedListNode LinkedList_getlast(LinkedList list) {
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