#ifndef _hashmap_h
#define _hashmap_h

typedef struct HashMapNode {
	int key;
	void* value;
	struct HashMapNode* next;
} *HashMapNode;

typedef struct HashMap {
	int capacity;
	int count;
	struct HashMapNode** buckets;
} *HashMap;

/*
 * HashMap Contstructor for given capacity.
 */
extern HashMap new_HashMap(int capacity);

/*
 * Frees HashMap. Make sure to free all keys/values before calling this if no longer used.
 */
extern void HashMap_free(HashMap map);

extern void HashMap_free_all(HashMap map, void (*free_fct)(void*));

/*
 * Inserts a value for given key. If value for key already exists, replaces value.
 */
extern void HashMap_insert(HashMap map, int (*hash_fct)(void*), void* value);

extern void HashMap_insert_at(HashMap map, int key, void* value);

/*
 * Returns value for given key.
 */
extern void* HashMap_get(HashMap map, int key);

extern int HashMap_containsKey(HashMap map, int key);

/*
 * Removes key and value at given key. Make sure to free it first if no longer used.
 */
extern void HashMap_remove(HashMap map, int key);

extern void HashMap_remove_free(HashMap map, void (*free_fct)(void*), int key);

#endif