/*
 * oeis_a089475.c
 * Calculates terms for OEIS Sequence A089475 (Nonsingular matrices).
 * * NOTE: This is a legacy/reference implementation. 
 * For N >= 8, use the Python pipeline with 'nauty' generation.
 *
 * * Strategy:
 * - Backtracking (Brute Force).
 * - Exact Verification: Uses the library's int64 determinant function.
 * - PURE INTEGER ARITHMETIC (No floating point heuristics).
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <omp.h>
#include "permanent.h"

#define N 6
// Note: For N=7, 7! = 5040. Max permanent increases rapidly.
#define MAX_PERM 40320 

// Global tracking
bool found_values[MAX_PERM + 1];
long long total_nonsingular_found = 0;

// --- RECURSIVE SEARCH ---
void dfs(int row_idx, int8_t *matrix_flat, int start_val) {
    // Base case: Matrix is full (N rows)
    if (row_idx == N) {
        
        // --- EXACT CHECK ---
        // Verify strict nonsingularity using the library's int64 determinant.
        // Bareiss algorithm is fast enough to run at leaves for N <= 7.
        if (determinant(matrix_flat, N) != 0) {
            
            // Calculate Exact Permanent (int64)
            int64_t p = permanent(matrix_flat, N, N);

            #pragma omp critical
            {
                if (p >= 0 && p <= MAX_PERM) {
                    found_values[p] = true;
                }
                total_nonsingular_found++;
            }
        }
        return;
    }

    int max_val = (1 << N);
    
    // Buffer for the current row
    int8_t row_vals[N];

    // Iterate through possible row values (Sorted Rows constraint)
    for (int val = start_val; val < max_val; val++) {
        
        // Convert integer 'val' to binary row
        for (int b = 0; b < N; b++) row_vals[b] = (val >> b) & 1;
        
        // Copy to matrix buffer
        for(int b=0; b<N; b++) matrix_flat[row_idx*N + b] = row_vals[b];
            
        // Recurse
        dfs(row_idx + 1, matrix_flat, val + 1);
    }
}

int main() {
    printf("--- OEIS A089475 Legacy Search (N=%d) ---\n", N);
    printf("--- Exact Integer Arithmetic Only ---\n");
    
    // Reset array
    for(int i=0; i<=MAX_PERM; i++) found_values[i] = false;
    
    double start_time = omp_get_wtime();
    int max_val = (1 << N);
    
    // Optimization: Upper limit for Row 0 due to sorting constraint
    int limit_row_0 = max_val - N + 1;

    #pragma omp parallel 
    {
        int8_t t_matrix[N * N];

        #pragma omp for schedule(dynamic, 1)
        for (int val = 1; val < limit_row_0; val++) {
            memset(t_matrix, 0, N*N*sizeof(int8_t));

            // Set Row 0
            for (int b = 0; b < N; b++) t_matrix[0*N + b] = (val >> b) & 1;

            dfs(1, t_matrix, val + 1);
        }
    }
    
    double end_time = omp_get_wtime();

    // Results printing
    int count = 0;
    FILE *fp = fopen("results_nonsingular_legacy.csv", "w");
    
    printf("\nValues found: ");
    for (int i = 0; i <= MAX_PERM; i++) {
        if (found_values[i]) {
            count++;
            printf("%d ", i);
            if(fp) fprintf(fp, "%d\n", i);
        }
    }
    printf("\n");
    
    if(fp) fclose(fp);
    
    printf("Total distinct values: %d\n", count);
    printf("Matrices checked (Nonsingular): %lld\n", total_nonsingular_found);
    printf("Calculation time: %.4f seconds\n", end_time - start_time);
    
    return 0;
}
