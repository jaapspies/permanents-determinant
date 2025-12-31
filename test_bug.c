#include <stdio.h>
#include <stdint.h> // Nodig voor int8_t
#include "permanent.h"

int main() {
    int n = 3;
    // We gebruiken int8_t omdat de header dat eist
    int8_t matrix[9] = {
        0, 1, 1,
        1, 0, 1,
        1, 1, 0
    };

    printf("Test Matrix (3x3) J-I (Verwachte Det = 2):\n");

    // De functie geeft een double terug volgens jouw error log
    double d = determinant(matrix, n);
    
    printf("Berekende Determinant: %f\n", d);

    // Check of hij (foutief) denkt dat het 0 is
    // We gebruiken een kleine marge voor het geval het 0.0000001 is
    if (d > -0.001 && d < 0.001) {
        printf("\nCONCLUSIE: BUG! Hij berekent 0 (Singular) terwijl het 2 is.\n");
    } else {
        printf("\nCONCLUSIE: OK. Hij ziet dat het niet 0 is.\n");
    }

    return 0;
}
