#ifndef _MOVIE_H_
#define _MOVIE_H_

#include <semaphore.h>
#include <stdbool.h>

typedef struct seat_s {
    unsigned int row;
    unsigned int col;
    bool reserved;
    int user_id;
} seat_t;

typedef struct movie_s {
    seat_t* seats;
    unsigned int nrows;
    unsigned int ncols;
    unsigned int vacancy;
    sem_t global_lock; // Semáforo para bloqueo global
    sem_t* row_locks;  // Array de semáforos para cada fila
    sem_t* seat_locks; // Array de Semáforos, uno por asiento.
} movie_t;

movie_t* movie_new(unsigned int cols, unsigned int rows);
bool movie_reserve_seat(movie_t* m, unsigned int col, unsigned int row, int id);
bool movie_has_vacants(const movie_t* m);
void movie_free(movie_t* m);

#endif
