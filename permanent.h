#ifndef PERMANENT_H
#define PERMANENT_H

#include <stdint.h>

/*
 * Calculates the permanent of an m x n matrix (A).
 * * Features:
 * - Supports rectangular matrices (m != n) via the Masschelein extension (padding).
 * - Uses Spies' algebraic formula (2006) for the underlying square calculation.
 * - Multithreaded execution (OpenMP) if compiled with -fopenmp.
 * * Input:
 * - matrix A must be provided as a flattened 1D array (row-major order).
 */
double permanent(const int8_t *A, int m, int n);

/*
 * Calculates the permanent using the classic Ryser Algorithm (Brualdi & Ryser Th 7.1.1).
 * Adapted from Spies' ryser.c (2006).
 * Best for matrices where m is significantly smaller than n.
 */
double permanent_ryser(const int8_t *A, int m, int n);

/* * Calculates the exact determinant using the Bareiss Algorithm.
 * * Features:
 * - Performs exact integer arithmetic (fraction-free Gaussian elimination).
 * - Ideal for (-1, 0, 1) matrices where precision is paramount.
 * - Returns a double to accommodate values up to ~15 digits (safe for standard n).
 */
double determinant(const int8_t *A, int n);

#endif
