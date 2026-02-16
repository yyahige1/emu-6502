#include <stdio.h>
#include <string.h>
#include "types.h"
#include "memory.h"
#include "cpu.h"

void run_builtin_test() {
    printf("=== Mode Test Interne ===\n");
    
    Memory mem;
    mem_init(&mem);

    mem_write(&mem, 0xFFFC, 0x00);
    mem_write(&mem, 0xFFFD, 0x80);

    u16 start = 0x8000;
    mem_write(&mem, start++, 0xA2); // LDX #$00
    mem_write(&mem, start++, 0x00);
    mem_write(&mem, start++, 0xE8); // INX
    mem_write(&mem, start++, 0xE0); // CPX #$05
    mem_write(&mem, start++, 0x05);
    mem_write(&mem, start++, 0xD0); // BNE (retour au INX)
    mem_write(&mem, start++, 0xFB); // Offset -5
    mem_write(&mem, start++, 0xEA); // NOP

    CPU cpu;
    cpu_reset(&cpu, &mem);

    printf("Lancement du test interne...\n");
    int max_steps = 50;
    while (max_steps > 0) {
        cpu_step(&cpu);
        max_steps--;
    }
    
    printf("X final : %d (Attendu : 5)\n", cpu.X);
}

int main(int argc, char **argv) {
    if (argc > 1) {
        printf("=== Emulateur 6502 ===\n");
        printf("Chargement du fichier : %s\n\n", argv[1]);

        Memory mem;
        mem_init(&mem);

        if (!mem_load(&mem, argv[1], 0x0000)) {
            return 1;
        }

        CPU cpu;
        cpu_reset(&cpu, &mem);

        printf("Execution...\n");
        
        while (1) {
            cpu_step(&cpu);
            
            // Sécurité
            if (cpu.cycles > 5000000) { // 5 millions de cycles max
                printf("Limite de cycles atteinte.\n");
                break;
            }
        }
        
    } else {
        run_builtin_test();
    }

    return 0;
}