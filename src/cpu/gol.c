#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

// Custom includes
#include "gol.h"

/**
 * Initialize all variables and structures required by GoL evolution.
 */
void initialize(life_t *life) {
    // 1. Initialize the random seed
    srand(life->seed);

    // 2. Check if an input file was specified in the args
    // and, in that case, update ncols and nrows.
    //
    // Use defaults, if no file is present.
    FILE *input_ptr = set_grid_dimens_from_file(life);

    // 3. Allocate memory for the grid
    malloc_grid(life);

    // 4. Initialize the grid with DEAD cells
    init_empty_grid(life);

    // 5. Initialize the grid with ALIVE cells...
    if (input_ptr != NULL) { // ...from file, if present...
        init_from_file(life, input_ptr);
    } else {  // ...or randomly, otherwise.
        init_random(life);
    }

    #ifdef GoL_DEBUG
    debug(*life);
    usleep(1000000);
    #endif
}

/**
 * Perform GoL evolution for a given amount of generations.
 * 
 * @return tot_gene_time    The total time devolved to GoL evolution
 */
double game(life_t *life) {
    int x, y, t;

    struct timeval gstart, gend;
    
    // Initialize the whole GoL grid
    initialize(life);

    int ncols = life->ncols;
    int nrows = life->nrows;

    double tot_gene_time = 0.;
    double cur_gene_time = 0.;

    display(*life, false);

    for(t = 0; t < life->timesteps; t++) { 
        // 1. Track the start time
        gettimeofday(&gstart, NULL);
        
        // 2. Evolve the current generation
        evolve(life);
        
        // 3. Track the end time
        gettimeofday(&gend, NULL);

        cur_gene_time = elapsed_wtime(gstart, gend);
        tot_gene_time += cur_gene_time;

        if (is_big(*life)) {
            printf("Generation #%d took %.5f ms\n", t, cur_gene_time);  

            // If the GoL grid is large, print it (to file)
            // only at the end of the last generation
            if (t == life->timesteps - 1) {
                display(*life, true);
            }
        } else {
            display(*life, true);
        }

        #ifdef GoL_DEBUG
        get_grid_status(*life);
        #endif
    }

    printf("\nEvolved GoL's grid for %d generations - ETA: %.5f ms\n",
        life->timesteps, tot_gene_time);

    return tot_gene_time;
}

/**
 * Perform one evolutionary step of the board, following GoL rules, in this order:
 *     1. A cell is born, if it has exactly 3 neighbours;
 *     2. A cell dies of loneliness, if it has less than 2 neighbours;
 *     3. A cell dies of overcrowding, if it has more than 3 neighbours;
 *     4. A cell survives to the next generation, if it doesn't die of loneliness or overcrowding.
 */
void evolve(life_t *life) {
    int x, y, i, j, r, c;

    int alive_neighbs; // # of alive neighbours

    int ncols = life->ncols;
    int nrows = life->nrows;
 
    // 1. Evolve every cell in the grid
    #ifdef _OPENMP
    #pragma omp parallel for private(alive_neighbs, y, i, j, r, c)
    #endif
    for (x = 0; x < nrows; x++) 
        for (y = 0; y < ncols; y++) {
            alive_neighbs = 0;

            // 1.a Check the 3x3 neighbourhood
            for (i = x - 1; i <= x + 1; i++)
                for (j = y - 1; j <= y + 1; j++) {
                    /* Compute the actual row/col coordinates in the GoL board. */
                    
                    // Remember that the board represents an hypothetically infinite world. In order to do that,
                    // it has to be modelled as a circular matrix, with cells along outer borders considered adjacent to one another.
                    // By applying the modulo operator, %, we account for this possibility. 
                    c = (i + nrows) % nrows;
                    r = (j + ncols) % ncols;

                    if (!(i == x && j == y) // Skip the current cell (x, y)
                            && life->grid[c][r] == ALIVE)
                        alive_neighbs++;
                }

            // 1.b Apply GoL rules to determine the cell's next state
            life->next_grid[x][y] = (alive_neighbs == 3
                    || (alive_neighbs == 2
                            && life->grid[x][y] == ALIVE)) \
                    ? ALIVE : DEAD;
        }

    // 2. Replace the old grid with the updated one.
    swap_grids(&life->grid, &life->next_grid);
}

