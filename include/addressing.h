#ifndef ADDRESSING_H
#define ADDRESSING_H

#include "cpu.h"

void addr_immediate(CPU *cpu);  // La donnée est juste après l'opcode
void addr_zero_page(CPU *cpu);  // Adresse dans la page 0 (1 octet)
void addr_absolute(CPU *cpu);   // Adresse complète (2 octets)

#endif