#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "gol.h"
#include "utils.h"


//TODO: add display function which makes use of CUDA

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
            printf(life.grid[x*ncols+y] == ALIVE ? "\033[07m  \033[m" : "  ");

        printf("\033[E");
    }

    fflush(stdout);
    usleep(160000);
}

/**
 * Print the current GoL board to either console or file depending on whether its size is larger than DEFAULT_MAX_SIZE.
 */
void display(struct life_t life, bool append) {
    if(is_big(life)) printbig(life, append);
    else show(life);
}

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


// CUDA facts:
//
// On devices of compute capability 2.x and beyond, 32-bit integer multiplication is natively supported,
// but 24-bit integer multiplication is not. __[u]mul24 is therefore implemented using multiple instructions
// and should not be used.
//
// Integer division and modulo operation are costly: below 20 instructions on devices of compute capability 2.x and
// higher. They can be replaced with bitwise operations in some cases: If n is a power of 2, (i/n) is equivalent to
// (i>>log2(n)) and (i%n) is equivalent to (i&(n-1)); the compiler will perform these conversions if n is literal.

/**
 * Perform one evolutionary step of the board, following GoL rules, in this order:
 * 
 *     1. A cell is born, if it has exactly 3 neighbours;
 *     2. A cell dies of loneliness, if it has less than 2 neighbours;
 *     3. A cell dies of overcrowding, if it has more than 3 neighbours;
 *     4. A cell survives to the next generation, if it does not die of loneliness or overcrowding.
 */
///
/// @param curr_grid  Linearized 2D array of life data with byte-per-cell density.
/// @param num_cols  Width of life world in cells (bytes).
/// @param num_rows  Height of life world in cells (bytes).
/// @param next_grid  Result buffer in the same format as input.
__global__ void cuda_evolve(bool * curr_grid, bool * next_grid, int num_rows, int num_cols) {
    // Total number of cells in the grid
	int world_size = num_cols * num_rows;

    /* for (uint cellId = blockIdx.x * blockDim.x + threadIdx.x;
        cellId < world_size;
        cellId += blockDim.x * gridDim.x) { */
    uint cellId = blockIdx.x * blockDim.x + threadIdx.x;
    // retrieve index of the column associated with cellId
    if (cellId >= world_size) {
        return;
    }
    uint x = cellId % num_cols;
    // retrieve the absolute index, inside the 1D board, of the start of the row which corresponds to cellId
    // absolute: the modulo operator is not applied (e.g. with 4 blocks, 4 threads per block, it would yield 15)
    uint yAbs = cellId - x;

    // retrieve the indexes of the columns of the left and right neibors, respectively
    // for example, if each block is composed by 4 threads, then the possible values for 
    // xLeft and xRight are integers in [0, 3]
    uint xLeft = (x + num_cols - 1) % num_cols; 
    uint xRight = (x + 1) % num_cols;

    // retrieve the absolute index, in the 1D board, of the start of the row which corresponds to the upper neighbor
    uint yAbsUp = (yAbs + world_size - num_cols) % world_size;
    // retrieve the absolute index, in the 1D board, of the start of the row which corresponds to the lower neighbor
    uint yAbsDown = (yAbs + num_cols) % world_size;

    // Count alive neighbors. To do so, use the indexes computed in the previous steps. 
    uint alive_cells = curr_grid[xLeft + yAbsUp] + curr_grid[x + yAbsUp] + curr_grid[xRight + yAbsUp] \
        + curr_grid[xLeft + yAbs] + curr_grid[xRight + yAbs] \
        + curr_grid[xLeft + yAbsDown] + curr_grid[x + yAbsDown] + curr_grid[xRight + yAbsDown];

    // ternary operator used to improve performance and avoid warp divergence
    // x + yAbs represents the index of the current cell processed by a thread on the 1D board
    next_grid[x + yAbs] = alive_cells == 3 || (alive_cells == 2 && curr_grid[x + yAbs]) ? 1 : 0;
    //}
}

