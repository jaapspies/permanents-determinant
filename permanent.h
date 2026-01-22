#ifndef PERMANENT_H
#define PERMANENT_H

#include <stdint.h>

/*
 * Calculates the permanent of an m x n matrix (A).
 * * Features:
 * - Supports rectangular matrices (m != n) via the Masschelein extension (padding).
 * - Uses Spies' algebraic formula (2003-2006) for the underlying square calculation.
 * - Multithreaded execution (OpenMP) if compiled with -fopenmp.
 * * Input:
 * - matrix A must be provided as a flattened 1D array (row-major order).
 * - Elements are 8-bit integers (ideal for 0/1 matrices).
 * * Returns:
 * - The permanent as a 64-bit integer (int64_t).
 */
int64_t permanent(const int8_t *A, int m, int n);

/*
 * Calculates the permanent using the classic Ryser Algorithm (Brualdi & Ryser Th 7.1.1).
 * Adapted from Spies' ryser.c (2006).
 * * Note: Best for matrices where m is significantly smaller than n.
 * Returns int64_t for exact integer precision.
 */
int64_t permanent_ryser(const int8_t *A, int m, int n);

/*
 * Brualdi–Ryser formula (rectangular, m <= n), optimized implementation.
 * Uses Gray-code traversal of subsets and incremental row-sum updates.
 * Returns int64_t for exact integer precision.
 */
int64_t ryser_new(const int8_t *A, int m, int n);

/*
 * Calculates the exact determinant using the Bareiss Algorithm.
 * * Features:
 * - Performs exact integer arithmetic (fraction-free Gaussian elimination).
 * - Ideal for (-1, 0, 1) matrices where precision is paramount.
 * - Returns int64_t. NOTE: Determinants grow faster than permanents; 
 * check for overflow on large n > 20.
 */
int64_t determinant(const int8_t *A, int n);

#endif
