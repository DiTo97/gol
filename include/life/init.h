#ifndef GoL_LIFE_INIT_H
#define GoL_LIFE_INIT_H

/**
 * Update the GoL board's dimensions from file, if it exists and it has a valid format.
 * 
 * @return file_ptr    The pointer to the open input file, NULL otherwise. 
 */
FILE* set_grid_dimens_from_file(struct life_t *life) {
    FILE *file_ptr;

    // what we do here if the file is not present or is not with the right format
    if (life->input_file != NULL) {
        if ((file_ptr = fopen(life->input_file, "r")) == NULL) {
            perror("[*] Failed to open the input file.\n");
            perror("[*] Launching the simulation in default configuration...\n");
        } else if (fscanf(file_ptr, "%d %d\n", &life->num_rows, &life->num_cols) == EOF) {
            perror("[*] The input file only defines GoL board's dimensions!\n");
            perror("[*] Launching the simulation in default configuration...\n");
        } else{
            return file_ptr;
        }
    }

    // An error has occured
    return NULL;
}

/**
 * Allocate memory for the current and next GoL board.
 *
 * @todo Account for ghost rows (+ 2) with MPI.
 */
void malloc_grid(struct life_t *life) {
    int i;

    int ncols = life->num_cols;
    int nrows = life->num_rows;

    life->grid      = (bool **) malloc(sizeof(bool *) * nrows);
    life->next_grid = (bool **) malloc(sizeof(bool *) * nrows);

    #ifdef _OPENMP
    #pragma omp parallel for
    #endif
    for (i = 0; i < nrows; i++) {
        life->grid[i]      = (bool *) malloc(sizeof(bool) * (ncols));
        life->next_grid[i] = (bool *) malloc(sizeof(bool) * (ncols));
    }
}

/**
 * Initialize the GoL board with DEAD values.
 */
void init_empty_grid(struct life_t *life) {
    int i, j;
  
    #ifdef _OPENMP
    #pragma omp parallel for private(j)
    #endif
    for (i = 0; i < life->num_rows; i++)
        for (j = 0; j < life->num_cols; j++) {
            life->grid[i][j]      = DEAD;
            life->next_grid[i][j] = DEAD;
        }
}

/**
 * Initialize the GoL board with ALIVE values from file.
 * 
 * @param file_ptr    The pointer to the open input file.
 */
void init_from_file(struct life_t *life, FILE *file_ptr) {
    int i, j, l;

    // this if is not necessary, I cannot call this funciton if 
    // life->input_file is NULL
    if(life->input_file != NULL){
        char *line = NULL;
        size_t len = 0;
        ssize_t read = 0;
        i = 0;

        // Every line from the file contains row/column coordinates
        // of every cell that has to be initialized as ALIVE.
        while ((read = getline(&line, &len, file_ptr)) >= 0) {
            if (i >= life->num_rows){
                perror("[*] The input file exceeds the number of rows!\n");
                exit(EXIT_FAILURE);
            }

            // `getline()` wrongly skips leading whitespaces until a non-whitespace character is met starting from the 1st row.
            // TODO: Impose the no-leading whitespaces constraint to the input format.

            if (read != life->num_cols + 1){
                fprintf(stderr, "[*] Row %d does not respect the number of columns!\n", i);
                exit(EXIT_FAILURE);
            }
            
            for (l = 0; l < read - 1; l++){
                if(line[l] == 'X'){
                    life->grid[i][l] = ALIVE;
                }
            }

            i = i + 1;
        }

        free(line);
        line = NULL;

        if (i != life->num_rows){
            perror("[*] The input file does not respect the number of rows!\n");
            exit(EXIT_FAILURE);
        }
    }

    fflush(file_ptr);
    fclose(file_ptr);
}

/**
 * Initialize the GoL board with ALIVE values randomly
 */
void init_random(struct life_t *life) {
    int i, j;

    for (i = 0; i < life->num_rows; i++) 
        for (j = 0; j < life->num_cols; j++) { 
            if (rand_double(0., 1.) < life->init_prob)
                life->grid[i][j] = ALIVE;
        }
}

#endif
