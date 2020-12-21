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

#include "gol.h"
#include "utils.h"

void debug(struct life_t life) {
    printf("Visualization interval: %d\n", life.vis_interval);
    printf("Number of cols: %d\n", life.num_cols);
    printf("Number of rows: %d\n", life.num_rows);
    printf("Number of timesteps: %d\n", life.timesteps);
    printf("Initialization probability: %f\n", life.init_prob);
    printf("Initialization seed: %d\n", life.seed);
    printf("Input file: %s\n", life.input_file == NULL ? "Ciao" : life.input_file);
    printf("Output file: %s\n", life.output_file == NULL ? "Ciao" : life.output_file);
}
 
/**
 * Function that shows the grid on the console
 */
void show(struct life_t * life) {
    int x,y;

    int n_cols = life->num_cols;
    int n_rows = life->num_rows;

    printf("\033[H");

    for (y = 0; y < n_cols; y++) {
        for (x = 0; x < n_rows; x++) {
            printf(life->grid[y][x] ? "\033[07m  \033[m" : "  ");
        }
        printf("\033[E");
    }

    fflush(stdout);
    usleep(200000);
}

/**
 * Function that saves the grid on file
 */
void printbig(struct life_t * life, bool append) {
    int x,y;
    
    int n_cols = life->num_cols;
    int n_rows = life->num_rows;

    FILE *f;
    
    if(append) f = fopen("glife.txt", "a" );
    else f = fopen("glife.txt", "w" );
    
    for (y = 0; y < n_cols; y++) {
        for (x = 0; x < n_rows; x++) 
            fprintf (f,"%c", life->grid[y][x] ? 'x' : ' ');
        fprintf(f,"\n");
    }

    fprintf(f,"\n\n\n\n\n\n ******************************************************************************************** \n\n\n\n\n\n");

    fflush(f);
    fclose(f);
}

/**
 * Function that show the grid on console or save it in a file
 * according to the result of the is_big(...) function
 */
void display(struct life_t * life, bool append){

    if(is_big(life)){
        printbig(life, append);
    } else{
        show(life);
        printbig(life, append);
    }
}

/**
 * Function that initialize all the required variables and structures
 */
void initialize(struct life_t *life) {
    debug(*life);
    // 1. Initialize the random seed
    srand(life->seed);

    // 2. Check if an input file was specified in the args
    // and, in that case, update num_cols and num_rows.
    // Use defaults, if no file is present
    FILE *input_ptr = set_grid_dimens_from_file(life);

    // 3. Allocate memory for the grid
    malloc_grid(life);

    // 4. Initialize the grid with DEAD cells
    init_empty_grid(life);

    // 5. Initialize the grid with ALIVE cells...
    
    if (input_ptr != NULL) { // ...from file, if present...
        init_from_file(life, input_ptr);
    } else { 
        init_random(life);
    }
}

/**
 * Function that returns the status of the current grid (the number of ALIVE and DEAD cells)
 */
void get_grid_status(struct life_t *life){
    int i,j;
    int alive = 0;
    int dead = 0;

    for (i = 0; i < life->num_cols; i++) 
        for (j = 0; j < life->num_rows; j++) 
            if(life->grid[i][j] == ALIVE)
                alive++;
            else
                dead++;
    
    printf("Alive cells: %d", alive);
    printf("Dead cells: %d", dead);
}

/**
 * Function that make one iteration of the system
 */
void evolve(struct life_t * life) {
    int x, y, x1, y1;

    int n_cols = life->num_cols;
    int n_rows = life->num_rows;
 
    for (y = 0; y < n_cols; y++) 
        for (x = 0; x < n_rows; x++) {
            int n = 0;
            for (y1 = y - 1; y1 <= y + 1; y1++)
                for (x1 = x - 1; x1 <= x + 1; x1++)
                    if (!(y1 == y && x == x1) && life->grid[(y1 + n_cols) % n_cols][(x1 + n_rows) % n_rows] == ALIVE) {
                         n++;
                    }

            if (n == 3 || (n == 2 && life->grid[y][x]))
                life->next_grid[y][x] = ALIVE;
            else
                life->next_grid[y][x] = DEAD;
            /*
            * How do cells evolve in GoL?
            *   - A cell is born, if it has exactly three neighbours;
            *   - A cell dies of loneliness, if it has less than two neighbours;
            *   - A cell dies of overcrowding, if it has more than three neighbours;
            *   - A cell survives to the next generation, if it does not die of loneliness or overcrowding.
            */
        }

    for (y = 0; y < n_cols; y++) 
        for (x = 0; x < n_rows; x++) 
            life->grid[y][x] = life->next_grid[y][x];
}
 
/**
 * Function that starts the system and measures the execution time
 */
void game(struct life_t *life) {
    int x, y, t;

    struct timeval start, end;
    
    initialize(life);

    int ncols = life->num_cols;
    int nrows = life->num_rows;
    
    display(life, false);

    for(t = 0; t < life->timesteps; t++) {
    	if(is_big(life)) 
            gettimeofday(&start, NULL);
        
    	evolve(life);

        if((t % life->vis_interval) == 0){
            display(life, true);
            fflush(stdout);
        }
        
    	if (is_big(life)) {
    		gettimeofday(&end, NULL);
    	    printf("Iteration %d is : %ld ms\n", t,
    	       ((end.tv_sec * 1000000 + end.tv_usec) - 
    	       (start.tv_sec * 1000000 + start.tv_usec))/1000 );
    	}
    }
}
 
int main(int argc, char **argv) {
    struct life_t life;

    // 1. Initialize vars from args
    parse_args(&life, argc, argv);

    // 2. Launch the simulation
    game(&life);
}