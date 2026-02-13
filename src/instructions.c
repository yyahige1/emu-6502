#include "instructions.h"

void ins_LDA_immediate(CPU *cpu) {
    // Lit la valeur qui suit l'instruction
    u8 value = mem_read(cpu->mem, cpu->PC);
    cpu->PC++;
    
    cpu->A = value;

    // Mettre à jour les flags (Zero et Negative)
    cpu_set_flag(cpu, FLAG_Z, cpu->A == 0);
    cpu_set_flag(cpu, FLAG_N, (cpu->A & 0x80) != 0); // Si le bit 7 est à 1
}

void ins_LDX_immediate(CPU *cpu) {
    u8 value = mem_read(cpu->mem, cpu->PC);
    cpu->PC++;
    cpu->X = value;

    cpu_set_flag(cpu, FLAG_Z, cpu->X == 0);
    cpu_set_flag(cpu, FLAG_N, (cpu->X & 0x80) != 0);
}

void ins_NOP(CPU *cpu) {
    // Ne rien faire (No Operation)
    (void)cpu; // Pour éviter le warning unused parameter
}