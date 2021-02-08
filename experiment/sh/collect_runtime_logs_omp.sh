#!/bin/bash
# Collect cumulative evolution and total execution times of Game of Life (GoL)'s binary for a variable \# of OpenMP threads.
#
# Please note - A: Execute this file with its folder as the CWD.
# Please note - B: OpenMP experiments can run a grand total of 256 threads due to hardware constraints (64 cores x 4 threads).
# Please note - C: GoL boards are assumed to be square, unless otherwise stated.
# Please note - D: GoL_LOG's guard is required in compilation to enable logging functions.
#

BOARD_DIMENS=(100 500 1000 5000 10000 20000 40000)
THREADS=(2 4 8 16 32 64 128 256)

best_opt_level=2
logical_cores=256

# I/O variables
binname="GoL_omp"

srcfile="../../src/cpu/gol.c"

binfile="../../bin/${binname}"
outfile="${binname}.out"

# Metadata variables
init_prob=0.5
seed=1
tsteps=100
reps=1

# Compile a GoL binary right before running the experiments
icc -O$best_opt_level -DGoL_LOG -qopenmp -ipo -xHost $srcfile -o $binfile

for (( rep=0; rep<$reps; ++rep ));
do
    for size in "${BOARD_DIMENS[@]}"
    do
        nrows=$size
        ncols=$size

        for t_num in "${THREADS[@]}"
        do
            if [ $t_num -gt $logical_cores ] # Ensure hardware constraints have not been exceeded
            then
                continue
            fi

            ./$binfile $ncols $nrows $tsteps $outfile $seed $init_prob $t_num
        done
    done
done