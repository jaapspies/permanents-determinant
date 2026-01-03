# Permanent & Determinant Library (High-Performance C)

**Author:** Jaap Spies  
**Date:** December 2025  
**License:** GPLv3

## Overview

This repository contains a highly optimized C library for calculating the **Permanent** and **Determinant** of (0,1)-matrices and other integer matrices. The primary goal of this software is to extend integer sequences in the **OEIS** (On-Line Encyclopedia of Integer Sequences) related to matrix permanents.

The library utilizes **OpenMP** for multithreading and **SIMD** instructions for vectorization, allowing it to process billions of matrices per hour on modern hardware.

## Features & Algorithms

* **Fast Permanent Calculation:**
    * Implementation of the **Spies' Formula** using Gray Codes.
    * Function call: double permanent(const int8_t *A, int m, int n);
    * Padding rows to match columns (Masschelein).
    * Optimized for dense matrices ($m \approx n$).
    * Complexity: $O(n 2^n)$.

* **Fast Permanent Calculation (Rectangular):**
    * Implementation of the **Brualdi–Ryser Algorithm** for rectangular permanents.
    * Legacy reference implementation (2006 port).
    * Function call: double permanent_ryser(const int8_t *A, int m, int n);
    * Optimized for rectangular matrices where $m < n$.
    * Complexity: $O(m \cdot \sum_{k=1}^m \binom{n}{k})$. 
      *(Approximates $O(m \cdot \binom{n}{m})$ for small $m$, and $O(m \cdot 2^n)$ when $m=n$)*.


* **Fast Permanent Calculation (Rectangular, Optimized):**
    * Optimized implementation of the **Brualdi–Ryser Algorithm** using **Gray-code traversal**
      and incremental row-sum updates.
    * Function call: double ryser_new(const int8_t *A, int m, int n);
    * Gray-code traversal over all subsets. Best for matrices where $m \approx n$.
    * Complexity: $O(m \cdot 2^n)$.

* **Exact Determinant:**
    * Implementation of the **Bareiss Algorithm** (fraction-free Gaussian elimination) for exact integer results.
    * Function call: double determinant(const int8_t *A, int n);

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
    *Estimated time:* ~4 hours on a 24-core machine.

### Final Verification (Overlap Check)
To reproduce the set analysis and confirm the overlap of 409:
1. Ensure the CSV output files from the previous steps exist.
2. Run the provided Python script:
   ```bash
   python3 final_check_n7.py
   ```

## Consistency Check

### Verification Logic (N=7)
We used the known total from sequence **A088672** (Total distinct permanent values) to verify our results.

* **A (Nonsingular)**: 888 (Calculated via `oeis_a089475`)
* **B (Singular)**: 700 (Calculated via `oeis_a089476`)
* **Union (Total)**: 1179 (Known value from A088672)

Using the Inclusion-Exclusion Principle ($|A \cup B| = |A| + |B| - |A \cap B|$):
> 1179 = 888 + 700 - **409**

**Conclusion:** The overlap (values found in both singular and nonsingular matrices) is **409**. The exact match with the known total confirms the correctness of both search programs.

## File Structure

* `permanent.c` / `permanent.h`: Core library implementation.
* `oeis_a089475.c`: Specialized searcher for nonsingular matrices (Rank Pruning).
* `oeis_a089476.c`: Specialized searcher for singular matrices (Determinant Check).
* `test_suite.c`: Unit tests.

## References

The algorithms and formulas implemented in this library are based on the following mathematical publications and theses:

1.  **Brualdi, R. A., & Ryser, H. J.** (1991). *Combinatorial Matrix Theory*. Cambridge University Press.
    * *The definitive text on combinatorial matrix theory and the source of the Brualdi-Ryser algorithm for rectangular permanents.*

2.  **Spies, J.** (2006). Dancing School problems. *Nieuw Archief voor Wiskunde*, 5/7, 283–284.
    * *First publication of the underlying combinatorial principles used in this library, including the first published appearence of Spies' Formula*

3.  **Spies, J.** (2020). A formula for the permanent. *Nieuw Archief voor Wiskunde*, 5/21(1).
    * *Formal derivation of Spies' Formula for calculating the permanent.*

4.  **Masschelein, C.** (2024). *Efficient Evaluation of Rectangular Matrix Permanents* (Master's Thesis). McMaster University, Hamilton, ON, Canada.
    * *Source for the rectangular extension (padding) techniques.*

5.  **Wikipedia**. [Permanent (wiskunde)](https://nl.wikipedia.org/wiki/Permanent_(wiskunde)).
    * *General background on the permanent and standard algorithms (Dutch).*

## License

This project is licensed under the GNU General Public License v3.0 (GPLv3).
See the `LICENSE` file for the full text.
