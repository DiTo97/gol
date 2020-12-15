#ifndef GLOBAL_H
#define GLOBAL_H 

const float DEFAULT_VIS_INTERVAL = 0.0005;
const int DEFAULT_TIMESTEPS = 1000;

const int DEFAULT_SIZE_ROWS = 30;
const int DEFAULT_SIZE_COLS = 30;

const float DEFAULT_INIT_PROB = 0.5;

const char DEFAULT_OUT_FILE[] = "gen_output.txt";

// Threshold on grid size, above which the grid's evolution
// is not displayed at every timestep.
const int DEFAULT_MAX_SIZE = DEFAULT_SIZE_ROWS * DEFAULT_SIZE_COLS;

const unsigned int DEFAULT_SEED = 1;

// Grid cell's state
enum {
    DEAD  = 0,
    ALIVE = 1
};

#endif