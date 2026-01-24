/*
 * benchmark.c
 * comprehensive performance shootout between library functions.
 * Compares:
 * 1. permanent()       - The main wrapper (Spies/Masschelein logic)
 * 2. ryser_new()       - The optimized C implementation
 * 3. permanent_ryser() - The legacy reference implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <omp.h>
#include <inttypes.h>
#include <string.h>
#include "permanent.h"

// Output Colors
#define GRN "\033[32m"
#define RED "\033[31m"
#define YEL "\033[33m"
#define CYN "\033[36m"
#define RST "\033[0m"

// Function signatures are imported via permanent.h
// We assume they all return int64_t.

void run_benchmark_scenario(const char* name, int rows, int cols) {
    printf("\n" CYN "=== BENCHMARK: %s (%dx%d) ===" RST "\n", name, rows, cols);

    // 1. Setup Matrix with random values (-1, 0, 1)
    // We use small integers to prevent overflow before the factorial limit.
    int8_t *mat = malloc(rows * cols * sizeof(int8_t));
    
    // Fixed seed for fairness and reproducibility
    srand(12345); 
    for(int i=0; i<rows*cols; i++) mat[i] = (rand() % 3) - 1;

    // Variables for results and timing
    int64_t res_main = 0, res_new = 0, res_old = 0;
    double t_main = 0, t_new = 0, t_old = 0;
    double start, end;

    // --- 1. Main Wrapper (permanent) ---
    // This is the primary entry point for users.
    start = omp_get_wtime();
    res_main = permanent(mat, rows, cols);
    end = omp_get_wtime();
    t_main = end - start;
    printf("  %-20s: Result = %20" PRId64 " | Time = %.6f s\n", "permanent()", res_main, t_main);

    // --- 2. New Optimized (ryser_new) ---
    // The underlying optimized C implementation.
    start = omp_get_wtime();
    res_new = ryser_new(mat, rows, cols);
    end = omp_get_wtime();
    t_new = end - start;
    printf("  %-20s: Result = %20" PRId64 " | Time = %.6f s\n", "ryser_new()", res_new, t_new);

    // --- 3. Old Implementation (permanent_ryser) ---
    // Legacy reference code. We run this to demonstrate the speedup.
    // Note: For very large inputs, this might be significantly slower.
    start = omp_get_wtime();
    res_old = permanent_ryser(mat, rows, cols);
    end = omp_get_wtime();
    t_old = end - start;
    printf("  %-20s: Result = %20" PRId64 " | Time = %.6f s\n", "permanent_ryser()", res_old, t_old);

    // --- Validation (Consistency Check) ---
    // Speed is useless if the answer is wrong. We require exact matches.
    if (res_main == res_new && res_new == res_old) {
        printf("  Consistency Check   : " GRN "PASS" RST " (All results identical)\n");
    } else {
        printf("  Consistency Check   : " RED "FAIL" RST "\n");
        printf("    Ref (Old): %" PRId64 "\n", res_old);
        printf("    New      : %" PRId64 "\n", res_new);
        printf("    Main     : %" PRId64 "\n", res_main);
    }

    // --- Speedup Calculation ---
    double fastest = t_main;
    if (t_new < fastest) fastest = t_new;
    if (t_old < fastest) fastest = t_old;
    
    // Determine the slowest method to calculate the speedup factor
    double slowest = (t_old > t_main) ? t_old : t_main; 
    if (t_new > slowest) slowest = t_new;

    if (fastest > 0) {
        printf("  Speedup vs Slowest  : " YEL "%.2fx" RST "\n", slowest / fastest);
    }

    free(mat);
}

int main() {
    printf("==============================================================\n");
    printf("   LIBRARY FUNCTION SHOOTOUT (Correctness & Speed)            \n");
    printf("==============================================================\n");

    // 1. Square Matrix (Safe limit for Int64)
    // 20x20 is the practical limit for dense matrices before int64 overflow.
    run_benchmark_scenario("Square Matrix", 20, 20);

    // 2. Rectangular Matrix (Spies Optimization Test)
    // The library should automatically handle the transpose/padding logic efficiency.
    // 8x15 is internally processed as a 15x15 problem.
    run_benchmark_scenario("Rectangular Small", 8, 15);

    // 3. Larger Rectangular Matrix
    // Demonstrates the scaling advantage of the new implementation.
    run_benchmark_scenario("Rectangular Medium", 8, 16);

    return 0;
}
