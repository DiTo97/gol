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
    printf("\nUsage [1]: gol [options]\n");
    printf("  -c|--columns number       Number of columns in grid. Default: %d\n", DEFAULT_SIZE_COLS);
    printf("  -r|--rows number          Number of rows in grid. Default: %d\n", DEFAULT_SIZE_ROWS);
    printf("  -t|--tsteps number        Number of timesteps to run. Default: %d\n", DEFAULT_TIMESTEPS);
    printf("  -s|--seed number          Random seed initializer. Default: %d\n", DEFAULT_SEED);
    printf("  -p|--init_prob number     Probability for grid initialization. Default: %d\n", DEFAULT_SEED);
    printf("  -i|--input filename       Input file. See README for format. Default: None.\n");
    printf("  -o|--output filename      Output file. Default: %s.\n", DEFAULT_OUT_FILE);
    printf("  -h|--help                 Show this help page.\n");
    printf("  -v|--vis_interval         Display frequency of the grid. Default: %f\n\n",DEFAULT_VIS_INTERVAL);

    printf("\nUsage (specify in the following order (at least one argument)) [2]: gol [no options]\n");
    printf("  1) Number of columns in grid. Default: %d\n", DEFAULT_SIZE_ROWS);
    printf("  2) Number of rows in grid. Default: %d\n", DEFAULT_SIZE_COLS);
    printf("  3) Number of timesteps to run. Default: %d\n", DEFAULT_TIMESTEPS);
    printf("  4) Output file. Default: %s.\n", DEFAULT_OUT_FILE);
    printf("  5) Display frequency of the grid. Default: %f\n", DEFAULT_VIS_INTERVAL);
    printf("  6) Random seed initializer. Default: %d\n", DEFAULT_SEED);
    printf("  7) Probability used for grid initialization. Default: %d\n\n", DEFAULT_INIT_PROB);

    printf("\nUsage (specify in the following order) [3]: gol [no options]\n");
    printf("  1) Input file. Default: None.\n");
    printf("  2) Number of timesteps to run. Default: %d\n", DEFAULT_TIMESTEPS);
    printf("  3) Output file. Default: %s.\n", DEFAULT_OUT_FILE);
    printf("  4) Display frequency of the grid. Default: %f\n\n", DEFAULT_VIS_INTERVAL);
            
    printf("\nSee README for more information.\n\n");

    exit(EXIT_FAILURE);
}

FILE* init_log_file(struct life_t * life) {
    char buffer[100];
    if (life->input_file!=NULL) {
        sprintf(buffer, "nc%d_nr%d_nt%d_%lu.dat", life->num_cols, life->num_rows,
                life->timesteps, (unsigned long) time(NULL));
    }
    else {
        sprintf(buffer, "nc%d_nr%d_nt%d_prob%.1f_seed%d_%lu.dat", life->num_cols, life->num_rows,
                life->timesteps, life->init_prob, life->seed, (unsigned long) time(NULL));
    }
    FILE * log_file = fopen(buffer, "a");
    fprintf(log_file, "timestep\tcurr_time\ttotal_time\n");
    return log_file;
}

