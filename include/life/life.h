#ifndef GoL_LIFE_H
#define GoL_LIFE_H

/**
 * All the data required by a Game of Life instance.
 */ 
struct life_t {
    int ncols;         // Number of columns in the grid
    int nrows;         // Number of rows in the gird
    int timesteps;     // Number of generations to simulate

    double init_prob;  // Probability to mark a cell as ALIVE
                       // when following a random initialization

    #ifdef _OPENMP
    int nthreads;      // Number of total OpenMP threads
    #endif

    #ifdef GoL_CUDA
    int block_size;    // Number of threads per CUDA block
    #endif

    unsigned int seed; // Random seed initializer

    /*
     * When using CUDA, GoL's grid is defined as a 1D array rather than a 2D one. This choice derives from the logic behind the computation
     * of the neighborhood that's being adopted in CUDA. Check the evolve() function for more details.
     */ 
    #ifdef GoL_CUDA
    bool *grid;        // Game grid at the current step
    #else
    bool **grid;       // Game grid at the current step
    bool **next_grid;  // Game grid at the next step
    #endif
    
    char *infile;      // Input filename
    char *outfile;     // Output filename
};

/***********************
 * Evolution functions *
 ***********************/

void initialize(struct life_t *life);
double game(struct life_t *life);

#ifdef GoL_CUDA
__global__ void evolve(bool *gpu_grid,
        bool *gpu_next_grid, int nrows, int ncols);
#else
void evolve(struct life_t *life);
#endif

void cleanup(struct life_t *life);

/***********************
 * Debugging functions *
 ***********************/

#ifdef GoL_DEBUG
/**
 * Print to console the status of the current GoL board: the number of ALIVE and DEAD cells.
 */
void show_grid_status(struct life_t life) {
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
        for (j = 0; j < ncols; j++) {
            #ifdef GoL_CUDA
            life.grid[i*ncols + j] == ALIVE \
                ? n_alive++ : n_dead++;
            #else
            life.grid[i][j] == ALIVE \
                ? n_alive++ : n_dead++;
            #endif
        }
    
    printf("Number of ALIVE cells: %d\n",  n_alive);
    printf("Number of DEAD cells: %d\n\n", n_dead);

    fflush(stdout);
    usleep(320000);
}

/**
 * Print to console the metadata that characterizes the current GoL board. 
 */ 
void debug(struct life_t life) {
    printf("Number of cols: %d\n", life.ncols);
    printf("Number of rows: %d\n", life.nrows);
    printf("Number of timesteps: %d\n", life.timesteps);
    printf("Probability for grid initialization: %f\n", life.init_prob);
    printf("Random seed initializer: %d\n", life.seed);

    #ifdef _OPENMP
    printf("Number of total OpenMP threads: %d\n", life.nthreads);
    #endif

    #ifdef GoL_CUDA
    printf("Number of threads per CUDA block: %d\n", life.block_size);
    #endif

    printf("Input file: %s\n", life.infile == NULL ? "None" : life.infile);
    printf("Output file: %s\n\n", life.outfile);

    fflush(stdout);
}
#endif

/*********************
 * Utility functions *
 *********************/

/**
 * Evaluate whether the GoL board is larger than DEFAULT_MAX_SIZE.
 * 
 * @return true if GoL grid larger, false otherwise
 */
bool is_big(struct life_t life) {
    return life.nrows * life.ncols > DEFAULT_MAX_SIZE;
}

/*********************
 * Display functions *
 *********************/

/**
 * Print the current GoL board to console.
 */
void show(struct life_t life) {
    int i, j;

    int ncols = life.ncols;
    int nrows = life.nrows;

    // \033[H: Move cursor to top-left corner;
    // \033[J: Clear console.
    printf("\033[H\033[J");

    for (i = 0; i < nrows; i++) {
        for (j = 0; j < ncols; j++) {
            #ifdef GoL_CUDA
            printf(life.grid[i*ncols + j] == ALIVE
                ? "\033[07m  \033[m" : "  ");
            #else
            printf(life.grid[i][j] == ALIVE
                ? "\033[07m  \033[m" : "  ");
            #endif
        }
        printf("\033[E"); // Move cursor to next line
    }

    fflush(stdout);
    usleep(160000);
}

/**
 * Print the current GoL board to file.
 *     1. A header will comprise the board dimensions (e.g., 6 6);
 *     2. A line filled with 'X' and ' ' will correspond to each row of GoL's board.
 * 
 * @param append    Whether to append to or to overwrite the output file.
 */
void printbig(struct life_t life, bool append) {
    int i, j;
    
    int ncols = life.ncols;
    int nrows = life.nrows;

    FILE *out_ptr = append \
        ? fopen(life.outfile, "a" ) \
        : fopen(life.outfile, "w" );

    if (out_ptr == NULL) {
        perror("[*] Failed to open the output file.");
        exit(EXIT_FAILURE);
    }
     
    if (!append) // Print board dimensions only once
        fprintf(out_ptr, "%d %d\n", nrows, ncols);

    for (i = 0; i < nrows; i++) {
        for (j = 0; j < ncols; j++) {
            #ifdef GoL_CUDA
            fprintf(out_ptr, life.grid[i*ncols + j] == ALIVE
                ? 'X' : ' ');
            #else
            fprintf(out_ptr, life.grid[i][j] == ALIVE
                ? 'X' : ' ');
            #endif
        }  
        fprintf(out_ptr, "\n");
    }

    fprintf(out_ptr, "****************************************************************************************************\n");

    fflush(out_ptr);
    fclose(out_ptr);
}

/**
 * Print the current GoL board to either console or file depending on whether its size is larger than DEFAULT_MAX_SIZE.
 * 
 * @param append    Whether to append to or to overwrite the output file, if in use.
 */
void display(struct life_t life, bool append) {
    if(is_big(life)) printbig(life, append);
    else show(life);
}

#endif
