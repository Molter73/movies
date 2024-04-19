#ifndef _MOVIE_H_
#define _MOVIE_H_

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
} movie_t;

movie_t* movie_new(unsigned int cols, unsigned int rows);
bool movie_reserve_seat(movie_t* m, unsigned int col, unsigned int row, int id);
void movie_free(movie_t* m);

#endif
