#ifndef _STATS_H_
#define _STATS_H_

#include "client.h"
#include "definitions.h"

#include <stdint.h>
#include <time.h>

typedef struct global_stats_s {
    unsigned int seats;  // Cantidad de asientos total
    uint64_t time_taken; // Tiempo que demora la ejecución en microsegundos
} global_stats_t;

uint64_t get_time_diff(const struct timespec* start, const struct timespec* end);
void dump_stats(const options_t* opts, const global_stats_t* gs, client_res_t** results);

#endif
