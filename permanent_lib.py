import ctypes
import numpy as np
import os

class PermanentLib:
    """
    Wrapper class for the high-performance C permanent library.
    """
    def __init__(self, lib_path=None):
        # If no path is provided, look for the library in the same directory as this script
        if lib_path is None:
            lib_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "libpermanent.so")

        if not os.path.exists(lib_path):
            raise FileNotFoundError(f"Library not found at: {lib_path}. Did you run 'make lib'?")
        
        # Load the shared library
        self.lib = ctypes.CDLL(lib_path)
        
        # 1. Configure 'permanent' function (Spies/Masschelein Padding - FASTEST/DEFAULT)
        # C signature: double permanent(int8_t *A, int m, int n)
        self.lib.permanent.argtypes = [ctypes.POINTER(ctypes.c_int8), ctypes.c_int, ctypes.c_int]
        self.lib.permanent.restype = ctypes.c_double

        # 2. Configure 'ryser_new' function (Rectangular Ryser with Gray Code)
        # C signature: double ryser_new(int8_t *A, int m, int n)
        self.lib.ryser_new.argtypes = [ctypes.POINTER(ctypes.c_int8), ctypes.c_int, ctypes.c_int]
        self.lib.ryser_new.restype = ctypes.c_double

    def calculate(self, matrix, algorithm="padding"):
        """
        Calculates the permanent of a given numpy matrix.
        
        Parameters:
            matrix (np.ndarray): The input matrix (will be converted to int8).
            algorithm (str): The algorithm to use.
                             - 'padding' (Default): Spies/Masschelein formula. Fastest in benchmarks.
                             - 'ryser': Optimized Ryser/Gray code without padding.
        
        Returns:
            double: The calculated permanent.
        """
        # Ensure input is a numpy array of type int8
        mat_int8 = np.array(matrix, dtype=np.int8)
        rows, cols = mat_int8.shape
        
        # Get a pointer to the raw data (C-contiguous)
        c_ptr = mat_int8.ctypes.data_as(ctypes.POINTER(ctypes.c_int8))
        
        if algorithm == "padding":
            # Calls 'double permanent(...)' in C -> Spies/Masschelein implementation
            return self.lib.permanent(c_ptr, rows, cols)
        elif algorithm == "ryser":
            # Calls 'double ryser_new(...)' in C -> Ryser implementation
            return self.lib.ryser_new(c_ptr, rows, cols)
        else:
            raise ValueError(f"Unknown algorithm '{algorithm}'. Choose 'padding' (default) or 'ryser'.")

# Singleton instance for easy direct usage
_inst = None

def permanent(matrix, algorithm="padding"):
    """
    Calculate the permanent of a matrix.
    
    Arguments:
        matrix: The input numpy array (or list of lists).
        algorithm: 'padding' (default, fastest) or 'ryser'.
    """
    global _inst
    if _inst is None:
        _inst = PermanentLib()
    return _inst.calculate(matrix, algorithm)
