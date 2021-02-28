#ifndef GoL_CHUNK_H
#define GoL_CHUNK_H

#include <mpi.h> // Enable MPI support
#include <stdio.h>
#include <unistd.h>

// Custom includes
#include "../globals.h"
#include "../life/life.h"

/**
 * All the data required by a single communicating process via MPI.
 */ 
typedef struct chunk {
    int nrows;         // Number of columns in the slice
    int ncols;         // Number of rows in the slice w/o ghost rows
    int rank;          // Rank of the calling MPI process in the communicator
    int size;          // Number of total MPI processes present in the communicator
    int displacement;  // Number of leftover rows assigned to the last process

    bool **slice;      // Chunk's slice of data at the current step
    bool **next_slice; // Chunk's slice of data at the next step
} chunk_t;

/***********************
 * Evolution functions *
 ***********************/

void initialize_chunk(chunk_t *chunk, life_t life,
        FILE *input_ptr, int from, int to);

double game_chunk(chunk_t *chunk, life_t life);
void evolve_chunk(chunk_t *chunk);

void cleanup_chunk(chunk_t *chunk);

/***********************
 * Debugging functions *
 ***********************/
#ifdef GoL_DEBUG
/**
 * Print to console the metadata that characterizes the calling process' slice of GoL data.
 */
void debug_chunk(chunk_t chunk) {
    printf("Process rank: %d\n", chunk.rank);
    printf("Number of cols: %d\n", chunk.ncols);
    printf("Number of rows: %d\n", chunk.nrows);
    printf("Communicator size: %d\n", chunk.size);
    printf("Number of leftover rows: %d\n\n", chunk.displacement);

    fflush(stdout);
}
#endif

/*********************
 * Display functions *
 *********************/

/**
 * Print the slice of GoL data assigned to the calling process to console.
 */
void show_chunk(chunk_t chunk) {
    int i, j;

    int ncols = chunk.ncols;
    int nrows = chunk.nrows;

    // \033[H: Move cursor to top-left corner;
    // \033[J: Clear console.
    printf("\033[H\033[J");

    for (i = 1; i < nrows + 1; i++) { // Skip top/bottom ghost rows
        for (j = 0; j < ncols; j++)
            printf(chunk.slice[i][j] == ALIVE
                    ? "\033[07m  \033[m" : "  ");

        printf("\033[E");
    }
}

/**
 * Print the buffer of GoL data received by the calling process to console.
 * 
 * @param nrows     The number of rows to read from the buffer. Indeed in case of displacement, that is, the data load not being shared equally
 *                  between the available MPI processes, the buffer will always be allocated as the largest chunk of data across said processes.
 *                  If a sender process manages a slimmer slice of GoL data, the calling process will print the proper amount of the buffer.
 * 
 * @param buffer    The buffer of data sent by another process via MPI.
 */
void show_buffer(int ncols, int nrows, bool *buffer) {
    int i, j;

    for (i = 0; i < nrows; i++) {
        for (j = 0; j < ncols; j++)
            printf(*((buffer + i*ncols) + j) == ALIVE
                    ? "\033[07m  \033[m" : "  ");

        printf("\033[E");
    }
}

/**
 * Print the slice of GoL data assigned to the calling process to file.
 * 
 * @param tot_rows    The overall number of rows in GoL's grid.
 * 
 * @param append      Whether to append to or to overwrite the output file.
 */
void print_chunk(chunk_t chunk, int tot_rows, char *outfile, bool append) {
    char *mode = append ? "a" : "w";

    int i, j;

    int nrows = chunk.nrows;
    int ncols = chunk.ncols;

    FILE *out_ptr;

    if ((out_ptr = fopen(outfile, mode)) == NULL) {
        perror("[*] Failed to open the output file.");
        MPI_Abort(MPI_COMM_WORLD, 1); // Any process in the comm has to stop
    }

    if (!append) // Print board dimensions only once
        fprintf(out_ptr, "%d %d\n", tot_rows, ncols);
        
    for (i = 0; i < nrows; i++) {
        for (j = 0; j < ncols; j++)
            fprintf(out_ptr, "%c", chunk.slice[i][j] == ALIVE
                    ? 'X' : ' ');

        fprintf(out_ptr, "\n");
    }

    fflush(out_ptr);
    fclose(out_ptr);
}

