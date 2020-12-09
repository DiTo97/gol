
#ifndef UTILS_H
#define UTILS_H 

#include <getopt.h>
#include "globals.h"



static const char * opts = "c:r:t:i:o:v:p::h?";
static const struct option long_opts[] = {
	{ "columns", required_argument, NULL, 'c' },
	{ "rows", required_argument, NULL, 'r' },
	{ "tsteps", optional_argument, NULL, 't' },
	{ "output", required_argument, NULL, 'o' },
	{ "input", optional_argument, NULL, 'i' },
	{ "vis_interval", optional_argument, NULL, 'v' },
	{ "init_prob", optional_argument, NULL, 'p' },
	{ "help", no_argument, NULL, 'h' },
	{ NULL, no_argument, NULL, 0 }
};

void usage (void) {
  printf("\nUsage: gol [options]\n");
  printf("  -c|--columns number   Number of columns in grid. Default: %d\n", DEFAULT_SIZE_ROWS);
  printf("  -r|--rows number      Number of rows in grid. Default: %d\n", DEFAULT_SIZE_COLS);
  printf("  -t|--tsteps number      Number of timesteps to run. Default: %d\n", DEFAULT_TIMESTEPS);
  printf("  -i|--input filename   Input file. See README for format. Default: none.\n");
  printf("  -o|--output filename  Output file. Default: none.\n");
  printf("  -h|--help             This help page.\n");
  printf("  -v[N]|--vis_interval[=N]  The display frequency of the grid. Default: %d\n",
    DEFAULT_VIS_INTERVAL);
  printf("\nSee README for more information.\n\n");

  exit(EXIT_FAILURE);
}

void parse_args (struct life_t * life, int argc, char ** argv) {
  int options       = 0;
  int opt_index = 0;
  int i;

  for (;;) {
    options = getopt_long(argc, argv, opts, long_opts, &opt_index);

    if (options == -1) break;

    switch (options) {
      case 'c':
        life->num_cols = strtol(optarg, (char**) NULL, 10);
        break;
      case 'r':
        life->num_rows = strtol(optarg, (char**) NULL, 10);
        break;
      case 't':
        if(optarg != NULL)
          life->timesteps = strtol(optarg, (char**) NULL, 10);
        else
          life->timesteps = DEFAULT_TIMESTEPS;
        break;
      case 'i':
        life->input_file = optarg;
        break;
      case 'o':
        life->output_file = optarg;
        break;
      case 'v':
        if (optarg != NULL)
          life->vis_interval = strtol(optarg, (char**) NULL, 10);
        else
          life->vis_interval = DEFAULT_VIS_INTERVAL;
        break;
      case 'p':
        if(optarg != NULL)
          life->init_prob = strtod(optarg, (char **) NULL);
        else
          life->init_prob = DEFAULT_INIT_PROB;
        break;
      case 'h':
      case '?':
        usage();
        break;

      default:
        break;
    }
  }

  // Taking the arguments withot the arguments specificatio 
  if (optind == 1) {
    if (argc > 1)
      life->num_rows = strtol(argv[1], (char**) NULL, 10);
    if (argc > 2)
      life->num_cols = strtol(argv[2], (char**) NULL, 10);
    if (argc > 3)
      life->timesteps = strtol(argv[3], (char**) NULL, 10);
    if (argc > 4)
      life->output_file = optarg;
  }
}

FILE * get_grid_dimension_from_file(struct life_t * life){
  FILE * fd;

  if (life->input_file != NULL) {
    if ((fd = fopen(life->input_file, "r")) == NULL) {
      perror("Failed to open file for input.\n File does not exist, launching the simulation with the default configuration...\n");
    }else if (fscanf(fd, "%d %d\n", &life->num_cols, &life->num_rows) == EOF) {
      printf("File must at least define grid dimensions!\nFile does not exist, launching the simulation with the default configuration...\n");
    }else{
      return fd;
    }
  }
  return NULL;
}

void init_default_grid(struct life_t * life){
  int i, j;
  for (i = 0; i < life->num_cols+2; i++) {
    for (j = 0; j < life->num_rows+2; j++) {
      life->grid[i][j]      = DEAD;
      life->next_grid[i][j] = DEAD;
    }
  }
}

void init_from_file(struct life_t * life, FILE * input_file){
int i,j;

if(life->input_file != NULL){
  while (fscanf(input_file, "%d %d\n", &i, &j) != EOF) {
    life->grid[i][j]      = ALIVE;
    life->next_grid[i][j] = ALIVE;
  }

  fclose(input_file);
}
}

void malloc_grids(struct life_t * life) {
	int i,j;
	int ncols = life->num_cols;
	int nrows = life->num_rows;

	life->grid      = (unsigned **) malloc(sizeof(unsigned *) * (ncols+2));
	life->next_grid = (unsigned **) malloc(sizeof(unsigned *) * (ncols+2));

	for (i = 0; i < ncols+2; i++) {
		life->grid[i]      = (unsigned *) malloc(sizeof(unsigned) * (nrows+2));
		life->next_grid[i] = (unsigned *) malloc(sizeof(unsigned) * (nrows+2));
	}
}

double rand_double() {
	return (double)random()/(double)RAND_MAX;
}

#endif