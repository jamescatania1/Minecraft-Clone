#ifndef _hashset_h
#define _hashset_h

#include <stdlib.h>
#include "structures/hashmap.h"

typedef struct HashSet {
	HashMap map;
} *HashSet;

extern HashSet new_HashSet(int capacity);

extern void HashSet_free(HashSet set);

extern void HashSet_insert(HashSet set, int key);

extern void HashSet_remove(HashSet set, int key);

extern int HashSet_contains(HashSet set, int key);

#endif