CC      = gcc
CFLAGS  = -O3 -march=native -fopenmp -Wall -Wextra
LDFLAGS = -lm

# Alles bouwen: de library én de tools
all: lib benchmark oeis_nonsingular oeis_singular test_suite

# --- 1. Python Library (Shared Object) ---
# Heeft -fPIC en -shared nodig
lib: permanent.c permanent.h
	$(CC) $(CFLAGS) -fPIC -shared -o libpermanent.so permanent.c

# --- 2. C Executables ---

test_suite: test_suite.c permanent.c permanent.h
	$(CC) $(CFLAGS) -o test_suite test_suite.c permanent.c $(LDFLAGS)

benchmark: benchmark.c permanent.c permanent.h
	$(CC) $(CFLAGS) -o benchmark benchmark.c permanent.c $(LDFLAGS)

oeis_nonsingular: oeis_a089475.c permanent.c permanent.h
	$(CC) $(CFLAGS) -o oeis_a089475 oeis_a089475.c permanent.c $(LDFLAGS)

oeis_singular: oeis_a089476.c permanent.c permanent.h
	$(CC) $(CFLAGS) -o oeis_a089476 oeis_a089476.c permanent.c $(LDFLAGS)

# --- 3. Utilities ---

clean:
	rm -f *.so benchmark oeis_a089476 oeis_a089475 test_suite *.o

.PHONY: all lib clean
