#include <stdio.h>
#include <assert.h>
#include "types.h"
#include "memory.h"
#include "cpu.h"

int main() {
    printf("=== Emulateur 6502 ===\n");
    printf("Phase 10 : Modes d'adressage Indexes.\n\n");

    Memory mem;
    mem_init(&mem);

    mem_write(&mem, 0xFFFC, 0x00);
    mem_write(&mem, 0xFFFD, 0x80);

    // Programme : Remplir un tableau en mémoire
    // But : Ecrire 1, 2, 3, 4, 5 aux adresses 0x10, 0x11, 0x12, 0x13, 0x14
    
    u16 start = 0x8000;
    
    // Initialisation X = 0
    mem_write(&mem, start++, 0xA2); // LDX #$00
    mem_write(&mem, start++, 0x00);

    // Début de la boucle
    u16 loop_start = start;

    // Charger une valeur dans A (ici X+1 pour que ça change à chaque tour)
    // Pour simplifier, on va juste écrire la valeur de X + 1
    // Mais ADC est complexe, on va faire plus simple :
    // TXA (Transfer X to A), puis INX, puis écrire.
    
    // 1. TXA (8A)
    mem_write(&mem, start++, 0x8A); // A = X
    
    // 2. CLC + ADC #$01 (A = A + 1) -> Pour écrire 1, 2, 3...
    mem_write(&mem, start++, 0x18); // CLC
    mem_write(&mem, start++, 0x69); // ADC #$01
    mem_write(&mem, start++, 0x01);

    // 3. STA $10, X (Écrire A à l'adresse 0x10 + X)
    mem_write(&mem, start++, 0x95); // STA ZP,X
    mem_write(&mem, start++, 0x10); // Base 0x10

    // 4. INX
    mem_write(&mem, start++, 0xE8); // INX

    // 5. CPX #$05 (Est-ce qu'on a fait 5 tours ?)
    mem_write(&mem, start++, 0xE0); // CPX #$05
    mem_write(&mem, start++, 0x05);

    // 6. BNE (Revenir au début de la boucle)
    mem_write(&mem, start++, 0xD0); // BNE
    s8 offset = loop_start - (start + 1);
    mem_write(&mem, start++, (u8)offset);

    // Fin
    mem_write(&mem, start++, 0xEA); // NOP

    CPU cpu;
    cpu_reset(&cpu, &mem);

    printf("Execution de la boucle indexee...\n");
    
    int max_steps = 200;
    while (max_steps > 0) {
        cpu_step(&cpu);
        max_steps--;
        if (cpu.cycles > 200) break;
    }

    // Vérification des résultats en mémoire
    printf("Verification du tableau en memoire (0x0010) :\n");
    printf("Mem[0x10] = %d (Attendu : 1)\n", mem_read(&mem, 0x0010));
    printf("Mem[0x11] = %d (Attendu : 2)\n", mem_read(&mem, 0x0011));
    printf("Mem[0x12] = %d (Attendu : 3)\n", mem_read(&mem, 0x0012));
    printf("Mem[0x13] = %d (Attendu : 4)\n", mem_read(&mem, 0x0013));
    printf("Mem[0x14] = %d (Attendu : 5)\n", mem_read(&mem, 0x0014));

    assert(mem_read(&mem, 0x0010) == 1);
    assert(mem_read(&mem, 0x0014) == 5);

    printf("\nSUCCES : L'adressage indexe fonctionne !\n");
    return 0;
}