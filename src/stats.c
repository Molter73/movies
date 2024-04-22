#include "stats.h"
#include <assert.h>
#include <linux/limits.h>
#include <stdio.h>

uint64_t get_time_diff(const struct timespec* start, const struct timespec* end) {
    assert(start != NULL);
    assert(end != NULL);
    return (end->tv_sec - start->tv_sec) * 1000000000 + (end->tv_nsec - start->tv_nsec);
}

void dump_thread_stats(const options_t* opts, const global_stats_t* gs, client_res_t** results) {
    assert(opts != NULL);
    assert(gs != NULL);
    assert(results != NULL);

    FILE* output_file = stdout;
    char path[PATH_MAX];

    // Genera Threads_Fila_Columnas_Método
    snprintf(path, PATH_MAX, "%s/threads_%d_%d_%d_%d%s.csv", opts->output, opts->threads, opts->rows, opts->cols,
             opts->method, opts->synthetic_load ? "_loaded" : "");
    output_file = fopen(path, "w");
    if (output_file == NULL) {
        fprintf(stderr, "Fallo al crear archivo de salida de hilos %s\n", path);
        return;
    }

    fprintf(output_file, "Estado reserva,Duracion,\n");
    for (int i = 0; i < opts->threads; i++) {
        if (results[i] == NULL) {
            continue;
        }
        fprintf(output_file, "%d,%lu\n", results[i]->success, results[i]->duration);
    }

    fclose(output_file);
}

void dump_global_stats(const options_t* opts, const global_stats_t* gs, client_res_t** results) {
    assert(opts != NULL);
    assert(gs != NULL);
    assert(results != NULL);

    FILE* output_file = stdout;
    char path[PATH_MAX];
    unsigned int successes = 0;

    if (opts->output != NULL) {
        snprintf(path, PATH_MAX, "%s/global_stats_%d_%d_%d_%d%s.csv", opts->output, opts->threads, opts->rows,
                 opts->cols, opts->method, opts->synthetic_load ? "_loaded" : "");
        output_file = fopen(path, "w");
        if (output_file == NULL) {
            fprintf(stderr, "Fallo al crear archivo de salida %s\n", path);
            return;
        }
    }

    for (int i = 0; i < opts->threads; i++) {
        if (results[i] != NULL && results[i]->success) {
            successes++;
        }
    }

    fprintf(output_file, "Duracion,Asientos,Reservas exitosas,\n");
    fprintf(output_file, "%lu,%u,%u,\n", gs->time_taken, gs->seats, successes);

    if (opts->output != NULL) {
        fclose(output_file);
    }
}

void dump_stats(const options_t* opts, const global_stats_t* gs, client_res_t** results) {
    assert(opts != NULL);
    assert(gs != NULL);
    assert(results != NULL);

    dump_global_stats(opts, gs, results);
    if (opts->output != NULL) {
        dump_thread_stats(opts, gs, results);
    }
}
