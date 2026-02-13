#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "cpu.h"

// Instructions de base
void ins_LDA_immediate(CPU *cpu); // Load Accumulator (Immediate mode)
void ins_LDX_immediate(CPU *cpu); // Load X (Immediate mode)
void ins_NOP(CPU *cpu);           // No Operation

#endif