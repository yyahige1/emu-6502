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

void ins_PHA(CPU *cpu);
void ins_PLA(CPU *cpu);
void ins_JSR(CPU *cpu); // Attention, conflit avec le nom de l'instruction JMP qu'on a mis avant
void ins_RTS(CPU *cpu);

// Arithmétique
void ins_ADC(CPU *cpu); // Addition avec retenue
void ins_SBC(CPU *cpu); // Soustraction avec retenue

// Comparaison
void ins_CMP(CPU *cpu); // Comparer A
void ins_CPX(CPU *cpu); // Comparer X
void ins_CPY(CPU *cpu); // Comparer Y

// Logique
void ins_AND(CPU *cpu); // ET binaire
void ins_ORA(CPU *cpu); // OU binaire
void ins_EOR(CPU *cpu); // OU exclusif binaire

void ins_CLC(CPU *cpu);
void ins_SEC(CPU *cpu);
void ins_CLD(CPU *cpu);
void ins_SED(CPU *cpu);
void ins_CLI(CPU *cpu);
void ins_SEI(CPU *cpu);
void ins_CLV(CPU *cpu);
#endif