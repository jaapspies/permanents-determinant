#!/bin/bash

# 1. Maak de "grote hoop" leeg
> dump.txt

# 2. Haal de eerste kolom uit bestand A (Nonsingular) en negeer de header
#    Zet in dump.txt
awk -F, 'NR>1 {print $1}' results_nonsingular_n7.csv >> dump.txt

# 3. Haal de eerste kolom uit bestand B (Singular) en negeer de header
#    Plak OOK in dump.txt
awk -F, 'NR>1 {print $1}' results_singular_n7.csv >> dump.txt

# 4. De magie: Sorteer en tel alleen de duplicaten (uniq -d)
echo "Overlap count (verwacht voor N=6: 9):"
sort dump.txt | uniq -d | wc -l

# Opruimen
rm dump.txt
