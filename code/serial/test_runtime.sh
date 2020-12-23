#!/bin/bash

BOARD_DIMENS=(30 100 300 1000 3000)

OPT_LEVELS=(0 1 2 3)

disp_freq=0.5
init_prob=0.5
outfile="out.txt"
seed=1
tsteps=1000

for i in "${BOARD_DIMENS[@]}"
do
    # For starters, we assume square GoL boards
    nrows=$i
    ncols=$i

    for j in "${OPT_LEVELS[@]}"
    do
        qopt_file="report_${i}_${j}"
        if [ $j -gt 1 ]
        then # Use xHost with at least -O2 optimization level
            for xhost in {0..1}
            do
                qopt_file="report_${i}_${j}_${xhost}"

                if [ $xhost -eq 1]
                then
                    icc -O$j -ipo -g -qopt-report=2 -qopt-report-file=$qopt_file -qopt-report-phase=vec -xHost gol.c -o gol
                else
                    icc -O$j -ipo -g -qopt-report=2 -qopt-report-file=$qopt_file -qopt-report-phase=vec gol.c -o gol

                ./gol $ncols $nrows $tsteps $outfile $disp_freq $seed $init_prob
            done
        else
            icc -O$j -ipo -g -qopt-report=2 -qopt-report-file=$qopt_file -qopt-report-phase=vec gol.c -o gol
            ./gol $ncols $nrows $tsteps $outfile $disp_freq $seed $init_prob
        fi
    done
done