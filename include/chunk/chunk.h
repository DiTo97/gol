#ifndef GoL_CHUNK_H
#define GoL_CHUNK_H

/**
 * All the data required by a single communicating process via MPI.
 */ 
struct chunk_t {
    int nrows;         // Number of columns in the chunk's slice
    int ncols;         // Number of rows in the chunk's slice
    int rank;          // Rank of the calling MPI process in the communicator
    int size;          // Number of total MPI processes present in the communicator
    int displacement;  // Number of leftover rows assigned to the last process

    bool **slice;      // Chunk's slice of data at the current step
    bool **next_slice; // Chunk's slice of data at the next step
};

/***********************
 * Evolution functions *
 ***********************/

void evolve_chunk(struct chunk_t *chunk);
void cleanup_chunk(struct chunk_t *chunk);
void initialize_chunk(struct chunk_t *chunk, struct life_t life,
        FILE *input_ptr, int from, int to);

double game_chunk(struct chunk_t *chunk, struct life_t life);

/***********************
 * Debugging functions *
 ***********************/
#ifdef GoL_DEBUG
void debug_chunk(struct chunk_t chunk) {
    printf("Rank: %d\n", chunk.rank);
    printf("Number of cols: %d\n", chunk.ncols);
    printf("Number of rows: %d\n", chunk.nrows);
    printf("Communicator size: %d\n", chunk.size);
    printf("Leftover rows displacement: %d\n\n", chunk.displacement);

    fflush(stdout);
}
#endif

/*********************
 * Display functions *
 *********************/

void show_chunk(struct chunk_t chunk);

/**
 * Print the current GoL chunk to console.
 */
#ifdef GoL_MPI
void show_chunk(struct chunk_t chunk) {
    int x, y;

    int ncols = chunk.num_cols;
    int nrows = chunk.num_rows;

    // \033[H: Move cursor to top-left corner;
    // \033[J: Clear console.
    printf("\033[H\033[J");

    for (x = 1; x < nrows + 1; x++) {
        for (y = 0; y < ncols; y++)
            printf(chunk.chunk[x][y] == ALIVE ? "\033[07m  \033[m" : "  ");

        printf("\033[E");
    }
}

void show_buffer(int ncols, int nrows, bool *buffer);
void write_chunk(struct chunk_t, int tot_rows, char *out_file, bool append);
void write_buffer(bool *buffer, int nrows, int ncols, int buf_rows, int rank, int size, FILE* out_ptr);

void display_chunk(struct chunk_t *chunk, bool big, int tot_rows, char *out_file, bool append);

#endif
