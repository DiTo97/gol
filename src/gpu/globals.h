#ifndef GoL_GLOBALS_H
#define GoL_GLOBALS_H 

#include <stdbool.h>

const int DEFAULT_TIMESTEPS = 100;

const int DEFAULT_SIZE_ROWS = 50;
const int DEFAULT_SIZE_COLS = 50;

const float DEFAULT_INIT_PROB = 0.33;

const char *DEFAULT_OUT_FILE = "GoL_CUDA.out";
const char *DEFAULT_LOGS_DIR = "logs";

// Threshold on grid size, above which the grid's evolution
// is not displayed at every timestep.
const int DEFAULT_MAX_SIZE = DEFAULT_SIZE_ROWS * DEFAULT_SIZE_COLS;

const unsigned int DEFAULT_SEED = 1; // to reproduce the same experiment

// as per CUDA specifications at https://en.wikipedia.org/wiki/Thread_block_(CUDA_programming)#:~:text='%20The%20maximum%20x%2C%20y%20and,65%2C535%20blocks%20in%20each%20dimension.
const int DEFAULT_NUM_THREADS_PER_BLOCK = 128; // default number of threads per block
const int DEFAULT_MAX_NUM_THREADS_PER_BLOCK = 1024; // as per our cluster specifications, max number of threads per block

// Grid cell's state
enum {
    DEAD  = false,
    ALIVE = true
};
#endif