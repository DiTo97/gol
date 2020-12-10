/*
 * Consway's Game of Life.
 *
 * Serial implementation in C inspired from
 * https://www.geeksforgeeks.org/conways-game-life-python-implementation/
 * 
 * This code should be taken solely as a baseline for the actual parallel
 * implementations either in OpenMP, MPI or CUDA.
 */

// TODO: Unify show() and printbig() under a single function via is_big().
// TODO: Adjust the grid evolution to use the life_t struct.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "gol.h"
#include "utils.h"
 
void show(void *u, int w, int h) {
    int x,y;
    int (*univ)[w] = u;

    printf("\033[H");

    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) printf(univ[y][x] ? "\033[07m  \033[m" : "  ");
        printf("\033[E");
    }

    fflush(stdout);
    usleep(200000);
}

void printbig(void *u, int w, int h, int z) {
    int x,y;
    int (*univ)[w] = u;
    
    FILE *f;
    
    if(z == 0) f = fopen("glife.txt", "w" );
    else f = fopen("glife.txt", "a" );
    
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) fprintf (f,"%c", univ[y][x] ? 'x' : ' ');
        fprintf(f,"\n");
    }

    fprintf(f,"\n\n\n\n\n\n ******************************************************************************************** \n\n\n\n\n\n");

    fflush(f);
    fclose(f);
}

void initialize(struct life_t *life) {
    // 1. Check if an input file was specified in the args
    // and, in that case, update num_cols and num_rows.
    // Use defaults, if no file is present
    FILE *input_ptr = set_grid_dimens_from_file(life);

    // 2. Allocate memory for the grid
    malloc_grid(life);

    // 3. Initialize the grid with DEAD cells
    init_empty_grid(life);

    // 4. Initialize the grid with ALIVE cells...
    if (input_ptr != NULL) { // ...from file, if present...
        init_from_file(life, input_ptr);
    } else { // ...or randomly, otherwise.
        init_random(life)
    }
}

void evolve(void *u, int w, int h) {
    unsigned (*univ)[w] = u;
    unsigned new[h][w];

    int x,y,x1,y1;
 
    for (y = 0; y < h; y++) for (x = 0; x < w; x++) {
        int n = 0;
        for (y1 = y - 1; y1 <= y + 1; y1++)
            for (x1 = x - 1; x1 <= x + 1; x1++)
                if (univ[(y1 + h) % h][(x1 + w) % w]) n++;
        if (univ[y][x]) n--;
        new[y][x] = (n == 3 || (n == 2 && univ[y][x]));
        /*
         * How do cells evolve in GoL?
         *   - A cell is born, if it has exactly three neighbours;
         *   - A cell dies of loneliness, if it has less than two neighbours;
         *   - A cell dies of overcrowding, if it has more than three neighbours;
         *   - A cell survives to the next generation, if it does not die of loneliness or overcrowding.
         */
    }

    for (y = 0; y < h; y++) for (x = 0; x < w; x++) univ[y][x] = new[y][x];
}
 
void game(struct life_t *life) {
    int x, y;

    struct timeval start, end;
    
    initialize(life);
    
    if (is_big(life))
        printbig(life->grid, ncols, nrows, 0)
    else
        show(life->grid, ncols, nrows)
    
    // for(z = 0; z < t;z++) {
    // 	if (x <= 1000) show(univ, w, h);
    // 	else gettimeofday(&start, NULL);
        
    // 	evolve(univ, w, h);

    // 	if (x > 1000) {
    // 		gettimeofday(&end, NULL);
    // 	    printf("Iteration %d is : %ld ms\n", z,
    // 	       ((end.tv_sec * 1000000 + end.tv_usec) - 
    // 	       (start.tv_sec * 1000000 + start.tv_usec))/1000 );
    // 	}
    // }

    // if (x > 1000) printbig(univ, w, h,1);
}
 
int main(int argc, char **argv) {
    struct life_t life;

    // 1. Initialize the random seed
    srand(time(NULL))

    // 2. Initialize vars from args
    parse_args(&life, argc, argv);

    // 3. Launch the simulation
    game(&life);
}
