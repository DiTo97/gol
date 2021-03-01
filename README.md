# Game of Life

Comparative analysis of possible parallel implementations of Conway's famous [Game of Life](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life) (GoL) using both GPU-based toolkits, CUDA, and CPU-based toolkits, OpenMP and MPI, on INFN's [Ocapie cluster](https://web.ge.infn.it/calcolo/joomla/2-uncategorised/106-farm-hpc-ocapie) for HPC.

**Authors**: F. Minutoli, M. Ghirardelli, and D. Surpanu.

## Useful links

- [Parallel Programming Illustrated through Conway's Game of Life](https://tcpp.cs.gsu.edu/curriculum/?q=system/files/ch10.pdf)
- [Parallelization: Conway's Game of Life](http://www.shodor.org/media/content/petascale/materials/UPModules/GameOfLife/Life_Module_Document_pdf.pdf)
- [BWPEP on Conway's Game of Life](http://shodor.org/petascale/materials/UPModules/exercises/Game_of_Life/)
- [A Performance Analysis of GoL](https://arxiv.org/pdf/1209.4408.pdf)
- [What is a Dwarf in HPC?](https://www5.in.tum.de/lehre/vorlesungen/hpc/WS15/structured.pdf)

## Useful information

### Defaults

- The borderline size to distinguish a small GoL's grid from a big one has been updated to *50*x*50*.

 ### Display file format

 Both the input and output file format comply with the full-matrix format (FM), that is, 

- in case the (0, 0) cellBefore reading the GoL matrix from an input file,  replace the first character of the first row with the 'A' character to
  avoid the skipping behavior of getline. Do this if the first character is a blank space (a DEAD cell).

### Folder structure

### Sample usage