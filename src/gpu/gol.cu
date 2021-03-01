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
 * Perform GoL evolution for a given amount of generations on a GPU kernel.
 * 
 * @return tot_gene_time    The total time devolved to GoL evolution
 */
double game(life_t *life) {
    int t;

    struct timeval gstart, gend;
    
    // initializing the whole matrix only if not running with MPI
    initialize(life);

    int ncols = life->ncols;
    int nrows = life->nrows;

    double tot_gene_time = 0.;
    double cur_gene_time = 0.;

    // Check whether the requested block size is valid,
    // or fewer threads have to be assigned to each block
    int block_size = life->block_size <= DEFAULT_MAX_BLOCK_SIZE \
            ? life->block_size : DEFAULT_MAX_BLOCK_SIZE;

    // Check whether the data can be evenly
    // distributed across blocks, or the grid size
    // has to be enlarged accordingly...
    int grid_size = -1;

    if ((ncols*nrows) % block_size == 0)
        grid_size = (int)(ncols*nrows) / block_size;
    else // ...via the formula presented in the HPC course slides
        grid_size = (int)((ncols*nrows + block_size - 1) / block_size);

    // Size of all cells in the grid
    size_t world_size = ncols*nrows * sizeof(bool);
    
    // Init the 1D data structures hosted on GPU
	bool *gpu_grid, *gpu_next_grid;

    // Copy the data from host to device in the 1st grid
    cudaMalloc((void **) &gpu_grid, world_size);
    cudaMemcpy(gpu_grid, life->grid, world_size,
            cudaMemcpyHostToDevice);

    // Fill with DEAD cells the 2nd grid
    cudaMalloc((void **) &gpu_next_grid, world_size);
    cudaMemset(gpu_next_grid, DEAD, world_size);

	display(*life, false);

    for(t = 0; t < life->timesteps; t++) { 
        // 1. Track the start time
        gettimeofday(&gstart, NULL);
        
		// 2. Evolve the current generation with a CUDA kernel
		evolve<<<grid_size, block_size>>>(
                gpu_grid, gpu_next_grid, nrows, ncols);
        
        // 3. Wait for all CUDA threads to finish
		cudaDeviceSynchronize();

        // 4. Swap the memory pointers on GPU
        swap_grids(&gpu_grid, &gpu_next_grid);

        // 5. Track the end time
		gettimeofday(&gend, NULL);

        cur_gene_time = elapsed_wtime(gstart, gend);
        tot_gene_time += cur_gene_time;

        if (is_big(*life)) {
            printf("Generation #%d took %.5f ms\n", t, cur_gene_time);  

            // If the GoL grid is large, print it (to file)
            // only at the end of the last generation
            if (t == life->timesteps - 1) {
				cudaMemcpy(life->grid, gpu_grid, world_size,
                        cudaMemcpyDeviceToHost);
                display(*life, true);
            }
        } else {
			cudaMemcpy(life->grid, gpu_grid, world_size,
                    cudaMemcpyDeviceToHost);
            display(*life, true);
        }
	}

    printf("\nEvolved GoL's grid for %d generations - ETA: %.5f ms\n",
            life->timesteps, tot_gene_time);

	// Free the memory on GPU
	cudaFree(gpu_grid);
	cudaFree(gpu_next_grid);

    return tot_gene_time;
}

/**
 * Perform one evolutionary step of the board, following GoL rules, in this order:
 *     1. A cell is born, if it has exactly 3 neighbours;
 *     2. A cell dies of loneliness, if it has less than 2 neighbours;
 *     3. A cell dies of overcrowding, if it has more than 3 neighbours;
 *     4. A cell survives to the next generation, if it doesn't die of loneliness or overcrowding.
 * 
 * All CUDA threads will access the global memory in order to establish the a cell's next state. Indeed in this case,
 * having each thread to access only 8 different memory locations, we'd argue that memory coalescing, that is,
 * the aggregation step that CUDA already makes under the wood when multiple threads try to access nearby memory location,
 * is an already sufficient optimization step without having to make expicit use of shared memory.
 * 
 * In order to avoid warp divergence with CUDA, here we follow a different approach from that on CPU. Indeed instead of
 * relying on a for loop looking for all neighbours, here we compute their indexes 1-by-1 straight away,
 * as suggested by http://www.marekfiser.com/Projects/Conways-Game-of-Life-on-GPU-using-CUDA
 * 
 * @param gpu_grid         The 1D data on GPU with byte-per-cell density
 * @param gpu_next_grid    The result buffer after evolution on GPU
 * @param ncols            The width of GoL's grid in bytes  (= # of cols)
 * @param nrows            The height of GoL's grid in bytes (= # of rows)
 */
