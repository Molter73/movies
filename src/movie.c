#include "movie.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

// Crea una nueva sala de cine.
movie_t* movie_new(unsigned int cols, unsigned int rows, bool synthetic_load) {
    // Asigna memoria para los asientos y la estructura de la sala de cine.
    seat_t* seats = (seat_t*)malloc(cols * rows * sizeof(seat_t));
    if (seats == NULL) {
        return NULL;
    }

    movie_t* movie = (movie_t*)malloc(sizeof(movie_t));
    if (movie == NULL) {
        free(seats);
        return NULL;
    }

    // Inicializa los asientos como no reservados.
    for (unsigned int i = 0; i < rows; i++) {
        for (unsigned int j = 0; j < cols; j++) {
            seat_t* seat   = &seats[(i * cols) + j];
            seat->col      = j;
            seat->row      = i;
            seat->reserved = false;
            seat->user_id  = -1;
        }
    }

    movie->seats = seats;
    movie->ncols = cols;
    movie->nrows = rows;
    movie->synthetic_load = synthetic_load;

    return movie;
}

// Intenta reservar un asiento en la sala.
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

    if (m->synthetic_load) {
        // Simulamos trabajo adicional en la hebra
        static const struct timespec hold = {
            .tv_sec = 0,
            .tv_nsec = 200,
        };
        nanosleep(&hold, NULL);
    }

    return true;
}

// Libera la memoria asociada con la sala de cine.
void movie_free(movie_t* m) {
    if (m != NULL) {
        free(m->seats);
        free(m);
    }
}
