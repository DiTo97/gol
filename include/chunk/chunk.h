#ifndef GoL_CHUNK_H
#define GoL_CHUNK_H

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
