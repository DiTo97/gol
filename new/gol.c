/*
 * Consway's Game of Life.
 *
 * Serial implementation in C inspired by
 * https://www.geeksforgeeks.org/conways-game-life-python-implementation/
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "gol.h"
#include "utils.h"

int main(int argc, char **argv) {
    struct life_t life;

    // 1. Initialize vars from args
    parse_args(&life, argc, argv);

    // 2. Launch the simulation
    game(&life);
}
