#!/bin/bash
# This file compiles a CUDA GoL executable and executes experiments.
# This file should be executed with folder experiments as the current working directory
BOARD_DIMENS=(100 500 1000 5000 10000 20000 40000)
BLOCK_SIZES=(32 64 128 256 512 1024)
reps=1

# Remove log folder contents before running CUDA experiments
# rm -rf logs


# I/O variables
srcfile="../gol_cuda.cu"
binfile="../bin/GoL_cuda"
outfile="${binfile}.out"



# Metadata variables
init_prob=0.5
seed=1
tsteps=100

nvcc -D GoL_DEBUG $srcfile -o $binfile

for (( rep=0; rep<$reps; ++rep)); do
    for b_dim in "${BOARD_DIMENS[@]}"
    do
        nrows=$b_dim
        ncols=$b_dim

        for b_size in "${BLOCK_SIZES[@]}"
        do
            ./$binfile $ncols $nrows $tsteps $outfile $seed $init_prob $b_size
        done
    done
done