#include "addressing.h"
#include "stdio.h"

// Fonction interne pour lire une adresse 16 bits et avancer le PC
static u16 addr_absolute_helper(CPU *cpu) {
    u16 lo = mem_read(cpu->mem, cpu->PC);
    cpu->PC++;
    u16 hi = mem_read(cpu->mem, cpu->PC);
    cpu->PC++;
    return (hi << 8) | lo;
}
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

// Mode Zero Page,X : L'adresse est (base + X) modulo 256 (on reste en page 0)
void addr_zero_page_x(CPU *cpu) {
    u8 base = mem_read(cpu->mem, cpu->PC);
    cpu->PC++;
    
    // L'addition se fait sur 8 bits, on ignore la retenue au-delà de 255
    cpu->addr_abs = (base + cpu->X) & 0x00FF; 
    
    cpu->fetched = mem_read(cpu->mem, cpu->addr_abs);
}

// Mode Zero Page,Y : Similaire mais avec Y (rare, utilisé pour LDX/STX)
void addr_zero_page_y(CPU *cpu) {
    u8 base = mem_read(cpu->mem, cpu->PC);
    cpu->PC++;
    
    cpu->addr_abs = (base + cpu->Y) & 0x00FF;
    
    cpu->fetched = mem_read(cpu->mem, cpu->addr_abs);
}

// Mode Absolute,X : Adresse 16 bits + registre X
void addr_absolute_x(CPU *cpu) {
    u16 base = addr_absolute_helper(cpu); // On va créer cette aide ci-dessous
    
    cpu->addr_abs = base + cpu->X;
    cpu->fetched = mem_read(cpu->mem, cpu->addr_abs);
}

// Mode Absolute,Y : Adresse 16 bits + registre Y
void addr_absolute_y(CPU *cpu) {
    u16 base = addr_absolute_helper(cpu);
    
    cpu->addr_abs = base + cpu->Y;
    cpu->fetched = mem_read(cpu->mem, cpu->addr_abs);
}

// Mode Accumulator : L'opération se fait sur le registre A
void addr_accumulator(CPU *cpu) {
    // On met fetched à la valeur de A pour que l'instruction puisse travailler dessus
    cpu->fetched = cpu->A;
}