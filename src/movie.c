#include "movie.h"
#include <assert.h>
#include <stdlib.h>

// Crea una nueva sala de cine.
movie_t* movie_new(unsigned int cols, unsigned int rows) {
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

    movie->seats   = seats;
    movie->ncols   = cols;
    movie->nrows   = rows;

    return movie;
}

// Intenta reservar un asiento en la sala.
bool movie_reserve_seat(movie_t* m, unsigned int col, unsigned int row, int id) {
    assert(m != NULL);

    if (col >= m->ncols || row >= m->nrows){
        return false;
    }

    seat_t* seat = &m->seats[(row * m->ncols) + col];
    if (seat->reserved){
        return false;
    }

    seat->reserved = true;
    seat->user_id  = id;

    return true;
}

// Libera la memoria asociada con la sala de cine.
void movie_free(movie_t* m) {
    if (m != NULL) {
        free(m->seats);
        free(m);
    }
}
