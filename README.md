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

### CUDA pseudo-code
