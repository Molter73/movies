#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "movie.h"
#include <pthread.h>
#include <stdbool.h>

typedef struct client_data_s {
    movie_t* movie;
    int id;
} client_data_t;

typedef struct client_res_s {
    bool success;
} client_res_t;

typedef struct client_s {
    pthread_t thread;
    client_data_t data;
    client_res_t res;
} client_t;

client_t* client_new(movie_t* movie, int id);
int client_start(client_t* client);
void client_free(client_t* client);

#endif
