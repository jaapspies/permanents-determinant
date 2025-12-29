#include <stdio.h>
#include <stdlib.h>
#include "permanent.h"

int main() {
    printf("--- Test Suite: Permanent & Determinant ---\n");

    // TEST 1: Rectangular Permanent
    // Matrix B (2x3):
    // 1 1 1
    // 1 2 3
    // Flattened representation:
    int8_t B[] = {1, 1, 1, 1, 2, 3};
    double p = permanent(B, 2, 3);
    printf("Permanent 2x3: %.0f (Expected: 12)\n", p);

    // TEST 2: Determinant (2x2)
    // Matrix D:
    // 2  1
    // 1  3
    // Det = (2*3) - (1*1) = 5
    int8_t D[] = {2, 1, 1, 3};
    printf("Determinant 2x2: %.0f (Expected: 5)\n", determinant(D, 2));

    // TEST 3: Identity Matrix (4x4)
    // Det should be 1
    int8_t Id4[] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    printf("Determinant I_4: %.0f (Expected: 1)\n", determinant(Id4, 4));
    
    // TEST 4: Singular Matrix
    // Rows are identical, Det should be 0
    int8_t Sing[] = {1, 1, 1, 1}; 
    printf("Determinant Singular: %.0f (Expected: 0)\n", determinant(Sing, 2));

    printf("--- Test Suite: Permanent (Spies vs Ryser) ---\n");

    // TEST 1: Rectangular Matrix 2x3
    // 1 1 1
    // 1 2 3
    int8_t E[] = {1, 1, 1, 1, 2, 3};
    
    double p_spies = permanent(E, 2, 3);
    double p_ryser = permanent_ryser(E, 2, 3);
    
    printf("Matrix 2x3:\n");
    printf("Spies/Masschelein : %.0f\n", p_spies);
    printf("Ryser (2006 port): %.0f\n", p_ryser);
    
    if (p_spies == p_ryser) printf("-> MATCH!\n\n");
    else printf("-> MISMATCH!\n\n");

    // TEST 2: Een grotere rechthoek (3x4)
    // 1 0 1 1
    // 0 1 1 1
    // 1 1 0 1
    int8_t C[] = {
        1, 0, 1, 1,
        0, 1, 1, 1,
        1, 1, 0, 1
    };
    
    p_spies = permanent(C, 3, 4);
    p_ryser = permanent_ryser(C, 3, 4);

    printf("Matrix 3x4:\n");
    printf("Spies/Masschelein : %.0f\n", p_spies);
    printf("Ryser (2006 port): %.0f\n", p_ryser);
    
    if (p_spies == p_ryser) printf("-> MATCH!\n\n");
    else printf("-> MISMATCH!\n");

    return 0;

    return 0;
}
