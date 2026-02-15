#include "addressing.h"
#include "stdio.h"
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

// Mode Implied : L'instruction n'a pas d'opérande (ex: NOP, INX)
void addr_implied(CPU *cpu) {
    // Rien à faire, pas d'adresse à calculer.
    // On met fetched à 0 par sécurité
    cpu->fetched = 0;
}

// Mode Relative : Utilisé pour les sauts conditionnels (BNE, BEQ...)
// L'opérande est un nombre signé (s8) qui dit de combien sauter.
void addr_relative(CPU *cpu) {
    // 1. Lire l'offset (signé)
    s8 offset = (s8)mem_read(cpu->mem, cpu->PC);
    cpu->PC++;

    // 2. Calculer l'adresse de destination
    // L'adresse cible = PC actuel + l'offset
    // Note: Le PC pointe déjà sur l'instruction suivante ici
    cpu->addr_abs = cpu->PC + offset;
    
    // On ne touche pas à fetched car les branchements n'ont pas besoin de lire une donnée,
    // ils modifient juste le PC.
}