__global__
void evolve(bool *gpu_grid, bool *gpu_next_grid,
        int nrows, int ncols) {
    // # of all cells in the grid
    int world_size = ncols*nrows;

    // 1. Identify the cell the calling thread works on
    // by using this standard CUDA formula
    int cell_id = blockIdx.x*blockDim.x + threadIdx.x;

    // If GoL's data couldn't be evenly distributed across blocks,
    // there will some threads instantiated in the last block
    // without an actual cell to work on; hence, if their target
    // cell Id is outside the world size, they have to return.
    if (cell_id >= world_size)
        return;

    /*
     * A running example with a 3x5 grid, a block size of 4 and a grid size of 4:
     * 0, A, 0, A, 0,
     * 0, 0, 0, A, A,
     * A, 0, 0, 0, 0
     * 
     * with both a block size and a grid size of 4:
     * [0, A, 0, A], [0, 0, 0, 0], [A, A, A, 0] [0, 0, 0, -]
     *  
     * The target cell P is at location (1, 3) = A and its Id is 2*4 + 0 = 8.
     */

    // 2. Retrieve the 0-indexed positions inside the 1D grid of the column, x,
    // and of the start of the row, y, of the target cell
    int x = cell_id % ncols;
    int y = cell_id - x;

    /*
     * x = 8 % 5 = 3
     * y = 8 - 3 = 5
     * 
     * P is indeed in the 3rd column, and the 5th element of
     * the flattened 1D array is the 0 at the start of the 1st row.
     */

    // 3. Retrieve the 0-indexed columns
    // of the left and right neighbours
    int x_left  = (x + ncols - 1) % ncols; 
    int x_right = (x + 1) % ncols;

    /*
     * x_left  = (3 + 5 - 1) % 5 = 2
     * x_right = (3 + 1) % 5 = 4
     */

    // 4. Retrieve the 0-indexed start of the row positions
    // of the upper and lower neighbours
    int y_up   = (y + world_size - ncols) % world_size;
    int y_down = (y + ncols) % world_size;

    /*
     * y_up   = (5 + 15 - 5) % 15 = 0
     * y_down =  (5 + 5) % 15 = 10
     * 
     * Complete the example by yourself as a counterproof
     * that this method works as intended.
     */

    // 5. Count how many neighbours are ALIVE
    int alive_neighbs = gpu_grid[x_left + y_up] // Top-left neighbour
            + gpu_grid[x + y_up]                // Upper neighbour
            + gpu_grid[x_right + y_up]          // Top-right neighbour
            + gpu_grid[x_left + y]              // Left neighbour
            + gpu_grid[x_right + y]             // Right neighbour
            + gpu_grid[x_left + y_down]         // Bottom-left neighbour
            + gpu_grid[x + y_down]              // Lower neighbour
            + gpu_grid[x_right + y_down]        // Bottom-right neighbour

    // 6. Update the next grid with the new state
    gpu_next_grid[x + y] = (alive_neighbs == 3
            || (alive_neighbs == 2 && gpu_grid[x + y]))
            ? ALIVE : DEAD;
}

void cleanup(life_t *life) {  
    free(life->grid);
}

/************************************
 * ================================ *
 ************************************/

int main(int argc, char **argv) {
    struct timeval start, end;

    life_t life;

    gettimeofday(&start, NULL);

    // 1. Initialize vars from args
    parse_args(&life, argc, argv);

    FILE *input_ptr = set_grid_dimens_from_file(&life);

	// 2. Launch the simulation
    double cum_gene_time = game(&life);
    cleanup(&life);

    gettimeofday(&end, NULL);

    double elapsed_prog_wtime = elapsed_wtime(start, end);
    
    #ifdef GoL_LOG
    FILE *log_ptr = init_log_file(life, 1);

    log_data(log_ptr, life.timesteps, cum_gene_time,
            elapsed_prog_wtime);

    fflush(log_ptr);
    fclose(log_ptr);
    #endif

    printf("Finalized the program - ETA: %.5f ms\n\n", elapsed_prog_wtime);
}
