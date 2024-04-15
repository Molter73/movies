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

void* (*client_run)(void*) = NULL;

void* client_global_mutex(client_t* client) {
    movie_t* movie = client->data.movie;
    int id         = client->data.id;
    bool reserved  = false;

    // Bloquear el acceso global a todos los asientos
    sem_wait(&movie->global_lock);

    // Intenta reservar un asiento
    for (unsigned int i = 0; i < movie->nrows; i++) {
        for (unsigned int j = 0; j < movie->ncols; j++) {
            if (movie_reserve_seat(movie, j, i, id)) {
                reserved = true;
                break;
            }
        }
        if (reserved)
            break;
    }

    // Desbloquear después de la operación
    sem_post(&movie->global_lock);

    client->res.success = reserved; // Guardar el resultado de la reserva
    return NULL;
}

void* client_per_row_mutex(client_t* client) {
    movie_t* movie = client->data.movie;
    int id         = client->data.id;
    bool reserved  = false;
    int row        = id % movie->nrows;

    // Bloquear solo la fila específica
    sem_wait(&movie->row_locks[row]);

    // Intentar reservar un asiento en la fila específica
    for (unsigned int j = 0; j < movie->ncols; j++) {
        if (movie_reserve_seat(movie, j, row, id)) {
            reserved = true;
            break;
        }
    }

    // Desbloquear la fila después de completar la operación
    sem_post(&movie->row_locks[row]);

    client->res.success = reserved; // Guardar el resultado de la reserva
    return NULL;
}

void* client_per_seat_mutex(client_t* client) {
    movie_t* movie = client->data.movie;
    int id         = client->data.id;
    bool reserved  = false;

    // Intenta reservar un asiento específico de forma secuencial
    for (unsigned int i = 0; i < movie->nrows; i++) {
        for (unsigned int j = 0; j < movie->ncols; j++) {
            int index = i * movie->ncols + j;
            sem_wait(&movie->seat_locks[index]); // Bloquear el semáforo del asiento específico

            if (!movie->seats[index].reserved) {
                reserved = movie_reserve_seat(movie, j, i, id);
                if (reserved) {
                    sem_post(&movie->seat_locks[index]);
                    break;
                }
            }

            sem_post(&movie->seat_locks[index]);
        }
        if (reserved)
            break;
    }

    client->res.success = reserved; // Guardar el resultado de la reserva
    return NULL;
}

client_t* client_new(movie_t* movie, int id, int method) {
    client_t* client = (client_t*)malloc(sizeof(client_t));
    if (client == NULL) {
        return NULL;
    }

    switch (method) {
    case 0:
        client_run = (void* (*)(void*))client_global_mutex;
        break;
    case 1:
        client_run = (void* (*)(void*))client_per_row_mutex;
        break;
    case 2:
        client_run = (void* (*)(void*))client_per_seat_mutex;
        break;
    default:
        client_run = NULL;
        free(client);
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
