# Permanent & Determinant Library (High-Performance C)

**Author:** Jaap Spies  
**Date:** December 2025  
**License:** GPLv3

## Overview

This repository contains a highly optimized C library for calculating the **Permanent** and **Determinant** of (0,1)-matrices (and other integer matrices). The primary goal of this software is to extend integer sequences in the **OEIS** (On-Line Encyclopedia of Integer Sequences) related to matrix permanents.

The library utilizes **OpenMP** for multithreading and **SIMD** instructions for vectorization, allowing it to process billions of matrices per hour on modern hardware.

## Features & Algorithms

* **Fast Permanent Calculation:**
    * Implementation of **Spies' Formula** using Gray Codes.
    * Function call: double permanent(const int8_t *A, int m, int n);
    * Optimized for dense matrices ($m \approx n$).
    * Complexity: $O(n 2^n)$.
* **Fast Permanent Calculation (Rectangular):**
    * Implementation of the **Ryser/Brualdi Algorithm** (2006 port).
    * Function call: double permanent_ryser(const int8_t *A, int m, int n);
    * Optimized for rectangular matrices where $m < n$.
    * Complexity: $O(2^m \cdot poly(n))$.
* **Exact Determinant:**
    * Implementation of the **Bareiss Algorithm** (fraction-free Gaussian elimination) for exact integer results.
    * Function call: double determinant(const int8_t *A, int m, int n);

##  OEIS Results (New for 2025)

Using this software, two sequences that had been stagnant since 2003 were extended to $N=7$ on December 28-29, 2025.

### 1. Sequence A089475
**Definition:** Number of different values taken by the permanent of a real **nonsingular** (0,1)-matrix of order $n$.

* **Previous terms (N=1..6):** 1, 1, 3, 9, 31, 149
* **New Result (N=7):** `888`
* **Methodology:** Backtracking with Real Rank Pruning (using Gaussian elimination on doubles) and strict Row Sorting to eliminate permutations.
* **Stats:** Checked **53,640,013,886** unique row-space configurations.

### 2. Sequence A089476
**Definition:** Number of different values taken by the permanent of a real **singular** (0,1)-matrix of order $n$.

* **Previous terms (N=1..6):** 1, 2, 4, 10, 32, 136
* **New Result (N=7):** `700`
* **Methodology:** Exhaustive search over sorted rows (combinations with replacement) combined with a fast Bareiss Determinant check.
* **Stats:** Checked **77,614,474,050** canonical singular matrices.

---

## Building the Software

### Requirements
* GCC (with OpenMP support)
* Make

### Compilation
To build all tools and searchers:

```bash
make
```
## Verification Instructions

To reproduce the N=7 results, follow these steps. 
**Warning:** These calculations require significant computational resources.

### Verifying A089475 (Nonsingular)
1.  Ensure `oeis_a089475.c` is configured with `#define N 7`.
2.  Run the search:
    ```bash
    ./oeis_a089475
    ```
    *Estimated time:* ~3 hours on a 24-core machine.

### Verifying A089476 (Singular)
1.  Ensure `oeis_a089476.c` is configured with `#define N 7`.
2.  Run the search:
    ```bash
    ./oeis_a089476
    ```
    *Estimated time:* ~4.5 hours on a 24-core machine.

## Mathematical Consistency Check

The results found for N=7 are consistent with sequence **A087983** (Total distinct permanent values for all matrices).

* |A| (Nonsingular, A089475) = 888
* |B| (Singular, A089476) = 700
* |A U B| (Total, A087983) = 1179 (Known value)

Applying the inclusion-exclusion principle: 1179 = 888 + 700 - Overlap.
The overlap (values occurring in both singular and nonsingular matrices) is **409**. This consistency strongly suggests the results are correct.

## File Structure

* `permanent.c` / `permanent.h`: Core library implementation.
* `oeis_a089475.c`: Specialized searcher for nonsingular matrices (Rank Pruning).
* `oeis_a089476.c`: Specialized searcher for singular matrices (Determinant Check).
* `test_suite.c`: Unit tests.
* `benchmark.c`: Performance testing.

## License

This project is licensed under the GNU General Public License v3.0 (GPLv3).
See the `LICENSE` file for the full text.
