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

typedef struct options_s {
    unsigned int cols;
    unsigned int rows;
    unsigned int threads;
    int method; // Método de reserva
} options_t;

// Función que muestra el uso del programa en la línea de comandos.
void usage(char* prog) {
    printf("Uso: %s [FLAGS]\n\n", prog);
    printf("FLAGS:\n");
    printf("-c\tNúmero de columnas en la sala de cine. Default: %d\n", DEFAULT_COLUMNS);
    printf("-h\tImprime este texto de ayuda\n");
    printf("-r\tNúmero de filas en la sala de cine. Default: %d\n", DEFAULT_ROWS);
    printf("-t\tNúmero de hilos cliente a crear. Default: %d\n", DEFAULT_THREADS);
    printf("-m\tMétodo de reserva (0: global, 1: por fila, 2: por asiento). Default: 0\n");
}

// Función principal que ejecuta el programa.
int run(const options_t* opts) {
    assert(opts != NULL); // Asegura que las opciones no sean NULL.
    srand((unsigned)time(NULL)); // Inicializa la semilla

    // Crea la sala de cine con el número especificado de filas y columnas.
    movie_t* movie = movie_new(opts->cols, opts->rows);
    if (movie == NULL) {
        fprintf(stderr, "Fallo al crear la sala de cine\n");
        return -1;
    }

    // Inicializa los mutexes según el método de reserva elegido.
    client_init_mutexes(opts->method, opts->rows, opts->cols);

    // Reserva memoria para almacenar los clientes.
    client_t** clients = (client_t**)calloc(opts->threads, sizeof(client_t*));
    if (clients == NULL) {
        fprintf(stderr, "Fallo al crear clientes\n");
        client_destroy_mutexes(opts->method, opts->rows, opts->cols);
        movie_free(movie);
        return -1;
    }

    // Inicializa y ejecuta todos los hilos de los clientes.
    for (int i = 0; i < opts->threads; i++) {
        clients[i] = client_new(movie, i, opts->method);
        if (clients[i] == NULL) {
            fprintf(stderr, "Fallo al crear cliente %d\n", i);
            goto cleanup;
        }
        if (client_start(clients[i]) != 0) {
            fprintf(stderr, "Fallo al iniciar cliente %d\n", i);
            goto cleanup;
        }
    }

    // Espera a que todos los hilos de los clientes terminen y maneja sus resultados.
    for (int i = 0; i < opts->threads; i++) {
        client_res_t* res = NULL;
        if (pthread_join(clients[i]->thread, (void**)&res) != 0) {
            fprintf(stderr, "Fallo al esperar el hilo del cliente %d\n", i);
            continue;
        }

        if (res == NULL) {
            fprintf(stderr, "Fallo al recibir resultado de cliente %d\n", i);
            continue;
        }

        printf("Cliente %d: %s\n", i, res->success ? "Reserva exitosa" : "Reserva fallida");
        client_res_free(res);
    }

// Limpia la memoria y recursos utilizados.
cleanup:
    for (int i = 0; i < opts->threads && clients[i] != NULL; i++) {
        client_free(clients[i]);
    }
    free(clients);
    client_destroy_mutexes(opts->method, opts->rows, opts->cols);
    movie_free(movie);
    return 0;
}

int main(int argc, char* argv[]) {
    options_t opts = {
        .cols    = DEFAULT_COLUMNS, // Columnas por defecto
        .rows    = DEFAULT_ROWS,    // Filas por defecto
        .threads = DEFAULT_THREADS, // Hilos por defecto
        .method  = 0               // Mutex Global por defecto
    };
    int opt = -1;
    char *endptr;

    // Línea de comandos.
    while ((opt = getopt(argc, argv, "hc:r:t:m:")) != -1) {
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
        case 'm':
            opts.method = strtol(optarg, &endptr, 10);
            if (*endptr != '\0' || opts.method < 0 || opts.method > 2) {
                fprintf(stderr, "Método de reserva inválido: '%s'.\n", optarg);
                return -1;
            }
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
