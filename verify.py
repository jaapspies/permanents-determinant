
with open('bestand1.txt', 'r') as f1, open('bestand2.txt', 'r') as f2:
    set1 = set(line.strip() for line in f1)
    set2 = set(line.strip() for line in f2)

print(set1)
print(set2)
gemeenschappelijk = set2 & set1
print(gemeenschappelijk)



