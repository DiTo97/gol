# Game of Life on CPU

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