void cleanup(life_t *life) {
    int i;

    #ifdef _OPENMP
    #pragma omp parallel for
    #endif
    for (i = 0; i < life->nrows; i++) {
        free(life->grid[i]);
        free(life->next_grid[i]);
    }

    free(life->grid);
    free(life->next_grid);
}

/************************************
 * ================================ *
 ************************************/

int main(int argc, char **argv) {
    struct timeval start, end;
    double cum_gene_time, elapsed_prog_wtime;

    int nprocs = 1; // # of running processes
    life_t life;    // GoL's main data structure

    gettimeofday(&start, NULL);

    // 1. Initialize vars from args
    parse_args(&life, argc, argv);

    #ifdef _OPENMP
    omp_set_num_threads(life.nthreads);
    #endif

    FILE *input_ptr = set_grid_dimens_from_file(&life);

    #ifdef GoL_MPI /* GoL parallel with MPI */
    int rows_per_process;

    int from; // Boundaries of the slices of data
    int to;   // each process will take care of

    // 2. Initialize MPI environment
    int status = MPI_Init(&argc, &argv);

    if (status != MPI_SUCCESS) {
        fprintf(stderr, "[*] Failed to initialize MPI environment - errcode %d", status);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    chunk_t chunk; // Per-process data structure

    // 3. Get info from the MPI communicator
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &chunk.rank);

    // Pass the size info to all processes
    chunk.size = nprocs;

    // 4. Launch GoL's evolution
    if (chunk.size != 1) { // If there are at least 2 MPI processes
                           // launch GoL's parallel evolution...
        MPI_Barrier(MPI_COMM_WORLD);

        // 4.a Calculate the number of rows
        // that each process will handle 
        rows_per_process = (int) life.nrows/chunk.size;
        chunk.displacement = life.nrows % chunk.size;

        // 4.b Identify the starting row of each process
        from = chunk.rank * rows_per_process;

        // 4.c Identify the last row of each process.
        if (chunk.rank == chunk.size - 1) { // Last process will keep all remaining rows
            to = life.nrows - 1;
            chunk.nrows = life.nrows - from;
        } else {
            to = (chunk.rank + 1) * rows_per_process - 1;
            chunk.nrows = rows_per_process;
        }

        chunk.ncols = life.ncols; // Data is split on rows; hence all processes
                                  // will have the same # of columns

        initialize_chunk(&chunk, life,
                input_ptr, from, to);

        double tot_gtime = game_chunk(&chunk, life);

        if (chunk.rank == 0) {
            cum_gene_time = tot_gtime;
        }

        MPI_Barrier(MPI_COMM_WORLD);

        cleanup_chunk(&chunk);

        if(chunk.rank == 0) {
            gettimeofday(&end, NULL);
            elapsed_prog_wtime = elapsed_wtime(start, end);
        }
    } else { // ...else fall back to the sequential procedure
        cum_gene_time = game(&life);
        cleanup(&life);

        gettimeofday(&end, NULL);
        elapsed_prog_wtime = elapsed_wtime(start, end);
    }

    status = MPI_Finalize();

    if (status != MPI_SUCCESS) {
        fprintf(stderr, "[*] Failed to finalize MPI environment - errcode %d", status);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    #else /* GoL sequential */
    cum_gene_time = game(&life);
    cleanup(&life);

    gettimeofday(&end, NULL);
    elapsed_prog_wtime = elapsed_wtime(start, end);
    #endif

    // Log to file, if requested
    #ifdef GoL_LOG

    #ifdef GoL_MPI
    if (chunk.rank == 0) { /* Enforce only the rank 0 process logs to file, if MPI was called.
                            * Indeed the call to MPI_Finalize() doesn't guarantee
                            * other processes won't execute the following code */
    #endif
    FILE *log_ptr = init_log_file(life, nprocs);

    log_data(log_ptr, life.timesteps, cum_gene_time,
            elapsed_prog_wtime);

    fflush(log_ptr);
    fclose(log_ptr);
    #ifdef GoL_MPI
    }
    #endif

    #endif

    #ifdef GoL_MPI
    if (chunk.rank == 0) {
    #endif
    printf("\nFinalized the program - ETA: %.5f ms\n\n", elapsed_prog_wtime);
    #ifdef GoL_MPI
    }
    #endif
}