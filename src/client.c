#include "client.h"
#include <assert.h>
#include <stdlib.h>

void client_data_init(client_data_t* data, movie_t* movie, int id) {
    data->movie = movie;
    data->id    = id;
}

void client_res_init(client_res_t* res) {
    res->success = false; // Por defecto la reserva comienza fallando.
}

void* client_run(client_t* client) {
    // De momento el cliente no hace nada.
    return NULL;
}

client_t* client_new(movie_t* movie, int id) {
    client_t* client = (client_t*)malloc(sizeof(client_t));
    if (client == NULL) {
        return NULL;
    }

    client_data_init(&client->data, movie, id);
    client_res_init(&client->res);

    return client;
}

int client_start(client_t* client) {
    assert(client != NULL);
    return pthread_create(&client->thread, NULL, (void* (*)(void*))client_run, &client->data);
}

void client_free(client_t* client) {
    free(client);
}

void client_init(client_t* client, movie_t* movie, int id) {
    assert(client != NULL);
    assert(movie != NULL);

    client_data_init(&client->data, movie, id);
    client_res_init(&client->res);
}
