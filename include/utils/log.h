#ifndef GoL_LOG_H
#define GoL_LOG_H

/**
 * Initialize a tab-separated log file, whose name varies with GoL configuration's settings.
 * 
 * @return log_ptr    The pointer to the tab-separated log file with (timestep, current time, total time) columns
 */
FILE* init_log_file(struct life_t life, int nprocs) {
    char logs_dir[100];

    #if (defined _OPENMP) && (defined GoL_MPI)
    sprintf(logs_dir, "%s_%s", (char*) DEFAULT_LOGS_DIR, "hybrid");
    #elif (defined _OPENMP)
    sprintf(logs_dir, "%s_%s", (char*) DEFAULT_LOGS_DIR, "omp");
    #elif (defined GoL_MPI)
    sprintf(logs_dir, "%s_%s", (char*) DEFAULT_LOGS_DIR, "mpi");
    #else
    sprintf(logs_dir, "%s", (char*) DEFAULT_LOGS_DIR);
    #endif
    
    char buffer[100];
    char __omp[7]; // 7 := "omp" + 3-digit nthreads + "_"
    char __mpi[8]; // 8 := "mpi" + 4-digit nprocs   + "_"

    struct stat st = {0};

    if (stat(logs_dir, &st) == -1) { // Create the logs dir if it doesn't exist​​​​
        mkdir(logs_dir, 0744);
    }
    
    #ifdef _OPENMP
    sprintf(__omp, "omp%d_", life.num_threads);
    #else
    sprintf(__omp, "");
    #endif

    #ifdef GoL_MPI
    sprintf(__mpi, "mpi%d_", nprocs);
    #else
    sprintf(__mpi, "");
    #endif

    if (life.input_file != NULL)
        sprintf(buffer, "%s/GoL_%s%snc%d_nr%d_nt%d_%lu.log", logs_dir, __mpi, __omp,
                life.num_cols, life.num_rows, life.timesteps,
                (unsigned long) time(NULL));
    else
        sprintf(buffer, "%s/GoL_%s%snc%d_nr%d_nt%d_prob%.1f_seed%d_%lu.log", logs_dir, __mpi, __omp,
                life.num_cols, life.num_rows, life.timesteps, life.init_prob,
                life.seed, (unsigned long) time(NULL));

    FILE *log_ptr = fopen(buffer, "a");
    fprintf(log_ptr, "timesteps\tcum_gene_time\ttot_prog_time\n");

    // The log file's name is guaranteed to be unique until year 2038,
    // as it implies the call to time(NULL).
    return log_ptr;
}

/**
 * Log a (timestep, current time, total time) triplet onto the log file.
 * 
 * @param timestep    The current generation of GoL's evolution.
 * @param cur_time    The execution time of the current generation of GoL.
 * @param tot_time    The total execution time since GoL's evolution started.
 */
void log_data(FILE *log_ptr, int timesteps, double cum_gene_time, double tot_prog_time) {
    fprintf(log_ptr, "%-9d\t%-13.3f\t%-13.3f\n", timesteps, cum_gene_time, tot_prog_time); // -9, as columns are 9-char long
}

#endif
