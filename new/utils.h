#ifndef GoL_UTILS_H
#define GoL_UTILS_H 

#include <getopt.h>
#include <stdbool.h>
#include <time.h>
#include "globals.h"

/*********************
 * Parsing functions *
 *********************/  

static const char *short_opts = "c:r:t:i:s:o:p:h?";
static const struct option long_opts[] = {
    { "columns", required_argument, NULL, 'c' },
    { "rows", required_argument, NULL, 'r' },
    { "tsteps", required_argument, NULL, 't' },
    { "output", required_argument, NULL, 'o' },
    { "input", required_argument, NULL, 'i' },
    { "seed", optional_argument, NULL, 's' },
    { "init_prob", required_argument, NULL, 'p' },
    { "help", no_argument, NULL, 'h' },
    { NULL, no_argument, NULL, 0 }
};

/**
 * Print all possible command line options settings to console and terminate.
 */
void show_usage() {
    printf("\nUsage [1]: gol [opts]\n");
    printf("  -c|--columns number       Number of columns in grid. Default: %d\n", DEFAULT_SIZE_COLS);
    printf("  -r|--rows number          Number of rows in grid. Default: %d\n", DEFAULT_SIZE_ROWS);
    printf("  -t|--tsteps number        Number of timesteps to run. Default: %d\n", DEFAULT_TIMESTEPS);
    printf("  -s|--seed number          Random seed initializer. Default: %d\n", DEFAULT_SEED);
    printf("  -p|--init_prob number     Probability for grid initialization. Default: %f\n", DEFAULT_INIT_PROB);
    printf("  -i|--input filename       Input file. See README for format. Default: None.\n");
    printf("  -o|--output filename      Output file. Default: %s.\n", DEFAULT_OUT_FILE);
    printf("  -h|--help                 Show this help page.\n\n");

    printf("\nUsage [2] (in the following order): gol [no opts]\n");
    printf("  1) Number of columns in grid. Default: %d\n", DEFAULT_SIZE_ROWS);
    printf("  2) Number of rows in grid. Default: %d\n", DEFAULT_SIZE_COLS);
    printf("  3) Number of timesteps to run. Default: %d\n", DEFAULT_TIMESTEPS);
    printf("  4) Output file. Default: %s.\n", DEFAULT_OUT_FILE);
    printf("  5) Random seed initializer. Default: %d\n", DEFAULT_SEED);
    printf("  6) Probability for grid initialization. Default: %f\n\n", DEFAULT_INIT_PROB);

    printf("\nUsage [3] (in the following order): gol [no opts]\n");
    printf("  1) Input file. Default: None.\n");
    printf("  2) Number of timesteps to run. Default: %d\n", DEFAULT_TIMESTEPS);
    printf("  3) Output file. Default: %s\n\n", DEFAULT_OUT_FILE);
            
    printf("\nSee README for more information.\n\n");

    exit(EXIT_FAILURE);
}

/**
 * Load all default GoL settings from globals.h
 * 
 * @param life    The main data structure behind a GoL instance.
 */ 
void load_defaults(struct life_t *life) {
    life->seed        = DEFAULT_SEED;
    life->num_cols    = DEFAULT_SIZE_COLS;
    life->num_rows    = DEFAULT_SIZE_ROWS;
    life->timesteps   = DEFAULT_TIMESTEPS;
    life->init_prob   = DEFAULT_INIT_PROB;
    life->input_file  = NULL;
    life->output_file = (char*) DEFAULT_OUT_FILE;
}

/**
 * Parse the seed's value from its command line argument. A 0 value will be turned into a random seed via time(NULL).
 * 
 * @param _seed    The command line argument.
 * 
 * @return seed    The corresponding seed's unsigned int value.
 */ 
unsigned int parse_seed(char *_seed) {
    int seed = strtol(_seed, (char **) NULL, 10);

    if (seed == 0)
        return (unsigned int) time(NULL);
    else
        return (unsigned int) seed;
}

/**
 * Parse command line arguments depending on whether opts are explicitly indicated or not.
 * 
 * @todo Avoid crashes due to the -restrict flag.
 */
