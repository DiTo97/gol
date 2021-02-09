##################################################################################
# Makefile for all versions (serial, vectorized, OMP, MPI, CUDA) of Game of Life #
##################################################################################

#############
# Compilers #
#############
# Intel C compiler
CC    = icc
# Intel C compiler, with MPI support
MPICC = mpiicc
# NVIDIA CUDA C compiler
NVCC  = nvcc

#####################
# Compilation flags #
#####################
# ICC vectorization flags
VEC_FLAGS  = -O2 -ipo -xHost
# OpenMP flags
OMP_FLAGS  = -qopenmp
# MPI flags
MPI_FLAGS  = -DGoL_MPI
# CUDA flags
CUDA_FLAGS = -DGoL_CUDA

####################
# Source endpoints #
####################
SRC_DIR = src
CPU_DIR = $(SRC_DIR)/cpu
GPU_DIR = $(SRC_DIR)/gpu

####################
# Output endpoints #
####################
# Binary files' common prefix
BIN_PRE = GoL
# Binary files' directory
BIN_DIR = bin

all: dirs no_opt vec omp vec_omp mpi vec_mpi hybrid vec_hybrid cuda

dirs:
	mkdir -p $(BIN_DIR)

no_opt: $(CPU_DIR)/gol.c
	$(CC) -O0 -o $(BIN_DIR)/$(BIN_PRE)_no_opt $(CPU_DIR)/gol.c

vec: $(CPU_DIR)/gol.c
	$(CC) $(VEC_FLAGS) -o $(BIN_DIR)/$(BIN_PRE)_vec $(CPU_DIR)/gol.c

omp: $(CPU_DIR)/gol.c
	$(CC) $(OMP_FLAGS) -o $(BIN_DIR)/$(BIN_PRE)_omp $(CPU_DIR)/gol.c

vec_omp: $(CPU_DIR)/gol.c
	$(CC) $(VEC_FLAGS) $(OMP_FLAGS) -o $(BIN_DIR)/$(BIN_PRE)_omp_vec $(CPU_DIR)/gol.c

mpi: $(CPU_DIR)/gol.c
	$(MPICC) $(MPI_FLAGS) -o $(BIN_DIR)/$(BIN_PRE)_mpi $(CPU_DIR)/gol.c

vec_mpi: $(CPU_DIR)/gol.c
	$(MPICC) $(VEC_FLAGS) $(MPI_FLAGS) -o $(BIN_DIR)/$(BIN_PRE)_mpi_vec $(CPU_DIR)/gol.c

hybrid: $(CPU_DIR)/gol.c
	$(MPICC) $(OMP_FLAGS) $(MPI_FLAGS) -o $(BIN_DIR)/$(BIN_PRE)_hybrid $(CPU_DIR)/gol.c

vec_hybrid: $(CPU_DIR)/gol.c
	$(MPICC) $(VEC_FLAGS) $(OMP_FLAGS) $(MPI_FLAGS) -o $(BIN_DIR)/$(BIN_PRE)_hybrid_vec $(CPU_DIR)/gol.c

cuda: $(GPU_DIR)/gol.cu
	$(NVCC) $(CUDA_FLAGS) $(GPU_DIR)/gol.cu -o $(BIN_DIR)/$(BIN_PRE)_cuda 

clean: 
	rm -rf $(BIN_DIR)
