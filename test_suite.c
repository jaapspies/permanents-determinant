#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include "permanent.h"

// Kleuren voor output
#define GRN "\033[32m"
#define RED "\033[31m"
#define YEL "\033[33m"
#define RST "\033[0m"

static int failures = 0;

// ============================================================================
//  1. BRUTE FORCE REFERENTIE (De 'Waarheid')
// ============================================================================

// Hulpfunctie: Bereken permanent van een VIERKANTE matrix via definitie (traag!)
// Perm(A) = som( a_1j * Perm(Minor_1j) )
int64_t bruteforce_square(int8_t *mat, int n, int stride) {
    if (n == 1) return mat[0];
    
    int64_t sum = 0;
    // Expansie langs eerste rij (rij 0)
    for (int j = 0; j < n; j++) {
        // Maak minor matrix (n-1 x n-1)
        int8_t *minor = malloc((n-1)*(n-1) * sizeof(int8_t));
        int p = 0;
        for (int r = 1; r < n; r++) {
            for (int c = 0; c < n; c++) {
                if (c == j) continue; // Sla kolom j over
                minor[p++] = mat[r * stride + c];
            }
        }
        
        // Recursie
        if (mat[0 * stride + j] != 0) {
            sum += mat[0 * stride + j] * bruteforce_square(minor, n-1, n-1);
        }
        free(minor);
    }
    return sum;
}

// Hulpfunctie: recursief combinaties genereren van kolommen
void combine_cols(int8_t *mat, int rows, int cols, int start_col, int depth, int *indices, int64_t *total_perm) {
    if (depth == rows) {
        // We hebben nu 'rows' kolommen gekozen. Maak de vierkante submatrix.
        int8_t *sub = malloc(rows * rows * sizeof(int8_t));
        for (int r = 0; r < rows; r++) {
            for (int k = 0; k < rows; k++) {
                sub[r * rows + k] = mat[r * cols + indices[k]];
            }
        }
        // Tel de permanent van deze submatrix op bij het totaal
        *total_perm += bruteforce_square(sub, rows, rows);
        free(sub);
        return;
    }

    for (int i = start_col; i < cols; i++) {
        indices[depth] = i;
        combine_cols(mat, rows, cols, i + 1, depth + 1, indices, total_perm);
    }
}

// De definitie van Permanent voor rechthoekige matrix (M < N):
// Som van permanenten van alle mogelijke M x M submatrices.
int64_t bruteforce_reference(int8_t *mat, int rows, int cols) {
    if (rows > cols) {
        // Transposeer virtueel door recursie andersom te doen, of return 0?
        // De library verwacht rows <= cols.
        return 0; 
    }
    if (rows == cols) {
        return bruteforce_square(mat, rows, cols);
    }
    
    // Rechthoekig: Sommeer over alle combinaties van kolommen
    int64_t total = 0;
    int *indices = malloc(rows * sizeof(int));
    combine_cols(mat, rows, cols, 0, 0, indices, &total);
    free(indices);
    return total;
}

// ============================================================================
//  2. TEST RUNNER
// ============================================================================

void run_test(int rows, int cols, const char* name) {
    printf("TEST: %s (%dx%d) -> ", name, rows, cols);
    
    // Maak random matrix (0 en 1)
    int8_t *mat = malloc(rows * cols * sizeof(int8_t));
    for(int i=0; i<rows*cols; i++) mat[i] = rand() % 2; // 0 of 1

    // 1. Bereken de WAARHEID (Brute Force)
    //    Alleen doen als matrix klein is, anders duurt het eeuwen
    int64_t truth = -1;
    int skip_bf = (cols > 12); // Grens voor brute force
    
    if (!skip_bf) {
        truth = bruteforce_reference(mat, rows, cols);
    }

    // 2. Bereken JOUW LIBRARY (Optimized)
    int64_t result = permanent(mat, rows, cols);

    // 3. Vergelijk
    if (skip_bf) {
        printf(YEL "SKIPPED BF (Too big)" RST " | Lib: %" PRId64 "\n", result);
    } else {
        if (result == truth) {
            printf(GRN "MATCH" RST " (Val: %" PRId64 ")\n", result);
        } else {
            printf(RED "FAIL" RST "\n");
            printf("   Echt (Definitie): %" PRId64 "\n", truth);
            printf("   Jouw Library:     %" PRId64 "\n", result);
            printf("   Verschil factor:  %.2f\n", (double)result / (double)truth);
            failures++;
        }
    }
    free(mat);
}

void test_padding_identity(int m, int n) {
    // Test de formule: Perm(J_mn) = n! / (n-m)!
    // Met 0/1 matrix vol enen
    printf("TEST: All-Ones %dx%d -> ", m, n);
    
    int8_t *J = malloc(m * n * sizeof(int8_t));
    for(int i=0; i<m*n; i++) J[i] = 1;
    
    int64_t res = permanent(J, m, n);
    
    // Bereken n! / (n-m)!
    int64_t expected = 1;
    for (int k = 0; k < m; k++) {
        expected *= (n - k);
    }
    
    if (res == expected) {
        printf(GRN "MATCH" RST " (%" PRId64 ")\n", res);
    } else {
        printf(RED "FAIL" RST "\n");
        printf("   Verwacht (n!/(n-m)!): %" PRId64 "\n", expected);
        printf("   Jouw Library:         %" PRId64 "\n", res);
        failures++;
    }
    free(J);
}

int main() {
    srand(42); // Vaste seed voor herhaalbaarheid
    
    printf("==========================================\n");
    printf("   STRICT MATHEMATICAL VALIDATION SUITE   \n");
    printf("==========================================\n");

    // 1. Vierkante tests
    run_test(4, 4, "Small Square");
    run_test(10, 10, "Medium Square");

    // 2. Rechthoekige tests (Hier ging het mis met Masschelein)
    //    Nu vergelijken we met de ECHTE definitie, niet met padding-trucs
    run_test(3, 4, "Rect 3x4");
    run_test(3, 5, "Rect 3x5");
    run_test(4, 6, "Rect 4x6");
    
    // 3. De 10x22 test (maar dan kleiner voor brute force verificatie)
    //    Laten we 5x10 proberen, dat is nog net te brute forcen
    run_test(5, 10, "Rect 5x10");

    // 4. Formule controle
    test_padding_identity(5, 10);
    test_padding_identity(8, 15);

    printf("\n------------------------------------------\n");
    if (failures == 0) {
        printf(GRN "ALL TESTS PASSED." RST "\n");
        return 0;
    } else {
        printf(RED "%d TESTS FAILED." RST "\n", failures);
        return 1;
    }
}
