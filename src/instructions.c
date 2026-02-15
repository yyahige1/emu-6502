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

// --- Transferts ---

void ins_TAX(CPU *cpu) {
    cpu->X = cpu->A;
    cpu_set_flag(cpu, FLAG_Z, cpu->X == 0);
    cpu_set_flag(cpu, FLAG_N, (cpu->X & 0x80) != 0);
}

void ins_TXA(CPU *cpu) {
    cpu->A = cpu->X;
    cpu_set_flag(cpu, FLAG_Z, cpu->A == 0);
    cpu_set_flag(cpu, FLAG_N, (cpu->A & 0x80) != 0);
}

// --- Incréments ---

void ins_INX(CPU *cpu) {
    cpu->X++;
    cpu_set_flag(cpu, FLAG_Z, cpu->X == 0);
    cpu_set_flag(cpu, FLAG_N, (cpu->X & 0x80) != 0);
}

void ins_DEX(CPU *cpu) {
    cpu->X--;
    cpu_set_flag(cpu, FLAG_Z, cpu->X == 0);
    cpu_set_flag(cpu, FLAG_N, (cpu->X & 0x80) != 0);
}

// --- Branchements ---
// Pour ces instructions, addr_abs a déjà été calculée par addr_relative

void ins_BEQ(CPU *cpu) {
    if (cpu_get_flag(cpu, FLAG_Z)) {
        cpu->PC = cpu->addr_abs; // On saute !
        cpu->cycles++; // Un cycle de plus car on a pris le saut
    }
}

void ins_BNE(CPU *cpu) {
    if (!cpu_get_flag(cpu, FLAG_Z)) { // Si Z est à 0
        cpu->PC = cpu->addr_abs;
        cpu->cycles++;
    }
}

// --- Contrôle ---

void ins_JMP(CPU *cpu) {
    // Pour JMP, addr_abs a été calculée par addr_absolute
    cpu->PC = cpu->addr_abs;
}