void parse_args(struct life_t *life, int argc, char **argv) {
    int opt = 0;
    int opt_idx = 0;
    int i;

    unsigned opt_params_count = 0;

    // Check whether command line options are malformed
    for(i = 1; i < argc; i++) {
        if (strstr(argv[i], "-h") != NULL) // If -h is included in argv show usage...
            show_usage();

        if(strchr(argv[i], '-') != NULL)   // ...else keep track of how many - are included in argv
            opt_params_count++;            // to discrimante between opts being explicitly indicated or not.
    }

    int limit = argc - 1;
    int diff  = limit - opt_params_count;

    // If diff == opt_params_count:
    //     Explicit opts were passed
    //
    // If diff == limit:
    //     No explicit opts were passed
    if (diff != opt_params_count && diff != limit) {
        perror("\n[*] Command line options are malformed!\n");
        exit(EXIT_FAILURE);
    }

    load_defaults(life);

    if (opt_params_count > 0) { // Explicit opts were passed
        printf("\nParsing arguments with options...\n\n");

        fflush(stdout);
        usleep(100000);

        for (;;) {
            // Return the subsequent long/short opt value
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
                    life->timesteps = strtol(optarg, (char **) NULL, 10);
                    break;
                case 's':
                    if (optarg != NULL)
                        life->seed = parse_seed(optarg);
                    break;
                case 'i':
                    life->input_file = optarg;
                    break;
                case 'o':
                    life->output_file = optarg;
                    break;
                case 'p':
                    life->init_prob = strtod(optarg, (char **) NULL);
                    break;
                case '?':
                    show_usage();
                default:
                    break;
            }
        }
    } else { // No explicit opts were passed  
        printf("\nParsing arguments with no options...\n\n");

        fflush(stdout);
        usleep(100000);
        
        // Non-explicit arguments must stick to the following sequence:
        //     columns, rows, (tsteps, output, seed, init_prob)
        // 
        // when not reading GoL from file. Viceversa, they should be specified as:
        //     input, (tsteps, output)
        // 
        // [*] Round brackets indicate optional arguments.

        long int parsed_arg;
        bool input_spec; // Whether an input file is specified for GoL

        // Are there enough arguments to parse?
        if (limit > 1) {
            parsed_arg = strtol(argv[1], (char **) NULL, 10);
            input_spec = (parsed_arg != 0) ? true : false;

            if (argc > 1) {
                if (input_spec)
                    life->num_cols = parsed_arg;
                else
                    life->input_file = argv[1];
            }

            if (argc > 2) {
                if (input_spec) 
                    life->num_rows = strtol(argv[2], (char **) NULL, 10);
                else 
                    life->timesteps = strtol(argv[2], (char **) NULL, 10);
            }

            if (argc > 3) {
                if (input_spec)
                    life->timesteps = strtol(argv[3], (char **) NULL, 10);
                else
                    life->output_file = argv[3];
            }

            if (argc > 4) {
                if (input_spec)
                    life->output_file = argv[4];
            }

            if (argc > 5) {
                if (input_spec)
                    life->seed = parse_seed(argv[5]);
            }

            if (argc > 6) {
                if (input_spec)
                    life->init_prob = strtod(argv[6], (char **) NULL);
            }
        }
    }
}

/*********************
 * Utility functions *
 *********************/ 

#ifdef GoL_DEBUG
void debug(struct life_t life) {
    printf("Number of cols: %d\n", life.num_cols);
    printf("Number of rows: %d\n", life.num_rows);
    printf("Number of timesteps: %d\n", life.timesteps);
    printf("Probability for grid initialization: %f\n", life.init_prob);
    printf("Random seed initializer: %d\n", life.seed);
    printf("Input file: %s\n", life.input_file == NULL ? "None" : life.input_file);
    printf("Output file: %s\n\n", life.output_file);

    fflush(stdout);
}
#endif

/**
 * Generate a random double from min to max.
 * 
 * Please, note that RAND_MAX returns a 32 bit integer, whereas a double has 53 bits of mantissa, by IEEE-754 standard. This means that there may be many more double values left out in the specified range which are not yet covered by this implementation.
 */
double rand_double(double min, double max) {
    double range = max - min;
    double div = RAND_MAX / range;

    return min + (double) random() / div;
}

/**
 * Evaluate whether the GoL board is larger than DEFAULT_MAX_SIZE.
 * 
 * @return true if GoL grid larger, false otherwise
 */
