#ifndef GoL_UTILS_H
#define GoL_UTILS_H 

#include <getopt.h>
#include <sys/time.h>

#ifdef GoL_DEBUG
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "globals.h"

/*********************
 * Parsing functions *
 *********************/  

static const char *short_opts = "c:r:t:i:s:b:n:o:p:h?";
static const struct option long_opts[] = {
    { "columns", required_argument, NULL, 'c' },
    { "rows", required_argument, NULL, 'r' },
    { "tsteps", required_argument, NULL, 't' },
    { "output", required_argument, NULL, 'o' },
    { "input", required_argument, NULL, 'i' },
    { "nthreads_per_block", required_argument, NULL, 'n'},
    { "seed", required_argument, NULL, 's' },
    { "init_prob", required_argument, NULL, 'p' },
    { "help", no_argument, NULL, 'h' },
    { NULL, no_argument, NULL, 0 }
};

/**
 * Print all possible command line options settings to console and terminate.
 */
void show_usage() {
    printf("\nUsage [1]: gol cuda [opts]\n");
    printf("  -c|--columns number       Number of columns in grid. Default: %d\n", DEFAULT_SIZE_COLS);
    printf("  -r|--rows number          Number of rows in grid. Default: %d\n", DEFAULT_SIZE_ROWS);
    printf("  -t|--tsteps number        Number of timesteps to run. Default: %d\n", DEFAULT_TIMESTEPS);
    printf("  -s|--seed number          Random seed initializer. Default: %d\n", DEFAULT_SEED);
    printf("  -p|--init_prob number     Probability for grid initialization. Default: %f\n", DEFAULT_INIT_PROB);
    printf("  -n|--nthreads_per_block       Number of threads per CUDA block. Default: %d\n", DEFAULT_NUM_THREADS_PER_BLOCK);
    printf("  -i|--input filename       Input file. See README for format. Default: None.\n");
    printf("  -o|--output filename      Output file. Default: %s.\n", DEFAULT_OUT_FILE);
    printf("  -h|--help                 Show this help page.\n\n");

    printf("\nUsage [2] (in the following order): gol cuda [no opts]\n");
    printf("  1) Number of columns in grid. Default: %d\n", DEFAULT_SIZE_ROWS);
    printf("  2) Number of rows in grid. Default: %d\n", DEFAULT_SIZE_COLS);
    printf("  3) Number of timesteps to run. Default: %d\n", DEFAULT_TIMESTEPS);
    printf("  4) Output file. Default: %s.\n", DEFAULT_OUT_FILE);
    printf("  5) Random seed initializer. Default: %d\n", DEFAULT_SEED);
    printf("  6) Probability for grid initialization. Default: %f\n", DEFAULT_INIT_PROB);
    printf("  7) Number of threads per CUDA block. Default: %d\n", DEFAULT_NUM_THREADS_PER_BLOCK);
    printf("\n");


    printf("\nUsage [3] (in the following order): gol cuda [no opts]\n");
    printf("  1) Input file. Default: None.\n");
    printf("  2) Number of timesteps to run. Default: %d\n", DEFAULT_TIMESTEPS);
    printf("  3) Output file. Default: %s\n", DEFAULT_OUT_FILE);
    printf("  4) Number of threads per CUDA block. Default: %d\n", DEFAULT_NUM_THREADS_PER_BLOCK);
    printf("\n");
            
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
    life->nthreads_per_block = DEFAULT_NUM_THREADS_PER_BLOCK;
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
 * Parse the number of threads of each CUDA block.
 * 
 * @param _threads_per_block The command line argument.
 * 
 * @return  The corresponding number of threads for each CUDA block or DEFAULT_MAX_NUM_THREADS_PER_BLOCK if the number is bigger than it.
 */ 
int parse_threads_per_block(char *_threads_per_block) {
    int num_threads = strtol(_threads_per_block, (char **) NULL, 10);
    return num_threads > DEFAULT_MAX_NUM_THREADS_PER_BLOCK \
        ? DEFAULT_NUM_THREADS_PER_BLOCK : num_threads;
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

        if(strchr(argv[i], '-') != NULL){   // ...else keep track of how many - are included in argv
            // ignoring the optional parameter
            if(strstr(argv[i], "-s") != NULL)
                continue;

            opt_params_count++;            // to discrimante between opts being explicitly indicated or not.
        }
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
                case 'n':
                    life->nthreads_per_block = parse_threads_per_block(optarg);
                    break;
                case 'p':
                    life->init_prob = strtod(optarg, (char **) NULL);
                    break;
                case '?':
                default:
                    show_usage();
            }
        }
    } else { // No explicit opts were passed  
        printf("\nParsing arguments with no options...\n\n");

        fflush(stdout);
        usleep(100000);
        
        // Non-explicit arguments must stick to the following sequence:
        //     columns, rows, (tsteps, output, seed, init_prob, nthreads)
        // 
        // when not reading GoL from file. Viceversa, they should be specified as:
        //     input, (tsteps, output, nthreads)
        // 
        // [*] Round brackets indicate optional arguments.

        long int parsed_arg;
        bool input_not_spec; // Whether an input file is specified for GoL

        // Are there enough arguments to parse?
        if (limit > 1) {
            parsed_arg = strtol(argv[1], (char **) NULL, 10);

            input_not_spec = (parsed_arg != 0) \
                ? true : false;

            if (argc > 1) {
                if (input_not_spec)
                    life->num_cols = parsed_arg;
                else
                    life->input_file = argv[1];
            }

            if (argc > 2) {
                if (input_not_spec) 
                    life->num_rows = strtol(argv[2], (char **) NULL, 10);
                else 
                    life->timesteps = strtol(argv[2], (char **) NULL, 10);
            }

            if (argc > 3) {
                if (input_not_spec)
                    life->timesteps = strtol(argv[3], (char **) NULL, 10);
                else
                    life->output_file = argv[3];
            }

            if (argc > 4) {
                if (input_not_spec)
                    life->output_file = argv[4];
                else
                    life->nthreads_per_block = parse_threads_per_block(argv[4]);
            }

            if (argc > 5) {
                if (input_not_spec)
                    life->seed = parse_seed(argv[5]);
            }

            if (argc > 6) {
                if (input_not_spec)
                    life->init_prob = strtod(argv[6], (char **) NULL);
            }

            if (argc > 7) {
                if (input_not_spec)
                    life->nthreads_per_block = parse_threads_per_block(argv[7]);
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
    printf("Number of CUDA threads for each block: %d\n", life.nthreads_per_block);
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

/**
 * Get the elapsed wall-clock time between two time intervals.
 *
 * @param start, end    The two time intervals.
 *
 * @return    The elapsed wall-clock time in ms.
 */
double elapsed_wtime(struct timeval start, struct timeval end) {
    return (double) ((end.tv_sec * 1000000 + end.tv_usec) \
            - (start.tv_sec * 1000000 + start.tv_usec)) / 1000;
}

/*********************
 * Logging functions *
 *********************/

#ifdef GoL_DEBUG
/**
 * Initialize a tab-separated log file, whose name varies with GoL configuration's settings.
 * 
 * @return log_ptr    The pointer to the tab-separated log file with (timestep, current time, total time) columns
 */
FILE* init_log_file(struct life_t life) {
    char *logs_dir = (char*) DEFAULT_LOGS_DIR;
    
    char buffer[100];
    char __cuda[15]; // 15 := "cuda" + 5-digit num_blocks + "_" + 4-digit num_threads_per_block + "_"

    struct stat st = {0};

    if (stat(logs_dir, &st) == -1) { // Create the logs dir if it doesn't exist​​​​
        mkdir(logs_dir, 0744);
    }
    
    // TODO: test if it works with number of threads less than max 
    sprintf(__cuda, "cuda%d_", life.nthreads_per_block);

    if (life.input_file != NULL)
        sprintf(buffer, "%s/GoL_%snc%d_nr%d_nt%d_%lu.log", logs_dir, __cuda, 
                life.num_cols, life.num_rows, life.timesteps,
                (unsigned long) time(NULL));
    else
        sprintf(buffer, "%s/GoL_%snc%d_nr%d_nt%d_prob%.1f_seed%d_%lu.log", logs_dir, __cuda,
                life.num_cols, life.num_rows, life.timesteps, life.init_prob,
                life.seed, (unsigned long) time(NULL));

    FILE *log_ptr = fopen(buffer, "a");
    fprintf(log_ptr, "timesteps\tcum_gene_time\ttot_prog_time\n");

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
void log_data(FILE *log_ptr, int timesteps, double cum_gene_time, double tot_prog_time) {
    fprintf(log_ptr, "%-9d\t%-13.3f\t%-13.3f\n", timesteps, cum_gene_time, tot_prog_time); // -8, as columns are 8-char long
}
#endif

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

    // what we do here if the file is not present or is not with the right format
    if (life->input_file != NULL) {
        if ((file_ptr = fopen(life->input_file, "r")) == NULL) {
            perror("[*] Failed to open the input file.\n");
            perror("[*] Launching the simulation in default configuration...\n");
        } else if (fscanf(file_ptr, "%d %d\n", &life->num_rows, &life->num_cols) == EOF) {
            perror("[*] The input file only defines GoL board's dimensions!\n");
            perror("[*] Launching the simulation in default configuration...\n");
        } else{
            return file_ptr;
        }
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
    int ncols = life->num_cols;
    int nrows = life->num_rows;

    life->grid = (bool *) malloc(nrows * ncols * sizeof(bool)); // Guarantee continuous blocks of memory
    life->next_grid = (bool *) malloc(nrows * ncols * sizeof(bool)); // Guarantee continuous blocks of memory
}

/**
 * Initialize the GoL board with DEAD values.
 */
void init_empty_grid(struct life_t *life) {

    int i;
    for (i = 0; i < life->num_rows * life->num_cols; i++) {
        life->grid[i] = DEAD;
        life->next_grid[i] = DEAD;
    }
}

/**
 * Initialize the GoL board with ALIVE values from file.
 * 
 * @param file_ptr    The pointer to the open input file.
 */

void init_from_file(struct life_t *life, FILE *file_ptr) {
    int i, l;

    // this if is not necessary, I cannot call this funciton if 
    // life->input_file is NULL
    if(life->input_file != NULL){
        char *line = NULL;
        size_t len = 0;
        ssize_t read = 0;
        i = 0;

        // Every line from the file contains row/column coordinates
        // of every cell that has to be initialized as ALIVE.
        while ((read = getline(&line, &len, file_ptr)) > 0) {
            if (i >= life->num_rows){
                perror("[*] The input file exceeds the number of rows!\n");
                exit(EXIT_FAILURE);
            }

            if (read != life->num_cols + 1){
                fprintf(stderr, "[*] Row %d does not respect the number of columns!\n", i);
                exit(EXIT_FAILURE);
            }
            
            for (l = 0; l < read - 1; l ++){
                if(line[l] == 'X'){
                    life->grid[i*life->num_cols+l] = ALIVE;
                }
            }

            i = i + 1;
        }

        free(line);
        line = NULL;

        if (i != life->num_rows){
            perror("[*] The input file does not respect the number of rows!\n");
            exit(EXIT_FAILURE);
        }
    }



    fflush(file_ptr);
    fclose(file_ptr);
}

/**
 * Initialize the GoL board with ALIVE values randomly
 */
void init_random(struct life_t *life) {
    int i;

    for (i = 0; i < life->num_rows*life->num_cols; i++) { // same as using i*life->num_cols+j with a double for loop
        if (rand_double(0., 1.) < life->init_prob)
            life->grid[i]=ALIVE;
    }
}

/**
 * Print the current GoL board to file.
 *      1. A header will comprise the board dimensions (e.g., 6 6)
 *      2. A line filled with 'X' and ' ' will correspond to each row of the output GoL matrix.
 * @param append    Whether to append to or to overwrite the output file.
 */
void printbig(struct life_t life, bool append) {
    int x, y;
    
    int ncols = life.num_cols;
    int nrows = life.num_rows;

    FILE * out_ptr;
    
    out_ptr = append ? fopen(life.output_file, "a" ) : fopen(life.output_file, "w" );
    
    if (!append)
        fprintf(out_ptr, "%d %d\n", nrows, ncols);

    for (x = 0; x < nrows; x++) {
        for (y = 0; y < ncols; y++) {
            char alive = life.grid[x * life.num_cols + y] ? 'X' : ' ';
            fprintf(out_ptr, "%c", alive);
        }
        fprintf(out_ptr, "\n");
    }

    fprintf(out_ptr, "****************************************************************************************************\n");

    fflush(out_ptr);
    fclose(out_ptr);
}
#endif