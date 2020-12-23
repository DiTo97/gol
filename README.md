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

<!-- TODO: Specify the input format -->
<!-- TODO: Remove ghost rows -->
<!-- TODO: Add OpenMP parallelization via pragmas -->

<!-- Compiler optimization -->
icc -O{i} -ipo -fast -g -opt-report -xHost -sse{k}

i = {0, 1, 2, 3} 
k = {1, 2, 3}

-qopt-report={0, ..., 5}
-qopt-report-phase=vec

-g, creates symbols for debugging

-ipo, slows compilation down in exchange of an appreciable boost in performance.

<!-- Parallelization via OpenMP -->

- Parallelize cells' update evaluation
- Parallelize cells' update value
- Parallelize board inner mallocs
- Parallelize board initialization to DEAD cells
- Parallelize board random initialization
- Parallelize board dump to file
- Parallelize board free memory