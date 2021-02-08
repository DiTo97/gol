#!/bin/bash
# Test compilation and execution times for different ICC optimization levels.

# This file should be executed with folder experiments as the current working directory

BOARD_DIMENS=(100 500 1000 5000 10000 20000 40000)

BEST_OPT_LEVEL=2

THREAD_NUM=(2 4 8 16 32 64 128 256)

# I/O variables
srcfile="../../gol.c"
binfile="../../bin/Gol_omp"
outfile="Gol​​_omp.out"

# Metadata variables
init_prob=0.5
seed=1
tsteps=100
reps=2

# Making a compilation before running the experiments
icc -O$BEST_OPT_LEVEL -D GoL_DEBUG -qopenmp -ipo -xHost $srcfile -o $binfile

for (( rep=0; rep<$reps; ++rep));
do
    for i in "${BOARD_DIMENS[@]}"
    do
        # For starters, we assume square GoL boards
        nrows=$i
        ncols=$i

        for t_num in "${THREAD_NUM[@]}"
        do
            ./$binfile $ncols $nrows $tsteps $outfile $seed $init_prob $t_num
        done
    done
done