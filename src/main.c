#include <stdio.h>
#include <assert.h>
#include "types.h"
#include "memory.h"
#include "cpu.h"

int main() {
    printf("=== Emulateur 6502 ===\n");
    printf("Phase 11 : Instructions Y, INC, DEC, Shifts.\n\n");

    Memory mem;
    mem_init(&mem);

    mem_write(&mem, 0xFFFC, 0x00);
    mem_write(&mem, 0xFFFD, 0x80);

    // Programme
    // 1. LDY #$05 (Charger 5 dans Y)
    // 2. STY $20 (Stocker Y à l'adresse 0x0020)
    // 3. INC $20 (Incrémenter la case mémoire -> devient 6)
    // 4. ASL $20 (Décalage à gauche -> 6 * 2 = 12)
    // 5. LDA $20 (Charger le résultat dans A pour vérifier)

    u16 start = 0x8000;
    mem_write(&mem, start++, 0xA0); // LDY #$05
    mem_write(&mem, start++, 0x05);
    
    mem_write(&mem, start++, 0x84); // STY $20
    mem_write(&mem, start++, 0x20);

    mem_write(&mem, start++, 0xE6); // INC $20
    mem_write(&mem, start++, 0x20);

    mem_write(&mem, start++, 0x06); // ASL $20
    mem_write(&mem, start++, 0x20);

    mem_write(&mem, start++, 0xA5); // LDA $20
    mem_write(&mem, start++, 0x20);

    CPU cpu;
    cpu_reset(&cpu, &mem);

    printf("Execution...\n");
    int max_steps = 20;
    while (max_steps > 0) {
        cpu_step(&cpu);
        max_steps--;
    }

    printf("Valeur en memoire (0x0020) : %d (Attendu : 12)\n", mem_read(&mem, 0x0020));
    printf("Registre A : %d\n", cpu.A);
    
    assert(mem_read(&mem, 0x0020) == 12); // 5 -> 6 -> 12

    printf("\nSUCCES : Manipulation memoire et bits OK !\n");
    return 0;
}