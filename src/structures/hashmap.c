#pragma warning( disable: 6385 )
#pragma warning( disable: 6386 )

#include <stdlib.h>
#include <stdio.h>
#include "hashmap.h"

HashMapNode new_HashMapNode(int key, void* value) {
	HashMapNode this = (HashMapNode)malloc(sizeof(struct HashMapNode));
	if (!this) return NULL;

	this->key = key;
	this->value = value;
	this->next = NULL;
}

HashMap new_HashMap(int capacity) {
	HashMap this = (HashMap)malloc(sizeof(struct HashMap));
	if (!this) return NULL;

	this->count = 0;
	this->capacity = capacity;
	this->buckets = (struct HashMapNode**)malloc(capacity * sizeof(HashMapNode));
	for (int i = 0; i < capacity; i++) this->buckets[i] = NULL;

	return this;
}

void HashMap_free(HashMap map) {
	for (int i = 0; i < map->capacity; i++) {
		HashMapNode node = map->buckets[i];
		while (node) {
			HashMapNode tmp = node;
			node = node->next;
			free(tmp);
		}
	}
	free(map->buckets);
	free(map);
}

void HashMap_free_all(HashMap map, void (*free_fct)(void*)) {
	for (int i = 0; i < map->capacity; i++) {
		HashMapNode node = map->buckets[i];
		while (node) {
			HashMapNode tmp = node;
			node = node->next;
			if (tmp->value) free_fct(tmp->value);
			free(tmp);
		}
	}
	free(map->buckets);
	free(map);
}


void HashMap_insert(HashMap map, int (*hash_fct)(void*), void* value) {
	if (!value) {
		printf("HashMap error: null key.\n"); return;
	}
	int key = hash_fct(value);
	int hashVal = key % map->capacity;
	if (hashVal < 0) hashVal += map->capacity;
	HashMapNode node = new_HashMapNode(key, value);
	HashMapNode tmp = map->buckets[hashVal];
	node->next = tmp;
	map->buckets[hashVal] = node;
	map->count++;
}

void HashMap_insert_at(HashMap map, int key, void* value) {
	if (!value) {
		printf("HashMap error: null key.\n"); return;
	}
	int hashVal = key % map->capacity;
	if (hashVal < 0) hashVal += map->capacity;
	HashMapNode node = new_HashMapNode(key, value);
	HashMapNode tmp = map->buckets[hashVal];
	node->next = tmp;
	map->buckets[hashVal] = node;
	map->count++;
}

void* HashMap_get(HashMap map, int key) {
	int hashVal = key % map->capacity;
	if (hashVal < 0) hashVal += map->capacity;
	HashMapNode node = map->buckets[hashVal];
	while (node) {
		if (node->key == key) return node->value;
		node = node->next;
	}
}

int HashMap_containsKey(HashMap map, int key) {
	return HashMap_get(map, key) != 0;
}

void HashMap_remove(HashMap map, int key) {
	int hashVal = key % map->capacity;
	if (hashVal < 0) hashVal += map->capacity;
	HashMapNode node = map->buckets[hashVal];
	HashMapNode prevNode = NULL;
	while (node) {
		if (node->key == key) {
			if (prevNode) prevNode->next = node->next;
			else {
				if (node->next) map->buckets[hashVal] = node->next;
				else map->buckets[hashVal] = NULL;
			}
			free(node);
			map->count--;
			return;
		}
		prevNode = node;
		node = node->next;
	}
}

void HashMap_remove_free(HashMap map, void (*free_fct)(void*), int key) {
	int hashVal = key % map->capacity;
	if (hashVal < 0) hashVal += map->capacity;
	HashMapNode node = map->buckets[hashVal];
	HashMapNode prevNode = NULL;
	while (node) {
		if (node->key == key) {
			if (prevNode) prevNode->next = node->next;
			else {
				if (node->next) map->buckets[hashVal] = node->next;
				else map->buckets[hashVal] = NULL;
			}
			if (node->value) free_fct(node->value);
			free(node);
			map->count--;
			return;
		}
		prevNode = node;
		node = node->next;
	}
}