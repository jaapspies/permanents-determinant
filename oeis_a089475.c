/*
 * oeis_a089475.c
 * Calculates terms for OEIS Sequence A089475
 * * Strategy:
 * - Backtracking with Rank Pruning (using Gaussian elimination with doubles).
 * - Exact Verification: Uses the Bareiss Algorithm (Integer Arithmetic) to ensure 
 * nonsingularity (Determinant != 0) before storing results.
 * * Dependencies: permanent.h, permanent.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include "permanent.h"

#define N 6
#define MAX_PERM 5040 
#define EPSILON 1e-9

// Global tracking
bool found_values[MAX_PERM + 1];
long long total_nonsingular_found = 0;

// --- 1. EXACT INTEGER DETERMINANT (BAREISS) ---
// Internal function: performs exact arithmetic with integers.
// Ensures strict filtering of singular matrices (where Det = 0).
long long det_exact(int n, int8_t *M_in) {
    long long M[N][N]; 
    
    // Copy input to long long buffer to prevent overflow
    for(int i=0; i<n; i++) 
        for(int j=0; j<n; j++) 
            M[i][j] = (long long)M_in[i*n + j];

    long long sign = 1;
    long long pivot_prev = 1;

    for(int k=0; k<n-1; k++) {
        // Find pivot
        if(M[k][k] == 0) {
            int swap_row = -1;
            for(int i=k+1; i<n; i++) {
                if(M[i][k] != 0) { swap_row = i; break; }
            }
            if(swap_row == -1) return 0; // Singular matrix!
            
            // Row swap
            sign = -sign;
            for(int j=k; j<n; j++) {
                long long temp = M[k][j];
                M[k][j] = M[swap_row][j];
                M[swap_row][j] = temp;
            }
        }
        
        // Bareiss update step (Exact division guaranteed)
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

// --- 2. FAST RANK PRUNING (FLOATING POINT) ---
// Heuristic check during search (uses doubles for speed).
// May produce rare false positives, which are filtered by det_exact later.
int is_independent_real(double *basis, int current_rank, int n_cols, const int8_t *new_row_vals) {
    double vec[N];
    for(int i=0; i<n_cols; i++) vec[i] = (double)new_row_vals[i];

    for(int i=0; i<current_rank; i++) {
        int pivot_col = -1;
        for(int k=0; k<n_cols; k++) {
            if (fabs(basis[i*N + k]) > EPSILON) { pivot_col = k; break; }
        }
        if (pivot_col != -1 && fabs(vec[pivot_col]) > EPSILON) {
            double factor = vec[pivot_col] / basis[i*N + pivot_col];
            for(int k=pivot_col; k<n_cols; k++) vec[k] -= factor * basis[i*N + k];
        }
    }

    for(int k=0; k<n_cols; k++) {
        if (fabs(vec[k]) > EPSILON) {
            for(int b=0; b<n_cols; b++) basis[current_rank*N + b] = vec[b];
            return 1;
        }
    }
    return 0; // Dependent
}

// --- 3. RECURSIVE SEARCH ---
void dfs(int row_idx, double *basis, int8_t *matrix_flat, int start_val) {
    // Base case: Matrix is full (N rows)
    if (row_idx == N) {
        
        // --- FINAL GATEKEEPER ---
        // Verify strict nonsingularity using exact integer math.
        // This eliminates any floating point errors from the pruning stage.
        if (det_exact(N, matrix_flat) != 0) {
            
            double p = permanent(matrix_flat, N, N);
            int p_int = (int)(p + 0.5);

            #pragma omp critical
            {
                if (p_int >= 0 && p_int <= MAX_PERM) {
                    found_values[p_int] = true;
                }
                total_nonsingular_found++;
            }
        }
        return;
    }

    int max_val = (1 << N);
    double next_basis[N * N];
    int8_t row_vals[N];

    // Iterate through possible row values
    // Row Sorting enforced: val starts at start_val to ensure row[i] > row[i-1]
    for (int val = start_val; val < max_val; val++) {
        for (int b = 0; b < N; b++) row_vals[b] = (val >> b) & 1;
        
        memcpy(next_basis, basis, N * N * sizeof(double));

        // Only recurse if the new row increases the rank (Pruning)
        if (is_independent_real(next_basis, row_idx, N, row_vals)) {
            for(int b=0; b<N; b++) matrix_flat[row_idx*N + b] = row_vals[b];
            dfs(row_idx + 1, next_basis, matrix_flat, val + 1);
        }
    }
}

int main() {
    printf("--- OEIS A089475 Search (N=%d) ---\n", N);
    
    // Reset array
    for(int i=0; i<=MAX_PERM; i++) found_values[i] = false;
    
    double start_time = omp_get_wtime();
    int max_val = (1 << N);
    
    // Optimization: Upper limit for Row 0 due to sorting constraint
    int limit_row_0 = max_val - N + 1;

    #pragma omp parallel 
    {
        double t_basis[N * N];
        int8_t t_matrix[N * N];

        #pragma omp for schedule(dynamic, 1)
        for (int val = 1; val < limit_row_0; val++) {
            memset(t_matrix, 0, N*N*sizeof(int8_t));
            memset(t_basis, 0, N*N*sizeof(double));

            for (int b = 0; b < N; b++) t_matrix[0*N + b] = (val >> b) & 1;
            for (int b = 0; b < N; b++) t_basis[0*N + b] = (double)t_matrix[b];

            dfs(1, t_basis, t_matrix, val + 1);
        }
    }
    
    double end_time = omp_get_wtime();

    // Results printing
    int count = 0;
    FILE *fp = fopen("results_nonsingular.csv", "w");
    
    printf("\nValues found: ");
    for (int i = 0; i <= MAX_PERM; i++) {
        if (found_values[i]) {
            count++;
            printf("%d ", i);
            if(fp) fprintf(fp, "%d\n", i);
        }
    }
    printf("\n");
    
    if(fp) {
        fclose(fp);
        printf("CSV written to results_nonsingular.csv\n");
    }
    
    printf("Total distinct values: %d\n", count);
    printf("Matrices checked (Passed Pruning): %lld\n", total_nonsingular_found);
    printf("Calculation time: %.4f seconds\n", end_time - start_time);
    
    return 0;
}
