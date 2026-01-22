import ctypes
import os
import numpy as np
from numpy.ctypeslib import ndpointer

class PermanentLib:
    _instance = None

    def __init__(self, lib_path='./libpermanent.so'):
        """
        Initialize the library wrapper.
        Expects a C-library compiled with int64_t return types.
        """
        cwd = os.getcwd()
        full_path = os.path.join(cwd, lib_path)
        
        if not os.path.exists(full_path):
             raise FileNotFoundError(f"Shared library not found at: {full_path}. Did you run 'make lib'?")

        # Load the shared library
        self.lib = ctypes.CDLL(full_path)

        # --------------------------------------------------------
        # TYPE CONFIGURATION (Must match permanent.h int64 version)
        # --------------------------------------------------------
        self.c_int64 = ctypes.c_int64
        self.c_int8 = ctypes.c_int8
        
        # Define argument types: [int8 array ptr, int m, int n]
        arg_types_rect = [
            ndpointer(self.c_int8, flags="C_CONTIGUOUS"),
            ctypes.c_int,
            ctypes.c_int
        ]
        
        # 1. Permanent (Masschelein/Spies)
        self.lib.permanent.argtypes = arg_types_rect
        self.lib.permanent.restype = self.c_int64

        # 2. Ryser (Optimized)
        self.lib.ryser_new.argtypes = arg_types_rect
        self.lib.ryser_new.restype = self.c_int64

        # 3. Determinant (Bareiss) - Input: [int8 array, int n]
        self.lib.determinant.argtypes = [
             ndpointer(self.c_int8, flags="C_CONTIGUOUS"),
             ctypes.c_int
        ]
        self.lib.determinant.restype = self.c_int64

    def permanent(self, matrix, algorithm="padding"):
        """
        Calculates the permanent of a matrix.
        
        Args:
            matrix (np.array): Input matrix (int8).
            algorithm (str): 'padding' (default) or 'ryser'.
        
        Returns:
            int: The permanent value.
        """
        # Ensure input is int8 (crucial for C compatibility)
        mat_np = np.array(matrix, dtype=np.int8)
        
        if mat_np.ndim != 2:
            raise ValueError("Input must be a 2D matrix.")
            
        m, n = mat_np.shape

        if algorithm == "ryser":
            return self.lib.ryser_new(mat_np, m, n)
        elif algorithm == "padding" or algorithm == "default":
            return self.lib.permanent(mat_np, m, n)
        else:
            raise ValueError(f"Unknown algorithm: {algorithm}")

    def determinant(self, matrix):
        """Calculates exact integer determinant."""
        mat_np = np.array(matrix, dtype=np.int8)
        if mat_np.shape[0] != mat_np.shape[1]:
            raise ValueError("Determinant requires a square matrix.")
        return self.lib.determinant(mat_np, mat_np.shape[0])

# --- Singleton Helper for direct imports ---
# This allows 'from permanent_lib import permanent' to work directly
_default_lib = None

def permanent(matrix, algorithm="padding"):
    global _default_lib
    if _default_lib is None:
        _default_lib = PermanentLib()
    return _default_lib.permanent(matrix, algorithm)

def determinant(matrix):
    global _default_lib
    if _default_lib is None:
        _default_lib = PermanentLib()
    return _default_lib.determinant(matrix)
