import numpy as np
import time
import math
from permanent_lib import permanent, determinant

def main():
    print("--- Demo: High Performance Permanent Library (Masschelein/Integer) ---")

    # 1. Correctness Test (J_12)
    # J_12 = 12! = 479,001,600
    n = 12
    print(f"\nTest 1: Calculating J_{n} (Matrix of ones)...")
    J = np.ones((n, n), dtype=int)

    start = time.time()
    # Implicitly uses algorithm="padding"
    res = permanent(J) 
    dt = time.time() - start

    expected = math.factorial(n)
    
    print(f"Result:   {res}")
    print(f"Expected: {expected}")
    print(f"Time:     {dt:.6f} sec")

    if res == expected:
        print(">> VALIDATION: \033[92mOK\033[0m")
    else:
        print(f">> VALIDATION: \033[91mFAILED\033[0m (Diff: {res - expected})")

    # 2. Performance & Algorithm Comparison
    # Note: Ryser is typically faster for m << n, 
    # but for square matrices, Spies (padding) is often optimized.
    rows, cols = 15, 20
    print(f"\nTest 2: Performance on random {rows}x{cols} (-1, 0, 1) matrix...")
    
    # Generate random matrix with -1, 0, 1
    R = np.random.randint(-1, 2, size=(rows, cols))

    # A. Run with Padding (Masschelein extension)
    start = time.time()
    p_pad = permanent(R, algorithm="padding")
    dt_pad = time.time() - start
    print(f"Method: Padding (Massch) | Result: {p_pad:<20} | Time: {dt_pad:.6f} sec")

    # B. Run with Ryser (Explicit)
    start = time.time()
    p_rys = permanent(R, algorithm="ryser")
    dt_rys = time.time() - start
    print(f"Method: Ryser (New)      | Result: {p_rys:<20} | Time: {dt_rys:.6f} sec")
    
    # Check consistency
    if p_pad == p_rys:
         print(">> CONSISTENCY: \033[92mMATCH\033[0m")
    else:
         print(">> CONSISTENCY: \033[91mMISMATCH\033[0m")

    # 3. Determinant Check (Square submatrix)
    print(f"\nTest 3: Exact Determinant of a square slice ({rows}x{rows})...")
    S = R[:rows, :rows]
    det = determinant(S)
    print(f"Determinant: {det}")

if __name__ == "__main__":
    main()
