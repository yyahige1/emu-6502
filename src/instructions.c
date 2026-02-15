#include "instructions.h"

// LDA : Charge une valeur dans A
void ins_LDA(CPU *cpu) {
    cpu->A = cpu->fetched; // La valeur a été calculée par l'adressage
    cpu_set_flag(cpu, FLAG_Z, cpu->A == 0);
    cpu_set_flag(cpu, FLAG_N, (cpu->A & 0x80) != 0);
}

// LDX : Charge une valeur dans X
void ins_LDX(CPU *cpu) {
    cpu->X = cpu->fetched;
    cpu_set_flag(cpu, FLAG_Z, cpu->X == 0);
    cpu_set_flag(cpu, FLAG_N, (cpu->X & 0x80) != 0);
}

// STA : Stocke A en mémoire
void ins_STA(CPU *cpu) {
    // Pour STA, on n'a pas besoin de 'fetched' (lecture),
    // on utilise l'adresse calculée 'addr_abs'
    mem_write(cpu->mem, cpu->addr_abs, cpu->A);
}

// NOP : Ne rien faire
void ins_NOP(CPU *cpu) {
    (void)cpu; // Evite le warning
}