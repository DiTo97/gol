# Game of Life

Comparative analysis of possible parallel implementations of Conway's famous [Game of Life](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life) (GoL) using CUDA, OpenMP and MPI toolkits on INFN's [Ocapie cluster](https://web.ge.infn.it/calcolo/joomla/2-uncategorised/106-farm-hpc-ocapie) for HPC.

**Authors**: F. Minutoli, M. Ghirardelli, and D. Surpanu.

## Tentative schedule

## Useful links

- [Parallel Programming Illustrated through Conway's Game of Life](https://tcpp.cs.gsu.edu/curriculum/?q=system/files/ch10.pdf)
- [Parallelization: Conway's Game of Life](http://www.shodor.org/media/content/petascale/materials/UPModules/GameOfLife/Life_Module_Document_pdf.pdf)
- [BWPEP on Conway's Game of Life](http://shodor.org/petascale/materials/UPModules/exercises/Game_of_Life/)
- [A Performance Analysis of GoL](https://arxiv.org/pdf/1209.4408.pdf)
- [What is a Dwarf in HPC?](https://www5.in.tum.de/lehre/vorlesungen/hpc/WS15/structured.pdf)

<!-- TODO: Specify the input file format -->

## Useful information

### ICC compilation

icc -DGoL_DEBUG gol.c -o GoL_deb
icc gol.c -o GoL

-qopenmp, enables OpenMP support

### Number of available threads

- **Number of threads per PID**: ps -o nlwp <pid>
- **Number of total threads per node**: ps -eo nlwp | tail -n +2 | awk '{ num_threads/- += $1 } END { print num_threads }'
- **Info on the architecture**: lscpu

256 processors, 64 cores per processor and 4 threads per core.

### Compiler optimization

icc -O{i} -ipo -fast -g -opt-report -xHost -sse{k}  
i = {0, 1, 2, 3}  
k = {1, 2, 3}

-qopt-report={0, ..., 5}  
-qopt-report-phase=vec

-g, creates symbols for debugging.

-ipo, slows compilation down in exchange of an appreciable boost in performance.

### FIXMEs

### TODOs

- Create a Makefile to run.

- Add a function to dump the grid to file sticking to the input format.
- Add MPI or CUDA support to log filenames.
  
- Enable MPI support and account for ghost rows.

- Add write_grid() to printbig().

- [How to measure elapsed wall-clock time?](https://stackoverflow.com/questions/12392278/measure-time-in-linux-time-vs-clock-vs-getrusage-vs-clock-gettime-vs-gettimeof)

- [GoL implementation with MPI](https://github.com/freetonik/MPI-life)

- [No global array](https://stackoverflow.com/questions/9269399/sending-blocks-of-2d-array-in-c-using-mpi)

- [Slice matrix with scatter](https://stackoverflow.com/questions/33507024/mpi-scatter-and-gather-for-2d-matrix-in-mpi-using-c)
#### MPI

Create new data structure <code>chunk_t</code>, with:  
<code>

    int num_rows;
    int num_cols;

    int rank;
    int size;

    #ifdef _OPENMP
    int num_threads;
    #endif

    unsigned int **chunk;
    unisgned int **next_chunk;
    
</code>  

-DGoL_MPI ---> #ifdef GoL_MPI  

1. Master parses arguments and creates life_t structure
2. Master gets number of processes and comm size from MPI
3. Master computes the correct slices for each process
4. Master sends slice to corresponding slave
5. Slaves wait for their slice to come
6. Slaves allocate memory for their slice
7. For the number of generations:
    - Slaves share ghost rows with adjacent slaves
    - Slaves evolve their slice
    - If the grid is small:
        - Slaves send their updated slice back to the master
        - Master prints the updated grid to console
8. Slaves send their slice back to the master
9. Master prints the final grid to console/file
