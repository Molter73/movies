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
            seat_t* seat = &seats[i + j];

            seat->col      = j;
            seat->row      = i;
            seat->reserved = false;
            seat->user_id  = -1;
        }
    }

    // Inicializamos la sala de cine
    movie->seats   = seats;
    movie->ncols   = cols;
    movie->nrows   = rows;
    movie->vacancy = cols * rows;

    return movie;
}

bool movie_reserve_seat(movie_t* m, unsigned int col, unsigned int row, int id) {
    assert(m != NULL);

    if (col >= m->nrows || row >= m->nrows) {
        return false;
    }

    seat_t* seat = &m->seats[col + row];

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
        free(m->seats);
        free(m);
    }
}
