/*
 * Copyright (c) 2024, < Farcasanu Bianca Ioana 313CAb 2023-2024 > 
 */

#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <stdbool.h>

/* DOUBLY LINKED LIST */

typedef struct dll_node_t dll_node_t;
struct dll_node_t
{
    void* data;
    dll_node_t *prev, *next;
};

typedef struct doubly_linked_list_t doubly_linked_list_t;
struct doubly_linked_list_t
{
    dll_node_t* head, *tail;
    unsigned int data_size;
    unsigned int size;
};

typedef struct info info;
struct info {
	void *key;
	void *value;
};

doubly_linked_list_t *dll_create(unsigned int data_size);
dll_node_t *dll_get_nth_node(doubly_linked_list_t *list, unsigned int n);
void dll_add_nth_node(doubly_linked_list_t *list, unsigned int n,
                      const void *data);
dll_node_t *dll_remove_nth_node(doubly_linked_list_t *list, unsigned int n);
void dll_free(doubly_linked_list_t **pp_list);
dll_node_t *dll_remove_node_with_key(doubly_linked_list_t *list, void *key);

/* HASHMAP */

typedef struct hashtable_t hashtable_t;
struct hashtable_t {
	/* Array of simply-linked lists */
	doubly_linked_list_t **buckets;
	/* No. total of currently existing nodes in all buckets */
	unsigned int size;
	/* No. of buckets */
	unsigned int hmax;
	/* (Pointer to) Function to calculate the hash value associated with key */
	unsigned int (*hash_function)(void*);
	/* (Pointer to) Function to compare two keys */
	int (*compare_function)(void*, void*);
	/* (Pointer to) Function to free the memory occupied by key and value */
	void (*key_val_free_function)(void*);
};

int compare_function_ints(void *a, void *b);
int compare_function_strings(void *a, void *b);
void key_val_free_function(void *data);
hashtable_t *ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		               int (*compare_function)(void*, void*),
		               void (*key_val_free_function)(void*));
int ht_has_key(hashtable_t *ht, void *key);
void *ht_get(hashtable_t *ht, void *key);
void ht_put(hashtable_t *ht, void *key, unsigned int key_size,
	        void *value, unsigned int value_size);
void ht_remove_entry(hashtable_t *ht, void *key);
void ht_free(hashtable_t *ht);
unsigned int ht_get_size(hashtable_t *ht);
unsigned int ht_get_hmax(hashtable_t *ht);

/* CACHE */

typedef struct lru_cache {
    /* TODO */
    doubly_linked_list_t *order_list;
    hashtable_t *ht;
} lru_cache;

lru_cache *init_lru_cache(unsigned int cache_capacity);

bool lru_cache_is_full(lru_cache *cache);

void free_lru_cache(lru_cache **cache);

/**
 * lru_cache_put() - Adds a new pair in our cache.
 * 
 * @param cache: Cache where the key-value pair will be stored.
 * @param key: Key of the pair.
 * @param value: Value of the pair.
 * @param evicted_key: The function will RETURN via this parameter the
 *      key removed from cache if the cache was full.
 * 
 * @return - true if the key was added to the cache,
 *      false if the key already existed.
 */
bool lru_cache_put(lru_cache *cache, void *key, void *value,
                   void **evicted_key);

/**
 * lru_cache_get() - Retrieves the value associated with a key.
 * 
 * @param cache: Cache where the key-value pair is stored.
 * @param key: Key of the pair.
 * 
 * @return - The value associated with the key,
 *      or NULL if the key is not found.
 */
void *lru_cache_get(lru_cache *cache, void *key);

/**
 * lru_cache_remove() - Removes a key-value pair from the cache.
 * 
 * @param cache: Cache where the key-value pair is stored.
 * @param key: Key of the pair.
*/
void lru_cache_remove(lru_cache *cache, void *key);

/* function added later */
void free_order_list(doubly_linked_list_t **list);

#endif /* LRU_CACHE_H */
