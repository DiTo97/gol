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

    #ifdef _OPENMP
    int num_threads;      // Number of threads adopted by OpenMP
    #endif

    unsigned int seed;    // Random seed initializer
    
    bool **grid;      // Game grid at the current step
    bool **next_grid; // Game grid at the next step
    
    char *input_file;     // Input filename
    char *output_file;    // Output filename
};

/*********************
 * Display functions *
 *********************/

/*********** -- SERIAL & OPENMP -- ************/
void display(struct life_t life, bool append);
void show(struct life_t life);
void printbig(struct life_t life, bool append);
void get_grid_status(struct life_t life);
void get_grid_status(struct life_t life);

/***********************
 * Evolution functions *
 ***********************/

/*********** -- SERIAL & OPENMP -- ************/
void cleanup(struct life_t *life);
void evolve(struct life_t *life);
double game(struct life_t *life);
void initialize(struct life_t *life);


/*********** -- MPI -- ************/

/**
 * Struct used by the different processes in the MPI implementation
 */
#ifdef GoL_MPI
struct chunk_t {
    int num_rows;                 // Number of columns in the chunk grid
    int num_cols;                 // Number of rows in the chunk grid
    int rank;                     // The rank of the process that uses the struct
    int size;                     // The number of processes present in the pool
    int displacement;             // The number of leftover rows assigned to the last process

    bool **chunk;          // Chunk grid at the current step
    bool **next_chunk;     // Chunk grid at the next step
};

/*********************
 * Display functions *
 *********************/

void display_chunk(struct chunk_t *chunk, bool big, int tot_rows, char *out_file, bool append);
void show_chunk(struct chunk_t chunk);
void show_buffer(int ncols, int nrows, bool *buffer);
void debug_chunk(struct chunk_t chunk);
void write_chunk(struct chunk_t, int tot_rows, char *out_file, bool append);
void write_buffer(bool *buffer, int nrows, int ncols, int buf_rows, int rank, int size, FILE* out_ptr);

/***********************
 * Evolution functions *
 ***********************/

void evolve_chunk(struct chunk_t *chunk);
double game_chunk(struct chunk_t *chunk, struct life_t life);
void initialize_chunk(struct chunk_t *chunk, struct life_t life, FILE *input_ptr, int from, int to);
#endif
#endif