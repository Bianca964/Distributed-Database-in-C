/*
 * Copyright (c) 2024, < Farcasanu Bianca Ioana 313CAb 2023-2024 >
 */

#include <stdio.h>
#include "server.h"
#include "lru_cache.h"

#include "utils.h"

/* QUEUE */

queue_t *q_create(unsigned int data_size, unsigned int max_size)
{
	queue_t *que = malloc(1 * sizeof(queue_t));
	DIE(!que, "malloc failed\n");

	que->max_size = max_size;
	que->size = 0;
	que->data_size = data_size;
	que->read_idx = 0;
	que->write_idx = 0;
	que->buff = malloc(max_size * sizeof(*que->buff));
	DIE(!que->buff, "malloc failed\n");

	for (unsigned int i = 0; i < que->max_size; i++) {
		que->buff[i] = malloc(que->data_size);
		DIE(!(que->buff[i]), "malloc failed\n");
		((request *)que->buff[i])->doc_content = NULL;
		((request *)que->buff[i])->doc_name = NULL;
	}

	return que;
}

unsigned int q_get_size(queue_t *q)
{
	return q->size;
}

unsigned int q_is_empty(queue_t *q)
{
	if (q->size == 0 || !q) {
		return 1;
	} else {
		return 0;
	}
}

/* function returns the first element in the queue, without removing it */
void *q_front(queue_t *q)
{
	if (q->size > 0)
		return q->buff[q->read_idx];
	return NULL;
}

int q_dequeue(queue_t *q)
{
	if (!q || q->size == 0)
		return 0;

	if (q->size > 0) {
		if (q->buff[q->read_idx] != NULL) {
			if (((request *)q->buff[q->read_idx])->doc_content) {
				free(((request *)q->buff[q->read_idx])->doc_content);
				((request *)q->buff[q->read_idx])->doc_content = NULL;
			}

			if (((request *)q->buff[q->read_idx])->doc_name) {
				free(((request *)q->buff[q->read_idx])->doc_name);
				((request *)q->buff[q->read_idx])->doc_name = NULL;
			}

			free(q->buff[q->read_idx]);
			q->buff[q->read_idx] = NULL;
		}
		q->read_idx = (q->read_idx + 1) % q->max_size;
		q->size--;
		return 1;
	} else {
		return 0;
	}
}

int q_enqueue(queue_t *q, void *new_data)
{
	if (q->size < q->max_size) {
		if (!q->buff[q->write_idx]) {
			q->buff[q->write_idx] = malloc(q->data_size);
			DIE(!(q->buff[q->write_idx]), "malloc failed\n");
			((request *)q->buff[q->write_idx])->doc_content = NULL;
			((request *)q->buff[q->write_idx])->doc_name = NULL;
		}
		memcpy(q->buff[q->write_idx], new_data, q->data_size);
		q->write_idx = (q->write_idx + 1) % q->max_size;

		q->size++;
		return 1;
	}

	return 0;
}

void q_clear(queue_t *q)
{
	unsigned int i;
	if (!q || !q->size)
		return;

	for (i = 0; i < q->max_size; i++) {
		if (q->buff[i] != NULL) {
			free(q->buff[i]);
			q->buff[i] = NULL;
		}
	}
}

void q_free(queue_t *q)
{
	if (!q)
		return;
	q_clear(q);
	free(q->buff);
	free(q);
}

/* SERVER */

