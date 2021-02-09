
/**
 * Allocate memory for the current and next GoL chunk of board.
 * 
 * @todo This could be done in the above malloc_grid by unsing the union inside a 
 *       generic struct
 */
void malloc_chunk(struct chunk_t *chunk) {
    int i;

    int ncols = chunk->num_cols;
    int nrows = chunk->num_rows;

    bool *data      = (bool *) malloc((nrows + 2) * ncols * sizeof(bool)); // Guarantee continuous blocks of memory
    bool *next_data = (bool *) malloc((nrows + 2) * ncols * sizeof(bool));

    chunk->chunk      = (bool **) malloc(sizeof(bool *) * (nrows + 2));
    chunk->next_chunk = (bool **) malloc(sizeof(bool *) * (nrows + 2));

    #ifdef _OPENMP
    #pragma omp parallel for
    #endif
    // we don't need two extra columns because each process already has the neighboor columns
    for (i = 0; i < nrows + 2; i++) {
        chunk->chunk[i]      = &(data[ncols * i]);
        chunk->next_chunk[i] = &(next_data[ncols * i]);
    }
}

/**
 * Initialize the GoL board with DEAD values.
 */
void init_empty_chunk(struct chunk_t *chunk) {
    int i, j;
  
    #ifdef _OPENMP
    #pragma omp parallel for private(j)
    #endif
    for (i = 0; i < chunk->num_rows + 2; i++)
        for (j = 0; j < chunk->num_cols; j++) {
            chunk->chunk[i][j]      = DEAD;
            chunk->next_chunk[i][j] = DEAD;
        }
}

/**
 * Initialize the chunk with ALIVE values from file.
 * 
 * @param file_ptr    The pointer to the open input file.
 */
void init_chunk_from_file(struct chunk_t *chunk, unsigned int num_rows, unsigned int num_cols, FILE *file_ptr, int from, int to) {
    int i, j, m, n, l, r;
    char *line = NULL;
    bool grow_top = false;
    size_t len = 0;
    ssize_t read = 0;
    i = 0;

    m = (from - 1 + num_rows) % num_rows;
    n = (to + 1) % num_rows;

    // Every line from the file contains row/column coordinates
    // of every cell that has to be initialized as ALIVE.
    while ((read = getline(&line, &len, file_ptr)) > 0) {

        if (i >= num_rows){
            perror("[*] The input file exceeds the number of rows!\n");
            MPI_Abort(MPI_COMM_WORLD, 1); 
        }

        if (read != num_cols + 1){
            fprintf(stderr, "[*] Row %d does not respect the number of columns!\n", i);
            MPI_Abort(MPI_COMM_WORLD, 1); 
        }

        if (i >= from  && i <= to){
            r = i - from + 1;
        } else if( i == m ){
            r = 0;
            grow_top = true;
        } else if( i == n ){
            r = chunk->num_rows + 1;
        } else {
            i = i + 1;
            continue;
        }

        // assigning the rows that actually belong to the chunk
        for (l = 0; l < read - 1; l++){
            if (line[l] == 'X')
                chunk->chunk[r][l] = ALIVE;
        }

        i = i + 1;

        if ( i > to && grow_top ){
            break;
        }
    }

    free(line);
    line = NULL;

    if ((chunk->rank == 0) && (i != num_rows)){
        perror("[*] The input file does not respect the number of rows!\n");
        MPI_Abort(MPI_COMM_WORLD, 1); 
    }
    
    // Every line from the file contains row/column coordinates
    // of every cell that has to be initialized as ALIVE.
    // here we have to read all the file, is necessary because is not ordered
    // while (fscanf(file_ptr, "%d %d\n", &i, &j) != EOF) {
    //     m = (from - 1 + num_rows) % num_rows;
    //     n = (to + 1) % num_rows;

    //     // assigning the rows that actually belong to the chunk
    //     if (i >= from <= to){
    //         chunk->chunk[i - from + 1][j] = ALIVE;
    //     } else if( i == m ){
    //         chunk->chunk[0][j] = ALIVE;
    //     } else if( i == n ){
    //         chunk->chunk[chunk->num_rows + 1][j] = ALIVE;
    //     }
    // }

    fflush(file_ptr);
    fclose(file_ptr);
}

/**
 * Initialize the GoL board with ALIVE values randomly.
 */
void init_random_chunk(struct chunk_t *chunk, struct life_t life, int from, int to) {
    int i, j, m, n;
    bool grow_top = false;

    // loop through the grid matrix and generate all the random values
    for (i = 0; i < life.num_rows; i++) {
        for (j = 0; j < life.num_cols; j++) { 
            float f = rand_double(0., 1.);    

            if (f < life.init_prob){
                m = (from - 1 + life.num_rows) % life.num_rows;
                n = (to + 1) % life.num_rows;

                // assigning the rows that actually belong to the chunk
                if (i >= from  && i <= to){
                    chunk->chunk[i - from + 1][j] = ALIVE;
                } else if( i == m ){ // controll if the last row was saved by the first process 
                    chunk->chunk[0][j] = ALIVE;
                    grow_top = true;
                } else if( i == n ){
                    chunk->chunk[chunk->num_rows + 1][j] = ALIVE;
                }
            }
        }

        // control if I processed all the elements and stop the loop
        if ( i > to && grow_top ){
            break;
        }
    } 
}