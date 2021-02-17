#ifndef GoL_CHUNK_INIT_H
#define GoL_CHUNK_INIT_H

/**
 * Allocate memory for the current and next slice of GoL data.
 */
void malloc_chunk(struct chunk_t *chunk) {
    int i;

    int ncols = chunk->ncols;
    int nrows = chunk->nrows;

    chunk->slice      = (bool **) malloc(sizeof(bool *) * (nrows + 2)); // + 2 to account for ghost rows
    chunk->next_slice = (bool **) malloc(sizeof(bool *) * (nrows + 2));

    // Dinamically allocate the actual slices of GoL data as 1D arrays to guarantee their continuity in memory,
    // which greatly favours their exchange with MPI routines...
    bool *data      = (bool *) malloc((nrows + 2)*ncols * sizeof(bool));
    bool *next_data = (bool *) malloc((nrows + 2)*ncols * sizeof(bool));

    #ifdef _OPENMP
    #pragma omp parallel for
    #endif
    // ...but let them be accessed as 2D matrices for ease of use
    for (i = 0; i < nrows + 2; i++) {
        chunk->slice[i]      = &(data[ncols*i]);
        chunk->next_slice[i] = &(next_data[ncols*i]);
    }
}

/**
 * Initialize the slices of GoL data with DEAD values.
 */
void init_empty_chunk(struct chunk_t *chunk) {
    int i, j;
  
    #ifdef _OPENMP
    #pragma omp parallel for private(j)
    #endif
    for (i = 0; i < chunk->nrows + 2; i++)
        for (j = 0; j < chunk->ncols; j++) {
            chunk->slice[i][j]      = DEAD;
            chunk->next_slice[i][j] = DEAD;
        }
}

/**
 * Initialize the slices of GoL data with ALIVE values randomly.
 * 
 * Each process will generate the very same sequence as a single process would do in the sequential case, but it will consider only
 * those values that belong to it, whose boundaries are passed in the arguments, plus its initial ghost rows.
 * 
 * @param from    The # of the top row belonging to the calling process
 * 
 * @param to      The # of the bottom row belonging to the calling process
 */
void init_random_chunk(struct chunk_t *chunk, struct life_t life, int from, int to) {
    int i, j, m, n;

    bool top_g_row = false; // Check whether the top ghost row of the calling process has been visited.
                            //
                            // It strengthens the exit condition especially in the case of the rank 0 process,
                            // whose top ghost row is actually the very last row in the whole GoL board.

    // 1. Generate nrows*ncols random values as in the sequential case
    for (i = 0; i < life.nrows; i++) {
        for (j = 0; j < life.ncols; j++) { 
            float f = rand_double(0., 1.);    

            if (f < life.init_prob) {
                m = (from - 1 + life.nrows) % life.nrows; // # of the top ghost row
                n = (to + 1) % life.nrows;                // # of the botton ghost row

                // 2. Assign values only if they belong to the process
                // or to either its top/bottom ghost rows
                if (i >= from  && i <= to) {
                    chunk->slice[i - from + 1][j] = ALIVE;
                } else if (i == m) {
                    chunk->slice[0][j] = ALIVE;
                    top_g_row = true;
                } else if (i == n) {
                    chunk->slice[chunk->nrows + 1][j] = ALIVE;
                }
            }
        }

        // If the process has already collected all its values interrupt the loop,
        // as there's no need to make it generate any more
        if (i > to && top_g_row)
            break;
    } 
}

/**
 * Initialize the slices of GoL data with ALIVE values from file.
 * 
 * Each process will read the whole file as a single process would do in the sequential case, but it will consider only
 * those values that belong to it, whose boundaries are passed in the arguments, plus its initial ghost rows.
 * 
 * @param tot_rows    The overall number of rows in GoL's board.
 * 
 * @param tot_cols    The overall number of columns in GoL's board.
 * 
 * @param file_ptr    The pointer to the open input file starting from the 2nd line.
 * 
 * @param from        The # of the top row belonging to the calling process
 * 
 * @param to          The # of the bottom row belonging to the calling process
 */
void init_chunk_from_file(struct chunk_t *chunk, int tot_rows, int tot_cols,
        FILE *file_ptr, int from, int to) {
    int i, m, n, l, r;

    bool top_g_row = false;

    char *line = NULL;
    size_t buf_size = 0; // Size of the buffer allocated to read the line
    ssize_t len = 0;     // Amount of characters in the read line

    bool finished = false; // Check whether all the necessary rows have been read correctly from the file,
                           // and the loop hasn't finished for whatever other reason

    i = 0;

    m = (from - 1 + tot_rows) % tot_rows; // # of the top ghost row
    n = (to + 1) % tot_rows;              // # of the botton ghost row

    // 1. Read all lines from the file
    while ((len = getline(&line, &buf_size, file_ptr)) != -1) {
        if (i >= tot_rows) {
            perror("[*] GoL's input file exceeds the number of rows!\n");
            MPI_Abort(MPI_COMM_WORLD, 1); 
        }

        if (len != tot_cols + 1) { // +1 for newline char, '\n'
            fprintf(stderr, "[*] Row %d does not respect the number of columns!\n", i);
            MPI_Abort(MPI_COMM_WORLD, 1); 
        }

        // 2. Check if the values belong to the process
        // or to either its top/bottom ghost rows
        if (i >= from  && i <= to) {
            r = i - from + 1;
        } else if( i == m ) {
            r = 0;
            top_g_row = true;
        } else if( i == n ) {
            r = chunk->nrows + 1;
        } else {
            i++;
            continue;
        }

        // 3. Assign all row values
        for (l = 0; l < len - 1; l++){
            if (line[l] == 'X')
                chunk->slice[r][l] = ALIVE;
        }

        i++;

        // As soon as the process has collected all its values interrupt the loop,
        // since there's no need to make it read any more lines
        if (i > to && top_g_row) {
            finished = true;
            break;
        }
    }

    free(line);
    line = NULL;

    if (!finished) {
        perror("[*] GoL's input file does not respect the number of rows!\n");
        MPI_Abort(MPI_COMM_WORLD, 1); 
    }

    fflush(file_ptr);
    fclose(file_ptr);
}

#endif
