#!/bin/bash
# Collect cumulative evolution and total execution times of Game of Life (GoL)'s binary for a variable \# of CUDA threads per block.
#
# Please note - A: Execute this file with its folder as the CWD.
# Please note - B: CUDA experiments can run a grand total of 1024 threads per block due to hardware constraints, but a minimum block size
# of 32 threads per block is suggested, as that coincides with the warp size in CUDA 7.5
# Please note - C: GoL boards are assumed to be square, unless otherwise stated.
# Please note - D: GoL_LOG's guard is required in compilation to enable logging functions.
#

BOARD_DIMENS=(100 500 1000 5000 10000 20000 40000)
BLOCK_SIZES=(32 64 128 256 512 1024)

max_block_size=1024
warp_size=32

# I/O variables
binname="GoL_cuda"

srcfile="../../src/gpu/gol.cu"

binfile="../../bin/${binname}"
outfile="${binname}.out"

# Metadata variables
init_prob=0.5
seed=1
tsteps=100
reps=1

# Compile a GoL binary right before running the experiments
nvcc -DGoL_CUDA -DGoL_LOG $srcfile -o $binfile

for (( rep=0; rep<$reps; ++rep )); do
    for size in "${BOARD_DIMENS[@]}"
    do
        nrows=$size
        ncols=$size

        for b_size in "${BLOCK_SIZES[@]}"
        do
            if [ $b_size -gt $max_block_size ] # Ensure hardware constraints have not been exceeded
            then
                continue
            elif [ $b_size -lt $warp_size ]
            then
                printf "[*] A block size smaller than the warp size may lead to a highly inefficient computation...\n"
            fi

            ./$binfile $ncols $nrows $tsteps $outfile $seed $init_prob $b_size
        done
    done
done