#!/bin/bash

# 1. Nonsingular: Header wegknippen, eerste kolom pakken, en als TEKST sorteren
awk -F, 'NR>1 {print $1}' results_nonsingular_n7.csv | sort > list_A.txt

# 2. Singular: Header wegknippen, eerste kolom pakken, en als TEKST sorteren
awk -F, 'NR>1 {print $1}' results_singular_n7.csv | sort > list_B.txt

# 3. De overlap tellen
echo "Overlap (Intersection count):"
comm -12 list_A.txt list_B.txt | wc -l

# Opruimen
# rm list_A.txt list_B.txt
