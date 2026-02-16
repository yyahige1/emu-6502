#ifndef ADDRESSING_H
#define ADDRESSING_H

#include "cpu.h"

void addr_immediate(CPU *cpu);  // La donnée est juste après l'opcode
void addr_zero_page(CPU *cpu);  // Adresse dans la page 0 (1 octet)
void addr_absolute(CPU *cpu);   // Adresse complète (2 octets)
void addr_implied(CPU *cpu);  // Pour les instructions sans paramètre (ex: INX, TAX)
void addr_relative(CPU *cpu); // Pour les branchements (sauts conditionnels)

void addr_zero_page_x(CPU *cpu); // Adresse = (Opérande + X) & 0xFF
void addr_zero_page_y(CPU *cpu); // Adresse = (Opérande + Y) & 0xFF
void addr_absolute_x(CPU *cpu);  // Adresse = Opérande + X
void addr_absolute_y(CPU *cpu);  // Adresse = Opérande + Y
#endif