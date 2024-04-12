#include "client.h"
#include "definitions.h"
#include "movie.h"

#include <assert.h>
#include <bits/getopt_core.h>
#include <linux/limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct options_s {
    unsigned int cols;
    unsigned int rows;
    unsigned int threads;
} options_t;

void usage(char* prog) {
    printf("Uso: %s [FLAGS]\n\n", prog);
    printf("FLAGS:\n");
    printf("-c\tNúmero de columnas en la sala de cine. Default: %d\n", DEFAULT_COLUMNS);
    printf("-h\tImprime este texto de ayuda\n");
    printf("-r\tNúmero de filas en la sala de cine. Default: %d\n", DEFAULT_ROWS);
    printf("-t\tNúmero de hilos cliente a crear. Default: %d\n", DEFAULT_THREADS);
}

int run(const options_t* opts) {
    assert(opts != NULL);
    int res = -1;

    movie_t* movie = movie_new(opts->cols, opts->rows);
    if (movie == NULL) {
        fprintf(stderr, "Fallo al crear la sala de cine\n");
        return -1;
    }

    client_t** clients = (client_t**)calloc(opts->threads, sizeof(client_t*));
    if (clients == NULL) {
        fprintf(stderr, "Fallo al crear clientes\n");
        goto cleanup;
    }

    // Creamos opts->threads clientes concurrentes.
    for (int i = 0; i < opts->threads; i++) {
        client_t* client = client_new(movie, i);
        if (client == NULL) {
            fprintf(stderr, "Fallo al crear cliente %d\n", i);
            goto cleanup;
        }

        clients[i] = client;
        if (!client_start(client)) {
            fprintf(stderr, "Fallo al iniciar cliente %d\n", i);
            goto cleanup;
        }
    }

    // Esperamos a que todos los clientes finalicen.
    for (int i = 0; i < opts->threads; i++) {
        pthread_join(clients[i]->thread, NULL);
    }

    res = 0;

cleanup:
    for (int i = 0; i < opts->threads && clients[i] != NULL; i++) {
        client_free(clients[i]);
    }
    free(clients);
    movie_free(movie);
    return res;
}

int main(int argc, char* argv[]) {
    options_t opts = {
        .cols    = DEFAULT_COLUMNS,
        .rows    = DEFAULT_ROWS,
        .threads = DEFAULT_THREADS,
    };
    int opt = -1;

    while ((opt = getopt(argc, argv, "hc:r:t:")) != -1) {
        switch (opt) {
        case 'h':
            usage(argv[0]);
            return 0;
        case 'c':
            opts.cols = strtoul(optarg, NULL, 10);
            break;
        case 'r':
            opts.rows = strtoul(optarg, NULL, 10);
            break;
        case 't':
            opts.threads = strtoul(optarg, NULL, 10);
            break;
        default:
            usage(argv[0]);
            return -1;
        }
    }

    if (argc - optind != 0) {
        fprintf(stderr, "Parámetros desconocidos.\n\n");
        usage(argv[0]);
        return -1;
    }

    return run(&opts);
}
