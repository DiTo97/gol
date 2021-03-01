#ifndef GoL_LOG_H
#define GoL_LOG_H

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef GoL_CUDA
#include <sys/time.h> // Enable struct timeval
#endif

#include <time.h>

// Custom includes
#include "../globals.h"
#include "../life/life.h"

/**
 * Initialize a tab-separated log file, whose name varies with GoL configuration's settings. Each row will have a fixed a priori set of (timesteps, cum_gene_time, tot_prog_time) columns, where:
 * 
 * - timesteps        The # of elapsed generations
 * - cum_gene_time    The total time devolved to GoL evolution
 * - tot_prog_time    The total runtime of the program
 * 
 * @param nprocs      The # of running processes | 1
 * 
 * @return log_ptr    The pointer to the tab-separated log file
 */
FILE* init_log_file(life_t life, int nprocs) {
    char *logs_dir = (char*) DEFAULT_LOGS_DIR;

    char buffer[100];

    // GoL configuration's tags
    char __omp[7];   // 7  := "omp"  + 3-digit nthreads + "_"
    char __mpi[8];   // 8  := "mpi"  + 4-digit nprocs   + "_"
    char __cuda[15]; // 15 := "cuda" + 5-digit nblocks  + "_" + 4-digit block_size + "_"

    #ifdef _OPENMP
    sprintf(__omp, "omp%d_", life.nthreads);
    #else
    sprintf(__omp, "");
    #endif

    #ifdef GoL_MPI
    sprintf(__mpi, "mpi%d_", nprocs);
    #else
    sprintf(__mpi, "");
    #endif

    #ifdef GoL_CUDA
    sprintf(__cuda, "cuda%d_", life.block_size);
    #else
    sprintf(__cuda, "");
    #endif

    struct stat st = {0};

    // Create the logs dir if it doesn't exist​​​​
    if (stat(logs_dir, &st) == -1) {
        mkdir(logs_dir, 0744); // TODO: Handle mkdir failure
    }

    if (life.infile != NULL)
        sprintf(buffer, "%s/GoL_%s%s%snc%d_nr%d_nt%d_%lu.log",
                logs_dir, __mpi, __omp, __cuda, 
                life.ncols, life.nrows, life.timesteps,
                (unsigned long) time(NULL));
    else
        sprintf(buffer, "%s/GoL_%s%s%snc%d_nr%d_nt%d_prob%.1f_seed%d_%lu.log",
                logs_dir, __mpi, __omp, __cuda,
                life.ncols, life.nrows, life.timesteps, life.init_prob,
                life.seed, (unsigned long) time(NULL));

    FILE *log_ptr = fopen(buffer, "a");
    fprintf(log_ptr, "timesteps\tcum_gene_time\ttot_prog_time\n");

    // The log file's name is guaranteed to be unique until year 2038,
    // as it implies the call to time(NULL).
    return log_ptr;    
}

/**
 * Log a (timesteps, cum_gene_time, tot_prog_time) triplet onto the log file.
 * 
 * @param timesteps        The # of elapsed generations
 * @param cum_gene_time    The total time devolved to GoL evolution
 * @param tot_prog_time    The total runtime of the program
 */
void log_data(FILE *log_ptr, int timesteps, double cum_gene_time, double tot_prog_time) {
    fprintf(log_ptr, "%-9d\t%-13.3f\t%-13.3f\n", timesteps, cum_gene_time, tot_prog_time); // -13, as columns are 13-char long
}

#endif
