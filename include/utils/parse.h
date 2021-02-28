#ifndef GoL_PARSE_H
#define GoL_PARSE_H

#include <getopt.h>

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
    printf("\nUsage [1]: gol [opts]\n");
    printf("  -c|--columns number       Number of columns in grid. Default: %d\n", DEFAULT_SIZE_COLS);
    printf("  -r|--rows number          Number of rows in grid. Default: %d\n", DEFAULT_SIZE_ROWS);
    printf("  -t|--tsteps number        Number of timesteps to run. Default: %d\n", DEFAULT_TIMESTEPS);
    printf("  -s|--seed number          Random seed initializer. Default: %d\n", DEFAULT_SEED);
    printf("  -p|--init_prob number     Probability for grid initialization. Default: %f\n", DEFAULT_INIT_PROB);
    #ifdef _OPENMP
    printf("  -n|--nthreads             Number of threads adopted by OpenMP. Default: %d\n", DEFAULT_NUM_THREADS);
    #endif
    printf("  -i|--input filename       Input file. See README for format. Default: None.\n");
    printf("  -o|--output filename      Output file. Default: %s.\n", DEFAULT_OUT_FILE);
    printf("  -h|--help                 Show this help page.\n\n");

    printf("\nUsage [2] (in the following order): gol [no opts]\n");
    printf("  1) Number of columns in grid. Default: %d\n", DEFAULT_SIZE_ROWS);
    printf("  2) Number of rows in grid. Default: %d\n", DEFAULT_SIZE_COLS);
    printf("  3) Number of timesteps to run. Default: %d\n", DEFAULT_TIMESTEPS);
    printf("  4) Output file. Default: %s.\n", DEFAULT_OUT_FILE);
    printf("  5) Random seed initializer. Default: %d\n", DEFAULT_SEED);
    printf("  6) Probability for grid initialization. Default: %f\n", DEFAULT_INIT_PROB);
    #ifdef _OPENMP
    printf("  7) Number of threads adopted by OpenMP. Default: %d\n", DEFAULT_NUM_THREADS);
    #else
    printf("\n");
    #endif

    printf("\nUsage [3] (in the following order): gol [no opts]\n");
    printf("  1) Input file. Default: None.\n");
    printf("  2) Number of timesteps to run. Default: %d\n", DEFAULT_TIMESTEPS);
    printf("  3) Output file. Default: %s\n", DEFAULT_OUT_FILE);
    #ifdef _OPENMP
    printf("  4) Number of threads adopted by OpenMP. Default: %d\n", DEFAULT_NUM_THREADS);
    #else
    printf("\n");
    #endif
            
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
    #ifdef _OPENMP
    life->num_threads = DEFAULT_NUM_THREADS;
    #endif
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
                #ifdef _OPENMP
                case 'n':
                    life->num_threads = parse_nthreads(optarg);
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
                #ifdef _OPENMP
                else
                    life->num_threads = parse_nthreads(argv[4]);
                #endif
            }

            if (argc > 5) {
                if (input_not_spec)
                    life->seed = parse_seed(argv[5]);
            }

            if (argc > 6) {
                if (input_not_spec)
                    life->init_prob = strtod(argv[6], (char **) NULL);
            }

            #ifdef _OPENMP
            if (argc > 7) {
                if (input_not_spec)
                    life->num_threads = parse_nthreads(argv[7]);
            }
            #endif
        }
    }

}


#endif
