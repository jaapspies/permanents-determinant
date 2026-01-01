/*
 * oeis_a089476.c
 * Calculates terms for OEIS Sequence A089476:
 * "Number of different values taken by the permanent of a real singular (0,1)-matrix of order n."
 *
 * Strategy:
 * - Iterate over canonical matrices (sorted rows).
 * - Skip Row 0 = [0,0...0] (trivial singular, permanent is always 0).
 * - Check Determinant == 0 using Exact Bareiss Algorithm.
 * - Dependencies: permanent.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <omp.h>
#include <math.h>
#include "permanent.h"

#define N 7
#define MAX_PERM 5040 

// Global tracking
bool found_values[MAX_PERM + 1];
long long total_singular_found = 0;

// --- 1. EXACT INTEGER DETERMINANT (BAREISS) ---
// We gebruiken hier dezelfde exacte check als bij de nonsinguliere versie.
long long det_exact(int n, int8_t *M_in) {
    long long M[N][N]; 
    for(int i=0; i<n; i++) 
        for(int j=0; j<n; j++) 
            M[i][j] = (long long)M_in[i*n + j];

    long long sign = 1;
    long long pivot_prev = 1;

    for(int k=0; k<n-1; k++) {
        if(M[k][k] == 0) {
            int swap_row = -1;
            for(int i=k+1; i<n; i++) {
                if(M[i][k] != 0) { swap_row = i; break; }
            }
            if(swap_row == -1) return 0; 
            
            sign = -sign;
            for(int j=k; j<n; j++) {
                long long temp = M[k][j];
                M[k][j] = M[swap_row][j];
                M[swap_row][j] = temp;
            }
        }
        for(int i=k+1; i<n; i++) {
            for(int j=k+1; j<n; j++) {
                long long num = M[i][j] * M[k][k] - M[i][k] * M[k][j];
                M[i][j] = num / pivot_prev;
            }
        }
        pivot_prev = M[k][k];
    }
    return sign * M[n-1][n-1];
}

// Recursive DFS
// row_idx: current row being filled (0..N-1)
// start_val: minimum integer value for this row (enforcing row[i] >= row[i-1])
// matrix_flat: buffer
void dfs(int row_idx, int start_val, int8_t *matrix_flat) {
    // Base Case: Matrix is full
    if (row_idx == N) {
        
        // 1. Exact Determinant Check (Bareiss)
        // We zoeken nu juist naar det == 0 (Singulier)
        if (det_exact(N, matrix_flat) == 0) { 
            
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
    int max_val = (1 << N);
    
    // Iterate from start_val (duplicates allowed for singular search)
    for (int val = start_val; val < max_val; val++) {
        for (int b = 0; b < N; b++) {
            matrix_flat[row_idx * N + b] = (val >> b) & 1;
        }
        dfs(row_idx + 1, val, matrix_flat);
    }
}

int main() {
    printf("--- OEIS Searcher A089476 (Singular) for N=%d ---\n", N);
    
    // Reset en init
    for(int i=0; i<=MAX_PERM; i++) found_values[i] = false;
    
    // 0 is altijd een oplossing (matrix met nul-rij), die vinken we alvast af.
    found_values[0] = true; 
    
    double start_time = omp_get_wtime();
    int max_val = (1 << N);
    long long scanned_counter = 0;

    #pragma omp parallel 
    {
        int8_t t_matrix[N * N];

        // OPTIMALISATIE: Start bij val=1. 
        // val=0 is een rij vol nullen. Permanent is dan altijd 0.
        // Die hebben we al. Dit scheelt enorm veel rekenwerk.
        #pragma omp for schedule(dynamic, 1)
        for (int val = 1; val < max_val; val++) {
            
            memset(t_matrix, 0, N*N*sizeof(int8_t));
            for (int b = 0; b < N; b++) t_matrix[0*N + b] = (val >> b) & 1;

            dfs(1, val, t_matrix);

            #pragma omp critical
            {
                scanned_counter++;
                // Print voortgang iets minder vaak om de console niet te spammen
                if (scanned_counter % 1 == 0) { 
                    fprintf(stderr, "\rProgress: Row 0 val %d / %d done. Singular found: %lld", 
                        val, max_val - 1, total_singular_found);
                }
            }
        }
    }

    double end_time = omp_get_wtime();
    
    // --- RESULTS WRITING (CRASH FIXED) ---
    int count = 0;
    FILE *fp = fopen("results_singular_7.csv", "w");

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

    // Nu pas sluiten, BUITEN de loop!
    if(fp) {
        fclose(fp);
        printf("CSV written to results_singular_7.csv\n");
    }
    
    printf("\n");
    printf("Total distinct permanent values: %d\n", count);
    printf("Singular matrices found: %lld\n", total_singular_found);
    printf("Time elapsed: %.4f seconds\n", end_time - start_time);

    return 0;
}
