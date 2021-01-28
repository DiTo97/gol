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
    for (i = 0; i < nrows; i++) 
        for (j = 0; j < ncols; j++)
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
void initialize_chunk(struct chunk_t *chunk, struct life_t life, FILE *input_ptr, int from, int to) {
    srand(life.seed);

    // 3. Allocate memory for the chunk
    malloc_chunk(chunk);

    // 4. Initialize the chunk with DEAD cells
    init_empty_chunk(chunk);
    
    // 5. Initialize the chunk with ALIVE cells...
    if (input_ptr != NULL) { // ...from file, if present...
        init_chunk_from_file(chunk, life.num_rows, input_ptr, from, to);
    } else {  // ...or randomly, otherwise.
        init_random_chunk(chunk, life, from, to);
    }

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
    for (y = 0; y < nrows; y++) 
        for (x = 0; x < ncols; x++) {
            alive_neighbs = 0;

            // 1.a Check the 3x3 neighbourhood
            for (i = y - 1; i <= y + 1; i++)
                for (j = x - 1; j <= x + 1; j++) {
                    // Compute the actual row/col coordinates in the GoL board.
                    //
                    // Remember that the board represents an hypothetically infinite world. In order to do that,
                    // it has to be modelled as a circular matrix, with cells along outer borders considered adjacent to one another.
                    // By applying the modulo operator, %, we account for this possibility. 
                    c = (i + nrows) % nrows;
                    r = (j + ncols) % ncols;

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
    for (y = 0; y < nrows; y++) 
        for (x = 0; x < ncols; x++) 
            life->grid[y][x] = life->next_grid[y][x];
}

/**
 * Perform GoL evolution for a given amount of generations and measure execution times.
 */
void game(struct life_t *life) {
    int x, y, t;

    struct timeval start, end;
    
    // initializing the whole matrix only if not running with MPI
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
    int nrows = life->num_rows;

    #pragma omp parallel for
    for (i = 0; i < nrows; i++) {
        free(life->grid[i]);
        free(life->next_grid[i]);
    }

    free(life->grid);
    free(life->next_grid);
}

void evolve_chunk(struct chunk_t *chunk){
    int x, y, i, j, r, c;

    int alive_neighbs; // Number of alive neighbours

    int ncols = chunk->num_cols;
    int nrows = chunk->num_rows;
 
    // 1. Let every cell in the grid evolve.
    #pragma omp parallel for private(alive_neighbs, x, i, j, r, c)
    for (x = 1; x < nrows + 1; x++) 
        for (y = 0; y < ncols; y++) {
            alive_neighbs = 0;

            // 1.a Check the 3x3 neighbourhood
            for (i = x - 1; i <= x + 1; i++)
                for (j = y - 1; j <= y + 1; j++) {
                    // Compute the actual row/col coordinates in the GoL board.
                    //
                    // Remember that the board represents an hypothetically infinite world. In order to do that,
                    // it has to be modelled as a circular matrix, with cells along outer borders considered adjacent to one another.
                    // By applying the modulo operator, %, we account for this possibility. 
                    c = (j + ncols) % ncols;

                    if (!(i == x && j == y) // Skip the current cell (x, y)
                            && chunk->chunk[i][c] == ALIVE)
                        alive_neighbs++;
                }

            // 1.b Apply GoL rules to determine the cell's state
            if (alive_neighbs == 3
                    || (alive_neighbs == 2 && chunk->chunk[x][y] == ALIVE))
                chunk->next_chunk[x][y] = ALIVE;
            else
                chunk->next_chunk[x][y] = DEAD;
        }

    // 2. Replace the old grid with the updated one.
    #pragma omp parallel for private(x)
    for (x = 1; x < nrows + 1; x++) 
        for (y = 0; y < ncols; y++) 
            chunk->chunk[x][y] = chunk->next_chunk[x][y];
}

void game_chunk(struct chunk_t *chunk, int timesteps, bool big){
    int i;
    MPI_Status status;

    for (i = 0; i < timesteps; i++){
        evolve_chunk(chunk);

        if(big) {
            // print the time for generation

        }else{
            if (chunk->rank == 0) {
                MPI_Recv();
            } else{
                MPI_Send();
            }
        }

        int prev_rank = (chunk->rank - 1 + chunk->size) % chunk->size;
        int next_rank = (chunk->rank + 1) % chunk->size;
        int n_rows = chunk->num_rows + 1;

        // fixme processes with different number of rows
        if (prev_rank == chunk->size - 1) {
            n_rows += chunk->displacement;
        }

        MPI_Sendrecv(chunk[0], chunk->num_cols, MPI_INT, prev_rank, TOP,
                chunk[n_rows], chunk->num_cols, MPI_INT, chunk->rank, BOTTOM,
                MPI_COMM_WORLD, &status);

        MPI_Sendrecv(chunk[chunk->num_rows + 1], chunk->num_cols, MPI_INT, next_rank,
            BOTTOM, chunk[0], chunk->num_cols, MPI_INT, chunk->rank,
            TOP, MPI_COMM_WORLD, &status);
    }
}

/************************************
 * ================================ *
 ************************************/

int main(int argc, char **argv) {
    struct life_t life;
    struct timeval start, end;

    gettimeofday(&start, NULL);

    // 1. Initialize vars from args
    parse_args(&life, argc, argv);

    #ifdef _OPENMP
    omp_set_num_threads(life.num_threads);
    #endif

    // reading the file if present and setting life dimensions
    FILE *input_ptr = set_grid_dimens_from_file(&life);

    #ifdef _MPI
    int error, i, j, from, to, rows_per_processor;

    // the documentation says that only fortran returns an error???????
    error = MPI_Init(&argc, &argv);

    // declare the chunk structure
    struct chunk_t chunk;
    MPI_Status msg_status;

    // get the processes info
    MPI_Comm_size(MPI_COMM_WORLD, &chunk.size);
    MPI_Comm_rank(MPI_COMM_WORLD, &chunk.rank);

    if(chunk.size != 1){
        MPI_Barrier(MPI_COMM_WORLD);

        // calculate the number of rows that each process has to handle 
        rows_per_processor = (int) life.num_rows/chunk.size;
        chunk.displacement = life.num_rows % chunk.size;

        // computing the begining row of each process
        from = chunk.rank * rows_per_processor;

        // computing the last row of each process
        // controlling if I'm the last process then I get all the remaining rows
        if (chunk.rank == chunk.size - 1){
            to = life.num_rows - 1;
            chunk.num_rows = life.num_rows - from;
        } else{
            to = (chunk.rank + 1) * rows_per_processor - 1;
            chunk.num_rows = rows_per_processor;
        }

        // define the dimension of each chunk
        chunk.num_cols = life.num_cols;

        // initializing the chunk
        initialize_chunk(&chunk, life, input_ptr, from, to);

        for (i = 0; i < chunk.num_rows + 2; i++){
            for (j = 0; j < chunk.num_cols; j++){
                printf("rank %d printed: [%d][%d] = %d\n", chunk.rank, i, j, chunk.chunk[i][j]);
            }
        }

        game_chunk(&chunk, life.timesteps, is_big(life));

        MPI_Barrier(MPI_COMM_WORLD);
        if(chunk.rank == 0){
            gettimeofday(&end, NULL);
            printf("The execurion time is: %d\n", elapsed_wtime(start, end));
        }
    }else{
        // 2. Launch the simulation
        game(&life);

        // 3. Free the memory
        cleanup(&life);
        gettimeofday(&end, NULL);
        printf("The execurion time is: %d\n", elapsed_wtime(start, end));
    }

    error = MPI_Finalize();

    #else
    // 2. Launch the simulation
    game(&life);

    // 3. Free the memory
    cleanup(&life);

    gettimeofday(&end, NULL);
    printf("The total execurion time in sequential case is: %d", elapsed_wtime(start, end));

    #endif
}
