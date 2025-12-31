import sys

def load_set(filename):
    s = set()
    try:
        with open(filename, 'r') as f:
            for line in f:
                line = line.strip()
                # Negeer lege regels of niet-numerieke regels
                if line and line.isdigit():
                    s.add(int(line))
    except FileNotFoundError:
        print(f"Fout: Bestand {filename} niet gevonden.")
        sys.exit(1)
    return s

# Pas hier eventueel de bestandsnamen aan
file_non = "nonsingular.txt"
file_sing = "singular.txt"

print(f"--- Bezig met inlezen van {file_non} en {file_sing} ---")

set_non = load_set(file_non)
set_sing = load_set(file_sing)

overlap = set_non.intersection(set_sing)
union = set_non.union(set_sing)

print(f"\n--- RESULTATEN ---")
print(f"Aantal Nonsingular (A) : {len(set_non)}")
print(f"Aantal Singular    (B) : {len(set_sing)}")
print(f"-----------------------")
print(f"OVERLAP (A en B)       : {len(overlap)}")
print(f"TOTAAL UNIEK (A of B)  : {len(union)}")
print(f"\nRekensom: {len(set_non)} + {len(set_sing)} - {len(overlap)} = {len(set_non) + len(set_sing) - len(overlap)}")
