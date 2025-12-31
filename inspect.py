import sys

def load_set(filename):
    s = set()
    try:
        with open(filename, 'r') as f:
            for line in f:
                line = line.strip()
                if line and line.isdigit():
                    s.add(int(line))
    except FileNotFoundError:
        print(f"File {filename} missing.")
        return set()
    return s

# Laad de lijsten
set_non = load_set("nonsingular.txt")
set_sing = load_set("singular.txt")

# Bereken de overlap
overlap = sorted(list(set_non.intersection(set_sing)))

print(f"--- DIAGNOSE N=6 ---")
print(f"Nonsingular count: {len(set_non)}")
print(f"Singular count:    {len(set_sing)}")
print(f"Overlap count:     {len(overlap)} (Moet 9 zijn)")
print(f"--------------------")
print(f"DE OVERLAP WAARDEN:")
print(overlap)
print(f"--------------------")

# De Lakmoesproef
if 0 in set_non:
    print("🚨 FOUT GEVONDEN: Het getal 0 zit in de Nonsingular lijst!")
    print("   Oorzaak: De determinant-check faalt of wordt verkeerd geïnterpreteerd.")
else:
    print("✅ Check: 0 zit correct niet in de Nonsingular lijst.")

# Check op de theoretische max (voor N=6 is max perm 720)
if len(overlap) > 0 and max(overlap) > 720:
     print(f"🚨 FOUT: Waarde {max(overlap)} is groter dan 6! (720).")
