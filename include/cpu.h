#ifndef CPU_H
#define CPU_H

#include "types.h"
#include "memory.h"

// Flags
#define FLAG_C (1 << 0)
#define FLAG_Z (1 << 1)
#define FLAG_I (1 << 2)
#define FLAG_D (1 << 3)
#define FLAG_B (1 << 4)
#define FLAG_U (1 << 5)
#define FLAG_V (1 << 6)
#define FLAG_N (1 << 7)

typedef struct {
    u8 A;      
    u8 X;      
    u8 Y;      
    u8 SP;     
    u16 PC;    
    u8 P;      
    
    u64 cycles;

    // C'est ici qu'il faut l'ajouter !
    Memory *mem; // Pointeur vers la mémoire (8 octets seulement)
} CPU;

void cpu_reset(CPU *cpu, Memory *mem);
void cpu_step(CPU *cpu);

// Helpers flags
void cpu_set_flag(CPU *cpu, u8 flag, int value);
int cpu_get_flag(CPU *cpu, u8 flag);

#endif