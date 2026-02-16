#include <stdio.h>
#include <assert.h>
#include "types.h"
#include "memory.h"
#include "cpu.h"

int main() {
    printf("=== Emulateur 6502 ===\n");
    printf("Phase 12 : Interruptions (BRK, RTI).\n\n");

    Memory mem;
    mem_init(&mem);

    // --- Configuration des Vecteurs ---
    // Reset Vector ($FFFC) -> Programme Principal à $8000
    mem_write(&mem, 0xFFFC, 0x00);
    mem_write(&mem, 0xFFFD, 0x80);

    // IRQ/BRK Vector ($FFFE) -> Routine Interruption à $9000
    mem_write(&mem, 0xFFFE, 0x00);
    mem_write(&mem, 0xFFFF, 0x90);

    // --- Programme Principal ($8000) ---
    // 1. LDA #$00 (A = 0)
    mem_write(&mem, 0x8000, 0xA9);
    mem_write(&mem, 0x8001, 0x00);
    
    // 2. BRK (Saut à l'interruption)
    mem_write(&mem, 0x8002, 0x00); // Opcode BRK
    mem_write(&mem, 0x8003, 0x00); // Signature byte (ignorée par notre BRK qui fait PC++)

    // 4. NOP (Retour ici après RTI)
    mem_write(&mem, 0x8004, 0xEA);

    // --- Routine d'Interruption ($9000) ---
    // Modifie A pour prouver qu'on est passé ici
    mem_write(&mem, 0x9000, 0xA9); // LDA #$FF
    mem_write(&mem, 0x9001, 0xFF);
    
    // Retour de l'interruption
    mem_write(&mem, 0x9002, 0x40); // RTI

    CPU cpu;
    cpu_reset(&cpu, &mem);

    printf("Execution...\n");
    
    // 1. Exécution normale (LDA)
    cpu_step(&cpu);
    printf("Avant BRK : A = %d\n", cpu.A);

    // 2. Exécution du BRK (interruption)
    cpu_step(&cpu); 
    
    // 3. Dans l'interruption (LDA #$FF)
    cpu_step(&cpu);
    
    // 4. Dans l'interruption (RTI)
    // 4. Dans l'interruption (RTI)
    cpu_step(&cpu);
    
    printf("Apres RTI  : A = %d (Attendu : 255)\n", cpu.A);
    printf("PC retourné: 0x%04X (Attendu : 0x8004)\n", cpu.PC);
    assert(cpu.A == 0xFF);
    assert(cpu.PC == 0x8004); // On est revenu, le NOP n'est pas encore exécuté

    // 5. Exécuter le NOP qui suit le retour
    cpu_step(&cpu);
    printf("Apres NOP  : PC = 0x%04X (Attendu : 0x8005)\n", cpu.PC);
    assert(cpu.PC == 0x8005);

    printf("\nSUCCES : Le CPU gère les interruptions et reprend son cours !\n");
    return 0;
}