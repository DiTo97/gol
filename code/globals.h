#ifndef GoL_GLOBALS_H
#define GoL_GLOBALS_H 

const int DEFAULT_TIMESTEPS = 100;

const int DEFAULT_SIZE_ROWS = 50;
const int DEFAULT_SIZE_COLS = 50;

const float DEFAULT_INIT_PROB = 0.33;

const char *DEFAULT_OUT_FILE = "GoL.out";
const char *DEFAULT_LOGS_DIR = "logs";

// Threshold on grid size, above which the grid's evolution
// is not displayed at every timestep.
const int DEFAULT_MAX_SIZE = DEFAULT_SIZE_ROWS * DEFAULT_SIZE_COLS;

const unsigned int DEFAULT_SEED = 1;

#ifdef _OPENMP
const int DEFAULT_NUM_THREADS = 4;
const int DEFAULT_MAX_THREADS = 256; // 4 threads x 64 cores per processor
#endif

// Grid cell's state
enum {
    DEAD  = 0,
    ALIVE = 1
};

enum TAGS {
    TOP,
    BOTTOM,
    PRINT
};

#endif
