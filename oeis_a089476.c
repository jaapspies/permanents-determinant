/*
 * oeis_a089476.c
 * Calculates terms for OEIS Sequence A089476 (Singular matrices).
 * * NOTE: This is a legacy/reference implementation.
 *
 * * Strategy:
 * - Backtracking (Brute Force) with Row Sorting constraint.
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
// Note: For N=7, max permanent is 5040 (permutation matrix) or higher for general matrices.
#define MAX_PERM 40320 

// Global tracking
bool found_values[MAX_PERM + 1];
long long total_singular_found = 0;

// --- RECURSIVE SEARCH ---
// row_idx: current row being filled (0..N-1)
// start_val: minimum integer value for this row (enforcing row[i] >= row[i-1])
//            Note: unlike nonsingular, we ALLOW duplicate rows (val, not val+1)
//            because duplicate rows create singular matrices.
void dfs(int row_idx, int start_val, int8_t *matrix_flat) {
    // Base Case: Matrix is full
    if (row_idx == N) {
        
        // 1. Exact Determinant Check (Library)
        // We strictly look for Det == 0 (Singular)
        if (determinant(matrix_flat, N) == 0) { 
            
            // 2. Calculate Exact Permanent (Library)
            int64_t p = permanent(matrix_flat, N, N);

            #pragma omp critical
            {
                if (p >= 0 && p <= MAX_PERM) {
                    found_values[p] = true;
                }
                total_singular_found++;
            }
        }
        return;
    }   

    // --- GENERATION ---
    int max_val = (1 << N);
    int8_t row_vals[N];
    
    for (int val = start_val; val < max_val; val++) {
        // Convert integer to binary row
        for (int b = 0; b < N; b++) {
            row_vals[b] = (val >> b) & 1;
        }
        
        // Copy to matrix buffer
        for(int b=0; b<N; b++) matrix_flat[row_idx*N + b] = row_vals[b];
        
        // Recurse
        // We pass 'val' (not val+1) to allow identical rows (which are singular)
        dfs(row_idx + 1, val, matrix_flat);
    }
}

int main() {
    printf("--- OEIS A089476 Legacy Search (Singular) N=%d ---\n", N);
    printf("--- Exact Integer Arithmetic Only ---\n");
    
    // Init
    for(int i=0; i<=MAX_PERM; i++) found_values[i] = false;
    
    // A zero matrix (or matrix with zero row) has permanent 0 and is singular.
    // Ensure 0 is marked found.
    found_values[0] = true; 
    
    double start_time = omp_get_wtime();
    int max_val = (1 << N);
    long long scanned_counter = 0;

    #pragma omp parallel 
    {
        int8_t t_matrix[N * N];

        // Optimization: Start loop at val=1. 
        // We handle the 'zero row' case implicitly or via the initialized found_values[0].
        // (Matrices with a zero row have perm=0, which is already set).
        #pragma omp for schedule(dynamic, 1)
        for (int val = 1; val < max_val; val++) {
            
            memset(t_matrix, 0, N*N*sizeof(int8_t));
            
            // Set Row 0
            for (int b = 0; b < N; b++) t_matrix[0*N + b] = (val >> b) & 1;

            // Recurse for Row 1..N-1
            dfs(1, val, t_matrix);

            #pragma omp critical
            {
                scanned_counter++;
            }
        }
    }

    double end_time = omp_get_wtime();
    
    // --- RESULTS WRITING ---
    int count = 0;
    FILE *fp = fopen("results_singular_legacy.csv", "w");

    printf("\n\n--- Results ---\n");
    printf("Values found (A089476): ");
    
    for (int i = 0; i <= MAX_PERM; i++) {
        if (found_values[i]) {
            count++;
            printf("%d ", i); 
            if(fp) fprintf(fp, "%d\n", i);
        }
    }
    printf("\n");

    if(fp) fclose(fp);
    
    printf("Total distinct permanent values: %d\n", count);
    printf("Singular matrices found: %lld\n", total_singular_found);
    printf("Time elapsed: %.4f seconds\n", end_time - start_time);

    return 0;
}
