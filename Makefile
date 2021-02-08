##################################################################################
# Makefile for all versions (serial, vectorized, OMP, MPI, CUDA) of Game of Life #
##################################################################################

# Intel C compiler 
CC = icc 

# Intel C compiler, with MPI support
MPICC = mpiicc

# OMP flag 
OMP_FLAGS = -qopenmp

# Vectorization flags
VEC_FLAGS = -O3 -ipo -xHost

# Create directory command
MKDIR = mkdir -p

# Output directories
OUT_DIR = bin
LOG_DIR = logs

all: dirs no_opt vec omp vec_omp mpi vec_mpi mpi_omp mpi_vec_omp

dirs:
    ${MKDIR} ${OUT_DIR} ${LOG_DIR}

no_opt: gol.c
    $(CC) -O0 -o $(OUT_DIR)/GoL_no_opt gol.c

vec: gol.c
    $(CC) $(VEC_FLAGS) -o $(OUT_DIR)/GoL_vec gol.c

omp: gol.c
    $(CC) $(OMP_FLAGS) -o $(OUT_DIR)/GoL_omp gol.c

vec_omp: gol.c
    $(CC) $(VEC_FLAGS) $(OMP_FLAGS) -o $(OUT_DIR)/GoL_omp_vec gol.c

mpi: gol.c
    $(MPICC) -o $(OUT_DIR)/GoL_mpi gol.c

vec_mpi: gol.c
    $(MPICC) $(VEC_FLAGS) -o $(OUT_DIR)/GoL_mpi_vec gol.c

mpi_omp: gol.c
    $(MPICC) $(OMP_FLAGS) -o $(OUT_DIR)/GoL_mpi_omp gol.c

mpi_vec_omp: gol.c
    $(MPICC) $(VEC_FLAGS) $(OMP_FLAGS) -o $(OUT_DIR)/GoL_mpi_omp_vec gol.c

clean: 
    rm -rf $(OUT_DIR) $(LOG_DIR)

# ##################################################################################
# # Makefile for the CUDA version of Game of Life #
# ##################################################################################

# # NVIDIA CUDA C compiler 
# CC = nvcc

# # Create directory command
# MKDIR = mkdir -p

# # Output directories
# OUT_DIR = bin
# LOG_DIR = logs

# all: dirs cuda

# dirs:
# 				${MKDIR} ${OUT_DIR} ${LOG_DIR}

# cuda: gol_cuda.cu
# 				$(CC) gol_cuda.cu -o $(OUT_DIR)/GoL_cuda 

# clean: 
# 				rm -rf $(OUT_DIR) $(LOG_DIR)



























