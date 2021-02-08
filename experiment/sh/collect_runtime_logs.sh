#!/bin/bash
# Test compilation and execution times for different ICC optimization levels.

# This file should be executed with folder experiments as the current working directory

BOARD_DIMENS=(100 500 1000 5000 10000)

OPT_LEVELS=(2)

# I/O variables
srcfile="../../gol.c"
binfile="../../bin/Gol"
outfile="Gol.out"

# Metadata variables
init_prob=0.5
seed=1
tsteps=100
reps=3

for (( rep=0; rep<$reps; ++rep));
do
    for j in "${OPT_LEVELS[@]}"
    do
        qopt_file="GoL_report_${j}"
        if [ $j -gt 1 ]
        then # Use xHost with at least -O2 optimization level
            for xhost in {0..1}
            do
                qopt_file="GoL_report_${j}_${xhost}"

                if [ $xhost -eq 1 ]
                then
                    icc -O$j -ipo -g -D GoL_DEBUG -qopt-report=2 -qopt-report-file=$qopt_file -qopt-report-phase=vec -xHost $srcfile -o $binfile
                else
                    icc -O$j -ipo -g -D GoL_DEBUG -qopt-report=2 -qopt-report-file=$qopt_file -qopt-report-phase=vec $srcfile -o $binfile
                fi
            done
        else
            icc -O$j -ipo -g -D GoL_DEBUG -qopt-report=2 -qopt-report-file=$qopt_file -qopt-report-phase=vec $srcfile -o $binfile
        fi

        for i in "${BOARD_DIMENS[@]}"
        do
            # For starters, we assume square GoL boards
            nrows=$i
            ncols=$i

            ./$binfile $ncols $nrows $tsteps $outfile $seed $init_prob
        done
    done
done