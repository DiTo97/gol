#!/bin/bash
# Test compilation and execution times for different ICC optimization levels.

# This file should be executed with folder experiments as the current working directory

BOARD_DIMENS=(100 500 1000 5000 10000 20000 40000 100000)

BEST_OPT_LEVEL=2
NODE_NUM=$1

if [ $NODE_NUM -eq 1 ]
then
    PROCESS_NUM=(2 4 8 16 32 64 128 256)
elif [ $NODE_NUM -eq 2 ]
then
    PROCESS_NUM=(2 4 8 16 32 64 128 256 512)
else
    PROCESS_NUM=(2 4 8 16 32 64 128 256 512 1024)
fi

THREAD_NUM=(2 4 8 16 32 64 128 256)

# I/O variables
srcfile="../../../gol.c"
nodesfile="mpi_nodes.txt"
binfile="../../../bin/Gol_hybrid_${NODE_NUM}"
outfile="Gol_hybrid_${NODE_NUM}​​.out"

# Metadata variables
init_prob=0.5
seed=1
tsteps=100
reps=1


# Making a compilation before running the experiments
mpiicc -O$BEST_OPT_LEVEL -D GoL_MPI -D GoL_DEBUG -qopenmp -ipo -xHost $srcfile -o $binfile

for (( rep=0; rep<$reps; ++rep));
do
    for i in "${BOARD_DIMENS[@]}"
    do
        # For starters, we assume square GoL boards
        ncols=$i

        if [ $i -eq 100000 ]
        then
            nrows=1000
        else
            nrows=$i
        fi

        for p_num in "${PROCESS_NUM[@]}"
        do
            proc_per_host_div=$((p_num/NODE_NUM))
            proc_per_host_val=0

            if [ $proc_per_host -eq 0 ]
            then
                proc_per_host_val=1
            else
                proc_per_host_val=$proc_per_host_div
            fi

            for thread_num in "${THREAD_NUM[@]}"
            do
                max_threads_per_node=256
                if [ $((thread_num * p_num)) -le $((max_threads_per_node * NODE_NUM)) ]
                then
                    mpiexec -hostfile $nodesfile -perhost $proc_per_host_val -np $p_num ./$binfile $ncols $nrows $tsteps $outfile $seed $init_prob $thread_num
                fi
            done
        done
    done
done