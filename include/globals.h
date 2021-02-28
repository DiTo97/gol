#ifndef GoL_GLOBALS_H
#define GoL_GLOBALS_H 

#include <stdbool.h>

const int DEFAULT_TIMESTEPS = 100;

const int DEFAULT_SIZE_ROWS = 50;
const int DEFAULT_SIZE_COLS = 50;

const float DEFAULT_INIT_PROB = 0.5;

#if (defined _OPENMP) && (defined GoL_MPI)
const char *DEFAULT_LOGS_DIR = "logs_hybrid";
const char *DEFAULT_OUT_FILE = "GoL_hybrid.out";
#elif defined(_OPENMP)
const char *DEFAULT_LOGS_DIR = "logs_omp";
const char *DEFAULT_OUT_FILE = "GoL_omp.out";
#elif defined(GoL_MPI)
const char *DEFAULT_LOGS_DIR = "logs_mpi";
const char *DEFAULT_OUT_FILE = "GoL_mpi.out";
#elif defined(GoL_CUDA)
const char *DEFAULT_LOGS_DIR = "logs_cuda";
const char *DEFAULT_OUT_FILE = "GoL_cuda.out";
#else
const char *DEFAULT_LOGS_DIR = "logs";
const char *DEFAULT_OUT_FILE = "GoL.out";
#endif

// Threshold on grid size, above which the grid's evolution
// is not displayed at every timestep.
const int DEFAULT_MAX_SIZE = DEFAULT_SIZE_ROWS * DEFAULT_SIZE_COLS;

const unsigned int DEFAULT_SEED = 1;

#ifdef _OPENMP
const int DEFAULT_NUM_THREADS = 4;
const int DEFAULT_MAX_THREADS = 256; // 4 threads x 64 cores per processor
#endif

#ifdef GoL_CUDA
// Consistent with CUDA 7.5 specifications at
// https://en.wikipedia.org/wiki/Thread_block_(CUDA_programming)
const int DEFAULT_BLOCK_SIZE     = 128;  // Number of threads per block
const int DEFAULT_MAX_BLOCK_SIZE = 1024;
#endif

#ifdef GoL_MPI
// Message passing tags
enum Tags {
    TOP,    // Send/receive data to/from the top MPI neighbour process
    BOTTOM, // Send/receive data to/from the bottom MPI neighbour process
    PRINT   // Send/receive data to print it to file/console
};
#endif

// Grid cells' states := 1 byte each
enum States {
    DEAD  = false,
    ALIVE = true
};

#endif
