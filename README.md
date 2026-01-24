# Permanent & Determinant Library (High-Performance C)

**Author:** Jaap Spies  
**Date:** January 2006  
**License:** GPLv3

## Overview

This repository contains a highly optimized C library for calculating the **Permanent** and **Determinant** of (0,1)-matrices and other integer matrices.
The library implements **exact integer arithmetic** (`int64_t`) to avoid floating-point errors. The library utilizes **OpenMP** for multithreading and **SIMD** instructions for vectorization. It includes **Python bindings** for easy integration with NumPy.

## Features & Algorithms
The distinguishing feature of this library is its approach to calculating the permanent:

* **Fast Permanent Calculationa (Rectangular):**
    * Implementation of the **Spies' Formula** using Gray Codes.
    * Function call: int64_t permanent(const int8_t *A, int m, int n);
    * Padding rows to match columns (Masschelein).
    * Complexity: $O(n 2^n)$.

* **Permanent Calculation (Rectangular):**
    * Implementation of the **Brualdi–Ryser Algorithm** for rectangular permanents.
    * Legacy reference implementation (2006 port).
    * Function call: int64_t permanent_ryser(const int8_t *A, int m, int n);
    * Optimized for rectangular matrices where $m < n$.
    * Complexity: $O(m \cdot \sum_{k=1}^m \binom{n}{k})$. 


* **Fast Permanent Calculation (Rectangular, Optimized):**
    * Optimized implementation of the **Brualdi–Ryser Algorithm** using **Gray-code traversal**
      and incremental row-sum updates.
    * Function call: int64_t ryser_new(const int8_t *A, int m, int n);
    * Gray-code traversal over all subsets. Best for matrices where $m \approx n$.
    * Complexity: $O(m \cdot 2^n)$.

* **Exact Determinant:**
    * Implementation of the **Bareiss Algorithm** (fraction-free Gaussian elimination) for exact integer results.
    * Function call: int64_t determinant(const int8_t *A, int n);

## Mathematical Formulation

The core algorithm for calculating the permanent in this library is based on the **Spies Formula**, extended for rectangular matrices using the **Masschelein** padding technique.

While the implementation uses bitwise optimizations (Gray codes) and processes the transpose of the matrix for cache efficiency ($A^T$), the underlying mathematical logic is defined as follows.

### The Spies/Masschelein Formula

Let $A$ be an $m \times n$ matrix with $m \le n$. We associate a variable $x_j \in \{-1, 1\}$ with each column $j$ of the matrix. The permanent is calculated by summing over all $2^{n-1}$ vectors $x$ (where $x_1 = 1$ is fixed due to symmetry).

$$
\text{per}(A) = \frac{1}{(n-m)! \cdot 2^{n-1}} \sum_{\substack{x \in \{-1, 1\}^n \\ x_1=1}} \left[ \left(\prod_{j=1}^n x_j\right) \cdot \underbrace{\left( \prod_{i=1}^m \sum_{j=1}^n a_{i,j} x_j \right)}_{\text{Original Rows}} \cdot \underbrace{\left( \sum_{j=1}^n x_j \right)^{n-m}}_{\text{Virtual (Padded) Rows}} \right]
$$

#### Derivation of Terms:

1.  **The Variables ($x_j$):** The algorithm iterates through state vectors $x$. In the C implementation, this iteration is optimized using a Gray code sequence to update sums incrementally.
2.  **Original Rows:** For the first $m$ rows, we compute the product of the weighted row sums.
3.  **Virtual Rows (Masschelein Extension):** To treat the rectangular matrix as square, $n-m$ rows of ones are virtually appended. The row sum for such a row is simply $\sum x_j$. Since these rows are identical, this results in the term $(\sum x_j)^{n-m}$.
4.  **Normalization:**
    * The factor $\frac{1}{(n-m)!}$ corrects for the permutation symmetries introduced by the identical padded rows.

## Performance

The library has been benchmarked against legacy Ryser implementations.
**Test System:** Single Thread performance (Standard CPU).

| Matrix Type | Size | Legacy Implementation | **This Library** | **Speedup** |
| :--- | :--- | :--- | :--- | :--- |
| **Square** | 20 x 20 | 0.2046 s | **0.0063 s** | **32x** |
| **Rectangular** | 8 x 16 | 0.0070 s | **0.0002 s** | **40x** |

*Benchmarks ran on random {-1, 0, 1} matrices. Results are validated for exact correctness.*


##  OEIS Results (New for 2025)

Using this software, two sequences that had been stagnant since 2003 were extended to $N=7$ on December 28-29, 2025.

### 1. Sequence A089475
**Definition:** Number of different values taken by the permanent of a real **nonsingular** (0,1)-matrix of order $n$.

* **Previous terms (N=1..6):** 1, 1, 3, 9, 31, 149
* **New Result (N=7):** `888`
* **Methodology:** Backtracking with Real Rank Pruning.


### 2. Sequence A089476
**Definition:** Number of different values taken by the permanent of a real **singular** (0,1)-matrix of order $n$.

* **Previous terms (N=1..6):** 1, 2, 4, 10, 32, 136
* **New Result (N=7):** `700`
* **Methodology:** Exhaustive search over sorted rows (combinations with replacement) combined with a fast Bareiss Determinant check.

---

## Building the Software
### Requirements
* GCC (with OpenMP support)
* Make
* Python 3 + NumPy (optional, for Python bindings)

### Compilation

To build the standard C executables (test suite, benchmarks, and OEIS generators):
```bash
make clean
make all
```

To build the **Python Shared Library** (`libpermanent.so`):
```bash
make lib
```

## Python Usage

The library includes a Python wrapper (`permanent_lib.py`) for high-performance calculations directly from NumPy.

1.  Build the library: `make lib`
2.  Run the example: `python3 example.py`
3.  Use in your own code:

```python
import numpy as np
from permanent_lib import permanent

# Create a random 15x20 matrix with entries {-1, 0, 1}
A = np.random.randint(-1, 2, size=(15, 20))

# Calculate Permanent
# Default algorithm is 'padding' (Spies/Masschelein) as it is the fastest.
p = permanent(A)

print(f"Permanent: {p}")
```


## File Structure
* **Core C Library:**
    * `permanent.c` / `permanent.h`: Implementation of Spies/Masschelein logic.
    * `libpermanent.so`: Compiled shared object.
* **Python Integration:**
    * `permanent_lib.py`: Ctypes wrapper.
    * `example.py`: Usage examples.
    * `test_lib.py`: Verification suite.
* **Legacy / Reference:**
    * `oeis_a089475.c`: Nonsingular matrices (N=7 reference).
    * `oeis_a089476.c`: Singular matrices (N=7 reference).

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
6.  **OEIS A089475 & A089476**.

## License

This project is licensed under the GNU General Public License v3.0 (GPLv3).
See the `LICENSE` file for the full text.
