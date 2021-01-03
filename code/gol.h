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

    #ifdef _OPENMP
    int num_threads;      // Number of threads adopted by OpenMP
    #endif

    unsigned int seed;    // Random seed initializer
    
    unsigned **grid;      // Game grid at the current step
    unsigned **next_grid; // Game grid at the next step
    
    char *input_file;     // Input filename
    char *output_file;    // Output filename
};

/*********************
 * Display functions *
 *********************/

void display(struct life_t life, bool append);
void get_grid_status(struct life_t life);
void printbig(struct life_t life, bool append);
void show(struct life_t life);

/***********************
 * Evolution functions *
 ***********************/

void cleanup(struct life_t *life);
void evolve(struct life_t *life);
void game(struct life_t *life);
void initialize(struct life_t *life);
