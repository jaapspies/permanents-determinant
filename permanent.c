#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <omp.h>
#include "permanent.h"

// --- INTERNAL HELPERS ---
//
// Factorial calculation (used for normalizing rectangular permanents)
static double factorial(int n) {
    double f = 1.0;
    for (int i = 2; i <= n; i++) f *= i;
    return f;
}
// --- RYSER IMPLEMENTATION HELPERS ---

// Efficient Binomial Coefficient for double precision
static double binomial(int n, int k) {
    if (k < 0 || k > n) return 0.0;
    if (k == 0 || k == n) return 1.0;
    if (k > n / 2) k = n - k;
    
    double res = 1.0;
    for (int i = 1; i <= k; i++) {
        res = res * (n - i + 1) / i;
    }
    return res;
}


// The Kernel: Spies' Algebraic Formula.
// Assumes 'matrix_transposed' is stored as T = A_padded^T to allow sequential access,
// drastically improving cache performance and enabling SIMD vectorization.

static double fast_permanent_kernel(const int8_t *matrix_transposed, int n) {
    double total = 0;
    
    // Row sums of A are Column sums of A^T.
    int64_t *row_sums = (int64_t*)malloc(n * sizeof(int64_t));
    if (!row_sums) return 0.0;
    
    // Initialize
    for (int r = 0; r < n; r++) {
        row_sums[r] = 0;
        for (int c = 0; c < n; c++) {
            row_sums[r] += matrix_transposed[c*n + r];
        }
    }

    double current_sign = 1.0; 
    unsigned long long num_loops = 1ULL << (n - 1); 
    unsigned long long old_gray = 0;

    for (unsigned long long i = 1; i <= num_loops; i++) {
        double product = 1.0;
        for (int k = 0; k < n; k++) product *= row_sums[k];
        total += current_sign * product;

        unsigned long long new_gray = i ^ (i >> 1);
        unsigned long long diff = old_gray ^ new_gray;
        int col_idx = __builtin_ctzll(diff);
        int64_t direction = (new_gray > old_gray) ? -2 : 2;

        const int8_t *row_ptr = &matrix_transposed[col_idx * n];
        
        #pragma omp simd
        for (int r = 0; r < n; r++) {
            row_sums[r] += row_ptr[r] * direction;
        }

        current_sign = -current_sign;
        old_gray = new_gray;
    }

    free(row_sums);
    return total / (double)num_loops;
}

// --- PUBLIC FUNCTIONS ---

// 1. Permanent Calculation
// Strict definition: if m > n, result is 0.
double permanent(const int8_t *A, int m, int n) {
    // Mathematical definition constraint:
    // Cannot select m distinct columns from n if m > n.
    if (m > n) {
        return 0.0;
    }

    // From here on, we know m <= n.
    // We pad rows to match columns (Masschelein).
    int target_n = n; 
    int diff = n - m; // Number of rows to add

    // Allocate memory for the padded matrix (stored TRANSPOSED for speed)
    int8_t *padded_transposed = (int8_t*)malloc(target_n * target_n * sizeof(int8_t));
    if (!padded_transposed) return 0.0;
    
    // 1. Fill with ones (Masschelein padding)
    for (int i = 0; i < target_n * target_n; i++) padded_transposed[i] = 1;

    // 2. Copy original data A (m x n) into padded_transposed (n x n).
    // We want to store A^T.
    // Input A[r][c] goes to padded_transposed[c][r].
    for (int r = 0; r < m; r++) {
        for (int c = 0; c < n; c++) {
            padded_transposed[c * target_n + r] = A[r * n + c];
        }
    }

    // 3. Calculate
    double res = fast_permanent_kernel(padded_transposed, target_n);
    free(padded_transposed);

    // 4. Normalize
    if (diff > 0) {
        res /= factorial(diff);
    }

    return res;
}

// 2. Exact Determinant (Bareiss)
double determinant(const int8_t *A, int n) {
    if (n == 0) return 1.0;
    int64_t *M = (int64_t*)malloc(n * n * sizeof(int64_t));
    if (!M) return 0.0;
    for (int i = 0; i < n * n; i++) M[i] = (int64_t)A[i];

    int64_t sign = 1;
    int64_t prev_pivot = 1; 

    for (int k = 0; k < n - 1; k++) {
        if (M[k*n + k] == 0) {
            int swap_row = -1;
            for (int r = k + 1; r < n; r++) {
                if (M[r*n + k] != 0) { swap_row = r; break; }
            }
            if (swap_row == -1) { free(M); return 0.0; }
            for (int c = 0; c < n; c++) {
                int64_t temp = M[k*n + c];
                M[k*n + c] = M[swap_row*n + c];
                M[swap_row*n + c] = temp;
            }
            sign = -sign;
        }
        int64_t pivot = M[k*n + k];
        for (int i = k + 1; i < n; i++) {       
            for (int j = k + 1; j < n; j++) {   
                int64_t term = M[i*n + j] * pivot - M[i*n + k] * M[k*n + j];
                M[i*n + j] = term / prev_pivot;
            }
        }
        prev_pivot = pivot;
    }
    int64_t result = M[(n-1)*n + (n-1)];
    free(M);
    return (double)(result * sign);
}

// 3.  Ryser's Algorithm (Rectangular m x n)
// Modernized port of Spies (2006) code.
double permanent_ryser(const int8_t *A, int m, int n) {
    // Constraint from Brualdi & Ryser: m <= n
    if (m > n) return 0.0;

    double total_sum = 0.0;
    
    // Arrays for Knuth's Algorithm L
    // We allocate once to avoid malloc in loops
    int *c = (int *)malloc((n + 3) * sizeof(int));
    // Column selector array
    int8_t *col_selected = (int8_t *)calloc(n, sizeof(int8_t)); 

    if (!c || !col_selected) {
        if(c) free(c);
        if(col_selected) free(col_selected);
        return 0.0;
    }

    double signum = 1.0;

    // Loop i from 0 to m (corresponds to deleting i columns? No, subset size t)
    // Original code: for(i=0; i < m; i++) -> t = m-i.
    // So t goes from m down to 1.
    
    for (int i = 0; i < m; i++) {
        int t = m - i; // Subset size
        
        // --- Knuth Algorithm L: Generate combinations of size t ---
        
        // L1: Initialize
        for (int j = 1; j <= t; j++) c[j] = j - 1;
        c[t + 1] = n;
        c[t + 2] = 0;
        
        int j = 1;
        while (j <= t) {
            // L2: Visit combination -> Calculate for this subset
            
            // 1. Reset selection vector
            // Optimization: Only clear what we set? For now, memset is safe.
            memset(col_selected, 0, n * sizeof(int8_t));
            
            // 2. Mark selected columns
            for (int k = 1; k <= t; k++) {
                col_selected[c[k]] = 1;
            }

            // 3. Calculate product of row sums restricted to these columns
            double prod = 1.0;
            for (int r = 0; r < m; r++) {
                double row_sum = 0.0;
                for (int col = 0; col < n; col++) {
                    if (col_selected[col]) {
                        row_sum += A[r * n + col];
                    }
                }
                prod *= row_sum;
            }

            // 4. Add contribution
            // Contribution = prod * signum * binomial(n-t, m-t)
            double term = prod * signum * binomial(n - t, m - t);
            total_sum += term;

            // L3: Find j
            j = 1;
            while (c[j] + 1 == c[j + 1]) {
                c[j] = j - 1;
                j++;
            }
            
            // L5: Increase c[j]
            c[j] += 1;
        }

        signum = -signum; // Flip sign for next subset size
    }

    free(c);
    free(col_selected);
    
    return total_sum;
}
