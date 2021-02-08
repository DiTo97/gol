#ifndef GoL_GOL_H
#define GoL_GOL_H 

#include <stdbool.h>

/**
 * All the data required by a Game of Life instance.
 */ 
struct life_t {
    int num_cols;         // Number of columns in the grid
    int num_rows;         // Number of rows in the gird
    int timesteps;        // Number of generations to simulate

    double init_prob;     // Probability to mark a cell as ALIVE
                          // when following a random initialization

    int nthreads_per_block; // Number of threads for each CUDA block

    unsigned int seed;    // Random seed initializer
    
    bool *grid;      // Game grid at the current step
    bool *next_grid; // Game grid at the next step
    
    char *input_file;     // Input filename
    char *output_file;    // Output filename
};


/*********************
 * Display functions *
 *********************/

void display(struct life_t life, bool append);
void show(struct life_t life);

/***********************
 * Evolution functions *
 ***********************/

void cleanup(struct life_t *life);
void evolve(struct life_t *life);
double game(struct life_t *life);
void initialize(struct life_t *life);
void initialize_chunk(struct chunk_t *chunk, struct life_t life, FILE *input_ptr, int from, int to);

#endif