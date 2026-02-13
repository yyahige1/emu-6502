#ifndef CPU_H
#define CPU_H

#include "types.h"
#include "memory.h"

// Définition des drapeaux (Flags) du processeur
// Le 6502 utilise un octet (8 bits) pour stocker ces drapeaux
#define FLAG_C (1 << 0) // Carry
#define FLAG_Z (1 << 1) // Zero
#define FLAG_I (1 << 2) // Interrupt Disable
#define FLAG_D (1 << 3) // Decimal Mode
#define FLAG_B (1 << 4) // Break Command
#define FLAG_U (1 << 5) // Unused (toujours 1 physiquement)
#define FLAG_V (1 << 6) // Overflow
#define FLAG_N (1 << 7) // Negative

typedef struct {
    // Registres principaux
    u8 A;      // Accumulateur
    u8 X;      // Registre d'index X
    u8 Y;      // Registre d'index Y
    u8 SP;     // Pointeur de pile (Stack Pointer) - 8 bits
    u16 PC;    // Compteur de programme (Program Counter) - 16 bits

    // Registre de statut (Processor Status)
    u8 P;      // Contient les flags (N V - B D I Z C)

    // Compteur de cycles (utile pour le timing futur)
    u64 cycles;
} CPU;

// Prototypes des fonctions
void cpu_reset(CPU *cpu, Memory *mem);

#endif