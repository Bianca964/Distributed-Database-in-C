/*
 * Copyright (c) 2024, < Farcasanu Bianca Ioana 313CAb 2023-2024 >
 */

#include <stdio.h>
#include <string.h>
#include "lru_cache.h"
#include "utils.h"

/* DOUBLY LINKED LIST */

doubly_linked_list_t *dll_create(unsigned int data_size) {
	doubly_linked_list_t *list = malloc(sizeof(doubly_linked_list_t));
    DIE(!list, "malloc failed\n");
	list->data_size = data_size;
	list->head = NULL;
	list->tail = NULL;
	list->size = 0;
	return list;
}

dll_node_t *dll_get_nth_node(doubly_linked_list_t *list, unsigned int n) {
	if (n >= list->size)
		n = n % list->size;

	if (n == 0)
		return list->head;

	unsigned int count = 0;
	dll_node_t *curr = list->head;
	while (curr) {
		if (count == n)
			return curr;
		count++;
		curr = curr->next;
	}
	return NULL;
}

void dll_add_nth_node(doubly_linked_list_t *list, unsigned int n,
					  const void *data) {
	/* add on first position */
	if (!list->head) {
		dll_node_t *new = malloc(sizeof(dll_node_t));
    	DIE(!new, "malloc failed\n");

		new->data = malloc(list->data_size);
    	DIE(!new->data, "malloc failed\n");
		memcpy(new->data, data, list->data_size);

		list->head = new;
		new->next = NULL;
		list->head->prev = NULL;
		list->tail = new;
		list->size++;
		return;
	}

	if (n == 0) {
		dll_node_t *new = malloc(1 * sizeof(dll_node_t));
    	DIE(!new, "malloc failed\n");

		new->data = malloc(list->data_size);
    	DIE(!new->data, "malloc failed\n");
		memcpy(new->data, data, list->data_size);

		new->next = list->head;
		new->prev = NULL;
		list->head->prev = new;
		list->head = new;
		list->size++;
		return;
	}

	/* add to end */
	if (n >= list->size) {
		dll_node_t *tail = list->tail;
		dll_node_t *new = malloc(1 * sizeof(dll_node_t));
    	DIE(!new, "malloc failed\n");

		new->data = malloc(list->data_size);
    	DIE(!new->data, "malloc failed\n");
		memcpy(new->data, data, list->data_size);

		tail->next = new;
		new->prev = tail;
		new->next = NULL;
		list->tail = new;
		list->size++;
		return;
	}

	/* add in the middle */
	unsigned int contor = 0;
	dll_node_t *curr_node = list->head;
	while (curr_node->next) {
		if (contor == n - 1) {
			dll_node_t *new = malloc(1 * sizeof(dll_node_t));
    		DIE(!new, "malloc failed\n");

			new->data = malloc(list->data_size);
    		DIE(!new->data, "malloc failed\n");
			memcpy(new->data, data, list->data_size);

			new->next = curr_node->next;
			new->prev = curr_node;
			curr_node->next = new;
			(new->next)->prev = new;
			list->size++;
			return;
		}
		/* go to next */
		contor++;
		curr_node = curr_node->next;
	}
}

dll_node_t *dll_remove_nth_node(doubly_linked_list_t *list, unsigned int n) {
	if (list == NULL || list->size == 0 || n >= list->size) {
		return NULL;
	}

	/* remove first element when it's the only one */
	if (n == 0 && list->size == 1) {
		dll_node_t *aux = list->head;
		list->head = NULL;
		list->tail = NULL;
		list->size--;
		return aux;
	}

	/* remove first element */
	if (n == 0) {
		dll_node_t *aux = list->head;
		list->head = aux->next;
		list->head->prev = NULL;
		list->size--;
		return aux;
	}

	/* remove last element */
	if (n == list->size - 1) {
		dll_node_t *curr_node = list->tail->prev;
		dll_node_t *aux = list->tail;
		curr_node->next = NULL;
		list->tail = curr_node;
		list->size--;
		return aux;
	}

	dll_node_t *node = list->head;
	unsigned int contor = 0;
	while (node) {
		if (contor == list->size - 2)
			break;
		if (contor == n - 1) {
			/* modify connections */
			dll_node_t *aux = node->next;
			node->next = aux->next;
			aux->next->prev = node;
			list->size--;
			return aux;
		}

		contor++;
		node = node->next;
	}

	return NULL;
}