void update_content_in_edit(server *s, char *doc_name, char *doc_content)
{
	dll_node_t *node_removed =
		dll_remove_node_with_key(s->cache->order_list, doc_name);

	free(((info *)node_removed->data)->key);
	free(((info *)node_removed->data)->value);
	free(((info *)node_removed->data));
	free(node_removed);

	/* add it at the end in order_list because it was recently edited */
	info *data = malloc(sizeof(info));
	DIE(!data, "malloc failed\n");

	data->key = malloc(DOC_NAME_LENGTH * sizeof(char));
	DIE(!data->key, "malloc failed\n");
	strcpy(data->key, doc_name);

	data->value = malloc(DOC_CONTENT_LENGTH * sizeof(char));
	DIE(!data->value, "malloc failed\n");
	strcpy(data->value, doc_content);

	dll_add_nth_node(s->cache->order_list, s->cache->order_list->size, data);
	free(data);

	/* make ht point to it (I modify the old content) */
	dll_node_t *last_node = s->cache->order_list->tail;

	int index = s->cache->ht->hash_function(doc_name) % s->cache->ht->hmax;
	dll_node_t *curr_node = s->cache->ht->buckets[index]->head;
	while (curr_node) {
		if (strcmp(doc_name, (char *)((info *)curr_node->data)->key) == 0) {
			((info *)curr_node->data)->value = last_node;
			break;
		}
		curr_node = curr_node->next;
	}

	/* modify the content from the database */
	dll_node_t *node = s->data_base->head;
	while (node) {
		if (strcmp(doc_name, ((doc_info *)node->data)->doc_name) == 0) {
			strcpy(((doc_info *)node->data)->doc_content, doc_content);
			break;
		}
		node = node->next;
	}
}

static response *server_edit_document(server *s, char *doc_name,
									  char *doc_content)
{
	/* TODO */
	response *my_resp = malloc(sizeof(response));
	DIE(!my_resp, "malloc failed\n");
	my_resp->server_id = s->server_id;

	/* if doc is in cache */
	if (lru_cache_get(s->cache, doc_name) != NULL) {
		my_resp->server_response = malloc(MAX_RESPONSE_LENGTH * sizeof(char));
		DIE(!my_resp->server_response, "malloc failed\n");
		sprintf(my_resp->server_response, MSG_B, doc_name);

		my_resp->server_log = malloc(MAX_LOG_LENGTH * sizeof(char));
		DIE(!my_resp->server_log, "malloc failed\n");
		sprintf(my_resp->server_log, LOG_HIT, doc_name);

		/* change the content and, implicitly, update the order in cache */
		update_content_in_edit(s, doc_name, doc_content);

		return my_resp;
	}

	/* if doc is not in cache, search in database */
	int found_int_database = 0;
	dll_node_t *node = s->data_base->head;
	while (node) {
		/* if doc is in database */
		if (strcmp(doc_name, (char *)((doc_info *)node->data)->doc_name) == 0) {
			found_int_database = 1;

			/* add to cache */
			char *evicted_key;
			lru_cache_put(s->cache, doc_name, doc_content,
						  (void **)&evicted_key);

			/* modify the content in database */
			strcpy(((doc_info *)node->data)->doc_content, doc_content);

			my_resp->server_log = malloc(MAX_LOG_LENGTH * sizeof(char));
			DIE(!my_resp->server_log, "malloc failed\n");
			if (evicted_key != NULL) {
				/* means the cache was full */
				sprintf(my_resp->server_log, LOG_EVICT, doc_name, evicted_key);
			} else {
				/* cache was not full */
				sprintf(my_resp->server_log, LOG_MISS, doc_name);
			}
			my_resp->server_response = malloc(MAX_RESPONSE_LENGTH *
											  sizeof(char));
			DIE(!my_resp->server_response, "malloc failed\n");
			sprintf(my_resp->server_response, MSG_B, doc_name);

			free(evicted_key);
			return my_resp;
		}
		node = node->next;
	}

	/* if doc is not in database */
	if (found_int_database == 0) {
		my_resp->server_response = malloc(MAX_RESPONSE_LENGTH * sizeof(char));
		DIE(!my_resp->server_response, "malloc failed\n");
		sprintf(my_resp->server_response, MSG_C, doc_name);

		/* add to data_base */
		doc_info *data = malloc(sizeof(doc_info));
		DIE(!data, "malloc failed\n");

		data->doc_name = malloc(DOC_NAME_LENGTH * sizeof(char));
		DIE(!data->doc_name, "malloc failed\n");
		strcpy(data->doc_name, doc_name);

		data->doc_content = malloc(DOC_CONTENT_LENGTH * sizeof(char));
		DIE(!data->doc_content, "malloc failed\n");
		strcpy(data->doc_content, doc_content);

		dll_add_nth_node(s->data_base, 0, data);
		free(data);

		/* add to cache */
		char *evicted_key;
		lru_cache_put(s->cache, doc_name, doc_content, (void **)(&evicted_key));

		my_resp->server_log = malloc(MAX_LOG_LENGTH * sizeof(char));
		DIE(!my_resp->server_log, "malloc failed\n");
		if (evicted_key == NULL) {
			/* cache was not full */
			sprintf(my_resp->server_log, LOG_MISS, doc_name);
		} else {
			/* cache was full */
			sprintf(my_resp->server_log, LOG_EVICT, doc_name, evicted_key);
		}

		free(evicted_key);
		return my_resp;
	}

	return NULL;
}

