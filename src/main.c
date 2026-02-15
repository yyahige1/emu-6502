#include <stdio.h>
#include <assert.h>
#include "types.h"
#include "memory.h"
#include "cpu.h"

int main() {
    printf("=== Emulateur 6502 ===\n");
    printf("Phase 7 : Transferts et Branchements.\n\n");

    Memory mem;
    mem_init(&mem);

    // Vecteur de Reset
    mem_write(&mem, 0xFFFC, 0x00);
    mem_write(&mem, 0xFFFD, 0x80);

    // Programme à 0x8000
    // Objectif : Faire une boucle qui décrémente X de 5 à 0
    u16 start = 0x8000;
    
    mem_write(&mem, start++, 0xA9); // LDA #$05
    mem_write(&mem, start++, 0x05);
    
    mem_write(&mem, start++, 0xAA); // TAX (X = 5)
    
    u16 loop_addr = start; // Adresse du début de la boucle
    
    mem_write(&mem, start++, 0xCA); // DEX (X--)
    
    mem_write(&mem, start++, 0xD0); // BNE (Branch if Not Equal) -> Retour au DEX
    // Calcul du saut relatif : loop_addr - (adresse_apres_offset)
    // adresse_apres_offset sera 'start + 1'
    s8 offset = loop_addr - (start + 1);
    mem_write(&mem, start++, (u8)offset); 

    mem_write(&mem, start++, 0xEA); // NOP (Fin)

    CPU cpu;
    cpu_reset(&cpu, &mem);

    printf("Execution de la boucle...\n");
    
    // On exécute pas à pas jusqu'à ce que X soit 0
    // Normalement, le CPU va boucler plusieurs fois sur DEX / BNE
    // On met une sécurité pour éviter une boucle infinie si le code bug
    int max_steps = 100;
    while (cpu.PC < 0x8008 && max_steps > 0) {
        cpu_step(&cpu);
        max_steps--;
    }

    printf("Valeur finale de X : %d (Attendu : 0)\n", cpu.X);
    printf("Cycles totaux : %lu\n", (unsigned long)cpu.cycles);

    assert(cpu.X == 0);

    printf("\nSUCCES : Le CPU a executé une boucle correctement !\n");
    return 0;
}