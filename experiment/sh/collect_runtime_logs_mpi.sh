#!/bin/bash
# Collect cumulative evolution and total execution times of Game of Life (GoL)'s binary for a variable \# of MPI processes
# given a \# of physical nodes.
#
# Please note - A: Execute this file with its folder as the CWD.
# Please note - B: MPI experiments can run a grand total of 256 processes per node due to hardware constraints (64 cores x 4 threads),
# but cap at a maximum of 1024 in order not to overly stress the load on the underlying INFN's architecture.
# Please note - C: GoL boards are assumed to be square, unless otherwise stated.
# Please note - D: GoL_LOG's guard is required in compilation to enable logging functions.
# Please note - E: Edit the list of endpoints from the nodesfile to limit the # of available nodes.
#

BOARD_DIMENS=(100 500 1000 5000 10000 20000 40000 100000)
PROCESSES=(2 4 8 16 32 64 128 256 512 1024)

best_opt_level=2
num_nodes=$1 # Pass it via CLI arguments
logical_cores=256

# I/O variables
binname="GoL_mpi_${num_nodes}"

srcfile="../../src/cpu/gol.c"
nodesfile="nodeslist.txt"

binfile="../../bin/${binname}"
outfile="${binname}.out"

# Metadata variables
init_prob=0.5
seed=1
tsteps=100
reps=1

# Compile a GoL binary right before running the experiments
mpiicc -DGoL_MPI -O$best_opt_level -DGoL_LOG -ipo -xHost $srcfile -o $binfile

for (( rep=0; rep<$reps; ++rep ));
do
    for size in "${BOARD_DIMENS[@]}"
    do
        ncols=$size

        if [ $i -eq 100000 ] # A size of 100000 was chosen to test the MPI implementation with a non-square matrix.
        then                 # Indeed since MPI processes split the data in groups of rows, this experiment
            nrows=1000       # would compare their behaviour with fewer rows and more columns
        else
            nrows=$size
        fi

        for p_num in "${PROCESSES[@]}"
        do
            if [ $p_num -gt $((logical_cores * num_nodes)) ] # Ensure hardware constraints have not been exceeded
            then
                continue
            fi

            p_per_host=$((p_num / num_nodes))

            if [ $p_per_host -lt 1 ] # Ensure each of the requested nodes has at least 1 process to run
            then
                continue
            fi

            mpiexec -hostfile $nodesfile -perhost $p_per_host -np $p_num ./$binfile $ncols $nrows $tsteps $outfile $seed $init_prob
        done
    done
done