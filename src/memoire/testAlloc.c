#include "mem.h"
#include <stdio.h>
#include <stdlib.h>

void affiche(void *pointeur, size_t taille, int libre) {
    if (libre == 1) {
        printf("Pointeur %p, Taille %ld, LIBRE\n", pointeur, taille);
    } else {
        printf("Pointeur %p, Taille %ld, OCCUPE\n", pointeur, taille);
    }
}

int main() {

    printf("Test allocation mémoire\n");

	void *allocationMemoire = malloc(1024);
    mem_init(allocationMemoire, 1024);
    
    printf("Première allocation, on alloue au total 800 (taille + header) et il reste 200\n\n");
    mem_alloc(128);
    void *mem2 = mem_alloc(128);
    void *mem3 = mem_alloc(256);
    mem_alloc(256);
    mem_show(&affiche);

    printf("\n\n");

    printf("On libère mem2 et mem3 pour voir dans la zone l'espace libre\n\n");

    mem_free(mem2);
    mem_free(mem3);
    mem_show(&affiche);

    free(allocationMemoire);

    return 0;
}
