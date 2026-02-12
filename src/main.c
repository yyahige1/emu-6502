#include <stdio.h>
#include <assert.h> // Pour les tests
#include "types.h"
#include "memory.h"

int main(int argc, char **argv) {
    printf("=== Emulateur 6502 ===\n");
    printf("Phase 2 : Test Memoire.\n\n");

    // 1. Créer et initialiser la mémoire
    Memory mem;
    mem_init(&mem);
    printf("Memoire initialisee (64 Ko alloues).\n");

    // 2. Tester l'écriture
    u16 test_addr = 0x1234;
    u8 test_val = 0xAB;
    printf("Ecriture de 0x%02X a l'adresse 0x%04X...\n", test_val, test_addr);
    mem_write(&mem, test_addr, test_val);

    // 3. Tester la lecture
    u8 read_val = mem_read(&mem, test_addr);
    printf("Lecture de l'adresse 0x%04X : 0x%02X\n", test_addr, read_val);

    // 4. Vérification automatique
    assert(read_val == test_val);
    printf("\nSUCCES : La memoire fonctionne correctement !\n");

    return 0;
}