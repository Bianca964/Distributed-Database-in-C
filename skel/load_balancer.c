/*
 * Copyright (c) 2024, < Farcasanu Bianca Ioana 313CAb 2023-2024 >
 */

#include "load_balancer.h"
#include "server.h"

load_balancer *init_load_balancer(bool enable_vnodes) {
	/* TODO */
    load_balancer *main = malloc(sizeof(load_balancer));
    DIE(!main, "malloc failed\n");
    main->hash_function_servers = hash_uint;
    main->hash_function_docs = hash_string;
    main->enable_vnodes = enable_vnodes;
    return main;
}

void loader_add_server(load_balancer* main, int server_id, int cache_size) {
    /* TODO: Remove test_server after checking the server implementation */
    main->test_server = init_server(cache_size);
    main->test_server->server_id = server_id;
}

void loader_remove_server(load_balancer* main, int server_id) {
    /* TODO */
    (void)main;
    (void)server_id;
}

response *loader_forward_request(load_balancer* main, request *req) {
    /* TODO */
    response *my_resp = server_handle_request(main->test_server, req);
    return my_resp;
}

void free_load_balancer(load_balancer** main) {
    /* TODO: get rid of test_server after testing the server implementation */
    free_server(&(*main)->test_server);
    free(*main);

    *main = NULL;
}


