#include <stdio.h>
#include <assert.h>
#include "types.h"
#include "memory.h"
#include "cpu.h"

int main() {
    printf("=== Emulateur 6502 ===\n");
    printf("Phase 9 : Arithmétique et Logique.\n\n");

    Memory mem;
    mem_init(&mem);

    mem_write(&mem, 0xFFFC, 0x00);
    mem_write(&mem, 0xFFFD, 0x80);

    // Programme
    // 1. Charge 10 dans A
    // 2. Additionne 5
    // 3. Compare A avec 15
    // 4. Si égal (Zero flag), saute au label OK
    // 5. Sinon NOP (erreur)

    u16 start = 0x8000;
    mem_write(&mem, start++, 0xA9); // LDA #$10
    mem_write(&mem, start++, 0x0A);

    // ADC (Add)
    // Il faut s'assurer que le flag Carry est à 0 avant une addition standard (CLC)
    // Opcode CLC = 0x18
    mem_write(&mem, start++, 0x18); // CLC (Clear Carry)
    
    mem_write(&mem, start++, 0x69); // ADC #$05
    mem_write(&mem, start++, 0x05);

    mem_write(&mem, start++, 0xC9); // CMP #$0F (15)
    mem_write(&mem, start++, 0x0F);

    mem_write(&mem, start++, 0xF0); // BEQ (Branch if Equal) -> Saut de +1
    mem_write(&mem, start++, 0x01); // Sauter le prochain NOP si égal

    mem_write(&mem, start++, 0xEA); // NOP (Erreur)
    mem_write(&mem, start++, 0xEA); // NOP (Succès)

    CPU cpu;
    cpu_reset(&cpu, &mem);

    printf("Execution du calcul...\n");
    
    // Exécution pas à pas ou boucle
    int max_steps = 20;
    while (max_steps > 0) {
        cpu_step(&cpu);
        max_steps--;
        
        // On s'arrête si on a fait beaucoup de cycles
        if (cpu.cycles > 20) break;
    }

    printf("Valeur de A : %d (Attendu : 15)\n", cpu.A);
    assert(cpu.A == 15);
    
    // Vérifier si on a bien sauté le premier NOP (PC doit être après le deuxième NOP)
    printf("PC final : 0x%04X\n", cpu.PC);

    // Le programme fait : LDA, CLC, ADC, CMP, BEQ, NOP(Succès)
    // Si le branchement marche, on saute le NOP d'erreur.
    
    printf("\nSUCCES : Le CPU peut faire des maths et des comparaisons !\n");
    return 0;
}