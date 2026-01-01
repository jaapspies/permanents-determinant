import os

def load_values(filename):
    s = set()
    if not os.path.exists(filename):
        print(f"⚠️ WARNING: Could not find file '{filename}'.")
        return s
    
    with open(filename, 'r') as f:
        # Read line by line (CSV format)
        for line in f:
            clean_line = line.strip()
            if clean_line.isdigit():
                s.add(int(clean_line))
            # Fallback for space-separated values
            elif ' ' in clean_line:
                for token in clean_line.split():
                    if token.isdigit():
                        s.add(int(token))
    return s

# Filenames
file_non = "results_nonsingular_7.csv" 
file_sing = "results_singular_7.csv"

print("--- ANALYSIS N=7 ---")
print("Loading data...")

set_non = load_values(file_non)
set_sing = load_values(file_sing)

intersect = set_non.intersection(set_sing)
union_len = len(set_non) + len(set_sing) - len(intersect)

print(f"------------------------------")
print(f"Nonsingular (A): {len(set_non)}")
print(f"Singular    (B): {len(set_sing)}")
print(f"------------------------------")
print(f"OVERLAP     (X): {len(intersect)}")
print(f"------------------------------")
print(f"TOTAL UNIQUE   : {union_len}")
print(f"(Formula: {len(set_non)} + {len(set_sing)} - {len(intersect)})")
print(f"------------------------------")

# Bonus: Are there any 'gaps' in the sequence?
max_val = 5040 # 7!
missing = []
# We only check even numbers (odd numbers do not occur for n>1)
for i in range(0, max_val + 1, 2):
    if i not in set_non and i not in set_sing:
        missing.append(i)

print(f"\nInfo: Of the even numbers up to {max_val}, {len(missing)} are missing.")
if len(missing) < 20:
    print(f"The missing numbers: {missing}")
