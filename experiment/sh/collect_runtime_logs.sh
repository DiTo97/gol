#!/bin/bash
# Collect cumulative evolution and total execution times of Game of Life (GoL)'s binary for different ICC optimization levels.
#
# Please note - A: Execute this file with its folder as the CWD.
# Please note - B: ICC experiments terminate at 10000 size due to time constraints.
# Please note - C: GoL boards are assumed to be square, unless otherwise stated.
# Please note - D: GoL_LOG's guard is required in compilation to enable logging functions.
#

BOARD_DIMENS=(100 500 1000 5000 10000)
OPT_LEVELS=(0 1 2 3)

# I/O variables
binname="GoL"

srcfile="../../src/cpu/gol.c"

binfile="../../bin/${binname}"
outfile="${binname}.out"

# Metadata variables
init_prob=0.5
seed=1
tsteps=100
reps=1

for (( rep=0; rep<$reps; ++rep ));
do
    for opt in "${OPT_LEVELS[@]}"
    do
        qopt_file="GoL_report_${opt}"

        if [ $opt -gt 1 ]
        then # Use xHost only with an opt level of at least -O2
            for xhost in {0..1}
            do
                qopt_file="GoL_report_${opt}_${xhost}"

                if [ $xhost -eq 1 ]
                then
                    icc -O$opt -ipo -g -DGoL_LOG -qopt-report=2 -qopt-report-file=$qopt_file -qopt-report-phase=vec -xHost $srcfile -o $binfile
                else
                    icc -O$opt -ipo -g -DGoL_LOG -qopt-report=2 -qopt-report-file=$qopt_file -qopt-report-phase=vec $srcfile -o $binfile
                fi

                for size in "${BOARD_DIMENS[@]}"
                do
                    nrows=$size
                    ncols=$size

                    if [ $size -gt 10000 ]
                    then
                        printf "[*] A too large board may lead to many hours of serial computation...\n"
                    fi

                    ./$binfile $ncols $nrows $tsteps $outfile $seed $init_prob
                done
            done
        else
            icc -O$opt -ipo -g -DGoL_LOG -qopt-report=2 -qopt-report-file=$qopt_file -qopt-report-phase=vec $srcfile -o $binfile

            for size in "${BOARD_DIMENS[@]}"
            do
                nrows=$size
                ncols=$size

                if [ $size -gt 10000 ]
                then
                    printf "[*] A too large board may lead to many hours of serial computation...\n"
                fi

                ./$binfile $ncols $nrows $tsteps $outfile $seed $init_prob
            done
        fi
    done
done