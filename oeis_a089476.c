/*
 * oeis_a089476.c
 * * Calculates terms for OEIS Sequence A089476:
 * "Number of different values taken by the permanent of a real singular (0,1)-matrix of order n."
 *
 * Strategy:
 * - Iterate over all canonical matrices (sorted rows: row[i] >= row[i-1]).
 * - Duplicate rows ARE allowed (unlike A089475), as duplicates imply singularity.
 * - Check Determinant == 0 using Bareiss Algorithm.
 * - If Singular, calculate Permanent.
 * * Note: No Rank Pruning possible here, as we are looking for dependent rows.
 * * Dependencies: permanent.h, permanent.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <omp.h>
#include "permanent.h"

#define N 7
#define MAX_PERM 5040 

// Global tracking
bool found_values[MAX_PERM + 1];
long long total_singular_found = 0;

// Recursive DFS
// row_idx: current row being filled (0..N-1)
// start_val: minimum integer value for this row (enforcing row[i] >= row[i-1])
// matrix_flat: buffer
void dfs(int row_idx, int start_val, int8_t *matrix_flat) {
    
    // Base Case: Matrix is full
    if (row_idx == N) {
        
        // 1. Exact Determinant Check (Bareiss)
        double det = determinant(matrix_flat, N);
        
        // If determinant is 0 (Singular)
        if (det == 0.0) { 
            
            // 2. Calculate Permanent
            double p = permanent(matrix_flat, N, N);
            int p_int = (int)(p + 0.5);

            #pragma omp critical
            {
                if (p_int >= 0 && p_int <= MAX_PERM) {
                    found_values[p_int] = true;
                }
                total_singular_found++;
            }
        }
        return;
    }

    // --- GENERATION ---
    // Iterate from start_val to 2^N - 1
    int max_val = (1 << N);
    
    for (int val = start_val; val < max_val; val++) {
        // Fill current row
        for (int b = 0; b < N; b++) {
            matrix_flat[row_idx * N + b] = (val >> b) & 1;
        }

        // Recurse
        // Note: pass 'val' as start_val, allowing duplicates (row[i] == row[i-1])
        dfs(row_idx + 1, val, matrix_flat);
    }
}

int main() {
    printf("--- OEIS Searcher A089476 (Singular) for N=%d ---\n", N);
    printf("Target: Singular matrices (det=0)\n");
    printf("Strategy: Combinations with replacement (sorted rows) + Determinant check\n");
    
    for(int i=0; i<=MAX_PERM; i++) found_values[i] = false;
    double start_time = omp_get_wtime();
    
    int max_val = (1 << N);
    long long scanned_counter = 0;

    // Parallelize Row 0
    // 'schedule(dynamic)' is essential because branch sizes vary wildly
    #pragma omp parallel 
    {
        int8_t t_matrix[N * N];

        #pragma omp for schedule(dynamic, 1)
        for (int val = 0; val < max_val; val++) {
            // Setup Row 0
            memset(t_matrix, 0, N*N*sizeof(int8_t));
            for (int b = 0; b < N; b++) t_matrix[0*N + b] = (val >> b) & 1;

            // Start recursion for row 1. 
            // Start value is 'val' (duplicates allowed)
            dfs(1, val, t_matrix);

            #pragma omp critical
            {
                scanned_counter++;
                fprintf(stderr, "\rProgress: Row 0 val %d / %d done. Singular found: %lld", 
                        val, max_val - 1, total_singular_found);
            }
        }
    }

    double end_time = omp_get_wtime();
    
    int count = 0;
    printf("\n\n--- Results ---\n");
    // printf("Values found (A089476): ");
    for (int i = 0; i <= MAX_PERM; i++) {
        if (found_values[i]) {
            count++;
            // printf("%d ", i); 
        }
    }
    printf("\n");
    printf("Total distinct permanent values: %d\n", count);
    printf("Singular matrices found (canonical sorted forms): %lld\n", total_singular_found);
    printf("Time elapsed: %.4f seconds\n", end_time - start_time);

    return 0;
}
