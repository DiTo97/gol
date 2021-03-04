# Game of Life on GPU

## Usage

1. From a local shell run:
```console
ssh -CX user_hpc<L>@130.251.61.97
```

2. Browse to desidered folder
3. Copy the required files from front node to a worker node with:
```console
scp -r "${PWD}" node<X>:/home/user_hpc<L>/path/to/remote/destination
```

4. If no GoL_cuda binary is present within the `bin` folder run:
```console
make cuda
```
   
## Implementation details

1. Create another version of the parse_args function, which uses the following parameters:

    - ncols
    - nrows
    - tsteps
    - output_file
    - input_file
    - seed
    - init_prob
    - help
    - nblocks
    - threads_per_block

   Delete argument life->num_threads, add arguments life->n_blocks and life->nthreads_per_block

2. set_grid_dimens_from_file remains the same

3. game should be modified as follows:

    - Function initialize should allocate a single 1D array (as performed for the MPI implementation) to 
      represent the gol matrix

    - After having called function initialize, use cudaMalloc just twice at the beginning of the GoL game to allocate
      the current board and the next board on the GPU. Use cudaMemcpy to initialize the corresponding values on the GPU, for both
      the current board and the next board. 
        Current board --> at the beginning, contains the initial gol board
        Next board --> at the beginning, contains only zeros
      Optionally, use cudaMemCpy multiple times if you want to retrieve partial results, or if you want to visualize them
      with function display.

    - Define __global__ kernel function *void cudaEvolve* in the main file *gol_cuda.cu*. This function is used to perform
      one step of the GoL game by using the GPU. The implementation of the kernel function uses a single contiguos array 
      instead of a matrix, for the purpose of reducing the number of modulo operations. To implement the kernel function,
      use the "Simple GPU implementation" reported at http://www.marekfiser.com/Projects/Conways-Game-of-Life-on-GPU-using-CUDA/2-Basic-implementation
     
    - The steps in which we measure the execution time remain the same

 4. Modify function cleanup and remove the pragmas

 5. Implement a function *cudaCleanup* to de-allocate the memory of the device
