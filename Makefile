CC = gcc
# Flags: -fPIC is essential for creating a shared library
CFLAGS = -O3 -march=native -fopenmp -Wall -Wextra -fPIC

# Source files
SRC_LIB = permanent.c
SRC_TEST = test_suite.c
SRC_BENCH = benchmark.c
SRC_A089475 = oeis_a089475.c
SRC_A089476 = oeis_a089476.c

# Object files
OBJ_LIB = permanent.o

# Executables
EXE_TEST = test_suite
EXE_BENCH = benchmark
EXE_A089475 = oeis_a089475
EXE_A089476 = oeis_a089476

# Library output name
LIB_SHARED = libpermanent.so

# Phony targets to avoid conflicts with files of the same name
.PHONY: all lib run clean

# Default target: build all executables
all: $(EXE_TEST) $(EXE_BENCH) $(EXE_A089475) $(EXE_A089476)

# Library target: builds the shared object
lib: $(LIB_SHARED)

# Link the object file into a shared library (.so)
$(LIB_SHARED): $(OBJ_LIB)
	$(CC) -shared -o $(LIB_SHARED) $(OBJ_LIB) -fopenmp

# Compile the library source to an object file
$(OBJ_LIB): $(SRC_LIB) permanent.h
	$(CC) $(CFLAGS) -c $(SRC_LIB) -o $(OBJ_LIB)

# Test Suite
$(EXE_TEST): $(OBJ_LIB) $(SRC_TEST)
	$(CC) $(CFLAGS) -o $(EXE_TEST) $(OBJ_LIB) $(SRC_TEST) -lm

# Benchmark
$(EXE_BENCH): $(OBJ_LIB) $(SRC_BENCH)
	$(CC) $(CFLAGS) -o $(EXE_BENCH) $(OBJ_LIB) $(SRC_BENCH) -lm

# OEIS A089475 (Nonsingular)
$(EXE_A089475): $(OBJ_LIB) $(SRC_A089475)
	$(CC) $(CFLAGS) -o $(EXE_A089475) $(OBJ_LIB) $(SRC_A089475) -lm

# OEIS A089476 (Singular)
$(EXE_A089476): $(OBJ_LIB) $(SRC_A089476)
	$(CC) $(CFLAGS) -o $(EXE_A089476) $(OBJ_LIB) $(SRC_A089476) -lm

# Run the test suite
run: $(EXE_TEST)
	./$(EXE_TEST)

# Clean up build artifacts
clean:
	rm -f *.o $(EXE_TEST) $(EXE_BENCH) $(EXE_A089475) $(EXE_A089476) $(LIB_SHARED)
