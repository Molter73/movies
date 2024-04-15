#include "client.h"
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

// Diferentes niveles de bloqueo en reservas.
static pthread_mutex_t global_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t* row_mutexes;
static pthread_mutex_t* seat_mutexes;

// Inicializa Datos del cliente
void client_data_init(client_data_t* data, movie_t* movie, int id) {
    data->movie = movie;
    data->id    = id;
}

// Inicializa el resultado de la reserva.
void client_res_init(client_res_t* res) {
    res->success = false;
}

// Puntero a función. Se asignará a la función de ejecución que le corresponda.
void* (*client_run)(void*) = NULL;

// Bloqueo Global
void* client_global_mutex(void* arg) {
    client_t* client = (client_t*)arg;
    movie_t* movie   = client->data.movie;
    int id           = client->data.id;
    bool reserved    = false;

    pthread_mutex_lock(&global_mutex); // Bloqueo global
    int rand_row = rand() % movie->nrows;
    int rand_col = rand() % movie->ncols;
    reserved     = movie_reserve_seat(movie, rand_col, rand_row, id);
    pthread_mutex_unlock(&global_mutex);

    client->res.success = reserved;
    return NULL;
}

// Bloqueo por Fila
void* client_per_row_mutex(void* arg) {
    client_t* client = (client_t*)arg;
    movie_t* movie   = client->data.movie;
    int id           = client->data.id;
    bool reserved    = false;
    int row          = id % movie->nrows;

    pthread_mutex_lock(&row_mutexes[row]); // Bloqueo de fila
    int rand_col = rand() % movie->ncols;
    reserved     = movie_reserve_seat(movie, rand_col, row, id);
    pthread_mutex_unlock(&row_mutexes[row]);

    client->res.success = reserved;
    return NULL;
}

// Bloqueo por Asiento
void* client_per_seat_mutex(void* arg) {
    client_t* client = (client_t*)arg;
    movie_t* movie   = client->data.movie;
    int id           = client->data.id;
    bool reserved    = false;

    int rand_index = rand() % (movie->nrows * movie->ncols);
    int rand_row   = rand_index / movie->ncols;
    int rand_col   = rand_index % movie->ncols;

    pthread_mutex_lock(&seat_mutexes[rand_index]); // Bloqueo de asiento
    reserved = movie_reserve_seat(movie, rand_col, rand_row, id);
    pthread_mutex_unlock(&seat_mutexes[rand_index]);

    client->res.success = reserved;
    return NULL;
}

client_t* client_new(movie_t* movie, int id, int method) {
    client_t* client = (client_t*)malloc(sizeof(client_t));
    if (client == NULL) {
        return NULL;
    }

    switch (method) {
    case 0:
        client_run = client_global_mutex;
        break;
    case 1:
        client_run = client_per_row_mutex;
        break;
    case 2:
        client_run = client_per_seat_mutex;
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

// Inicia el hilo del cliente.
int client_start(client_t* client) {
    assert(client != NULL);
    return pthread_create(&client->thread, NULL, client_run, client);
}

// Libera la memoria ocupada por el cliente.
void client_free(client_t* client) {
    if (client != NULL) {
        free(client);
    }
}

// Inicializa los datos del cliente.
void client_init(client_t* client, movie_t* movie, int id) {
    assert(client != NULL);
    assert(movie != NULL);

    client_data_init(&client->data, movie, id);
    client_res_init(&client->res);
}

// Inicializa los mutexes según el método de reserva seleccionado.
void client_init_mutexes(int method, int nrows, int ncols) {
    switch (method) {
    case 0:
        pthread_mutex_init(&global_mutex, NULL);
        break;
    case 1:
        row_mutexes = malloc(nrows * sizeof(pthread_mutex_t));
        for (int i = 0; i < nrows; i++) {
            pthread_mutex_init(&row_mutexes[i], NULL);
        }
        break;
    case 2:
        seat_mutexes = malloc(nrows * ncols * sizeof(pthread_mutex_t));
        for (int i = 0; i < nrows * ncols; i++) {
            pthread_mutex_init(&seat_mutexes[i], NULL);
        }
        break;
    }
}

// Destruye los mutexes según el método de reserva seleccionado.
void client_destroy_mutexes(int method, int nrows, int ncols) {
    switch (method) {
    case 0:
        pthread_mutex_destroy(&global_mutex);
        break;
    case 1:
        if (row_mutexes != NULL) {
            for (int i = 0; i < nrows; i++) {
                pthread_mutex_destroy(&row_mutexes[i]);
            }
            free(row_mutexes);
            row_mutexes = NULL;
        }
        break;
    case 2:
        if (seat_mutexes != NULL) {
            for (int i = 0; i < nrows * ncols; i++) {
                pthread_mutex_destroy(&seat_mutexes[i]);
            }
            free(seat_mutexes);
            seat_mutexes = NULL;
        }
        break;
    }
}