/**
 * Perform GoL evolution for a given amount of generations and measure execution times.
*/
// TODO: implement CUDA printf for GoL board
double game(struct life_t *life) {
    int t;

    struct timeval start, end;
    
    // initializing the whole matrix only if not running with MPI
    initialize(life);

    int ncols = life->num_cols;
    int nrows = life->num_rows;

    double cum_gen_time = 0.;
    double cur_time = 0.;
	
	// compute number of threads, check whether this number does not exceed the max number of threads per block
	int threads_count = life->nthreads_per_block <= DEFAULT_MAX_NUM_THREADS_PER_BLOCK ? life->nthreads_per_block : DEFAULT_MAX_NUM_THREADS_PER_BLOCK;
	int blocks_count;
	if ((ncols*nrows)%threads_count!=0) {
		// if the number of threads per block is not a multiple of the board size, compute the number of blocks accordingly
		blocks_count = (int)((ncols * nrows + (threads_count - 1))/threads_count);
	}
	else {
		// else, compute the number of threads by diving the total size of the board
		blocks_count = (ncols * nrows) / threads_count;
	}
	
	bool* curr, *next;
	// allocate memory on the device, the board is again expressed as a single array
	cudaMalloc((void **)&curr, ncols * nrows * sizeof(bool));
	// copy board to device, on the allocated space
	cudaMemcpy(curr, life->grid, ncols * nrows * sizeof(bool), cudaMemcpyHostToDevice);

	// initialize another board on the GPU with zeros
	cudaMalloc((void **)&next, ncols * nrows * sizeof(bool));
	cudaMemset(next, 0, ncols * nrows * sizeof(bool));

	display(*life, false);

	// each block is associated with threads_count threads
	dim3 block_size(threads_count);
	// the grid contains block_count blocks
	dim3 grid_size(blocks_count);

    for(t = 0; t < life->timesteps; t++) { 
        // 1. Track the start time
        gettimeofday(&start, NULL);
        
		// 2. Let the current generation evolve, using a CUDA kernel
		cuda_evolve<<<grid_size, block_size>>>(curr, next, life->num_rows, life->num_cols);
        
		// TODO: implement ifdefs to remove synchronization across timesteps, in case of "performance mode"
		cudaDeviceSynchronize();

        // 3. Track the end time
		gettimeofday(&end, NULL);
        

		bool* temp = curr;
		curr=next;
		next=temp;

        cur_time = elapsed_wtime(start, end);
        cum_gen_time += cur_time;

        if (is_big(*life)) {
            printf("Generation #%d took %.5f ms\n", t, cur_time);  

            // If the GoL grid is large, print it (to file)
            // only at the end of the last generation
            if (t == life->timesteps - 1) {
				cudaMemcpy(life->grid, curr, ncols * nrows * sizeof(bool), cudaMemcpyDeviceToHost);
                display(*life, true);
            }
        } else {
			cudaMemcpy(life->grid, curr, ncols * nrows * sizeof(bool), cudaMemcpyDeviceToHost);
            display(*life, true);
        }

	}
	
    printf("\nTotal processing time of GoL evolution for %d generations: %.5f ms\n",
        life->timesteps, cum_gen_time);

	// free memory on GPU
	cudaFree(curr);
	cudaFree(next);

    return cum_gen_time;
}

void cleanup(struct life_t *life) {  
    free(life->grid);
    free(life->next_grid);
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

    // reading the file if present and setting life dimensions
    FILE *input_ptr = set_grid_dimens_from_file(&life);

	// 2. Launch the simulation
    double cum_gen_time = game(&life);

    // 3. Free the memory
    cleanup(&life);

    gettimeofday(&end, NULL);

    double elapsed_prog_wtime = elapsed_wtime(start, end);
    
    printf("The total execution time is %.5f ms", elapsed_prog_wtime);

    #ifdef GoL_DEBUG
    FILE *log_ptr = init_log_file(life);
    log_data(log_ptr, life.timesteps, cum_gen_time, elapsed_prog_wtime);
    fflush(log_ptr);
    fclose(log_ptr);
    #endif
}