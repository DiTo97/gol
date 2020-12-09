
// All the data needed by an instance of Life
struct life_t {
	int  vis_interval;		//Print matrix to file every vis_interval if the matrix is bigger then X
  int  num_cols;			  //Number of Columns in the grid
	int  num_rows;			  //Number of Rows in the gird
	unsigned  ** grid;		      //Current grid
	unsigned  ** next_grid;	  //Grid with updated values
	int  timesteps;	    //How many generations the program will simulate
	char * input_file;		    //Input variable
	char * output_file;		    //Output Variabled
  double init_prob;
};