void update_order_cache_in_get(server *s, char *doc_name)
{
	dll_node_t *node_to_update =
		dll_remove_node_with_key(s->cache->order_list, doc_name);

	dll_add_nth_node(s->cache->order_list, s->cache->order_list->size,
					 node_to_update->data);
	free(node_to_update->data);
	free(node_to_update);
	node_to_update = NULL;

	ht_remove_entry(s->cache->ht, doc_name);
	dll_node_t *to_add = s->cache->order_list->tail;
	unsigned int key_size = DOC_NAME_LENGTH * sizeof(char);
	unsigned int value_size = sizeof(dll_node_t *);
	ht_put(s->cache->ht, doc_name, key_size, to_add, value_size);
}

static response *server_get_document(server *s, char *doc_name)
{
	/* TODO */

	response *my_resp = malloc(sizeof(response));
	DIE(!my_resp, "malloc failed\n");
	my_resp->server_id = s->server_id;

	dll_node_t *value_cache = (dll_node_t *)(lru_cache_get(s->cache, doc_name));

	/* if doc is in cache */
	if (value_cache != NULL) {
		my_resp->server_log = malloc(MAX_LOG_LENGTH * sizeof(char));
		DIE(!my_resp->server_log, "malloc failed\n");
		sprintf(my_resp->server_log, LOG_HIT, doc_name);

		my_resp->server_response = malloc(MAX_RESPONSE_LENGTH * sizeof(char));
		DIE(!my_resp->server_response, "malloc failed\n");
		sprintf(my_resp->server_response, "%s",
				(char *)((info *)value_cache->data)->value);

		/* update order in cache (order_list) */
		update_order_cache_in_get(s, doc_name);

		return my_resp;
	}

	/* if doc is not in cache, search in database */
	int found_in_database = 0;
	dll_node_t *node = s->data_base->head;
	while (node) {
		/* if doc is in database */
		if (strcmp(doc_name, (char *)((doc_info *)node->data)->doc_name) == 0) {
			found_in_database = 1;

			/* add to cache */
			char *evicted_key;
			lru_cache_put(s->cache, doc_name,
						  ((doc_info *)node->data)->doc_content,
						  (void **)&evicted_key);

			my_resp->server_log = malloc(MAX_LOG_LENGTH * sizeof(char));
			DIE(!my_resp->server_log, "malloc failed\n");
			if (evicted_key != NULL) {
				/* cache was full */
				sprintf(my_resp->server_log, LOG_EVICT, doc_name, evicted_key);
			} else {
				/* cache was not full */
				sprintf(my_resp->server_log, LOG_MISS, doc_name);
			}
			my_resp->server_response = malloc(MAX_RESPONSE_LENGTH *
											  sizeof(char));
			DIE(!my_resp->server_response, "malloc failed\n");
			snprintf(my_resp->server_response, MAX_RESPONSE_LENGTH, "%s",
					 ((doc_info *)node->data)->doc_content);

			free(evicted_key);
			return my_resp;
		}
		node = node->next;
	}

	/* doc is not in database either */
	if (found_in_database == 0) {
		my_resp->server_log = malloc(MAX_LOG_LENGTH * sizeof(char));
		DIE(!my_resp->server_log, "malloc failed\n");
		snprintf(my_resp->server_log, MAX_LOG_LENGTH, LOG_FAULT, doc_name);

		my_resp->server_response = NULL;
		return my_resp;
	}

	return NULL;
}

