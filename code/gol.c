/*
 * Consway's Game of Life.
 *
 * Serial implementation in C inspired by
 * https://www.geeksforgeeks.org/conways-game-life-python-implementation/
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "gol.h"
#include "utils.h"

/*********************
 * Display functions *
 *********************/

/**
 * Print the current GoL board to console.
 */
void show(struct life_t life) {
    int x, y;

    int ncols = life.num_cols;
    int nrows = life.num_rows;

    // \033[H: Move cursor to top-left corner;
    // \033[J: Clear console.
    printf("\033[H\033[J");

    for (x = 0; x < nrows; x++) {
        for (y = 0; y < ncols; y++)
            printf(life.grid[y][x] ? "\033[07m  \033[m" : "  ");

        printf("\033[E");
    }

    fflush(stdout);
    usleep(160000);
}

/**
 * Print the current GoL board to console.
 */
void show_chunk(struct chunk_t chunk) {
    int x, y;

    int ncols = chunk.num_cols;
    int nrows = chunk.num_rows;

    // \033[H: Move cursor to top-left corner;
    // \033[J: Clear console.
    printf("\033[H\033[J");

    for (x = 0; x < nrows; x++) {
        for (y = 0; y < ncols; y++)
            printf(chunk.chunk[y][x] ? "\033[07m  \033[m" : "  ");

        printf("\033[E");
    }

    fflush(stdout);
    usleep(160000);
}

/**
 * Print the current GoL board to file.
 * 
 * @param append    Whether to append to or to overwrite the output file.
 */
void printbig(struct life_t life, bool append) {
    int x, y;
    
    int ncols = life.num_cols;
    int nrows = life.num_rows;

    FILE *out_ptr;
    
    if(append) out_ptr = fopen(life.output_file, "a" );
    else out_ptr = fopen(life.output_file, "w" );
    
    for (x = 0; x < nrows; x++) {
        for (y = 0; y < ncols; y++) 
            fprintf(out_ptr, "%c", life.grid[y][x] ? 'x' : ' ');
            
        fprintf(out_ptr, "\n");
    }
    fprintf(out_ptr, "\n\n\n\n\n\n****************************************************************************************************\n\n\n\n\n\n");

    fflush(out_ptr);
    fclose(out_ptr);
}

/**
 * Print the current GoL board to either console or file depending on whether its size is larger than DEFAULT_MAX_SIZE.
 */
void display(struct life_t life, bool append) {
    if(is_big(life)) printbig(life, append);
    else show(life);
}

#ifdef GoL_DEBUG
/**
 * Print to console the status of the current GoL board: the number of ALIVE and DEAD cells.
 */
void get_grid_status(struct life_t life) {
    int i, j;

    int ncols = life.num_cols;
    int nrows = life.num_rows;

    int n_alive = 0;
    int n_dead  = 0;

    #pragma omp parallel for private(j) \
                reduction(+:n_alive, n_dead)
    for (i = 0; i < ncols; i++) 
        for (j = 0; j < nrows; j++)
            life.grid[i][j] == ALIVE ? n_alive++ : n_dead++;
    
    printf("Number of alive cells: %d\n",  n_alive);
    printf("Number of dead cells: %d\n\n", n_dead);

    fflush(stdout);
    usleep(320000);
}
#endif

/***********************
 * Evolution functions *
 ***********************/

/**
 * Initialize all variables and structures required by GoL evolution.
 */
