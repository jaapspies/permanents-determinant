CC = gcc
# Flags: -fPIC is essential for creating a shared library
# Added -O3 and -march=native for performance
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

# Phony targets
.PHONY: all lib run clean

# Default target
all: $(EXE_TEST) $(EXE_BENCH) $(EXE_A089475) $(EXE_A089476)

# Library target
lib: $(LIB_SHARED)

# Link the shared library
$(LIB_SHARED): $(OBJ_LIB)
	$(CC) -shared -o $(LIB_SHARED) $(OBJ_LIB) -fopenmp

# Compile the library object
$(OBJ_LIB): $(SRC_LIB) permanent.h
	$(CC) $(CFLAGS) -c $(SRC_LIB) -o $(OBJ_LIB)

# Executable Rules
$(EXE_TEST): $(OBJ_LIB) $(SRC_TEST)
	$(CC) $(CFLAGS) -o $(EXE_TEST) $(OBJ_LIB) $(SRC_TEST) -lm

$(EXE_BENCH): $(OBJ_LIB) $(SRC_BENCH)
	$(CC) $(CFLAGS) -o $(EXE_BENCH) $(OBJ_LIB) $(SRC_BENCH) -lm

$(EXE_A089475): $(OBJ_LIB) $(SRC_A089475)
	$(CC) $(CFLAGS) -o $(EXE_A089475) $(OBJ_LIB) $(SRC_A089475) -lm

$(EXE_A089476): $(OBJ_LIB) $(SRC_A089476)
	$(CC) $(CFLAGS) -o $(EXE_A089476) $(OBJ_LIB) $(SRC_A089476) -lm

# Utility targets
run: $(EXE_TEST)
	./$(EXE_TEST)

clean:
	rm -f *.o $(EXE_TEST) $(EXE_BENCH) $(EXE_A089475) $(EXE_A089476) $(LIB_SHARED)
