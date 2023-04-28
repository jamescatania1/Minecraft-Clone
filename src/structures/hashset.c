#include <stdlib.h>
#include "structures/hashset.h"

HashSet new_HashSet(int capacity) {
	HashSet set = (HashSet)malloc(sizeof(struct HashSet));
	if (!set) return NULL;
	set->map = new_HashMap(capacity);
	return set;
}

void HashSet_free(HashSet set) {
	HashMap_free(set->map);
	free(set);
}

void HashSet_insert(HashSet set, int key) {
	HashMap_insert_at(set->map, key, NULL);
}

void HashSet_remove(HashSet set, int key) {
	HashMap_remove(set->map, key);
}

int HashSet_contains(HashSet set, int key) {
	return HashMap_containsKey(set->map, key);
}