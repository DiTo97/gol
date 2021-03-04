#ifndef GoL_PARSE_H
#define GoL_PARSE_H

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef GoL_CUDA
#include <sys/time.h> // Enable struct timeval
#endif

#include <time.h>

// Custom includes
#include "../globals.h"
#include "../life/life.h"

static const char *short_opts = "c:r:t:i:s::n:o:p:h?";
static const struct option long_opts[] = {
    { "columns", required_argument, NULL, 'c' },
    { "rows", required_argument, NULL, 'r' },
    { "tsteps", required_argument, NULL, 't' },
    { "output", required_argument, NULL, 'o' },
    { "input", required_argument, NULL, 'i' },
    #ifdef _OPENMP
    { "nthreads", required_argument, NULL, 'n' },
    #endif
    #ifdef GoL_CUDA
    { "block_size", required_argument, NULL, 'b' },
    #endif
    { "seed", required_argument, NULL, 's' },
    { "init_prob", required_argument, NULL, 'p' },
    { "help", no_argument, NULL, 'h' },
    { NULL, no_argument, NULL, 0 }
};

/**
 * Print all possible command line options settings to console and terminate.
 */
void show_usage() {
    printf("\nUsage [1]: GoL [opts]\n");
    printf("  -c|--columns     number      Number of columns in grid. Default: %d\n", DEFAULT_SIZE_COLS);
    printf("  -r|--rows        number      Number of rows in grid. Default: %d\n", DEFAULT_SIZE_ROWS);
    printf("  -t|--tsteps      number      Number of timesteps to run. Default: %d\n", DEFAULT_TIMESTEPS);
    printf("  -s|--seed        number      Random seed initializer. Default: %d\n", DEFAULT_SEED);
    printf("  -p|--init_prob   number      Probability for grid initialization. Default: %f\n", DEFAULT_INIT_PROB);
    #ifdef _OPENMP 
    printf("  -n|--nthreads    number      Number of threads adopted by OpenMP. Default: %d\n", DEFAULT_NUM_THREADS);
    #endif 
    #ifdef GoL_CUDA 
    printf("  -b|--block_size  number      Number of threads per CUDA block. Default: %d\n", DEFAULT_BLOCK_SIZE);
    #endif 
    printf("  -i|--input       filename    Input file. See README for format. Default: None.\n");
    printf("  -o|--output      filename    Output file. Default: %s.\n", DEFAULT_OUT_FILE);
    printf("  -h|--help                    Show this help page.\n\n");

    printf("\nUsage [2] (in the following order): GoL [no opts]\n");
    printf("  1) Number of columns in grid. Default: %d\n", DEFAULT_SIZE_ROWS);
    printf("  2) Number of rows in grid. Default: %d\n", DEFAULT_SIZE_COLS);
    printf("  3) Number of timesteps to run. Default: %d\n", DEFAULT_TIMESTEPS);
    printf("  4) Output file. Default: %s.\n", DEFAULT_OUT_FILE);
    printf("  5) Random seed initializer. Default: %d\n", DEFAULT_SEED);
    printf("  6) Probability for grid initialization. Default: %f\n", DEFAULT_INIT_PROB);
    #ifdef _OPENMP
    printf("  7.a) Number of threads adopted by OpenMP. Default: %d\n", DEFAULT_NUM_THREADS);
    #endif
    #ifdef GoL_CUDA
    printf("  7.b) Number of threads per CUDA block. Default: %d\n", DEFAULT_BLOCK_SIZE);
    #endif
    printf("\n");

    printf("\nUsage [3] (in the following order): GoL [no opts]\n");
    printf("  1) Input file. Default: None.\n");
    printf("  2) Number of timesteps to run. Default: %d\n", DEFAULT_TIMESTEPS);
    printf("  3) Output file. Default: %s\n", DEFAULT_OUT_FILE);
    #ifdef _OPENMP
    printf("  4.a) Number of threads adopted by OpenMP. Default: %d\n", DEFAULT_NUM_THREADS);
    #endif
    #ifdef GoL_CUDA
    printf("  4.b) Number of threads per CUDA block. Default: %d\n", DEFAULT_BLOCK_SIZE);
    #endif
    printf("\n");
            
    printf("See README for more information.\n\n");

    exit(EXIT_FAILURE);
}

/**
 * Load all default GoL settings from globals.h
 * 
 * @param life    The main data structure behind a GoL instance.
 */ 
