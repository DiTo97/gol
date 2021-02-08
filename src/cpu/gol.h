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

/**
 * Struct used by the different processes in the MPI implementation
 */
struct chunk_t {
    int num_rows;                 // Number of columns in the chunk grid
    int num_cols;                 // Number of rows in the chunk grid
    int rank;                     // The rank of the process that uses the struct
    int size;                     // The number of processes present in the pool
    int displacement;             // The number of leftover rows assigned to the last process

    unsigned int **chunk;          // Chunk grid at the current step
    unsigned int **next_chunk;     // Chunk grid at the next step
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
void initialize_chunk(struct chunk_t *chunk, struct life_t life, FILE *input_ptr, int from, int to);