server *init_server(unsigned int cache_size)
{
	/* TODO */

	server *my_server = malloc(sizeof(server));
	DIE(!my_server, "malloc failed\n");
	my_server->cache = init_lru_cache(cache_size);
	my_server->data_base = dll_create(sizeof(doc_info));
	my_server->request_q = q_create(sizeof(request), TASK_QUEUE_SIZE);
	my_server->server_id = 0;

	return my_server;
}

response *server_handle_request(server *s, request *req)
{
	/* TODO */

	char *type = get_request_type_str(req->type);

	if (strcmp(type, EDIT_REQUEST) == 0) {
		/* make copy of request */
		request *req_copy = malloc(sizeof(request));
		DIE(!req_copy, "malloc failed\n");

		req_copy->doc_name = malloc(DOC_NAME_LENGTH * sizeof(char));
		DIE(!req_copy->doc_name, "malloc failed\n");
		strcpy(req_copy->doc_name, req->doc_name);

		req_copy->doc_content = NULL;
		if (req->doc_content) {
			req_copy->doc_content = malloc(DOC_CONTENT_LENGTH * sizeof(char));
			DIE(!req_copy->doc_content, "malloc failed\n");
			strcpy(req_copy->doc_content, req->doc_content);
		}
		req_copy->type = req->type;

		/* add the request to queue */
		q_enqueue(s->request_q, req_copy);

		/* print response */
		response *my_response = malloc(sizeof(response));
		DIE(!my_response, "malloc failed\n");
		my_response->server_id = s->server_id;
		my_response->server_response = malloc(MAX_RESPONSE_LENGTH *
											  sizeof(char));
		DIE(!my_response->server_response, "malloc failed\n");
		my_response->server_log = malloc(MAX_LOG_LENGTH * sizeof(char));
		DIE(!my_response->server_log, "malloc failed\n");

		sprintf(my_response->server_log, LOG_LAZY_EXEC, s->request_q->size);
		sprintf(my_response->server_response, MSG_A, "EDIT", req->doc_name);

		free(req_copy);
		return my_response;
	} else if (strcmp(type, GET_REQUEST) == 0) {
		/* execute the task queue */
		while (q_is_empty(s->request_q) == 0) {
			request *curr_request = q_front(s->request_q);
			response *resp = server_edit_document(s, curr_request->doc_name,
												  curr_request->doc_content);
			PRINT_RESPONSE(resp);
			q_dequeue(s->request_q);
		}

		/* queue is empty: execute get */
		response *resp = server_get_document(s, req->doc_name);

		return resp;
	}
	return NULL;
}

void free_request_queue(queue_t *q)
{
	for (unsigned int i = 0; i < q->max_size; i++) {
		request *req = (request *)(q->buff[i]);
		if (req) {
			if ((req->doc_name)) {
				free(req->doc_name);
				req->doc_name = NULL;
			}
			if ((req->doc_content)) {
				free(req->doc_content);
				req->doc_content = NULL;
			}

			free(req);
			q->buff[i] = NULL;
		}
	}
	free(q->buff);
	free(q);
}

void free_database(doubly_linked_list_t **data_base)
{
	dll_node_t *aux = (*data_base)->head;
	while (aux) {
		free(((doc_info *)aux->data)->doc_content);
		free(((doc_info *)aux->data)->doc_name);

		(*data_base)->head = aux->next;
		if (aux->data) {
			free(aux->data);
		}
		free(aux);
		aux = (*data_base)->head;
	}
	free(*data_base);
	*data_base = NULL;
}

void free_server(server **s)
{
	/* TODO */

	free_request_queue((*s)->request_q);
	free_lru_cache(&((*s)->cache));
	free_database(&(*s)->data_base);
	free(*s);
}