bool is_big(struct life_t life) {
    return life.num_rows * life.num_cols > DEFAULT_MAX_SIZE;
}

/*********************
 * Logging functions *
 *********************/ 

/**
 * Initialize a tab-separated log file, whose name varies with GoL configuration's settings.
 * 
 * @return log_ptr    The pointer to the tab-separated log file with (timestep, current time, total time) columns
 */
FILE* init_log_file(struct life_t life) {
    char buffer[100];

    if (life.input_file != NULL)
        sprintf(buffer, "GoL_nc%d_nr%d_nt%d_%lu.log", life.num_cols, life.num_rows,
                life.timesteps, (unsigned long) time(NULL));
    else
        sprintf(buffer, "GoL_nc%d_nr%d_nt%d_prob%.1f_seed%d_%lu.log", life.num_cols, life.num_rows,
                life.timesteps, life.init_prob, life.seed, (unsigned long) time(NULL));

    FILE *log_ptr = fopen(buffer, "a");
    fprintf(log_ptr, "timestep\tcur_time\ttot_time\n");

    // The log file's name is guaranteed to be unique until year 2038,
    // as it implies the call to time(NULL).
    return log_ptr;
}

/**
 * Log a (timestep, current time, total time) triplet onto the log file.
 * 
 * @param timestep    The current generation of GoL's evolution.
 * @param cur_time    The execution time of the current generation of GoL.
 * @param tot_time    The total execution time since GoL's evolution started.
 */
void log_data(FILE *log_ptr, int timestep, double cur_time, double tot_time) {
    fprintf(log_ptr, "%-8d\t%-8.3f\t%-8.3f\n", timestep, cur_time, tot_time); // -8, as columns are 8-char long
}

/****************************
 * Initialization functions *
 ****************************/ 

/**
 * Update the GoL board's dimensions from file, if it exists and it has a valid format.
 * 
 * @return file_ptr    The pointer to the open input file, NULL otherwise. 
 */
FILE* set_grid_dimens_from_file(struct life_t *life) {
    FILE *file_ptr;

    if (life->input_file != NULL) {
        if ((file_ptr = fopen(life->input_file, "r")) == NULL) {
            perror("[*] Failed to open the input file.\n");
            perror("[*] Launching the simulation in default configuration...\n");
        } else if (fscanf(file_ptr, "%d %d\n", &life->num_cols, &life->num_rows) == EOF) {
            perror("[*] The input file only defines GoL board's dimensions!\n");
            perror("[*] Launching the simulation in default configuration...\n");
        } else
            return file_ptr;
    }

    // An error has occured
    return NULL;
}

/**
 * Allocate memory for the current and next GoL board.
 * 
 * @todo Account for ghost rows (+ 2) with MPI.
 */
void malloc_grid(struct life_t *life) {
    int i, j;

    int ncols = life->num_cols;
    int nrows = life->num_rows;

    life->grid      = (unsigned **) malloc(sizeof(unsigned *) * (ncols));
    life->next_grid = (unsigned **) malloc(sizeof(unsigned *) * (ncols));

    for (i = 0; i < ncols; i++) {
        life->grid[i]      = (unsigned *) malloc(sizeof(unsigned) * (nrows));
        life->next_grid[i] = (unsigned *) malloc(sizeof(unsigned) * (nrows));
    }
}

/**
 * Initialize the GoL board with DEAD values.
 */
void init_empty_grid(struct life_t *life) {
    int i, j;
  
    for (i = 0; i < life->num_cols; i++)
        for (j = 0; j < life->num_rows; j++) {
            life->grid[i][j]      = DEAD;
            life->next_grid[i][j] = DEAD;
        }
}

/**
 * Initialize the GoL board with ALIVE values from file.
 * 
 * @param file_ptr    The pointer to the open input file.
 */
void init_from_file(struct life_t *life, FILE *file_ptr) {
    int i, j;

    if(life->input_file != NULL)
        // Every line from the file contains row/column coordinates
        // of every cell that has to be initialized as ALIVE.
        while (fscanf(file_ptr, "%d %d\n", &i, &j) != EOF) {
            life->grid[i][j]      = ALIVE;
            life->next_grid[i][j] = ALIVE;
        }

    fclose(file_ptr);
}

/**
 * Initialize the GoL board with ALIVE values randomly.
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
