CC      = gcc
CFLAGS  = -O3 -march=native -fopenmp -Wall -Wextra
LDFLAGS = -lm
LIB_SHARED = libpermanent.so

all: lib benchmark oeis_a089745 oeis_a089746 test_suite

# --- 1. Python Library (Shared Object) ---

lib: permanent.c permanent.h
	$(CC) $(CFLAGS) -fPIC -shared -o libpermanent.so permanent.c

# --- 2. C Executables ---

test_suite: test_suite.c permanent.c permanent.h
	$(CC) $(CFLAGS) -o test_suite test_suite.c permanent.c $(LDFLAGS)

benchmark: benchmark.c permanent.c permanent.h
	$(CC) $(CFLAGS) -o benchmark benchmark.c permanent.c $(LDFLAGS)

oeis_a089745: oeis_a089475.c permanent.c permanent.h
	$(CC) $(CFLAGS) -o oeis_a089475 oeis_a089475.c permanent.c $(LDFLAGS)

oeis_a089746: oeis_a089476.c permanent.c permanent.h
	$(CC) $(CFLAGS) -o oeis_a089476 oeis_a089476.c permanent.c $(LDFLAGS)

# --- 3. Utilities ---
# 3. Utilities

# Build and run test
test: test_suite
	./test_suite 

bench: benchmark
	./benchmark

clean:
	rm -f *.so benchmark oeis_a089476 oeis_a089475 test_suite *.o

.PHONY: all lib clean
