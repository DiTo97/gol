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
            printf(life.grid[x][y] ? "\033[07m  \033[m" : "  ");

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

    for (x = 1; x < nrows + 1; x++) {
        for (y = 0; y < ncols; y++)
            printf(chunk.chunk[x][y] ? "\033[07m  \033[m" : "  ");

        printf("\033[E");
    }
}

/**
 * Print the current GoL board to console.
 */
void show_buffer(int ncols, int nrows, unsigned int *buffer) {
    int x, y;

    for (x = 0; x < nrows; x++) {
        for (y = 0; y < ncols; y++)
            printf(*((buffer + x*ncols) + y) ? "\033[07m  \033[m" : "  ");

        printf("\033[E");
    }
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

#ifdef GoL_MPI
void evolve_chunk(struct chunk_t *chunk){
    int x, y, i, j, r, c;

    int alive_neighbs; // Number of alive neighbours

    int ncols = chunk->num_cols;
    int nrows = chunk->num_rows;
 
    // 1. Let every cell in the grid evolve.
    #pragma omp parallel for private(alive_neighbs, y, i, j, r, c)
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

void display_chunk(struct chunk_t *chunk) {
    if (chunk->rank == 0) {
        int r;
        MPI_Status status;

        // 1. Print its chunk
        show_chunk(*chunk);

        int nrows = chunk->num_rows;
        int ncols = chunk->num_cols;

        unsigned int buffer[nrows + chunk->displacement][ncols];

        int j, k;

        for (j = 0; j < nrows + chunk->displacement; j++)
            for(k = 0; k < ncols; k++)
                buffer[j][k] = DEAD;

        // 2. Collect and print other processes'
        for (r = 1; r < chunk->size; r++) {
            MPI_Recv(&buffer[0][0], (nrows + chunk->displacement) * ncols,
                        MPI_INT, r, PRINT, MPI_COMM_WORLD, &status);

            int rrows = (r == chunk->size - 1) \
                        ? nrows + chunk->displacement \
                        : nrows;

            show_buffer(ncols, rrows, &buffer[0][0]);

            // printf("%d %d\n", rrows, ncols);

            // for (j = 0; j < rrows; j++) {
            //     for(k = 0; k < ncols; k++)
            //         printf(buffer[j][k] == ALIVE ? "X" : " ");

            //     printf("\n");
            // }
        }

        // 3. Flush buffer to stdout
        fflush(stdout);
    } else{
        MPI_Send(&chunk->chunk[1][0], chunk->num_rows * chunk->num_cols,
                    MPI_INT, 0, PRINT, MPI_COMM_WORLD);
    }

    usleep(160000);
}

void game_chunk(struct chunk_t *chunk, int timesteps, bool big){
    int i;
    MPI_Status status;

    struct timeval gstart, gend;

    if (!big)
        display_chunk(chunk);

    for (i = 0; i < timesteps; i++){
        if (chunk->rank == 0)
            // Track the start time
            gettimeofday(&gstart, NULL);

        // Let the current chunk evolve
        evolve_chunk(chunk);

        if (chunk->rank == 0)
            // Track the end time
            gettimeofday(&gend, NULL);

        if(big) {
            if (chunk->rank == 0) {
                double cur_gtime = elapsed_wtime(gstart, gend);
                printf("Generation #%d took %.5f ms on process 0\n", i, cur_gtime);  
            }
        }else{
            display_chunk(chunk);
        }

        int prev_rank = (chunk->rank - 1 + chunk->size) % chunk->size;
        int next_rank = (chunk->rank + 1) % chunk->size;

        // int recv_rows = chunk->num_rows + 1;

        // if (prev_rank == chunk->size - 1) {
        //     recv_rows += chunk->displacement;
        // }

        MPI_Sendrecv(&chunk->chunk[1][0], chunk->num_cols, MPI_INT, prev_rank, TOP,
                     &chunk->chunk[chunk->num_rows + 1][0], chunk->num_cols, MPI_INT, next_rank, TOP,
                     MPI_COMM_WORLD, &status);

        MPI_Sendrecv(&chunk->chunk[chunk->num_rows][0], chunk->num_cols, MPI_INT, next_rank, BOTTOM,
                     &chunk->chunk[0][0], chunk->num_cols, MPI_INT, prev_rank, BOTTOM,
                     MPI_COMM_WORLD, &status);
    }
}

void debug_chunk(struct chunk_t chunk) {
    printf("Number of cols: %d\n", chunk.num_cols);
    printf("Number of rows: %d\n", chunk.num_rows);
    printf("Rank: %d\n", chunk.rank);
    printf("Communicator size: %d\n", chunk.size);
    printf("Rows displacement: %d\n", chunk.displacement);

    fflush(stdout);
}
#endif

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

    #ifdef GoL_MPI
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


        // from = 0;
        // to = 1;

        // define the dimension of each chunk
        chunk.num_cols = life.num_cols;

        // initializing the chunk
        initialize_chunk(&chunk, life, input_ptr, from, to);

        game_chunk(&chunk, life.timesteps, is_big(life));

        // debug_chunk(chunk);

        MPI_Barrier(MPI_COMM_WORLD);
        // printf("Processor %d: %d, %d\n", chunk.rank, from, to);


        // if (chunk.rank == 1)
        //     usleep(1000);

        // show_chunk(chunk);
        if(chunk.rank == 0){
            gettimeofday(&end, NULL);
            printf("The execurion time is %.5f ms\n", elapsed_wtime(start, end));
        }
    }else{
        // 2. Launch the simulation
        game(&life);

        // 3. Free the memory
        cleanup(&life);
        gettimeofday(&end, NULL);
        printf("The execurion time is %.5f ms\n", elapsed_wtime(start, end));
    }

    error = MPI_Finalize();

    #else
    // 2. Launch the simulation
    game(&life);

    // 3. Free the memory
    cleanup(&life);

    gettimeofday(&end, NULL);
    printf("The total execurion time in sequential case is %.5f ms", elapsed_wtime(start, end));

    #endif
}
