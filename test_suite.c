/*
 * test_suite.c
 * MERGED VERSION (English)
 *
 * 1. Strict Brute Force Verification (Mathematical Definition)
 * 2. Edge Cases & Signed Values
 * 3. Spies/Masschelein Identity Checks (J_mn)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <time.h> 
#include "permanent.h"

// --- Output Colors ---
#define GRN "\033[32m"
#define RED "\033[31m"
#define YEL "\033[33m"
#define RST "\033[0m"

static int failures = 0;

// ============================================================================
//  HELPERS: Brute Force Engine (The "Ground Truth")
// ============================================================================

// Helper: Calculate permanent of a SQUARE matrix via definition (slow!)
// Perm(A) = sum( a_1j * Perm(Minor_1j) )
int64_t bruteforce_square(int8_t *mat, int n, int stride) {
    if (n == 0) return 1; // Definition for empty matrix
    if (n == 1) return mat[0];
    
    int64_t sum = 0;
    // Expansion along the first row (row 0)
    for (int j = 0; j < n; j++) {
        // Create minor matrix (n-1 x n-1)
        int8_t *minor = malloc((n-1)*(n-1) * sizeof(int8_t));
        int p = 0;
        for (int r = 1; r < n; r++) {
            for (int c = 0; c < n; c++) {
                if (c == j) continue; // Skip column j
                minor[p++] = mat[r * stride + c];
            }
        }
        
        // Recursive call
        if (mat[0 * stride + j] != 0) {
            sum += mat[0 * stride + j] * bruteforce_square(minor, n-1, n-1);
        }
        free(minor);
    }
    return sum;
}

// Helper: Recursively generate column combinations for rectangular matrices
void combine_cols(int8_t *mat, int rows, int cols, int start_col, int depth, int *indices, int64_t *total_perm) {
    if (depth == rows) {
        // We have selected 'rows' columns. Create the square submatrix.
        int8_t *sub = malloc(rows * rows * sizeof(int8_t));
        for (int r = 0; r < rows; r++) {
            for (int k = 0; k < rows; k++) {
                sub[r * rows + k] = mat[r * cols + indices[k]];
            }
        }
        // Add permanent of this submatrix to total
        *total_perm += bruteforce_square(sub, rows, rows);
        free(sub);
        return;
    }

    for (int i = start_col; i < cols; i++) {
        indices[depth] = i;
        combine_cols(mat, rows, cols, i + 1, depth + 1, indices, total_perm);
    }
}

// The Definition of Permanent for rectangular matrix (M < N):
// Sum of permanents of all possible M x M submatrices.
int64_t bruteforce_reference(int8_t *mat, int rows, int cols) {
    if (rows == 0) return 1;
    if (rows > cols) return 0; // Library expects m <= n
    if (rows == cols) return bruteforce_square(mat, rows, cols);
    
    // Rectangular: Sum over all column combinations
    int64_t total = 0;
    int *indices = malloc(rows * sizeof(int));
    combine_cols(mat, rows, cols, 0, 0, indices, &total);
    free(indices);
    return total;
}

// Helper to check specific values
void check_val(const char *label, int64_t got, int64_t expected) {
    if (got == expected) {
        printf("  %-35s: " GRN "OK" RST " (%" PRId64 ")\n", label, got);
    } else {
        printf("  %-35s: " RED "FAIL" RST "\n", label);
        printf("    -> Got:      %" PRId64 "\n", got);
        printf("    -> Expected: %" PRId64 "\n", expected);
        failures++;
    }
}

// ============================================================================
//  TEST CATEGORIES
// ============================================================================

void test_edge_cases() {
    printf("\n[1] Edge Cases (0x0, 1x1)...\n");
    
    // 0x0 Matrix (Definition: 1)
    int8_t dummy = 0;
    check_val("Permanent 0x0", permanent(&dummy, 0, 0), 1);
    
    // 1x1 Matrix [5] -> 5
    int8_t m1[] = {5};
    check_val("Permanent 1x1 [5]", permanent(m1, 1, 1), 5);
    
    // 2x0 Matrix (Invalid, rows > cols) -> 0
    check_val("Permanent 2x0", permanent(&dummy, 2, 0), 0);
}

void test_signed_values() {
    printf("\n[2] Signed Integer Arithmetic...\n");
    
    // Matrix:
    // [ 1  -1 ]
    // [ 1   1 ]
    // Perm = 1*1 + (-1)*1 = 0
    int8_t M[] = {1, -1, 1, 1};
    check_val("Signed 2x2 (Res=0)", permanent(M, 2, 2), 0);
    
    // Matrix:
    // [ -2  -2 ]
    // [ -3  -4 ]
    // Perm = (-2*-4) + (-2*-3) = 8 + 6 = 14
    int8_t M2[] = {-2, -2, -3, -4};
    check_val("Signed 2x2 (Res=14)", permanent(M2, 2, 2), 14);
}

void test_random_validation(int rows, int cols) {
    // Generate random matrix (0/1)
    int8_t *mat = malloc(rows * cols * sizeof(int8_t));
    for(int i=0; i<rows*cols; i++) mat[i] = rand() % 2;

    // 1. Calculate GROUND TRUTH (Brute Force)
    int64_t truth = bruteforce_reference(mat, rows, cols);

    // 2. Calculate LIBRARY RESULT
    int64_t result = permanent(mat, rows, cols);

    // 3. Compare
    if (result == truth) {
        printf("  Random %dx%d %-23s: " GRN "OK" RST " (%" PRId64 ")\n", rows, cols, "", result);
    } else {
        printf("  Random %dx%d %-23s: " RED "FAIL" RST "\n", rows, cols, "");
        printf("    -> Lib:  %" PRId64 "\n", result);
        printf("    -> True: %" PRId64 "\n", truth);
        failures++;
    }
    free(mat);
}

void test_padding_identity(int m, int n) {
    // Test: Perm(J_mn) = n! / (n-m)!
    int8_t *J = malloc(m * n * sizeof(int8_t));
    for(int i=0; i<m*n; i++) J[i] = 1;
    
    int64_t res = permanent(J, m, n);
    
    // Calculate n! / (n-m)!
    int64_t expected = 1;
    for (int k = 0; k < m; k++) {
        expected *= (n - k);
    }
    
    char label[64];
    snprintf(label, 64, "All-Ones J_%dx%d", m, n);
    check_val(label, res, expected);
    
    free(J);
}

int main() {
    srand(42); // Fixed seed for reproducibility
    
    printf("==========================================\n");
    printf("   FULL LIBRARY TEST SUITE (Merged)       \n");
    printf("==========================================\n");

    // 1. Edge Cases & Signed
    test_edge_cases();
    test_signed_values();

    // 2. Mathematical Identities (J_mn)
    // Note: 10x22 triggers overflow in int64, using safe limit 8x15.
    printf("\n[3] Spies/Masschelein Identities...\n");
    test_padding_identity(5, 10);
    test_padding_identity(8, 15); 

    // 3. Random Matrices vs Brute Force Definition
    printf("\n[4] Brute Force Verification (Random Matrices)...\n");
    test_random_validation(3, 3);
    test_random_validation(4, 4);
    test_random_validation(10, 10); // Larger square
    
    test_random_validation(3, 4);   // Small rectangular
    test_random_validation(3, 5);
    test_random_validation(4, 6);
    test_random_validation(5, 10);  // Larger rectangular

    printf("\n------------------------------------------\n");
    if (failures == 0) {
        printf(GRN "ALL TESTS PASSED." RST "\n");
        return 0;
    } else {
        printf(RED "%d TESTS FAILED." RST "\n", failures);
        return 1;
    }
}
