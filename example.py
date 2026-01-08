import numpy as np
import time
import math  # Added for factorial
from permanent_lib import permanent

def main():
    print("--- Demo: High Performance Permanent Library (Spies/Masschelein) ---")

    # 1. Correctness Test
    n = 12
    print(f"\nTest 1: Calculating J_{n} (Matrix of ones)...")
    J = np.ones((n, n), dtype=int)

    start = time.time()
    # Uses default="padding" implicitly
    res = permanent(J) 
    dt = time.time() - start

    # FIX: Standard math library in place of np.math
    expected = math.factorial(n)
    
    print(f"Result:   {res:.0f}")
    print(f"Expected: {expected}")
    print(f"Time:     {dt:.6f} sec")

    if abs(res - expected) < 0.1:
        print(">> VALIDATION: OK")
    else:
        print(">> VALIDATION: FAILED")

    # 2. Performance & Algorithm Comparison
    rows, cols = 15, 20
    print(f"\nTest 2: Performance on random {rows}x{cols} (-1, 0, 1) matrix...")
    
    R = np.random.randint(-1, 2, size=(rows, cols))

    # Run with Padding (Default)
    start = time.time()
    p_pad = permanent(R, algorithm="padding")
    dt_pad = time.time() - start
    print(f"Method: Padding (Default) | Result: {p_pad:.0f} | Time: {dt_pad:.6f} sec")

    # Run with Ryser (Explicit)
    start = time.time()
    p_rys = permanent(R, algorithm="ryser")
    dt_rys = time.time() - start
    print(f"Method: Ryser (Explicit)  | Result: {p_rys:.0f} | Time: {dt_rys:.6f} sec")

if __name__ == "__main__":
    main()
