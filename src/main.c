#include <stdio.h>
#include <assert.h>
#include "types.h"
#include "memory.h"
#include "cpu.h"

int main() {
    printf("=== Emulateur 6502 ===\n");
    printf("Phase 4 : Fetch Decode Execute.\n\n");

    // 1. Init Mémoire
    Memory mem;
    mem_init(&mem);

    // 2. Charger un petit programme en mémoire à l'adresse 0x8000
    // A9 05 -> LDA #$05
    // A2 0A -> LDX #$0A
    // EA    -> NOP
    mem_write(&mem, 0x8000, 0xA9);
    mem_write(&mem, 0x8001, 0x05);
    mem_write(&mem, 0x8002, 0xA2);
    mem_write(&mem, 0x8003, 0x0A);
    mem_write(&mem, 0x8004, 0xEA);

    // Indiquer au CPU de démarrer à 0x8000
    mem_write(&mem, 0xFFFC, 0x00);
    mem_write(&mem, 0xFFFD, 0x80);

    // 3. Init CPU
    CPU cpu;
    cpu_reset(&cpu, &mem); // On passe l'adresse de 'mem', le CPU la stocke dans son pointeur

    // 4. Exécuter quelques pas
    printf("Execution des instructions...\n");
    
    // Instruction 1 : LDA
    cpu_step(&cpu);
    printf("Apres LDA #$05 : A = 0x%02X (Attendu: 0x05)\n", cpu.A);
    assert(cpu.A == 0x05);

    // Instruction 2 : LDX
    cpu_step(&cpu);
    printf("Apres LDX #$0A : X = 0x%02X (Attendu: 0x0A)\n", cpu.X);
    assert(cpu.X == 0x0A);

    // Instruction 3 : NOP
    cpu_step(&cpu);
    printf("Apres NOP : PC = 0x%04X (Attendu: 0x8005)\n", cpu.PC);
    assert(cpu.PC == 0x8005);

    printf("\nSUCCES : Le CPU execute du code !\n");
    // Correction du warning : on utilise %lu (long unsigned) au lieu de %llu
    printf("Cycles totaux : %lu\n", (unsigned long)cpu.cycles);

    return 0;
}