void dll_free(doubly_linked_list_t **pp_list) {
	if (*pp_list) {
		dll_node_t *aux = (*pp_list)->head;
		while (aux) {
			(*pp_list)->head = aux->next;
			if (aux->data) {
				free(aux->data);
			}
			free(aux);
			aux = (*pp_list)->head;
		}

		free(*pp_list);
		*pp_list = NULL;
	}
}

/* function to remove a node from order_list from a specific key;
 * order list has data of type 'info*' : key = doc_name, value = doc_content */
dll_node_t *dll_remove_node_with_key(doubly_linked_list_t *list, void *key)
{
	if (list == NULL || list->size == 0) {
		return NULL;
	}

	dll_node_t *node = list->head;
	unsigned int pos = 0;
	while (node) {
		/* got on the node i want to remove */
		if (strcmp((char *)((info *)node->data)->key, (char *)key) == 0) {
			/* if it's the first and the only one */
			if (pos == 0 && list->size == 1) {
				dll_node_t *aux = list->head;
				list->head = NULL;
				list->tail = NULL;
				list->size--;
				return aux;
			}

			/* if it's the first */
			if (pos == 0 && list->size > 1) {
				dll_node_t *aux = list->head;
				list->head = aux->next;
				list->head->prev = NULL;
				list->size--;
				return aux;
			}

			/* it it's the last */
			if (pos == list->size - 1) {
				/* position on a node further back */
				node = node->prev;
				dll_node_t *aux = node->next;
				node->next = NULL;
				list->tail = node;
				list->size--;
				return aux;
			}

			/* if it's in the middle */
			node = node->prev;
			dll_node_t *aux = node->next;
			node->next = aux->next;
			aux->next->prev = node;
			list->size--;
			return aux;
		}

		node = node->next;
		pos++;
	}

	return NULL;
}

/* HASHMAP */

int compare_function_ints(void *a, void *b) {
	int int_a = *((int *)a);
	int int_b = *((int *)b);

	if (int_a == int_b) {
		return 0;
	} else if (int_a < int_b) {
		return -1;
	} else {
		return 1;
	}
}

int compare_function_strings(void *a, void *b) {
	char *str_a = (char *)a;
	char *str_b = (char *)b;

	return strcmp(str_a, str_b);
}

void key_val_free_function(void *data) {
	info *aux = (info *)data;
	if (aux->key)
		free(aux->key);
	aux->key = NULL;
}

hashtable_t *ht_create(unsigned int hmax, unsigned int (*hash_function)(void *),
					   int (*compare_function)(void *, void *),
					   void (*key_val_free_function)(void *)) {
	hashtable_t *ht = (hashtable_t *)malloc(sizeof(hashtable_t));
    DIE(!ht, "malloc failed\n");
	ht->hmax = hmax;
	ht->size = 0;
	ht->compare_function = compare_function;
	ht->hash_function = hash_function;
	ht->key_val_free_function = key_val_free_function;

	/* allocate memory for the 'hmax' terms of the array */
	ht->buckets = (doubly_linked_list_t **)malloc(hmax *
										   sizeof(doubly_linked_list_t *));
    DIE(!ht->buckets, "malloc failed\n");

	for (unsigned int i = 0; i < hmax; i++) {
		ht->buckets[i] = (doubly_linked_list_t *)malloc(1 *
												 sizeof(doubly_linked_list_t));
    	DIE(!ht->buckets[i], "malloc failed\n");

		ht->buckets[i]->head = NULL;
		ht->buckets[i]->size = 0;
		ht->buckets[i]->data_size = sizeof(info);
	}
	return ht;
}

