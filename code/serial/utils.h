#ifndef UTILS_H
#define UTILS_H 

// TODO: Switch from vis interval to fraction in [0, 1]

#include <getopt.h>
#include <stdbool.h>
#include <time.h>
#include "globals.h"

static const char *short_opts = "c:r:t:i:s:o:v:p:h?";
static const struct option long_opts[] = {
    { "columns", required_argument, NULL, 'c' },
    { "rows", required_argument, NULL, 'r' },
    { "tsteps", required_argument, NULL, 't' },
    { "output", required_argument, NULL, 'o' },
    { "input", required_argument, NULL, 'i' },
    { "seed", optional_argument, NULL, 's' },
    { "vis_interval", required_argument, NULL, 'v' },
    { "init_prob", required_argument, NULL, 'p' },
    { "help", no_argument, NULL, 'h' },
    { NULL, no_argument, NULL, 0 }
};

void show_usage() {
    printf("\nUsage: gol [options]\n");
    printf("  -c|--columns number       Number of columns in grid. Default: %d\n", DEFAULT_SIZE_ROWS);
    printf("  -r|--rows number          Number of rows in grid. Default: %d\n", DEFAULT_SIZE_COLS);
    printf("  -t|--tsteps number        Number of timesteps to run. Default: %d\n", DEFAULT_TIMESTEPS);
    printf("  -s|--seed number          Random seed initializer. Default: %d\n", DEFAULT_SEED);
    printf("  -i|--input filename       Input file. See README for format. Default: None.\n");
    printf("  -o|--output filename      Output file. Default: None.\n");
    printf("  -h|--help                 Show this help page.\n");
    printf("  -v[N]|--vis_interval[=N]  Display frequency of the grid. Default: %d\n",
            DEFAULT_VIS_INTERVAL);
    printf("\nSee README for more information.\n\n");

    exit(EXIT_FAILURE);
}

void load_defaults(struct life_t *life) {
    life->vis_interval = 1.; //DEFAULT_VIS_INTERVAL * DEFAULT_TIMESTEPS;
    life->num_cols = DEFAULT_SIZE_COLS;
    life->num_rows = DEFAULT_SIZE_ROWS;
    life->timesteps = DEFAULT_TIMESTEPS;
    life->init_prob = DEFAULT_INIT_PROB;
    life->input_file = NULL;
    life->output_file = (char*) DEFAULT_OUT_FILE;
    life->seed = DEFAULT_SEED;
}

void parse_args(struct life_t *life, int argc, char **argv) {
    int opt = 0;
    int opt_idx = 0;
    int i;

    // TODO: Load all defaults into life
    load_defaults(life);
    
    for (;;) {
        opt = getopt_long(argc, argv, short_opts, long_opts, &opt_idx);

        if (opt == -1) break;

        switch (opt) {
            case 'c':
                life->num_cols = strtol(optarg, (char **) NULL, 10);
                break;
            case 'r':
                life->num_rows = strtol(optarg, (char **) NULL, 10);
                break;
            case 't':
                if (optarg != NULL)
                    life->timesteps = strtol(optarg, (char **) NULL, 10);
                break;
            case 's':
                if (optarg != NULL) {
                    int seed = strtol(optarg, (char **) NULL, 10);

                    if (seed == 0) {
                        life->seed = (unsigned int) time(NULL);
                    } else {
                        life->seed = (unsigned int) seed;
                    }
                }
                break;
            case 'i':
                life->input_file = optarg;
                break;
            case 'o':
                life->output_file = optarg;
                break;
            case 'v':
                if (optarg != NULL)
                    life->vis_interval = strtol(optarg, (char **) NULL, 10);
                break;
            case 'p':
                if (optarg != NULL)
                    life->init_prob = strtod(optarg, (char **) NULL);
                break;
            case 'h':
            case '?':
                show_usage();
                break;
            default:
                break;
        }
    }

    // Read arguments when opts are not specified 
    if (optind == 1) {
        if (argc > 1)
            life->num_rows = strtol(argv[1], (char **) NULL, 10);

        if (argc > 2)
            life->num_cols = strtol(argv[2], (char **) NULL, 10);

        if (argc > 3)
            life->timesteps = strtol(argv[3], (char **) NULL, 10);

        if (argc > 4)
            life->output_file = optarg;
    }
}

/**
 * Generate a random double from min to max. Please, note that RAND_MAX returns a 32 bit integer, whereas a double has 53 bits of mantissa, by IEEE-754 standard. This means that there may be many more double values left out in the specified range.
 */
double rand_double(double min, double max) {
    double range = max - min;
    double div = RAND_MAX / range;

    return min + (double) random() / div;
}

bool is_big(struct life_t *life) {
    if (life->num_rows * life->num_cols > DEFAULT_MAX_SIZE)
        return true;
    else 
        return false;
}

/**
 * @return The pointer to the open input file, for later use. 
 */
FILE * set_grid_dimens_from_file(struct life_t *life) {
    FILE *file_ptr;

    if (life->input_file != NULL) {
        if ((file_ptr = fopen(life->input_file, "r")) == NULL) {
            perror("Failed to open input file.\nLaunching the simulation in default configuration...\n");
        } else if (fscanf(file_ptr, "%d %d\n", &life->num_cols, &life->num_rows) == EOF) {
            perror("Input file must at least define grid dimensions!\nLaunching the simulation in default configuration...\n");
        } else {
            return file_ptr;
        }
    }

    // An error has occured
    return NULL;
}

void malloc_grid(struct life_t *life) {
    int i, j;

    int ncols = life->num_cols;
    int nrows = life->num_rows;

    // Add 2 rows/columns to account for ghost rows.
    life->grid      = (unsigned **) malloc(sizeof(unsigned *) * (ncols + 2));
    life->next_grid = (unsigned **) malloc(sizeof(unsigned *) * (ncols + 2));

    for (i = 0; i < ncols + 2; i++) {
        life->grid[i]      = (unsigned *) malloc(sizeof(unsigned) * (nrows + 2));
        life->next_grid[i] = (unsigned *) malloc(sizeof(unsigned) * (nrows + 2));
    }
}

void init_empty_grid(struct life_t *life) {
    int i, j;
  
    for (i = 0; i < life->num_cols + 2; i++)
        for (j = 0; j < life->num_rows + 2; j++) {
            life->grid[i][j]      = DEAD;
            life->next_grid[i][j] = DEAD;
        }
}

void init_from_file(struct life_t *life, FILE *file_ptr) {
    int i, j;

    if(life->input_file != NULL)
        // Every line from the file will contain row/column coordinates
        // of every cell that has to be initialized as ALIVE.
        while (fscanf(file_ptr, "%d %d\n", &i, &j) != EOF) {
            life->grid[i][j]      = ALIVE;
            life->next_grid[i][j] = ALIVE;
        }

    fclose(file_ptr);
}

void init_random(struct life_t *life) {
    int x, y;

    for (x = 0; x < life->num_cols; x++) 
        for (y = 0; y < life->num_rows; y++) { 
            if (rand_double(0., 1.) < life->init_prob)
                life->grid[y][x] = ALIVE;
        }
}

#endif