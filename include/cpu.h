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
    u8 A, X, Y, SP;
    u16 PC;
    u8 P;
    u64 cycles;
    Memory *mem;

    // NOUVEAU : Variables temporaires pour les modes d'adressage
    u16 addr_abs;     // L'adresse calculée (ex: 0x1234)
    u8 fetched;       // La donnée lue à cette adresse

} CPU;

// Prototypes
void cpu_reset(CPU *cpu, Memory *mem);
void cpu_step(CPU *cpu);
void cpu_set_flag(CPU *cpu, u8 flag, int value);
int cpu_get_flag(CPU *cpu, u8 flag);

// Définition du type Pointeur de Fonction pour les instructions/adressages
typedef void (*InstructionFunc)(CPU *cpu);
typedef void (*AddrModeFunc)(CPU *cpu);

void cpu_push_byte(CPU *cpu, u8 value);
u8 cpu_pull_byte(CPU *cpu);
void cpu_push_word(CPU *cpu, u16 value);
u16 cpu_pull_word(CPU *cpu);

#endif