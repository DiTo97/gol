# Experiments on Game of Life

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
