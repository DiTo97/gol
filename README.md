# Game of Life

Comparative analysis of possible parallel implementations of Conway's famous [Game of Life](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life) (GoL) using CUDA, OpenMP and MPI toolkits on INFN's [Ocapie cluster](https://web.ge.infn.it/calcolo/joomla/2-uncategorised/106-farm-hpc-ocapie) for HPC.

**Authors**: F. Minutoli, M. Ghirardelli, and D. Surpanu.

## Useful links

- [Parallel Programming Illustrated through Conway's Game of Life](https://tcpp.cs.gsu.edu/curriculum/?q=system/files/ch10.pdf)
- [Parallelization: Conway's Game of Life](http://www.shodor.org/media/content/petascale/materials/UPModules/GameOfLife/Life_Module_Document_pdf.pdf)
- [BWPEP on Conway's Game of Life](http://shodor.org/petascale/materials/UPModules/exercises/Game_of_Life/)
- [A Performance Analysis of GoL](https://arxiv.org/pdf/1209.4408.pdf)
- [What is a Dwarf in HPC?](https://www5.in.tum.de/lehre/vorlesungen/hpc/WS15/structured.pdf)

## Useful information

### Full-matrix format

### CPU compilation

Sample command: **icc gol.c -o GoL**

-qopenmp, enables OpenMP support

### Number of available threads

- **Number of threads per PID**: ps -o nlwp <pid>
- **Info on the architecture**: lscpu

256 processors, 64 cores per processor and 4 threads per core.

### Compiler optimization

icc -O{i} -ipo -fast -g -opt-report -xHost -sse{k}  

i := {0, 1, 2, 3}  
k := {1, 2, 3}

-qopt-report={0, ..., 5}  
-qopt-report-phase=vec

-g, creates symbols for debugging.

-ipo, slows compilation down in exchange of an appreciable boost in performance.

### FIXMEs

### TODOs

- Add MPI/CUDA support to log filenames.

- [How to measure elapsed wall-clock time?](https://stackoverflow.com/questions/12392278/measure-time-in-linux-time-vs-clock-vs-getrusage-vs-clock-gettime-vs-gettimeof)

### Defaults

- The borderline size to distinguish a small GoL's grid from a big one has been updated to $50$x$50$.

### MPI pseudo-code

**Please note:** The rank 0 process is the only process authorized to interface with display operations.

1. The rank 0 process parses arguments and creates a `life_t` structure
2. All processes store the number of processes and the communicator size from MPI
3. All processes compute the correct indexes for their chunk of data
4. All processes allocate memory for their chunk of data in a `chunk_t` structure
5. All processes fill in the memory with their chunk of data either via file or via random generation. The data will also comprise generation 0's ghost rows, in order to avoid the 1st round of message passing.
6. For the number of generations `N`:
    - All processes evolve their chunk
    - If the grid is small:
        - All other processes send their updated chunk back to the rank 0 process
        - The rank 0 process sequentially prints the updated grid to console
    - All processes share ghost rows with adjacent peers
7. All other processes send their final chunk back to the rank 0 process
8. The rank 0 process sequentially prints the final grid to console/file

### CUDA

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

 ### Input file format

- Before reading the GoL matrix from an input file, replace the first character of the first row with the 'A' character to
  avoid the skipping behavior of getline. Do this if the first character is a blank space (a DEAD cell).


### Metrics used for the experiments

1. Number of processed cells per generation (alternative to cumulative execution time)
  - (Milliseconds for one generation * 1000) / (world size)
2. Total execution time 

### Classes of experiments, for each GoL version (CUDA, MPI, OPENMP, HYBRID ETC ETC)

#### General settings

Max grid dimensions: 
  44000 x 44000 (half of the available space GPU global memory for CUDA)

Dimensions used for the all the experiments:
  100x100, 500x500, 1000x1000, 5000x5000, 10000x10000
  
Dimensions used for parallel GoL programs:
  20000x20000, 40000x40000

Grid dimensions for MPI only:
    1000x100000

Number of repetitions for each experiment: 3

1 node:
  - 256 CPUs
  - 64 cores per processor
  - 4 threads per core

12 nodes on the whole INFN cluster

Probability?

Timesteps?

Seed fixed!

#### How to perform the experimental analysis (as explained in class) and recommended parameter values for Xeon Phi architecture

- Compute the sequential time, that is, we consider p=1 as a baseline for comparisons

- Time sequential / time parallel = Ts/Tp = speedup

- Speedup / p = efficiency


##### OpenMP

OpenMP threads (SMT):
  2, 4, 8, 16, 32, 64, 128, 256
    
- 256 := Total processing capability, 64 cores x 4 threads
- 64  := Total physical processing units, 64 cores
- 32  := Half the above quantity, 64 / 2 cores

Guidelines:

- p values interesting for our system: numbers in 2..1024*
  - Among these values, use first p=2, which is the smallest
    number of cores used to parallelize a program.
  - Then, use 64 physical cores, as reported in the specifications for each CPU of the INFN hpc ocapie cluster.
  64 is the maximum number of cores per CPU.
  - Then try to use 32 cores (half of the available cores for each CPU), so to make each thread exploit more efficiently the L2 cache, which is shared among multiple threads (reduce number of accesses in global memory)
  - You may also try 16 cores.
  - Try also 256, which is the maximum number of threads per CPU. This is obtained with hyperthreading (simultaneous multi-threading).
  - For further experiments, use powers of 2 for the number of cores for a single CPU

The above observations apply for a single node (single process, multiple cores). 

##### MPI

MPI processes:
    1 node:
        2, 4, 8, 16, 32, 64, 128, 256

    2 nodes:
        2, 4, 8, 16, 32, 64, 128, 256, 512

    4 and 8 nodes:
        2, 4, 8, 16, 32, 64, 128, 256, 512, 1024

Guidelines:

- Verify what happens if you use the same numbers defined before with a different number of nodes (machines) and MPI processes, as well as if you use other values.

- Note on nodes vs processes: each node may contain more than 1 process, depending on the node architecture. The processes may be distributed equally among different nodes, using the following command:
  - mpiexec -hostfile NODES_LIST_FILE -perhost PROCESSES_PER_NODE -np TOT_NUM_OF_PROCESSES ./BIN_FILE ARGS
  In particular, NODES_LIST_FILE is a list of nodes, with a line for every hpcocapieX, with X = 01, 02, ..., 11.
  Add the optional -nolocal flag, if the calling node shouldn't do any processing.

- The resources requested with MPI commands MUST be available in your system

- For further experiments, use powers of 2 in [1..12] for the number of nodes, powers of 2 in [1..1024] for the number of processes

##### Hybrid architectures

OpenMP + MPI hybrid:
    Extending the considerations made for OpenMP and MPI by enforcing the total number of available OpenMP threads per MPI process,
    # of OpenMP threads * # of MPI processes = 256 * # of nodes

##### CUDA

CUDA block size:
    32, 64, 128, 256, 512, 1024

    No lower than the warp size (32), because that would be highly inefficient

##### Other observations to be taken into account

- The improvements in speed are negligible for small grid sizes (may result in smaller speedup), depending on the number of threads: using more physical cores to solve the same problem results in performance reduction

##### Core question

Have I achieved acceptable performance on my software for a suitable range of data and the resources I'm using?