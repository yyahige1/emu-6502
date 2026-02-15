#include "addressing.h"

// Mode Immediate: La valeur est celle à PC
void addr_immediate(CPU *cpu) {
    // L'adresse "effective" est juste PC, mais pour simplifier,
    // on lit directement la valeur dans 'fetched'
    cpu->fetched = mem_read(cpu->mem, cpu->PC);
    cpu->PC++;
}

// Mode Zero Page: L'adresse est un octet (0x00 à 0xFF)
void addr_zero_page(CPU *cpu) {
    cpu->addr_abs = mem_read(cpu->mem, cpu->PC); // Lit l'adresse
    cpu->PC++;
    // On lit la donnée à cette adresse
    cpu->fetched = mem_read(cpu->mem, cpu->addr_abs);
}

// Mode Absolute: L'adresse est sur 2 octets
void addr_absolute(CPU *cpu) {
    u16 lo = mem_read(cpu->mem, cpu->PC);
    cpu->PC++;
    u16 hi = mem_read(cpu->mem, cpu->PC);
    cpu->PC++;
    
    cpu->addr_abs = (hi << 8) | lo;
    // On lit la donnée à cette adresse
    cpu->fetched = mem_read(cpu->mem, cpu->addr_abs);
}