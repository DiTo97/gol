/*
 * Consway's Game of Life.
 *
 * Serial implementation in C inspired from
 * https://www.geeksforgeeks.org/conways-game-life-python-implementation/
 * 
 * This code should be taken solely as a baseline for the actual parallel
 * implementations either in OpenMP, MPI or CUDA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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
 
void game(struct life_t * life){
	int x, y;
	int nrows = life->num_rows;
	int ncols = life->num_cols;

	unsigned ** grid = life->grid;

	struct timeval start, end;
	
	// controlling if there is an input file and take the 
	// ncols and nrows, if no file present then we use the
	// default values
	FILE * input_file = get_grid_dimension_from_file(life);

	// allocating the memory for the grid
	malloc_grids(life);

	// filling the grid with DEAD states, for initialization
	init_default_grid(life);

	if(input_file != NULL){
		init_from_file(life, input_file);
	}else{
		for (x = 0; x < ncols; x++) 
			for (y = 0; y < nrows; y++){ 
					life->grid[y][x] = rand_double() < life->init_prob ? ALIVE : DEAD;
			}

		for(x = 0; x < ncols; x++)
			for(y = 0; y < nrows; y++){
				printf("%d", life->grid[y][x]);
			}
	}
	
	// if (x > 1000) printbig(univ, w, h,0);
	
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

	parse_args(&life, argc, argv);

	game(&life);
}
