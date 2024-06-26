#ifndef _DEFINITIONS_H_
#define _DEFINITIONS_H_

#include <stdbool.h>

#define DEFAULT_COLUMNS (15)
#define DEFAULT_ROWS    (10)
#define DEFAULT_THREADS (DEFAULT_ROWS * DEFAULT_COLUMNS)

typedef struct options_s {
    unsigned int cols;
    unsigned int rows;
    unsigned int threads;
    int method; // Método de reserva
    char* output;
    bool synthetic_load;
} options_t;

#endif
