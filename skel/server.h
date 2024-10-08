/*
 * Copyright (c) 2024, < Farcasanu Bianca Ioana 313CAb 2023-2024 >
 */

#ifndef SERVER_H
#define SERVER_H

#include "utils.h"
#include "constants.h"
#include "lru_cache.h"

#define TASK_QUEUE_SIZE         1000
#define MAX_LOG_LENGTH          1000
#define MAX_RESPONSE_LENGTH     4096

/* QUEUE */

typedef struct queue_t queue_t;
struct queue_t
{
	/* the maximum size of the queue */
	unsigned int max_size;
	/* the size of the queue */
	unsigned int size;
	/* the size in bytes of the data type stored in the queue */
	unsigned int data_size;
	/* the index from which front and dequeue operations will be performed */
	unsigned int read_idx;
	/* the index from which the enqueue operations will be performed */
	unsigned int write_idx;
	/* the buffer that stores the elements of the queue */
	void **buff;
};

queue_t * q_create(unsigned int data_size, unsigned int max_size);
unsigned int q_get_size(queue_t *q);
unsigned int q_is_empty(queue_t *q);
void *q_front(queue_t *q);
int q_dequeue(queue_t *q);
int q_enqueue(queue_t *q, void *new_data);
void q_clear(queue_t *q);
void q_free(queue_t *q);

/* the struct for DATA_BASE */
typedef struct doc_info {
	char *doc_name;
	char *doc_content;
} doc_info;

typedef struct server {
    lru_cache *cache;
    queue_t *request_q;
    int server_id;
    doubly_linked_list_t *data_base;

    /* TODO: add needed fields */
} server;

typedef struct request {
    request_type type;
    char *doc_name;
    char *doc_content;
} request;

typedef struct response {
    char *server_log;
    char *server_response;
    int server_id;
} response;


server *init_server(unsigned int cache_size);

/**
 * @brief Should deallocate completely the memory used by server,
 *     taking care of deallocating the elements in the queue, if any,
 *     without executing the tasks
 */
void free_server(server **s);

/**
 * server_handle_request() - Receives a request from the load balancer
 *      and processes it according to the request type
 * 
 * @param s: Server which processes the request.
 * @param req: Request to be processed.
 * 
 * @return response*: Response of the requested operation, which will
 *      then be printed in main.
 * 
 * @brief Based on the type of request, should call the appropriate
 *     solver, and should execute the tasks from queue if needed (in
 *     this case, after executing each task, PRINT_RESPONSE should
 *     be called).
 */
response *server_handle_request(server *s, request *req);

#endif  /* SERVER_H */