// timestep, curr_time
void log_data(FILE* log_file, int timestep, double curr_time, double total_time) {
    fprintf(log_file,"%d\t%.5f\t%.5f\n", timestep, curr_time, total_time);
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
/* This function may give rise to problems when specyfing the -restrict flag  */
void parse_args(struct life_t *life, int argc, char **argv) {
    int opt = 0;
    int opt_idx = 0;
    int i;

    unsigned opt_param_count = 0;
    int limit = argc - 1;
    // controlling that command line options are not malformed
    
    for(i = 1; i < argc; i++) {
        if (strstr(argv[i], "-h")!=NULL) {
            show_usage();
            exit(0);
        }
        if(strchr(argv[i], '-') != NULL) {
            opt_param_count++;
        }
    } 
    int diff = limit - opt_param_count;
    if (diff!=opt_param_count && diff!=limit) {
        printf("\nExiting the program: malformed command line argument sequence!\n");
        exit(0);
    }

    // TODO: Load all defaults into life
    load_defaults(life);
    if (opt_param_count > 0) {
        printf("\nParsing arguments with options...\n");
        for (;;) {
            // return the option character when a short option is recognized. 
            // For a long option, they return val if flag is NULL, and 0 otherwise
            opt = getopt_long(argc, argv, short_opts, long_opts, &opt_idx);

            if (opt == -1) break;

            // TODO Add optarg != NULL in tutti i case
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
                        life->vis_interval = strtod(optarg, (char **) NULL);
                    break;
                case 'p':
                    if (optarg != NULL)
                        life->init_prob = strtod(optarg, (char **) NULL);
                    break;
                case '?':
                    show_usage();
                    exit(0);
                default:
                    break;
            }
        }
    }
    else {       
        printf("\nParsing arguments without options...\n");
        /*
            In case of ordinal arguments, you must specify them either as the sequence

                num_cols, (num_rows, timesteps, output_file, vis_interval, seed, init_prob)
            
            when not reading from file. Viceversa, you should specify them as
            
                input_file, (timesteps, output_file, vis_interval)
                
            Round brackets mean that the included parameters may not be specified directly. In that case, 
            they will be set to the default values defined in globals.h
        */

        long int parsed_arg;
        if (limit > 1)
            parsed_arg = strtol(argv[1], (char **) NULL, 10);
        if (argc > 1) {
            if (parsed_arg!=0) {
                life->num_cols = parsed_arg;
            }
            else {
                // if the user does not stick with the above specifications
                // we have to handel at least the fact that argv[1] is a string
                // and has an txt extension
                life->input_file = argv[1];
            }
        }

        if (argc > 2) {
            if (parsed_arg!=0) 
                life->num_rows = strtol(argv[2], (char **) NULL, 10);
            else 
                life->timesteps = strtol(argv[2], (char **) NULL, 10);
        }

        if (argc > 3) {
            if (parsed_arg!=0)
                life->timesteps = strtol(argv[3], (char **) NULL, 10);
            else
                life->output_file = argv[3];
        }

        if (argc > 4) {
            if (parsed_arg!=0)
                life->output_file = argv[4];
            else    
                life->vis_interval = strtod(argv[4], (char **) NULL);
        }

        if (argc > 5) {
            if (parsed_arg!=0)
                life->vis_interval = strtod(argv[5], (char **) NULL);
        }

        if (argc > 6) {
            if (parsed_arg!=0) {
                int seed = strtol(argv[6], (char **) NULL, 10);
                if (seed == 0) {
                    life->seed = (unsigned int) time(NULL);
                } else {
                    life->seed = (unsigned int) seed;
                }
            }
        }

        if (argc > 7) {
            if (parsed_arg!=0)
                life->init_prob = strtod(argv[7], (char **) NULL);
        }
        
    }
}

/**
 * Generate a random double from min to max. Please, note that RAND_MAX returns a 
 * 32 bit integer, whereas a double has 53 bits of mantissa, by IEEE-754 standard. 
 * This means that there may be many more double values left out in the specified range.
 */
double rand_double(double min, double max) {
    double range = max - min;
    double div = RAND_MAX / range;

    return min + (double) random() / div;
}

/**
 * Function that control if the output has to be visualized in the console,
 * or it has to be printed on file.
 * @return true if the output is printed on file, false otherwise
 */
bool is_big(struct life_t *life) {
    if (life->num_rows * life->num_cols > DEFAULT_MAX_SIZE)
        return true;
    else 
        return false;
}

/**
 * @return The pointer to the open input file, if can be opened and has a valid format, 
 * NULL otherwise. 
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

/**
 * Function that allocates the memory for the grid
 */
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

/**
 * Function that initialize the grids with DEAD values
 */
void init_empty_grid(struct life_t *life) {
    int i, j;
  
    for (i = 0; i < life->num_cols + 2; i++)
        for (j = 0; j < life->num_rows + 2; j++) {
            life->grid[i][j]      = DEAD;
            life->next_grid[i][j] = DEAD;
        }
}

/**
 * Funciton that initialize the grids from the file passed as parameter
 * 
 * This function may give rise to problems when specyfing the -restrict flag 
 */
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

/**
 * Function that initializes the grid with ALIVE values at random positions
 */
void init_random(struct life_t *life) {
    int x, y;

    for (x = 0; x < life->num_cols; x++) 
        for (y = 0; y < life->num_rows; y++) { 
            if (rand_double(0., 1.) < life->init_prob)
                life->grid[y][x] = ALIVE;
        }
}

#endif
