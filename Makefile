CC = gcc
CFLAGS = -O3 -march=native -fopenmp -Wall -Wextra

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

# Targets
all: $(EXE_TEST) $(EXE_BENCH) $(EXE_A089475) $(EXE_A089476)

# Library Object
$(OBJ_LIB): $(SRC_LIB) permanent.h
	$(CC) $(CFLAGS) -c $(SRC_LIB) -o $(OBJ_LIB)

# Test Suite
$(EXE_TEST): $(OBJ_LIB) $(SRC_TEST)
	$(CC) $(CFLAGS) -o $(EXE_TEST) $(OBJ_LIB) $(SRC_TEST)

# Benchmark
$(EXE_BENCH): $(OBJ_LIB) $(SRC_BENCH)
	$(CC) $(CFLAGS) -o $(EXE_BENCH) $(OBJ_LIB) $(SRC_BENCH)

# OEIS A089475 (Nonsingular)
$(EXE_A089475): $(OBJ_LIB) $(SRC_A089475)
	$(CC) $(CFLAGS) -o $(EXE_A089475) $(OBJ_LIB) $(SRC_A089475) -lm

# OEIS A089476 (Singular)
$(EXE_A089476): $(OBJ_LIB) $(SRC_A089476)
	$(CC) $(CFLAGS) -o $(EXE_A089476) $(OBJ_LIB) $(SRC_A089476) -lm

# Commands
run: $(EXE_TEST)
	./$(EXE_TEST)

clean:
	rm -f *.o $(EXE_TEST) $(EXE_BENCH) $(EXE_A089475) $(EXE_A089476)
