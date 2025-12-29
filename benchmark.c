#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <omp.h>
#include "permanent.h"

// --- DE "OUDE" SITUATIE (NAÏEF) ---
// Dit simuleert hoe de code zou werken zonder de Transpose-truc.
// De CPU moet hier springen (stride) door het geheugen om kolommen op te tellen.
double naive_permanent_simulation(const int8_t *matrix, int n) {
    int64_t *row_sums = (int64_t*)malloc(n * sizeof(int64_t));
    
    // Init (ook hier al springen we door het geheugen, maar dat is eenmalig)
    for (int r = 0; r < n; r++) {
        row_sums[r] = 0;
        for (int c = 0; c < n; c++) row_sums[r] += matrix[r*n + c];
    }

    double total = 0;
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

        // --- DE VERTRAAGDE INNER LOOP ---
        // Hier moeten we de 'col_idx'-de kolom pakken.
        // matrix[r*n + col_idx]. 
        // Bij elke stap van r springen we 'n' posities verder in geheugen.
        // Dit breekt vectorisatie en cache-prefetching.
        for (int r = 0; r < n; r++) {
            row_sums[r] += matrix[r*n + col_idx] * direction;
        }

        current_sign = -current_sign;
        old_gray = new_gray;
    }

    free(row_sums);
    return total / (double)num_loops;
}

int main() {
    int n = 30; // Groot genoeg om pijn te doen, klein genoeg om te wachten
    printf("--- SPEED TEST (Matrix size: %dx%d) ---\n", n, n);
    printf("Generating random matrix...\n");

    int8_t *A = (int8_t*)malloc(n * n * sizeof(int8_t));
    for(int i=0; i<n*n; i++) A[i] = (rand() % 3) - 1; // -1, 0, 1

    // 1. TEST NAÏEF
    printf("Running Naive Spies (Column Strides)...\n");
    double start = omp_get_wtime();
    double res1 = naive_permanent_simulation(A, n);
    double end = omp_get_wtime();
    double time_naive = end - start;
    printf(" -> Result: %.0f\n", res1);
    printf(" -> Time:   %.4f seconds\n\n", time_naive);

    // 2. TEST OPTIMALISATIE (uit permanent.c)
    printf("Running Optimized Spies (Transposed Access)...\n");
    start = omp_get_wtime();
    // We roepen de functie uit jouw library aan
    double res2 = permanent(A, n, n); 
    end = omp_get_wtime();
    double time_opt = end - start;
    printf(" -> Result: %.0f\n", res2);
    printf(" -> Time:   %.4f seconds\n", time_opt);

    // CONCLUSIE
    printf("\n--- RESULTS ---\n");
    printf("Speedup factor: %.2fx\n", time_naive / time_opt);
    if (time_naive / time_opt > 1.5) {
        printf("Verdict: Optimization SUCCESSFUL. Transposition paid off.\n");
    } else {
        printf("Verdict: Difference negligible (try larger n).\n");
    }

    free(A);
    return 0;
}
