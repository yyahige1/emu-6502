#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "cpu.h"

void ins_LDA(CPU *cpu);
void ins_LDX(CPU *cpu);
void ins_STA(CPU *cpu);
void ins_NOP(CPU *cpu);
// Transferts
void ins_TAX(CPU *cpu); // A -> X
void ins_TXA(CPU *cpu); // X -> A

// Incréments
void ins_INX(CPU *cpu); // X + 1
void ins_DEX(CPU *cpu); // X - 1

// Branchements (Sauts conditionnels)
void ins_BEQ(CPU *cpu); // Branch if Equal (Z == 1)
void ins_BNE(CPU *cpu); // Branch if Not Equal (Z == 0)

// Contrôle
void ins_JMP(CPU *cpu); // Saut inconditionnel
#endif