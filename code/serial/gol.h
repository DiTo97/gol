// All the data needed by an instance of Game of Life
struct life_t {
    int vis_interval;     // Display frequency of the grid (if bigger than DEFAULT_MAX_SIZE)
    int num_cols;         // Number of columns in the grid
    int num_rows;         // Number of rows in the gird
    int timesteps;        // Number of generations to simulate

    double init_prob;     // Probability to mark a cell as ALIVE
                          // when following a random initialization
    
    unsigned **grid;      // Game grid at the current step
    unsigned **next_grid; // Game grid at the next step
    
    char *input_file;     // Input filename
    char *output_file;    // Output filename
};

/* Function declarations */
void initialize(struct life_t life);