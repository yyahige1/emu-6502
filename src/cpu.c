#include "cpu.h"

void cpu_reset(CPU *cpu, Memory *mem) {
    // 1. Initialiser les registres à 0 (état par défaut sûr)
    cpu->A = 0;
    cpu->X = 0;
    cpu->Y = 0;

    // 2. Initialiser la pile (Stack Pointer)
    // Le 6502 a une pile fixe à l'adresse 0x0100.
    // Le pointeur SP (8 bits) descend de 0xFD au démarrage.
    cpu->SP = 0xFD;

    // 3. Initialiser les flags
    // Le flag 'Interrupt Disable' (I) est souvent mis à 1 au reset.
    // Le bit 'Unused' (U) est toujours à 1.
    // Donc P = 0x24 (0010 0100 en binaire : U=1, I=1).
    cpu->P = 0x24;

    cpu->cycles = 0;

    // 4. Charger le vecteur de RESET
    // Le CPU 6502 lit les adresses 0xFFFC (poids faible) et 0xFFFD (poids fort)
    // pour savoir où commencer à exécuter le code.
    u16 lo = mem_read(mem, 0xFFFC);
    u16 hi = mem_read(mem, 0xFFFD);
    cpu->PC = (hi << 8) | lo;
}