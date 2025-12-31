import sys
import os

def load_values(filename):
    s = set()
    if not os.path.exists(filename):
        print(f"Error: {filename} not found.")
        return s
    with open(filename, 'r') as f:
        content = f.read().replace(',', ' ').replace('\n', ' ')
        for token in content.split():
            if token.isdigit():
                s.add(int(token))
    return s

# 1. Laad de Nonsingular set (die we eerder 'schoon' hebben gemaakt)
# Pas de bestandsnaam aan als hij anders heet!
file_non = "nonsingular.txt" 
set_non = load_values(file_non)

# 2. Laad de Singular set (die u net heeft geplakt)
file_sing = "singular.txt"
set_sing = load_values(file_sing)

# 3. Bereken overlap
intersect = set_non.intersection(set_sing)
sorted_intersect = sorted(list(intersect))

print(f"--- FINAL VERIFICATION N=6 ---")
print(f"Nonsingular (A): {len(set_non)} (Verwacht: 149)")
print(f"Singular    (B): {len(set_sing)} (Verwacht: 136)")
print(f"------------------------------")
print(f"OVERLAP     (X): {len(intersect)}")
print(f"------------------------------")

if len(intersect) == 9:
    print("✅ SUCCES! De overlap is exact 9.")
    print(f"De waarden zijn: {sorted_intersect}")
    print(f"Totaal uniek (A088672): {len(set_non) + len(set_sing) - len(intersect)} (Verwacht: 276)")
else:
    print(f"❌ NOG NIET GOED. Overlap is {len(intersect)} (moet 9 zijn).")
    print(f"De waarden: {sorted_intersect}")
