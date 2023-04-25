#pragma warning( disable: 6386 )

#include <stdlib.h>
#include <stdio.h>
#include "list.h"

void List_resizedouble(List list);

List new_List() {
	List this = (List)malloc(sizeof(struct List));
	if (this == NULL) return NULL;

	this->capacity = 10;
	this->count = 0;
	this->activeCount = 0;
	this->data = (void**)malloc(this->capacity * sizeof(void*));
	for (int i = 0; i < this->capacity; i++) this->data[i] = NULL;
	return this;
}

void List_free(List list) {
	free(list->data);
	free(list);
}

void List_freeAll(List list, void (*free_fct)(void* ptr)) {
	for (int i = 0; i < list->count; i++) {
		if (List_get(list, i)) {
			free_fct(List_get(list, i));
		}
	}
	free(list->data);
	free(list);
}


void* List_get(List list, int index) {
	if (index >= list->capacity || index < 0) {
		printf("List error: index out of bounds\n");
		exit(EXIT_FAILURE);
	}
	if (list->data[index] == NULL) {
		return NULL;
	}
	return list->data[index];
} 

void List_insert(List list, void* data, int index) {
	if (list->count >= list->capacity) {
		List_resizedouble(list);
	}
	if (index >= list->count || index < 0) {
		printf("List error: index out of bounds\n");
		exit(EXIT_FAILURE);
	}
	for (int i = list->count - 1; i >= index; i--) {
		list->data[i + 1] = list->data[i];
		list->data[i] = NULL;
	}
	list->data[index] = data;
	list->count++;
	list->activeCount++;
}

void List_remove(List list, int index) {
	if (index >= list->capacity || index < 0) {
		printf("List error: index out of bounds\n");
		exit(EXIT_FAILURE);
	}
	if (list->data[index]) {
		list->data[index] = NULL;
		list->activeCount--;
	}
}

void List_set(List list, void* data, int index) {
	if (index > list->count || index < 0) {
		printf("List error: index out of bounds\n");
		exit(EXIT_FAILURE);
	}
	else if (index == list->count) {
		List_add(list, data);
		return;
	}
	if (!list->data[index]) list->activeCount++;
	list->data[index] = data;
}

void List_add(List list, void* data) {
	if (list->count >= list->capacity) {
		List_resizedouble(list);
	}
	list->data[list->count] = data;
	list->count++;
	list->activeCount++;
}

void* List_getfirst(List list) {
	if (list->count == 0) {
		printf("List error: list is empty\n");
		exit(EXIT_FAILURE);
	}
	return list->data[0];
}

void* List_getlast(List list) {
	if (list->count == 0) {
		printf("List error: list is empty\n");
		exit(EXIT_FAILURE);
	}
	return list->data[list->count - 1];
}

void List_resizedouble(List list) {
	void** newData = (void**)malloc(list->capacity * 2 * sizeof(void*));
	if (newData == NULL) {
		printf("List error: could not resize list to capacity %d\n", list->capacity);
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < list->capacity * 2; i++) {
		if (i < list->capacity) newData[i] = list->data[i];
		else newData[i] = NULL;
	}
	list->capacity *= 2;
	free(list->data);
	list->data = newData;
}