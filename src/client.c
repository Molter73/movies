#include "client.h"
#include "stats.h"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
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

// Crea un nuevo resultado de reserva.
client_res_t* client_res_new() {
    client_res_t* res = (client_res_t*)malloc(sizeof(client_res_t));
    if (res == NULL) {
        return NULL;
    }
    res->success = false;
    return res;
}

// Libera un resultado de reserva.
void client_res_free(client_res_t* res) {
    free(res);
}

// Puntero a función. Se asignará a la función de ejecución que le corresponda.
client_res_t* (*client_run)(client_data_t*) = NULL;

// Bloqueo Global
client_res_t* client_global_mutex(client_data_t* data) {
    static int seed = 0xdeadbeef;
    movie_t* movie  = data->movie;

    int row = rand_r(&seed) % movie->nrows;
    int col = rand_r(&seed) % movie->ncols;

    struct timespec start;
    struct timespec end;

    client_res_t* res = client_res_new();
    if (res == NULL) {
        return NULL;
    }

    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    pthread_mutex_lock(&global_mutex);
    res->success = movie_reserve_seat(movie, col, row, data->id);
    pthread_mutex_unlock(&global_mutex);
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);

    res->duration = get_time_diff(&start, &end);

    return res;
}

// Bloqueo por Fila
client_res_t* client_per_row_mutex(client_data_t* data) {
    static int seed = 0xdeadbeef;
    movie_t* movie  = data->movie;

    int row = rand_r(&seed) % movie->nrows;
    int col = rand_r(&seed) % movie->ncols;

    struct timespec start;
    struct timespec end;

    client_res_t* res = client_res_new();
    if (res == NULL) {
        return NULL;
    }

    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    pthread_mutex_lock(&row_mutexes[row]);
    res->success = movie_reserve_seat(movie, col, row, data->id);
    pthread_mutex_unlock(&row_mutexes[row]);
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);

    res->duration = get_time_diff(&start, &end);

    return res;
}

// Bloqueo por Asiento
client_res_t* client_per_seat_mutex(client_data_t* data) {
    static int seed = 0xdeadbeef;
    movie_t* movie  = data->movie;

    int index = rand_r(&seed) % (movie->nrows * movie->ncols);
    int row   = index / movie->ncols;
    int col   = index % movie->ncols;

    struct timespec start;
    struct timespec end;

    client_res_t* res = client_res_new();
    if (res == NULL) {
        return NULL;
    }

    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    pthread_mutex_lock(&seat_mutexes[index]);
    res->success = movie_reserve_seat(movie, col, row, data->id);
    pthread_mutex_unlock(&seat_mutexes[index]);
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);

    res->duration = get_time_diff(&start, &end);

    return res;
}

client_t* client_new(movie_t* movie, int id, locking_method method) {
    client_t* client = (client_t*)malloc(sizeof(client_t));
    if (client == NULL) {
        return NULL;
    }

    switch (method) {
    case GLOBAL:
        client_run = client_global_mutex;
        break;
    case ROW:
        client_run = client_per_row_mutex;
        break;
    case SEAT:
        client_run = client_per_seat_mutex;
        break;
    default:
        client_run = NULL;
        free(client);
        return NULL;
    }

    client_data_init(&client->data, movie, id);
    return client;
}

// Inicia el hilo del cliente.
int client_start(client_t* client) {
    assert(client != NULL);
    return pthread_create(&client->thread, NULL, (void* (*)(void*))client_run, &client->data);
}

// Libera la memoria ocupada por el cliente.
void client_free(client_t* client) {
    free(client);
}

bool client_init_mutexes(locking_method method, int nrows, int ncols) {
    int nseats;

    switch (method) {
    case GLOBAL:
        pthread_mutex_init(&global_mutex, NULL);
        break;
    case ROW:
        row_mutexes = malloc(nrows * sizeof(pthread_mutex_t));
        if (row_mutexes == NULL) {
            perror("Failed to allocate memory for row mutexes");
            return false;
        }
        for (int i = 0; i < nrows; i++) {
            pthread_mutex_init(&row_mutexes[i], NULL);
        }
        break;
    case SEAT:
        nseats       = nrows * ncols;
        seat_mutexes = malloc(nseats * sizeof(pthread_mutex_t));
        if (seat_mutexes == NULL) {
            perror("Failed to allocate memory for seat mutexes");
            return false;
        }
        for (int i = 0; i < nseats; i++) {
            pthread_mutex_init(&seat_mutexes[i], NULL);
        }
        break;
    default:
        perror("Invalid locking method specified");
        return false;
    }
    return true;
}

// Destruye los mutexes según el método de reserva seleccionado.
void client_destroy_mutexes(locking_method method, int nrows, int ncols) {
    switch (method) {
    case GLOBAL:
        break;
    case ROW:
        if (row_mutexes != NULL) {
            for (int i = 0; i < nrows; i++) {
                pthread_mutex_destroy(&row_mutexes[i]);
            }
            free(row_mutexes);
            row_mutexes = NULL;
        }
        break;
    case SEAT:
        if (seat_mutexes != NULL) {
            for (int i = 0; i < nrows * ncols; i++) {
                pthread_mutex_destroy(&seat_mutexes[i]);
            }
            free(seat_mutexes);
            seat_mutexes = NULL;
        }
        break;
    default:
        exit(EXIT_FAILURE);
    }
}