/* function that returns: 1, if a hashtable value was previously associated
 * with the key using the put function and returns 0, otherwise */
int ht_has_key(hashtable_t *ht, void *key) {
	unsigned int index = ht->hash_function(key);
	index = index % ht->hmax;

	if (ht->buckets[index] == NULL)
		return 0;

	dll_node_t *node = ht->buckets[index]->head;
	while (node != NULL) {
		if (ht->compare_function(((info *)node->data)->key, key) == 0) {
			return 1;
		}
		node = node->next;
	}
	return 0;
}

void *ht_get(hashtable_t *ht, void *key) {
	int index = ht->hash_function(key);
	index = index % ht->hmax;
	dll_node_t *node = ht->buckets[index]->head;

	while (node != NULL) {
		if (ht->compare_function(((info *)node->data)->key, key) == 0) {
			return ((info *)node->data)->value;
		}
		node = node->next;
	}

	return NULL;
}

void ht_put(hashtable_t *ht, void *key, unsigned int key_size,
			void *value, unsigned int value_size) {
	/* make copy */
	info *data = malloc(sizeof(info));
	DIE(!data, "malloc failed\n");

	data->key = malloc(key_size);
	DIE(!data->key, "malloc failed\n");
	memcpy(data->key, key, key_size);

	/* value is pointer to node in the list(no need to allocate memory to it) */
	data->value = value;
	(void)value_size;

	/* add to ht */
	unsigned int new_hash = (ht->hash_function(data->key)) % ht->hmax;
	int last_position = ht->buckets[new_hash]->size;
	dll_add_nth_node(ht->buckets[new_hash], last_position, data);
	free(data);
	ht->size++;
}

/* procedure that removes from the ht the entry associated with the key */
void ht_remove_entry(hashtable_t *ht, void *key) {
	unsigned int index = (ht->hash_function(key)) % ht->hmax;

	dll_node_t *node = ht->buckets[index]->head;
	int index_nod_to_remove = 0;
	while (node != NULL) {
		if (ht->compare_function(((info *)node->data)->key, key) == 0) {
			dll_node_t *node_to_remove =
				dll_remove_nth_node(ht->buckets[index], index_nod_to_remove);
			ht->key_val_free_function(node_to_remove->data);
			free(node_to_remove->data);
			free(node_to_remove);

			ht->size--;
			return;
		}
		node = node->next;
		index_nod_to_remove++;
	}
}

void ht_free(hashtable_t *ht) {
	for (unsigned int i = 0; i < ht->hmax; i++) {
		/* free what's in every list */
		dll_node_t *node = ht->buckets[i]->head;
		while (node != NULL) {
			ht->key_val_free_function(node->data);
			node = node->next;
		}
		dll_free(&(ht->buckets[i]));
	}
	free(ht->buckets);
	free(ht);
}

unsigned int ht_get_size(hashtable_t *ht) {
	if (ht == NULL)
		return 0;

	return ht->size;
}

unsigned int ht_get_hmax(hashtable_t *ht) {
	if (ht == NULL)
		return 0;

	return ht->hmax;
}

/* CACHE */

lru_cache *init_lru_cache(unsigned int cache_capacity) {
	/* TODO */

	/* keep the name of the documents (the key) and the value (pointer to a
	 * node in the order_list) in a node */
	doubly_linked_list_t *order_list = dll_create(sizeof(info));
	hashtable_t *ht = ht_create(cache_capacity, hash_string,
								compare_function_strings, key_val_free_function);
	lru_cache *cache = malloc(sizeof(lru_cache));
	DIE(!cache, "malloc failed\n");
	cache->ht = ht;
	cache->order_list = order_list;
	return cache;
}

