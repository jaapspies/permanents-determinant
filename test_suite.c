#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "permanent.h"

/* Als je ryser_new() nog niet in permanent.h hebt gezet, houdt dit de build werkend. */
double ryser_new(const int8_t *A, int m, int n);

static int failures = 0;

static void check_eq_d(const char *label, double got, double expected) {
    if (got != expected) {
        printf("FAIL: %s: got %.0f, expected %.0f\n", label, got, expected);
        failures++;
    } else {
        printf("OK  : %s: %.0f\n", label, got);
    }
}

static void check_match3(const char *label, double a, double b, double c) {
    if (a == b && b == c) {
        printf("OK  : %s: %.0f (all match)\n", label, a);
    } else {
        printf("FAIL: %s: spies=%.0f ryser2006=%.0f ryser_new=%.0f\n", label, a, b, c);
        failures++;
    }
}

/* --- brute force permanent voor héél kleine maten (portable C) --- */
typedef struct {
    const int8_t *A;
    int m, n;
    int chosen[12];   /* gekozen kolommen (size m) */
    int pick[12];     /* permutatie van gekozen kolommen (size m) */
    long long total;
} BFContext;

/* Heap's algorithm */
static void bf_permute(BFContext *ctx, int k) {
    if (k == 1) {
        long long prod = 1;
        for (int r = 0; r < ctx->m; r++) {
            prod *= (long long)ctx->A[r * ctx->n + ctx->pick[r]];
        }
        ctx->total += prod;
        return;
    }
    bf_permute(ctx, k - 1);
    for (int i = 0; i < k - 1; i++) {
        int a = (k % 2 == 0) ? i : 0;
        int tmp = ctx->pick[a];
        ctx->pick[a] = ctx->pick[k - 1];
        ctx->pick[k - 1] = tmp;
        bf_permute(ctx, k - 1);
    }
}

static void bf_choose(BFContext *ctx, int start, int depth) {
    if (depth == ctx->m) {
        for (int i = 0; i < ctx->m; i++) ctx->pick[i] = ctx->chosen[i];
        bf_permute(ctx, ctx->m);
        return;
    }
    for (int col = start; col <= ctx->n - (ctx->m - depth); col++) {
        ctx->chosen[depth] = col;
        bf_choose(ctx, col + 1, depth + 1);
    }
}

static long long perm_bruteforce(const int8_t *A, int m, int n) {
    if (m < 0 || n < 0) return 0;
    if (m == 0) return 1;
    if (m > n) return 0;
    if (n > 12) return 0; /* buffers zijn fixed */

    BFContext ctx;
    ctx.A = A;
    ctx.m = m;
    ctx.n = n;
    ctx.total = 0;
    bf_choose(&ctx, 0, 0);
    return ctx.total;
}

int main(void) {
    printf("--- Test Suite: Permanent & Determinant ---\n");

    /* Grensgevallen / definities */
    {
        double p00 = permanent(NULL, 0, 0);
        check_eq_d("permanent(0x0)=1", p00, 1.0);

        double p0n = permanent(NULL, 0, 5);
        check_eq_d("permanent(0x5)=1", p0n, 1.0);

        int8_t dummy = 7;
        double pn0 = permanent(&dummy, 2, 0);
        check_eq_d("permanent(2x0)=0", pn0, 0.0);

        double p_mgt = permanent(&dummy, 3, 2);
        check_eq_d("permanent(m>n)=0", p_mgt, 0.0);
    }

    /* TEST 1: rechthoek 2x3 */
    {
        int8_t B[] = {1, 1, 1,
                      1, 2, 3};
        double p = permanent(B, 2, 3);
        check_eq_d("Permanent 2x3 (B)", p, 12.0);
    }

    /* Determinant tests */
    {
        int8_t D[] = {2, 1,
                      1, 3};
        check_eq_d("Determinant 2x2", determinant(D, 2), 5.0);

        int8_t Id4[] = {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        };
        check_eq_d("Determinant I4", determinant(Id4, 4), 1.0);

        int8_t Sng[] = {1, 2,
                        2, 4};
        check_eq_d("Determinant singular", determinant(Sng, 2), 0.0);
    }

    printf("\n--- Test Suite: Permanent (Spies vs Ryser2006 vs Ryser_new) ---\n");

    /* TEST A: 2x3 (B) */
    {
        int8_t B[] = {1, 1, 1,
                      1, 2, 3};
        double p_spies = permanent(B, 2, 3);
        double p_ryser = permanent_ryser(B, 2, 3);
        double p_new   = ryser_new(B, 2, 3);
        check_match3("Compare 2x3 (B)", p_spies, p_ryser, p_new);
    }

    /* TEST B: 3x4 (C) */
    {
        int8_t C[] = {
            1, 0, 1, 1,
            0, 1, 1, 1,
            1, 1, 0, 1
        };
        double p_spies = permanent(C, 3, 4);
        double p_ryser = permanent_ryser(C, 3, 4);
        double p_new   = ryser_new(C, 3, 4);
        check_match3("Compare 3x4 (C)", p_spies, p_ryser, p_new);
    }

    /* Random mini-tests vs brute force */
    printf("\n--- Random small tests vs brute-force ---\n");
    srand(1);
    for (int tcase = 0; tcase < 40; tcase++) {
        int m = 1 + (rand() % 4);      /* 1..4 */
        int n = m + (rand() % 3);      /* m..m+2 (<=6) */

        int8_t A[4 * 6];
        for (int i = 0; i < m * n; i++) {
            A[i] = (int8_t)((rand() % 3) - 1);  /* -1,0,1 */
        }

        long long bf = perm_bruteforce(A, m, n);
        double p_spies = permanent(A, m, n);
        double p_ryser = permanent_ryser(A, m, n);
        double p_new   = ryser_new(A, m, n);

        char label[64];
        snprintf(label, sizeof(label), "random %dx%d #%d", m, n, tcase);

        if ((long long)p_spies != bf || (long long)p_ryser != bf || (long long)p_new != bf) {
            printf("FAIL: %s: brute=%lld spies=%.0f ryser2006=%.0f ryser_new=%.0f\n",
                   label, bf, p_spies, p_ryser, p_new);
            failures++;
        } else {
            printf("OK  : %s: %lld\n", label, bf);
        }
    }

    printf("\nSummary: %s (%d failures)\n", failures ? "FAIL" : "PASS", failures);
    return failures ? 1 : 0;
}
