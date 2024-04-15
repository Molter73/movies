#include "movie.h"
#include <assert.h>
#include <stdlib.h>

movie_t* movie_new(unsigned int cols, unsigned int rows) {
    seat_t* seats = (seat_t*)malloc(cols * rows * sizeof(seat_t));
    if (seats == NULL) {
        return NULL;
    }

    movie_t* movie = (movie_t*)malloc(sizeof(movie_t));
    if (movie == NULL) {
        free(seats);
        return NULL;
    }

    // Inicializamos los asientos
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            seat_t* seat = &seats[(i * cols) + j];

            seat->col      = j;
            seat->row      = i;
            seat->reserved = false;
            seat->user_id  = -1;
        }
    }
    movie->seats   = seats;
    movie->ncols   = cols;
    movie->nrows   = rows;
    movie->vacancy = cols * rows;

    // Inicialización del semáforo global
    sem_init(&movie->global_lock, 0, 1);

    // Inicialización de semáforos para cada fila
    movie->row_locks = malloc(rows * sizeof(sem_t));
    if (movie->row_locks == NULL) {
        free(seats);
        free(movie);
        return NULL;
    }
    for (unsigned int i = 0; i < rows; i++) {
        sem_init(&movie->row_locks[i], 0, 1);
    }

    // Inicialización de semáforos para cada asiento
    movie->seat_locks = malloc(cols * rows * sizeof(sem_t));
    if (movie->seat_locks == NULL) {
        free(movie->row_locks);
        free(seats);
        free(movie);
        return NULL;
    }
    for (unsigned int i = 0; i < cols * rows; i++) {
        sem_init(&movie->seat_locks[i], 0, 1);
    }

    // Configuración inicial de los asientos
    for (unsigned int i = 0; i < rows; i++) {
        for (unsigned int j = 0; j < cols; j++) {
            int index             = i * cols + j;
            seats[index].col      = j;
            seats[index].row      = i;
            seats[index].reserved = false;
            seats[index].user_id  = -1;
        }
    }

    return movie;
}

bool movie_reserve_seat(movie_t* m, unsigned int col, unsigned int row, int id) {
    assert(m != NULL);

    if (col >= m->ncols || row >= m->nrows) {
        return false;
    }

    seat_t* seat = &m->seats[(row * m->ncols) + col];
    if (seat->reserved) {
        return false;
    }

    seat->reserved = true;
    seat->user_id  = id;
    m->vacancy--;
    return true;
}

bool movie_has_vacants(const movie_t* m) {
    return m->vacancy != 0;
}

void movie_free(movie_t* m) {
    if (m != NULL) {
        // Destruir el semáforo global
        sem_destroy(&m->global_lock);

        // Destruir semáforos de cada fila
        if (m->row_locks != NULL) {
            for (unsigned int i = 0; i < m->nrows; i++) {
                sem_destroy(&m->row_locks[i]);
            }
            free(m->row_locks);
        }

        // Destruir semáforos de cada asiento
        if (m->seat_locks != NULL) {
            for (unsigned int i = 0; i < m->ncols * m->nrows; i++) {
                sem_destroy(&m->seat_locks[i]);
            }
            free(m->seat_locks);
        }

        // Liberar los asientos
        if (m->seats != NULL) {
            free(m->seats);
        }

        // Finalmente, liberar la estructura de la película
        free(m);
    }
}