void initialize(struct life_t *life) {
    // 1. Initialize the random seed
    srand(life->seed);

    // 2. Check if an input file was specified in the args
    // and, in that case, update num_cols and num_rows.
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
 * Initialize all variables and structures required by GoL evolution.
 */
void initialize_chunk(struct chunk_t *chunk, struct life_t life, int from, int to) {
    srand(life.seed);
    // 2. Check if an input file was specified in the args
    // and, in that case, update num_cols and num_rows.
    //
    // Use defaults, if no file is present.
    // FILE *input_ptr = set_grid_dimens_from_file(chunk);

    // 3. Allocate memory for the grid
    malloc_chunk(chunk);

    // 4. Initialize the grid with DEAD cells
    init_empty_chunk(chunk);
    
    // int i, j;
    // for (i = 0; i < chunk->num_cols + 2; i++){
    //     for (j = 0; j < chunk->num_rows + 2; j++){
    //         printf("rank %d printed: [%d][%d] = %d\n", chunk->rank, i, j, chunk->chunk[i][j]);
    //     }
    // }   
    // 5. Initialize the grid with ALIVE cells...
    // if (input_ptr != NULL) { // ...from file, if present...
        // init_from_file(life, input_ptr);
    // } else {  // ...or randomly, otherwise.
    init_random_chunk(chunk, life, from, to);
    // }

    #ifdef GoL_DEBUG
    // debug(*life);
    usleep(1000000);
    #endif
}

/**
 * Perform one evolutionary step of the board, following GoL rules, in this order:
 * 
 *     1. A cell is born, if it has exactly 3 neighbours;
 *     2. A cell dies of loneliness, if it has less than 2 neighbours;
 *     3. A cell dies of overcrowding, if it has more than 3 neighbours;
 *     4. A cell survives to the next generation, if it does not die of loneliness or overcrowding.
 */
void evolve(struct life_t *life) {
    int x, y, i, j, r, c;

    int alive_neighbs; // Number of alive neighbours

    int ncols = life->num_cols;
    int nrows = life->num_rows;
 
    // 1. Let every cell in the grid evolve.
    #pragma omp parallel for private(alive_neighbs, x, i, j, r, c)
    for (y = 0; y < ncols; y++) 
        for (x = 0; x < nrows; x++) {
            alive_neighbs = 0;

            // 1.a Check the 3x3 neighbourhood
            for (i = y - 1; i <= y + 1; i++)
                for (j = x - 1; j <= x + 1; j++) {
                    // Compute the actual row/col coordinates in the GoL board.
                    //
                    // Remember that the board represents an hypothetically infinite world. In order to do that,
                    // it has to be modelled as a circular matrix, with cells along outer borders considered adjacent to one another.
                    // By applying the modulo operator, %, we account for this possibility. 
                    c = (i + ncols) % ncols;
                    r = (j + nrows) % nrows;

                    if (!(i == y && j == x) // Skip the current cell (x, y)
                            && life->grid[c][r] == ALIVE)
                        alive_neighbs++;
                }

            // 1.b Apply GoL rules to determine the cell's state
            if (alive_neighbs == 3
                    || (alive_neighbs == 2 && life->grid[y][x] == ALIVE))
                life->next_grid[y][x] = ALIVE;
            else
                life->next_grid[y][x] = DEAD;
        }

    // 2. Replace the old grid with the updated one.
    #pragma omp parallel for private(x)
    for (y = 0; y < ncols; y++) 
        for (x = 0; x < nrows; x++) 
            life->grid[y][x] = life->next_grid[y][x];
}

/**
 * Perform GoL evolution for a given amount of generations and measure execution times.
 */
void game(struct life_t *life) {
    int x, y, t;

    struct timeval start, end;
    
    initialize(life);

    int ncols = life->num_cols;
    int nrows = life->num_rows;

    double tot_time = 0.;
    double cur_time = 0.;

    #ifdef GoL_DEBUG
    FILE *log_ptr = init_log_file(*life);
    #endif

    display(*life, false);

    for(t = 0; t < life->timesteps; t++) { 
        // 1. Track the start time
        gettimeofday(&start, NULL);
        
        // 2. Let the current generation evolve
        evolve(life);
        
        // 3. Track the end time
        gettimeofday(&end, NULL);

        cur_time = elapsed_wtime(start, end);
        tot_time += cur_time;

        if (is_big(*life)) {
            printf("Generation #%d took %.5f ms\n", t, cur_time);  

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
        log_data(log_ptr, t, cur_time, tot_time);
        #endif
    }

    printf("\nTotal processing time of GoL evolution for %d generations: %.5f ms\n",
        life->timesteps, tot_time);

    #ifdef GoL_DEBUG
    fflush(log_ptr);
    fclose(log_ptr);
    #endif
}

void cleanup(struct life_t *life) {
    int i;
    int ncols = life->num_cols;

    #pragma omp parallel for
    for (i = 0; i < ncols; i++) {
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
    struct life_t life;

    // 1. Initialize vars from args
    parse_args(&life, argc, argv);

    #ifdef _OPENMP
    omp_set_num_threads(life.num_threads);
    #endif

    // the approach of master is not the correct one, is not scallable and has 
    // allot of dropdowsn (look readme)
    #ifdef _MPI
    int error, i, j, from, to, rows_per_processor;

    // The MPI standard does not say what a program can do before an MPI_INIT or after an 
    // MPI_FINALIZE. In the MPICH implementation, you should do as little as possible. 
    // In particular, avoid anything that changes the external state of the program, 
    // such as opening files, reading standard input or writing to standard output.
    error = MPI_Init(&argc, &argv);

    // declare the chunk structure
    struct chunk_t chunk;
    MPI_Status msg_status;

    // get the processes info
    MPI_Comm_size(MPI_COMM_WORLD, &chunk.size);
    MPI_Comm_rank(MPI_COMM_WORLD, &chunk.rank);

    // calculate the number of rows that each process has to handle 
    // handle the case in which the rows are less then the processors
    rows_per_processor = (int) life.num_rows/chunk.size;

    // printf("The row per process are %d \n", rows_per_processor);
    // computing the begining row of each process
    from = chunk.rank * rows_per_processor;

    // printf("%d processes rows from %d\n", chunk.rank, from);
    // computing the last row of each process
    // handle the last chunk that could have more rows
    to = (chunk.rank + 1) * rows_per_processor;
    // printf("%d processes rows until %d\n", chunk.rank, to);

    // define the dimension of each chunk
    // handle the last chunk that could have more rows
    chunk.num_rows = rows_per_processor;
    chunk.num_cols = life.num_cols;

    // initializing the chunk
    initialize_chunk(&chunk, life, from, to);

    for (i = 0; i < chunk.num_cols + 2; i++){
        for (j = 0; j < chunk.num_rows + 2; j++){
            printf("rank %d printed: [%d][%d] = %d\n", chunk.rank, i, j, chunk.chunk[i][j]);
        }
    }

    error = MPI_Finalize();
    #endif

    // 2. Launch the simulation
    // game(&life);

    // 3. Free the memory
    // cleanup(&life);
}
