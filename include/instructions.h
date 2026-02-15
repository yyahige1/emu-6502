#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "cpu.h"

void ins_LDA(CPU *cpu);
void ins_LDX(CPU *cpu);
void ins_STA(CPU *cpu);
void ins_NOP(CPU *cpu);

#endif