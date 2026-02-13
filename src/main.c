#include <stdio.h>
#include <assert.h>
#include "types.h"
#include "memory.h"
#include "cpu.h"

int main(int argc, char **argv) {
    printf("=== Emulateur 6502 ===\n");
    printf("Phase 3 : Test du CPU.\n\n");

    // 1. Initialiser la mémoire
    Memory mem;
    mem_init(&mem);

    // 2. Préparer le vecteur de RESET
    // On va dire au CPU de commencer à l'adresse 0x8000
    // On écrit 0x00 à l'adresse 0xFFFC (Low byte)
    // On écrit 0x80 à l'adresse 0xFFFD (High byte)
    mem_write(&mem, 0xFFFC, 0x00);
    mem_write(&mem, 0xFFFD, 0x80);

    // 3. Initialiser le CPU
    CPU cpu;
    cpu_reset(&cpu, &mem);

    // 4. Vérifications
    printf("Registres après reset :\n");
    printf("PC : 0x%04X (Attendu : 0x8000)\n", cpu.PC);
    printf("SP : 0x%02X (Attendu : 0xFD)\n", cpu.SP);
    printf("P  : 0x%02X (Attendu : 0x24)\n", cpu.P);

    assert(cpu.PC == 0x8000);
    assert(cpu.SP == 0xFD);
    assert(cpu.P == 0x24);

    printf("\nSUCCES : CPU initialisé correctement !\n");

    return 0;
}