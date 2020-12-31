#ifndef UTILS_H
#define UTILS_H 

#include <getopt.h>
#include <stdbool.h>
#include <time.h>
#include "globals.h"

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
 * @param life, the main data structure behind a GoL instance.
 */ 
void load_defaults(struct life_t *life) {
    life->num_cols = DEFAULT_SIZE_COLS;
    life->num_rows = DEFAULT_SIZE_ROWS;
    life->timesteps = DEFAULT_TIMESTEPS;
    life->init_prob = DEFAULT_INIT_PROB;
    life->input_file = NULL;
    life->output_file = DEFAULT_OUT_FILE;
    life->seed = DEFAULT_SEED;
}

/**
 * Parse the seed's value from its command line argument. A 0 value will be turned into a random seed via time(NULL).
 * 
 * @param _seed, the command line argument.
 * 
 * @return seed, the corresponding seed's unsigned int value.
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
        printf("\nParsing arguments with options...\n");

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
        printf("\nParsing arguments with no options...\n");
        
        // Non-explicit arguments must stick to the following sequence:
        //     columns, rows, (tsteps, output, seed, init_prob)
        // 
        // when not reading GoL from file. Viceversa, they should be specified as:
        //     input, (tsteps, output)
        // 
        // [*] Round brackets indicate optional arguments.

        long int parsed_arg;
        bool input_spec; // Whether an input file is specified for GoL or not

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
                    life->seed = parse_seed(argv[6]);
            }

            if (argc > 6) {
                if (input_spec)
                    life->init_prob = strtod(argv[7], (char **) NULL);
            }
        }
    }
}

#endif
