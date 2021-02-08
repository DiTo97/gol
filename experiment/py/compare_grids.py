"""
Compare Game of Life grids represented in the Full-matrix (FM) format.

Every file's first row will indicate the total number of rows and cols of GoL's matrix, followed by a line for each row, wherein 'X' will indicate alive cells, whereas blank spaces dead cells. That matrix will represent the initial state (0-th generation) of GoL's evolution. A delimiter of 100 '*' characters will separate it from the final state matrix (N-th generation).
"""

import argparse
import os

import numpy as np

from concurrent.futures import thread # Parallelize comparisons

DELIMITER = "*" * 100

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

    print("[*] Reading GoL's initial ground-truth matrix...")

    # Read the first line of the ground-truth file,
    # which contains GoL matrix's number of rows and columns
    gt_file = open(args.gt, "r")

    first_line_gt = gt_file.readline().split()
    rows, cols = list(map(lambda val: int(val.strip()),
                          first_line_gt))

    ###############################
    # 1. Compare initial matrices #
    ###############################

    # Initialize the ground-truth's NumPy matrix with
    # that number of rows and columns
    gt = np.zeros((rows, cols)).astype(int)

    # Fill the ground-truth matrix
    for i, line in enumerate(gt_file):
        line = line.rstrip("\n")
        if (line == DELIMITER):
            break

        row = list(map(lambda val: 1 if val == 'X' else 0, line))
        gt[i] = np.array(row)

    print("[*] Read GoL's initial ground-truth matrix.\n")

    def compare_initial(filename):
        cmp_file = open(filename, "r")
        first_line_cmp = cmp_file.readline().split()

        rows_cmp, cols_cmp = list(map(lambda val: int(val.strip()),
                                      first_line_cmp))

        if rows_cmp != rows or cols_cmp != cols:
            return [False, cmp_file]
        else:
            cmp = np.zeros((rows, cols)).astype(int)
   
            for i, line in enumerate(cmp_file):
                line = line.rstrip("\n")
                if (line == DELIMITER):
                    break

                row = list(map(lambda val: 1 if val == 'X' else 0, line))
                cmp[i] = np.array(row)
       
            return [np.allclose(gt, cmp), cmp_file, cmp]

    print("[*] Reading GoL's initial comparison matrices...")

    with thread.ThreadPoolExecutor(max_workers=16) as exec:
        cmp_results = list(exec.map(compare_initial, args.cmp)) # Store comparison results

    print("[*] Read GoL's initial comparison matrices.\n")

    basename_cmps = []
    basename_gt = os.path.basename(args.gt)

    # Print initial comparison results
    for i, filename in enumerate(args.cmp):
        basename_cmp = os.path.basename(filename)
        basename_cmps.append(basename_cmp)

        # cmp_results[i][0] := Initial matrix comparison result
        # cmp_results[i][1] := Open file pointer or None
        # cmp_results[i][2] := Initial matrix, if it exists

        if cmp_results[i][0]:
            print(basename_gt, "initial matrix is equal to", basename_cmp)
        else:
            print(basename_gt, "initial matrix is different from", basename_cmp)

            # Close file pointers whose initial matrix
            # is different from the ground-truth.
            cmp_results[i][1].close() # Contains the open file pointer
            cmp_results[i][1] = None

    #############################
    # 2. Compare final matrices #
    #############################

    print("\n[*] Reading GoL's final ground-truth matrix...")

    # Fill the ground-truth matrix
    for i, line in enumerate(gt_file):
        line = line.rstrip("\n")
        if (line == DELIMITER):
            break

        row = list(map(lambda val: 1 if val == 'X' else 0, line))
        gt[i] = np.array(row)

    gt_file.close()

    print("[*] Read GoL's final ground-truth matrix.\n")

    def compare_final(data):
        # data[0] := Initial matrix comparison result
        # data[1] := Open file pointer or None
        # data[2] := Initial matrix, if it exists

        if not data[0]:
            return False
        else:
            for i, line in enumerate(data[1]):
                line = line.rstrip("\n")
                if (line == DELIMITER):
                    break

                row = list(map(lambda val: 1 if val == 'X' else 0, line))
                data[2][i] = np.array(row)

            data[1].close() # Close file pointer
       
            return np.allclose(gt, data[2])

    print("[*] Reading GoL's final comparison matrices...")

    with thread.ThreadPoolExecutor(max_workers=16) as exec:
        cmp_results = list(exec.map(compare_final, cmp_results)) # Store comparison results

    print("[*] Read GoL's final comparison matrices.\n")

    # Print final comparison results
    for i, filename in enumerate(args.cmp):
        basename_cmp = basename_cmps[i]

        if cmp_results[i]:
            print(basename_gt, "final matrix is equal to", basename_cmp)
        else:
            print(basename_gt, "final matrix is different from", basename_cmp)