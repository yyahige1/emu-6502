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

// Mode Indirect : Utilisé par JMP (0x6C)
void addr_indirect(CPU *cpu) {
    // 1. Lire l'adresse pointeur (16 bits)
    u16 ptr_lo = mem_read(cpu->mem, cpu->PC);
    cpu->PC++;
    u16 ptr_hi = mem_read(cpu->mem, cpu->PC);
    cpu->PC++;
    
    u16 ptr = (ptr_hi << 8) | ptr_lo;

    // 2. Lire l'adresse de destination à l'adresse pointeur
    // Simulation du Bug du 6502 : Si le pointeur est sur une frontière de page (ex: $xxFF),
    // l'octet haut est lu au début de la même page (ex: $xx00) au lieu de la page suivante.
    
    u16 addr_lo = mem_read(cpu->mem, ptr);
    u16 addr_hi;
    
    // Si le pointeur fini par FF, on fait l'erreur (wrap around)
    if ((ptr & 0x00FF) == 0x00FF) {
        addr_hi = mem_read(cpu->mem, ptr & 0xFF00); // On revient au début de la page
    } else {
        addr_hi = mem_read(cpu->mem, ptr + 1); // Cas normal
    }
    
    cpu->addr_abs = (addr_hi << 8) | addr_lo;
}
// Mode Zero Page,Y
// Mode (Indirect, X) : "Indexed Indirect"
// Ex: LDA ($20, X). On prend l'adresse $20, on ajoute X, on lit l'adresse réelle à cet endroit.
void addr_indirect_x(CPU *cpu) {
    u8 zp_base = mem_read(cpu->mem, cpu->PC);
    cpu->PC++;
    
    // L'adresse du pointeur est (zp_base + X) & 0xFF (on reste dans la Zero Page)
    u16 ptr_addr = (u16)(zp_base + cpu->X) & 0x00FF;
    
    // On lit l'adresse 16 bits à l'adresse du pointeur
    u16 lo = mem_read(cpu->mem, ptr_addr);
    u16 hi = mem_read(cpu->mem, (ptr_addr + 1) & 0x00FF); // Wrap si on dépasse la page
    
    cpu->addr_abs = (hi << 8) | lo;
    cpu->fetched = mem_read(cpu->mem, cpu->addr_abs);
}

// Mode (Indirect), Y : "Indirect Indexed"
// Ex: LDA ($20), Y. On lit l'adresse à $20, puis on ajoute Y.
void addr_indirect_y(CPU *cpu) {
    u8 zp_base = mem_read(cpu->mem, cpu->PC);
    cpu->PC++;
    
    // On lit l'adresse 16 bits stockée dans la Zero Page (sans ajouter Y !)
    u16 lo = mem_read(cpu->mem, (u16)zp_base);
    u16 hi = mem_read(cpu->mem, (u16)((zp_base + 1) & 0xFF)); // Wrap
    
    u16 base = (hi << 8) | lo;
    
    cpu->addr_abs = base + cpu->Y;
    cpu->fetched = mem_read(cpu->mem, cpu->addr_abs);
}