/**
 * Print the buffer of GoL data received by the calling process to file. Since this function will be called by one process only (i.e., rank 0),
 * we can assume beforehand that the data will always have to be appended to file and we will let the calling process take care of closing
 * the file pointer once all buffers received via MPI have been printed to file. 
 * 
 * @param buffer       The buffer of data sent by another process via MPI.
 *    
 * @param nrows        The number of rows to read from the buffer. Indeed in case of displacement, that is, the data load not being shared equally
 *                     between the available MPI processes, the buffer will always be allocated as the largest chunk of data across said processes.
 *                     If a sender process manages a slimmer slice of GoL data, the calling process will print the proper amount of the buffer.
 *    
 * @param rank         The rank of the sender process in the MPI communicator.
 * 
 * @param comm_size    The overall number of MPI processes present in the communicator.
 */
void print_buffer(bool *buffer, int ncols, int nrows, int rank,
        int comm_size, FILE* out_ptr) {
    int i, j;

    // Check if the sender process is the last one in the MPI communicator...
    bool last = (rank == comm_size - 1);

    for (i = 0; i < nrows; i++) {
        for (j = 0; j < ncols; j++)
            printf("%c", *((buffer + i*ncols) + j) == ALIVE
                    ? 'X' : ' ');

        fprintf(out_ptr, "\n");
    }

    if (last) // ...because in that case it will have to print a separator to file.
        fprintf(out_ptr, "****************************************************************************************************\n");
}

/**
 * Print the current GoL board to either console or file depending on whether its size is larger than DEFAULT_MAX_SIZE. Only one process
 * within the MPI communicator is allowed to perform printing operations (i.e., rank 0). It will print its own chuck of GoL data, and then
 * wait for all the other processes to send theirs sequentially in buffers. This procedure ensures to restore the proper order of
 * the overall GoL's grid, since the data was originaly split sequentially in chunks between all MPI processes.
 * 
 * @param big         Whether GoL's grid is larger than DEFAULT_MAX_SIZE.
 * 
 * @param tot_rows    The overall number of rows in GoL's grid.
 * 
 * @param append      Whether to append to or to overwrite the output file, if in use.
 */
void display_chunk(chunk_t *chunk, bool big, int tot_rows,
        char *outfile, bool append) {
    int status; // All MPI routines in C return an int error value

    if (chunk->rank == 0) {
        int r, j, k;

        // 1. Print its chunk to console/file
        if (!big)
            show_chunk(*chunk);
        else
            print_chunk(*chunk, tot_rows, outfile, append);

        int nrows = chunk->nrows;
        int ncols = chunk->ncols;

        // 2. Statically allocate the buffer as a 2D matrix in order to guarantee its continuity in memory. This ensures that the whole
        // slice of GoL data a process sends could be received with a single MPI_Recv call.
        bool buffer[nrows + chunk->displacement][ncols];

        for (j = 0; j < nrows + chunk->displacement; j++)
            for(k = 0; k < ncols; k++)
                buffer[j][k] = DEAD;

        FILE *out_ptr;

        if (big)
            if ((out_ptr = fopen(outfile, "a")) == NULL) {
                perror("[*] Failed to open the output file.");
                MPI_Abort(MPI_COMM_WORLD, 1);
            }

        MPI_Status mstatus; // Required as a MPI_Recv argument, but never used

        // 3. Collect and print other processes'
        for (r = 1; r < chunk->size; r++) {
            status = MPI_Recv(&buffer[0][0], (nrows + chunk->displacement) * ncols,
                              MPI_C_BOOL, r, PRINT, MPI_COMM_WORLD, &mstatus);

            if (status != MPI_SUCCESS) {
                fprintf(stderr, "[*] Failed to receive data from process %d - errcode %d", r, status);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }

            // Account for displacement if last rank process
            int rrows = (r == chunk->size - 1) \
                        ? nrows + chunk->displacement : nrows;

            if (!big)
                show_buffer(ncols, rrows, &buffer[0][0]);
            else
                print_buffer(&buffer[0][0], ncols, rrows, r,
                             chunk->size, out_ptr);
        }

        if (!big)
            fflush(stdout);
        else {
            fflush(out_ptr);
            fclose(out_ptr);
        }
    } else {
        status = MPI_Send(&chunk->slice[1][0], chunk->nrows * chunk->ncols,
                          MPI_C_BOOL, 0, PRINT, MPI_COMM_WORLD); // Start from 2nd row, [1][0], for nrows*ncols elements
                                                                 // to skip both top and bottom ghost rows

        if (status != MPI_SUCCESS) {
            fprintf(stderr, "[*] Failed to send data to process 0 - errcode %d", status);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    if (!big)
        usleep(160000); // For visualization purposes
}

#endif
