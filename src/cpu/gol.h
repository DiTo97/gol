#ifndef GoL_H
#define GoL_H

#include <stdlib.h>
#include <unistd.h>

#ifdef _OPENMP
#include <omp.h> // Enable OpenMP support
#endif

#ifdef GoL_MPI
#include <mpi.h> // Enable MPI support
#endif

// Custom includes
#include "../../include/globals.h"

#include "../../include/utils/log.h"
#include "../../include/utils/func.h"
#include "../../include/utils/parse.h"

#include "../../include/life/init.h"

/**
 * Swap the memory pointers between two 2D matrices.
 */
void swap_grids(bool ***old, bool ***new) {
    bool **temp = *old;

    *old = *new;
    *new = temp;
}

/**
 * Print to console the status of the current GoL board: the number of ALIVE and DEAD cells.
 */
void get_grid_status(life_t life) {
    int i, j;

    int ncols = life.ncols;
    int nrows = life.nrows;

    int n_alive = 0;
    int n_dead  = 0;

    #ifdef _OPENMP
    #pragma omp parallel for private(j) \
                reduction(+:n_alive, n_dead)
    #endif
    for (i = 0; i < nrows; i++) 
        for (j = 0; j < ncols; j++)
            life.grid[i][j] == ALIVE ? n_alive++ : n_dead++;
    
    printf("Number of ALIVE cells: %d\n",  n_alive);
    printf("Number of DEAD cells: %d\n\n", n_dead);

    fflush(stdout);
    usleep(320000);
}

#ifdef GoL_MPI
#include "../../include/chunk/init.h"

/**
 * Initialize all variables and structures required by a single GoL chunk.
 */
void initialize_chunk(chunk_t *chunk, life_t life,
        FILE *input_ptr, int from, int to) {
    srand(life.seed);

    // 1. Allocate memory for the chunk
    malloc_chunk(chunk);

    // 2. Initialize the chunk with DEAD cells
    init_empty_chunk(chunk);
    
    // 3. Initialize the chunk with ALIVE cells...
    if (input_ptr != NULL) { // ...from file, if present...
        init_chunk_from_file(chunk, life.nrows, life.ncols,
                input_ptr, from, to);
    } else {  // ...or randomly, otherwise.
        init_random_chunk(chunk, life, from, to);
    }

    #ifdef GoL_DEBUG
    debug_chunk(*chunk);
    usleep(1000000);
    #endif
}

/**
 * Perform GoL evolution on a single chunk for a given amount of generations.
 * 
 * @return tot_gene_time    The total time devolved to GoL evolution
 */
double game_chunk(chunk_t *chunk, life_t life) {
    int i;
    MPI_Status status;

    int timesteps = life.timesteps;
    int tot_rows  = life.nrows;
    char *outfile = life.outfile;

    bool big = is_big(life);

    struct timeval gstart, gend;

    double cur_gene_time = 0.0;
    double tot_gene_time = 0.0;

    display_chunk(chunk, big, tot_rows,
            outfile, false);

    /*
     * Only one process (rank 0) will be allowed to track evolution timings.
     * 
     * TODO: Track the average evolution timings across all processes.
     */
    for (i = 0; i < timesteps; i++) {
        MPI_Barrier(MPI_COMM_WORLD);

        if (chunk->rank == 0)
            // Track the start time
            gettimeofday(&gstart, NULL);

        // Evolve the current chunk
        evolve_chunk(chunk);

        // Identify top/bottom neighbours ranks
        int prev_rank = (chunk->rank - 1 + chunk->size) % chunk->size;
        int next_rank = (chunk->rank + 1) % chunk->size;

        // Share ghost rows with top/bottom neighbours
        MPI_Sendrecv(&chunk->slice[1][0], chunk->ncols, MPI_C_BOOL, prev_rank, TOP,
                     &chunk->slice[chunk->nrows + 1][0], chunk->ncols, MPI_C_BOOL, next_rank, TOP,
                     MPI_COMM_WORLD, &status);

        MPI_Sendrecv(&chunk->slice[chunk->nrows][0], chunk->ncols, MPI_C_BOOL, next_rank, BOTTOM,
                     &chunk->slice[0][0], chunk->ncols, MPI_C_BOOL, prev_rank, BOTTOM,
                     MPI_COMM_WORLD, &status);

        MPI_Barrier(MPI_COMM_WORLD);

        if (chunk->rank == 0) {
            // Track the end time
            gettimeofday(&gend, NULL);

            cur_gene_time = elapsed_wtime(gstart, gend);
            tot_gene_time += cur_gene_time;
        }

        if(big) {
            if (chunk->rank == 0)
                printf("Generation #%d took %.5f ms on process 0\n", i, cur_gene_time);  

            // If the GoL grid is large, print it (to file)
            // only at the end of the last generation
            if (i == timesteps - 1) {
                display_chunk(chunk, big, tot_rows,
                        outfile, true);
            }
        } else {
            display_chunk(chunk, big, tot_rows,
                    outfile, true);
        }
    }

    if (chunk->rank == 0)
        printf("\nEvolved GoL's grid for %d generations - ETA: %.5f ms\n",
                timesteps, tot_gene_time);

    return tot_gene_time;
}

void evolve_chunk(chunk_t *chunk) {
    int x, y, i, j, r, c;

    int alive_neighbs; // # of alive neighbours

    int ncols = chunk->ncols;
    int nrows = chunk->nrows;
 
    // 1. Evolve every cell in the chunk
    #ifdef _OPENMP
    #pragma omp parallel for private(alive_neighbs, y, i, j, r, c)
    #endif
    for (x = 1; x < nrows + 1; x++) // Skip ghost rows: (1, ..., nrows + 1)
        for (y = 0; y < ncols; y++) {
            alive_neighbs = 0;

            // 1.a Check the 3x3 neighbourhood
            for (i = x - 1; i <= x + 1; i++)
                for (j = y - 1; j <= y + 1; j++) {
                    /* Compute the actual row/col coordinates in the GoL chunk. */

                    c = (j + ncols) % ncols;

                    if (!(i == x && j == y) // Skip the current cell (x, y)
                            && chunk->slice[i][c] == ALIVE)
                        alive_neighbs++;
                }

            // 1.b Apply GoL rules to determine the cell's next state
            chunk->next_slice[x][y] = (alive_neighbs == 3
                    || (alive_neighbs == 2
                            && chunk->slice[x][y] == ALIVE)) \
                    ? ALIVE : DEAD;
        }

    // 2. Replace the old grid with the updated one
    swap_grids(&chunk->slice, &chunk->next_slice);
}

void cleanup_chunk(chunk_t *chunk) {
    int i;

    free(chunk->slice);
    free(chunk->next_slice);
}
#endif

#endif