void load_defaults(life_t *life) {
    life->seed       = DEFAULT_SEED;
    life->ncols      = DEFAULT_SIZE_COLS;
    life->nrows      = DEFAULT_SIZE_ROWS;
    life->timesteps  = DEFAULT_TIMESTEPS;
    life->init_prob  = DEFAULT_INIT_PROB;
    #ifdef _OPENMP
    life->nthreads   = DEFAULT_NUM_THREADS;
    #endif
    #ifdef GoL_CUDA
    life->block_size = DEFAULT_BLOCK_SIZE;
    #endif
    life->infile     = NULL;
    life->outfile    = (char*) DEFAULT_OUT_FILE;
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

#ifdef _OPENMP
/**
 * Parse the number of threads adopted by OpenMP.
 * 
 * @param _nthreads    The command line argument.
 * 
 * @return    The corresponding number of threads or DEFAULT_MAX_THREADS if the number is bigger than it.
 */ 
int parse_nthreads(char *_nthreads) {
    int nthreads = strtol(_nthreads, (char **) NULL, 10);

    return nthreads > DEFAULT_MAX_THREADS \
        ? DEFAULT_MAX_THREADS : nthreads;

}
#endif

#ifdef GoL_CUDA
/**
 * Parse the number of threads per CUDA block.
 * 
 * @param _block_size    The command line argument.
 * 
 * @return    The corresponding number of threads or DEFAULT_BLOCK_SIZE if the number is bigger than it.
 */ 
int parse_block_size(char *_block_size) {
    int block_size = strtol(_block_size, (char **) NULL, 10);

    return block_size > DEFAULT_BLOCK_SIZE \
        ? DEFAULT_BLOCK_SIZE : block_size;

}
#endif

/**
 * Parse command line arguments depending on whether opts are explicitly indicated or not.
 */
void parse_args(life_t *life, int argc, char **argv) {
    int opt = 0;
    int opt_idx = 0;
    int i;

    unsigned opt_params_count = 0;

    // Check whether command line options are malformed
    for(i = 1; i < argc; i++) {
        if (strstr(argv[i], "-h") != NULL) // If -h is included in argv show usage...
            show_usage();

        if(strchr(argv[i], '-') != NULL) { // ...else keep track of how many - are included in argv
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
    if (diff != opt_params_count
            && diff != limit) {
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
                    life->ncols = strtol(optarg, (char **) NULL, 10);
                    break;
                case 'r':
                    life->nrows = strtol(optarg, (char **) NULL, 10);
                    break;
                case 't':
                    life->timesteps = strtol(optarg, (char **) NULL, 10);
                    break;
                case 's':
                    life->seed = parse_seed(optarg);
                    break;
                case 'i':
                    life->infile = optarg;
                    break;
                case 'o':
                    life->outfile = optarg;
                    break;
                #ifdef _OPENMP
                case 'n':
                    life->nthreads = parse_nthreads(optarg);
                    break;
                #endif
                #ifdef GoL_CUDA
                case 'b':
                    life->block_size = parse_block_size(optarg);
                    break;
                #endif
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
        //     columns, rows, (tsteps, output, seed, init_prob, nthreads|block_size)
        // 
        // when not reading GoL from file. Viceversa, they should be specified as:
        //     input, (tsteps, output, nthreads|block_size)
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
                    life->ncols = parsed_arg;
                else
                    life->infile = argv[1];
            }

            if (argc > 2) {
                if (input_not_spec) 
                    life->nrows = strtol(argv[2], (char **) NULL, 10);
                else 
                    life->timesteps = strtol(argv[2], (char **) NULL, 10);
            }

            if (argc > 3) {
                if (input_not_spec)
                    life->timesteps = strtol(argv[3], (char **) NULL, 10);
                else
                    life->outfile = argv[3];
            }

            if (argc > 4) {
                if (input_not_spec)
                    life->outfile = argv[4];
                #ifdef _OPENMP
                else
                    life->nthreads = parse_nthreads(argv[4]);
                #elif defined(GoL_CUDA)
                else
                    life->block_size = parse_block_size(argv[4]);
                #endif
            }

            if (argc > 5) {
                if (input_not_spec)
                    life->seed = parse_seed(argv[5]);
                #if defined(_OPENMP) \
                        && defined(GoL_CUDA)
                else
                    life->block_size = parse_block_size(argv[5]);
                #endif
            }

            if (argc > 6) {
                if (input_not_spec)
                    life->init_prob = strtod(argv[6], (char **) NULL);
            }

            if (argc > 7) {
                #ifdef _OPENMP
                if (input_not_spec)
                    life->nthreads = parse_nthreads(argv[7]);
                #elif defined(GoL_CUDA)
                if (input_not_spec)
                    life->block_size = parse_block_size(argv[7]);
                #endif
            }

            if (argc > 8) {
                #if defined(_OPENMP) \
                        && defined(GoL_CUDA)
                if (input_not_spec)
                    life->block_size = parse_block_size(argv[8]);
                #endif
            }
        }
    }
}

#endif
