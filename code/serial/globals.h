#ifndef GLOBAL_H
#define GLOBAL_H 

const int DEFAULT_VIS_INTERVAL = 60;
const int DEFAULT_TIMESTEPS = 1000;

const int DEFAULT_SIZE_ROWS = 100;
const int DEFAULT_SIZE_COLS = 100;

const int DEFAULT_INIT_PROB = 0.5;

// Threshold on grid size, above which the grid's evolution
// is not displayed at every timestep.
const int DEFAULT_MAX_SIZE  = DEFAULT_SIZE_ROWS * DEFAULT_SIZE_COLUMNS;

// Grid cell's state
enum {
    DEAD  = 0,
    ALIVE = 1
};

#endif