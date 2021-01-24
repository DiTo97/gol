import argparse
import numpy as np
import os


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Comparing results from GoL experiments.')
    # Default paths are relative to the current directory
    parser.add_argument('--gt', type=str, default="serial_results.txt")
    parser.add_argument('--cmp', type=str, default="[vect.txt, openmp.txt, mpi.txt, hybrid.txt]") 
    args = parser.parse_args()
    
    # Read the first line of the ground-truth file, which contains the number of rows and columns of the GoL matrix
    gt_file = open(args.gt, "r")
    first_line_gt = gt_file.readline()
    rows_cols_gt = list(map(lambda val:int(val.strip()), first_line_gt.split()))

    # Initialize ground-truth numpy arrays with the number of rows and columns retrieved before
    gt = np.zeros((rows_cols_gt[0], rows_cols_gt[1])).astype(int)
    
    # Preprocess command line inputs
    cmp_file_names = list(map(lambda val:val.strip(),args.cmp.replace("[", "").replace("]", "").split(",")))
    
    # Create array to store comparison results
    cmp_results = []

    # Fill the ground-truth array
    gt_file.close()
    gt_file = open(args.gt, "r")
    next(gt_file)
    for line in gt_file:
        coord = list(map(lambda val:int(val.strip()), line.split()))
        gt[coord[0],coord[1]] = 1
    gt_file.close()

    # Check if the result obtained with each variant is equal to the ground-truth one
    for fpath in cmp_file_names:
        cmp_file = open(fpath, "r")
        first_line_cmp = cmp_file.readline()
        cmp_file.close()
        rows_cols_cmp = list(map(lambda val:int(val.strip()), first_line_cmp.split()))
        if (rows_cols_cmp[0] != rows_cols_gt[0] or rows_cols_cmp[1] != rows_cols_gt[1]):
            cmp_results.append(False)
        else:
            cmp = np.zeros((rows_cols_gt[0], rows_cols_gt[1])).astype(int)
            cmp_file = open(fpath, "r")
            next(cmp_file)
            for line in cmp_file:
                coord = list(map(lambda val:int(val.strip()), line.split()))
                cmp[coord[0],coord[1]] = 1
            cmp_file.close()
            cmp_results.append(np.allclose(gt, cmp))

    base_name_gt = os.path.basename(args.gt)
    # Print the results of the comparisons
    print("\n")
    for index, fpath in enumerate(cmp_file_names):
        fname = os.path.basename(fpath)
        if cmp_results[index]:
            print("The output of ", base_name_gt, " proved equal to ", fname)
        else:
            print("The output of ", base_name_gt, " is different from ", fname)
    print("\n")



    

