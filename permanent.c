#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#ifdef _OPENMP
#  include <omp.h>
#endif
#include "permanent.h"

// --- INTERNAL HELPERS ---

// Factorial calculation (int64_t)
static int64_t factorial(int n) {
    int64_t f = 1;
    for (int i = 2; i <= n; i++) f *= i;
    return f;
}

// --- RYSER IMPLEMENTATION HELPERS ---

// Efficient Binomial Coefficient for integers
// Uses multiplication before division to maintain integer precision.
static int64_t binomial(int n, int k) {
    if (k < 0 || k > n) return 0;
    if (k == 0 || k == n) return 1;
    if (k > n / 2) k = n - k;
    
    int64_t res = 1;
    for (int i = 1; i <= k; i++) {
        // Warning: Watch for overflow here if n is large, 
        // though for n <= 20 this is safe in int64.
        res = res * (n - i + 1) / i; 
    }
    return res;
}

// The Kernel: Spies' Algebraic Formula.
// Assumes 'matrix_transposed' is stored as T = A_padded^T.
static int64_t fast_permanent_kernel(const int8_t *matrix_transposed, int n) {
    // Guard against undefined behavior and degenerate cases
    if (n < 0) return 0;
    if (n == 0) return 1;
    if (n > 63) return 0; // Cannot represent steps in 64-bit loop

    int64_t total = 0;
    
    // Row sums of A are Column sums of A^T.
    int64_t *row_sums = (int64_t*)malloc(n * sizeof(int64_t));
    if (!row_sums) return 0;
    
    // Initialize
    for (int r = 0; r < n; r++) {
        row_sums[r] = 0;
        for (int c = 0; c < n; c++) {
            row_sums[r] += matrix_transposed[c*n + r];
        }
    }

    int64_t current_sign = 1; 
    unsigned long long num_loops = 1ULL << (n - 1); 
    unsigned long long old_gray = 0;

    // Note: 'total' accumulates signed products. 
    // Ideally, for n=20, int64 is sufficient for 0/1 matrices.
    for (unsigned long long i = 1; i <= num_loops; i++) {
        int64_t product = 1;
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
    
    // Spies formula: result / 2^(n-1). 
    // Since we use exact integers, this division must be exact.
    // We can use bitwise shift.
    return total / (int64_t)num_loops;
}

// --- PUBLIC FUNCTIONS ---

// 1. Permanent Calculation
// Strict definition: if m > n, result is 0.
int64_t permanent(const int8_t *A, int m, int n) {
    if (m < 0 || n < 0) return 0;
    if (m == 0) return 1;
    if (!A) return 0;

    if (m > n) {
        return 0;
    }

    // From here on, we know m <= n.
    int target_n = n; 
    int diff = n - m; // Number of rows to add

    // Allocate memory for the padded matrix (stored TRANSPOSED)
    int8_t *padded_transposed = (int8_t*)malloc(target_n * target_n * sizeof(int8_t));
    if (!padded_transposed) return 0;
    
    // 1. Fill with ones (Masschelein padding)
    for (int i = 0; i < target_n * target_n; i++) padded_transposed[i] = 1;

    // 2. Copy original data A (m x n) into padded_transposed (n x n).
    // Store as A^T.
    for (int r = 0; r < m; r++) {
        for (int c = 0; c < n; c++) {
            padded_transposed[c * target_n + r] = A[r * n + c];
        }
    }

    // 3. Calculate
    int64_t res = fast_permanent_kernel(padded_transposed, target_n);
    free(padded_transposed);

    // 4. Normalize
    // The result of the padded matrix is P(A) * (n-m)!
    if (diff > 0) {
        res /= factorial(diff);
    }

    return res;
}

// 2. Exact Determinant (Bareiss)
int64_t determinant(const int8_t *A, int n) {
    if (n == 0) return 1;
    int64_t *M = (int64_t*)malloc(n * n * sizeof(int64_t));
    if (!M) return 0;
    
    // Copy to int64 workspace
    for (int i = 0; i < n * n; i++) M[i] = (int64_t)A[i];

    int64_t sign = 1;
    int64_t prev_pivot = 1; 

    for (int k = 0; k < n - 1; k++) {
        // Pivot finding
        if (M[k*n + k] == 0) {
            int swap_row = -1;
            for (int r = k + 1; r < n; r++) {
                if (M[r*n + k] != 0) { swap_row = r; break; }
            }
            if (swap_row == -1) { free(M); return 0; } // Singular
            
            // Swap rows
            for (int c = 0; c < n; c++) {
                int64_t temp = M[k*n + c];
                M[k*n + c] = M[swap_row*n + c];
                M[swap_row*n + c] = temp;
            }
            sign = -sign;
        }
        
        int64_t pivot = M[k*n + k];
        
        // Bareiss update
        for (int i = k + 1; i < n; i++) {       
            for (int j = k + 1; j < n; j++) {   
                int64_t term = M[i*n + j] * pivot - M[i*n + k] * M[k*n + j];
                // Integer division is exact in Bareiss algorithm
                M[i*n + j] = term / prev_pivot;
            }
        }
        prev_pivot = pivot;
    }
    
    int64_t result = M[(n-1)*n + (n-1)];
    free(M);
    return result * sign;
}

// 3. Ryser's Algorithm (Rectangular m x n)
// Modernized port of Spies (2006) code.
int64_t permanent_ryser(const int8_t *A, int m, int n) {
    if (m > n) return 0;

    int64_t total_sum = 0;
    
    int *c = (int *)malloc((n + 3) * sizeof(int));
    int8_t *col_selected = (int8_t *)calloc(n, sizeof(int8_t)); 

    if (!c || !col_selected) {
        if(c) free(c);
        if(col_selected) free(col_selected);
        return 0;
    }

    int64_t signum = 1;

    for (int i = 0; i < m; i++) {
        int t = m - i; // Subset size
        
        // --- Knuth Algorithm L ---
        for (int j = 1; j <= t; j++) c[j] = j - 1;
        c[t + 1] = n;
        c[t + 2] = 0;
        
        int j = 1;
        while (j <= t) {
            memset(col_selected, 0, n * sizeof(int8_t));
            
            for (int k = 1; k <= t; k++) {
                col_selected[c[k]] = 1;
            }

            int64_t prod = 1;
            for (int r = 0; r < m; r++) {
                int64_t row_sum = 0;
                for (int col = 0; col < n; col++) {
                    if (col_selected[col]) {
                        row_sum += A[r * n + col];
                    }
                }
                prod *= row_sum;
            }

            int64_t term = prod * signum * binomial(n - t, m - t);
            total_sum += term;

            j = 1;
            while (c[j] + 1 == c[j + 1]) {
                c[j] = j - 1;
                j++;
            }
            c[j] += 1;
        }
        signum = -signum;
    }

    free(c);
    free(col_selected);
    
    return total_sum;
}

// 4. Ryser's Algorithm (Rectangular m x n) - Optimized Gray Code
// ryser_new(A, m, n)
int64_t ryser_new(const int8_t *A, int m, int n) {
    if (m < 0 || n < 0) return 0;
    if (m == 0) return 1;
    if (m > n) return 0;
    if (!A) return 0;

    if (n == 0) return 0;
    if (n > 62) return 0; 

    int64_t *row_sums = (int64_t*)calloc((size_t)m, sizeof(int64_t));
    if (!row_sums) return 0;

    int64_t total = 0;
    uint64_t old_gray = 0;
    uint64_t limit = 1ULL << n;

    for (uint64_t i = 1; i < limit; i++) {
        uint64_t gray = i ^ (i >> 1);
        uint64_t bit  = gray ^ old_gray;
        int col = __builtin_ctzll(bit); 
        int turned_on = (gray & bit) != 0;
        int64_t delta = turned_on ? 1 : -1;

        // Update row sums
        for (int r = 0; r < m; r++) {
            row_sums[r] += delta * (int64_t)A[(size_t)r * (size_t)n + (size_t)col];
        }

        int k = __builtin_popcountll(gray); // |S|
        if (k <= m) {
            // Weight calculation (Exact Integer)
            int64_t w = binomial(n - k, m - k);
            if (((m - k) & 1) != 0) w = -w;

            int64_t prod = 1;
            for (int r = 0; r < m; r++) prod *= row_sums[r];

            total += w * prod;
        }
        old_gray = gray;
    }

    free(row_sums);
    return total;
}