bool lru_cache_is_full(lru_cache *cache) {
	/* TODO */

	if (cache->ht->size == cache->ht->hmax) {
		return true;
	}
	return false;
}

void free_order_list(doubly_linked_list_t **list) {
	dll_node_t *aux = (*list)->head;
	while (aux) {
		(*list)->head = aux->next;

		if (((info *)aux->data)->key) {
			free(((info *)aux->data)->key);
			((info *)aux->data)->key = NULL;
		}
		if (((info *)aux->data)->value) {
			free(((info *)aux->data)->value);
			((info *)aux->data)->value = NULL;
		}
		if (aux->data) {
			free(aux->data);
		}
		free(aux);

		aux = (*list)->head;
	}

	free(*list);
	*list = NULL;
}

void free_lru_cache(lru_cache **cache) {
	/* TODO */

	free_order_list(&((*cache)->order_list));
	ht_free((*cache)->ht);
	free(*cache);
	*cache = NULL;
}

bool lru_cache_put(lru_cache *cache, void *key, void *value,
				   void **evicted_key) {
	/* TODO */

	*evicted_key = NULL;

	/* if the key already exists */
	if (ht_has_key(cache->ht, key) == 1) {
		return false;
	}

	/* if the cache is full, remove the corresponding key */
	if (lru_cache_is_full(cache) == true) {
		/* remove the oldest key from order_list */
		dll_node_t *removed_node = dll_remove_nth_node(cache->order_list, 0);

		*((char **)evicted_key) = malloc(DOC_NAME_LENGTH * sizeof(char));
		DIE(!(*((char **)evicted_key)), "malloc failed\n");
		strcpy(*((char **)evicted_key),
			   (char *)(((info *)removed_node->data)->key));

		/* free the memory for the content of the removed key */
		if ((char *)(((info *)removed_node->data)->value)) {
			free(((info *)removed_node->data)->value);
		}
		free((((info *)removed_node->data)->key));
		free((info *)removed_node->data);
		free(removed_node);
		removed_node = NULL;

		/* remove the key from cache (ht) */
		ht_remove_entry(cache->ht, *evicted_key);
	}

	/* add to order_list */
	info *data_to_add = malloc(sizeof(info));
	DIE(!data_to_add, "malloc failed\n");

	data_to_add->key = malloc(DOC_NAME_LENGTH * sizeof(char));
	DIE(!data_to_add->key, "malloc failed\n");
	memcpy(data_to_add->key, key, DOC_NAME_LENGTH);

	data_to_add->value = malloc(DOC_CONTENT_LENGTH * sizeof(char));
	DIE(!data_to_add->value, "malloc failed\n");
	memcpy(data_to_add->value, value, DOC_CONTENT_LENGTH);

	dll_add_nth_node(cache->order_list, cache->order_list->size, data_to_add);
	free(data_to_add);

	/* add to ht */
	unsigned int key_size = DOC_NAME_LENGTH * sizeof(char);
	unsigned int value_size = sizeof(dll_node_t *);
	dll_node_t *new_value = cache->order_list->tail;
	ht_put(cache->ht, key, key_size, new_value, value_size);

	return true;
}

void *lru_cache_get(lru_cache *cache, void *key) {
	/* TODO */

	/* if key exists in cache, return the value associated with the key */
	dll_node_t *value_ht = (dll_node_t *)ht_get(cache->ht, key);
	/* if it doesn't exist, the function makes value_ht == NULL anyway */
	return value_ht;
}

void lru_cache_remove(lru_cache *cache, void *key) {
	/* TODO */

	/* remove from order_list */
	dll_node_t *removed_node = dll_remove_node_with_key(cache->order_list, key);
	if (removed_node) {
		if (removed_node->data) {
			free(((info *)removed_node->data)->key);
			free(((info *)removed_node->data)->value);
			free((info *)removed_node->data);
		}
		free(removed_node);
		removed_node = NULL;
	}

	/* remove from ht */
	ht_remove_entry(cache->ht, key);
}
