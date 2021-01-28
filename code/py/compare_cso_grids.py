"""
Compare Game of Life grids represented in the Compressed Sparse cOordinate (CSO) format.
"""

import argparse
import os

import numpy as np

from concurrent.futures import thread # Parallelize comparisons

def parse_args():
    parser = argparse.ArgumentParser(add_help=False,
                            description='Comparing results from Game of Life experiments.')

    # Split required/optional args
    required = parser.add_argument_group('required arguments')
    optional = parser.add_argument_group('optional arguments')

    # Add back help 
    optional.add_argument(
        '-h',
        '--help',
        action='help',
        default=argparse.SUPPRESS,
        help='Show this help message and exit'
    )

    required.add_argument('--gt',  type=str, required=True, nargs=None, 
                        help="The filename of GoL's ground-truth grid")
    required.add_argument('--cmp', type=str, required=True, nargs='+',
                        help="The list of filenames of GoL's grids to compare. ") 

    return parser.parse_args()

if __name__ == "__main__":
    args = parse_args()
    
    # Read the first line of the ground-truth file,
    # which contains GoL matrix's number of rows and columns
    gt_file = open(args.gt, "r")

    first_line_gt = gt_file.readline().split()
    rows, cols = list(map(lambda val: int(val.strip()),
                          first_line_gt))

    # Initialize the ground-truth's NumPy matrix with
    # that number of rows and columns
    gt = np.zeros((rows, cols)).astype(int)

    # Fill the ground-truth matrix
    for coords in gt_file:
        row, col = list(map(lambda val: int(val.strip()),
                            coords.split()))
        gt[row, col] = 1

    gt_file.close()

    def compare(filename):
        cmp_file = open(filename, "r")
        first_line_cmp = cmp_file.readline().split()

        rows_cmp, cols_cmp = list(map(lambda val: int(val.strip()),
                                      first_line_cmp))

        if rows_cmp != rows or cols_cmp != cols:
            cmp_file.close()
            return False
        else:
            cmp = np.zeros((rows, cols)).astype(int)

            for coords in cmp_file:
                row, col = list(map(lambda val: int(val.strip()),
                                    coords.split()))
                cmp[row, col] = 1

            cmp_file.close()
            return np.allclose(gt, cmp)

    with thread.ThreadPoolExecutor(max_workers=16) as exec:
        cmp_results = list(exec.map(compare, args.cmp)) # Store comparison results

    print("") # Add a blank line of separation

    basename_gt = os.path.basename(args.gt)

    # Print comparison results
    for idx, filename in enumerate(args.cmp):
        basename_cmp = os.path.basename(filename)

        if cmp_results[idx]:
            print(basename_gt, "is equal to", basename_cmp)
        else:
            print(basename_gt, "is different from", basename_cmp)
