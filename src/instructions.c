#include "instructions.h"
#include <stdio.h>
// LDA : Charge une valeur dans A
void ins_LDA(CPU *cpu) {
    cpu->A = cpu->fetched; // La valeur a été calculée par l'adressage
    cpu_set_flag(cpu, FLAG_Z, cpu->A == 0);
    cpu_set_flag(cpu, FLAG_N, (cpu->A & 0x80) != 0);
}

// LDX : Charge une valeur dans X
void ins_LDX(CPU *cpu) {
    cpu->X = cpu->fetched;
    cpu_set_flag(cpu, FLAG_Z, cpu->X == 0);
    cpu_set_flag(cpu, FLAG_N, (cpu->X & 0x80) != 0);
}

// STA : Stocke A en mémoire
void ins_STA(CPU *cpu) {
    // Pour STA, on n'a pas besoin de 'fetched' (lecture),
    // on utilise l'adresse calculée 'addr_abs'
    mem_write(cpu->mem, cpu->addr_abs, cpu->A);
}

// NOP : Ne rien faire
void ins_NOP(CPU *cpu) {
    (void)cpu; // Evite le warning
}

// --- Transferts ---

void ins_TAX(CPU *cpu) {
    cpu->X = cpu->A;
    cpu_set_flag(cpu, FLAG_Z, cpu->X == 0);
    cpu_set_flag(cpu, FLAG_N, (cpu->X & 0x80) != 0);
}

void ins_TXA(CPU *cpu) {
    cpu->A = cpu->X;
    cpu_set_flag(cpu, FLAG_Z, cpu->A == 0);
    cpu_set_flag(cpu, FLAG_N, (cpu->A & 0x80) != 0);
}

// --- Incréments ---

void ins_INX(CPU *cpu) {
    //printf("[DEBUG] INX appelé ! X passe de %d à %d\n", cpu->X, cpu->X + 1);

    cpu->X++;
    cpu_set_flag(cpu, FLAG_Z, cpu->X == 0);
    cpu_set_flag(cpu, FLAG_N, (cpu->X & 0x80) != 0);
}

void ins_DEX(CPU *cpu) {
    cpu->X--;
    cpu_set_flag(cpu, FLAG_Z, cpu->X == 0);
    cpu_set_flag(cpu, FLAG_N, (cpu->X & 0x80) != 0);
}

// --- Branchements ---
// Pour ces instructions, addr_abs a déjà été calculée par addr_relative

void ins_BEQ(CPU *cpu) {
    if (cpu_get_flag(cpu, FLAG_Z)) {
        cpu->PC = cpu->addr_abs; // On saute !
        cpu->cycles++; // Un cycle de plus car on a pris le saut
    }
}

void ins_BNE(CPU *cpu) {
    // DEBUG : Si on est à l'adresse 36BC (le blocage)
    if (cpu->PC == 0x36C0) { // PC a avancé après D0 FE
        //printf("[BNE DEBUT] P = 0x%02X (Flag Z = %d)\n", cpu->P, cpu_get_flag(cpu, FLAG_Z));
    }

    if (!cpu_get_flag(cpu, FLAG_Z)) {
        cpu->PC = cpu->addr_abs;
        cpu->cycles++;
    }
}

// --- Contrôle ---

void ins_JMP(CPU *cpu) {
    // Pour JMP, addr_abs a été calculée par addr_absolute
    cpu->PC = cpu->addr_abs;
}

// --- Instructions Pile ---

// PHA : Push Accumulator
void ins_PHA(CPU *cpu) {
    cpu_push_byte(cpu, cpu->A);
}

void ins_PLA(CPU *cpu) {
    cpu->A = cpu_pull_byte(cpu);
    cpu_set_flag(cpu, FLAG_Z, cpu->A == 0);
    cpu_set_flag(cpu, FLAG_N, (cpu->A & 0x80) != 0);
}

// --- Instructions Sous-Programmes ---

// JSR : Jump to SubRoutine (Appel de fonction)
void ins_JSR(CPU *cpu) {
    // addr_abs a été calculée par addr_absolute
    // On doit pousser PC-1 sur la pile (standard 6502)
    cpu_push_word(cpu, cpu->PC - 1);
    
    // Sauter à l'adresse
    cpu->PC = cpu->addr_abs;
}

// RTS : ReTurn from Subroutine (Retour de fonction)
void ins_RTS(CPU *cpu) {
    // Retirer l'adresse de la pile
    u16 return_addr = cpu_pull_word(cpu);
    
    // Restaurer PC (et ajouter 1 car on avait sauvé PC-1)
    cpu->PC = return_addr + 1;
}
// --- Arithmétique ---

void ins_ADC(CPU *cpu) {
    u8 value = cpu->fetched;
    u16 sum = (u16)cpu->A + (u16)value + (u16)cpu_get_flag(cpu, FLAG_C);

    // Mise à jour des flags
    cpu_set_flag(cpu, FLAG_C, sum > 0xFF);       // Carry si résultat > 255
    cpu_set_flag(cpu, FLAG_Z, (sum & 0x00FF) == 0); // Zero
    cpu_set_flag(cpu, FLAG_N, sum & 0x80);       // Négatif (bit 7)
    
    // Overflow (V) : Si le signe du résultat est incorrect par rapport aux opérandes
    // Formule complexe simplifiée : V = (A ^ resultat) & (valeur ^ resultat) & 0x80
cpu_set_flag(cpu, FLAG_V, ((~(cpu->A ^ value) & (cpu->A ^ sum) & 0x80) != 0));
    cpu->A = sum & 0xFF; // On garde l'octet bas
}
void ins_SBC(CPU *cpu) {
    u8 value = cpu->fetched;

    // Vérifie si le mode Décimal (BCD) est actif
    if (cpu_get_flag(cpu, FLAG_D)) {
        // Soustraction BCD (Decimal)
        // Algorithme simplifié mais efficace pour passer les tests
        int diff = (cpu->A & 0x0F) - (value & 0x0F) - (1 - cpu_get_flag(cpu, FLAG_C));
        if (diff < 0) diff -= 6;
        int al = diff;
        diff = (cpu->A >> 4) - (value >> 4) + (al >> 4); // Ajoute la retenue négative
        if (diff < 0) diff -= 6;
        
        // Mise à jour des flags
        cpu_set_flag(cpu, FLAG_Z, ((cpu->A - value - (1 - cpu_get_flag(cpu, FLAG_C))) & 0xFF) == 0);
        cpu_set_flag(cpu, FLAG_N, diff & 0x80);
        cpu_set_flag(cpu, FLAG_C, diff <= 0); // Carry inversé
        
        cpu->A = ((diff << 4) | (al & 0x0F)) & 0xFF;
        
        // En BCD, le flag V n'est pas défini de la même manière, on le met souvent à 0 ou on le laisse
        cpu_set_flag(cpu, FLAG_V, 0); 
    } else {
        // Soustraction Binaire (Ton code normal)
        u16 sub = (u16)cpu->A - (u16)value - (1 - (u16)cpu_get_flag(cpu, FLAG_C));
        cpu_set_flag(cpu, FLAG_C, sub < 0x100);
        cpu_set_flag(cpu, FLAG_Z, (sub & 0x00FF) == 0);
        cpu_set_flag(cpu, FLAG_N, sub & 0x80);
        cpu_set_flag(cpu, FLAG_V, ((cpu->A ^ value) & (cpu->A ^ sub) & 0x80));
        cpu->A = sub & 0xFF;
    }
}

// --- Comparaison ---
// Compare un registre avec une valeur. Le registre n'est pas modifié.
// Flags : Z (égalité), C (Registre >= Valeur), N (Signe du résultat)
void ins_CMP(CPU *cpu) {
    u8 value = cpu->fetched;
    //u8 value = mem_read(cpu->mem, cpu->addr_abs);
    u16 result = (u16)cpu->A - (u16)value;

    // --- DEBUG TEMPORAIRE ---
    // Si A vaut 0x9A et que le résultat n'est pas zéro, il y a un bug
    if (cpu->A == 0x9A && result != 0) {
        printf("\n[DEBUG CMP] A: 0x%02X, Fetched: 0x%02X, Result: 0x%04X\n", 
               cpu->A, value, result);
        printf("Addr lue: 0x%04X\n", cpu->addr_abs);
    }
    // ------------------------

    cpu_set_flag(cpu, FLAG_C, cpu->A >= value);
    cpu_set_flag(cpu, FLAG_Z, result == 0);
    cpu_set_flag(cpu, FLAG_N, result & 0x80);

     // DEBUG : Afficher si on est au point de blocage
    if (cpu->PC == 0x36BC || cpu->PC == 0x36BD) { 
        printf("\n[DEBUG TEST 96] PC: 0x%04X | A: 0x%02X | Fetched: 0x%02X | Addr: 0x%04X\n", 
               cpu->PC, cpu->A, value, cpu->addr_abs);
    }
}
void ins_CPX(CPU *cpu) {
    u8 value = cpu->fetched;
    u16 result = (u16)cpu->X - (u16)value;

    cpu_set_flag(cpu, FLAG_C, cpu->X >= value);
    cpu_set_flag(cpu, FLAG_Z, result == 0);
    cpu_set_flag(cpu, FLAG_N, result & 0x80);
}

void ins_CPY(CPU *cpu) {
    u8 value = cpu->fetched;
    u16 result = (u16)cpu->Y - (u16)value;

    cpu_set_flag(cpu, FLAG_C, cpu->Y >= value);
    cpu_set_flag(cpu, FLAG_Z, result == 0);
    cpu_set_flag(cpu, FLAG_N, result & 0x80);
}

// --- Logique ---

void ins_AND(CPU *cpu) {
    cpu->A = cpu->A & cpu->fetched;
    cpu_set_flag(cpu, FLAG_Z, cpu->A == 0);
    cpu_set_flag(cpu, FLAG_N, (cpu->A & 0x80) != 0);
}

void ins_ORA(CPU *cpu) {
    cpu->A = cpu->A | cpu->fetched;
    cpu_set_flag(cpu, FLAG_Z, cpu->A == 0);
    cpu_set_flag(cpu, FLAG_N, (cpu->A & 0x80) != 0);
}

void ins_EOR(CPU *cpu) {
    cpu->A = cpu->A ^ cpu->fetched;
    cpu_set_flag(cpu, FLAG_Z, cpu->A == 0);
    cpu_set_flag(cpu, FLAG_N, (cpu->A & 0x80) != 0);
}

// --- Drapeaux (Flags) ---

void ins_CLC(CPU *cpu) { cpu_set_flag(cpu, FLAG_C, 0); } // Clear Carry
void ins_SEC(CPU *cpu) { cpu_set_flag(cpu, FLAG_C, 1); } // Set Carry
void ins_CLD(CPU *cpu) { cpu_set_flag(cpu, FLAG_D, 0); } // Clear Decimal
void ins_SED(CPU *cpu) { cpu_set_flag(cpu, FLAG_D, 1); } // Set Decimal
void ins_CLI(CPU *cpu) { cpu_set_flag(cpu, FLAG_I, 0); } // Clear Interrupt
void ins_SEI(CPU *cpu) { cpu_set_flag(cpu, FLAG_I, 1); } // Set Interrupt
void ins_CLV(CPU *cpu) { cpu_set_flag(cpu, FLAG_V, 0); } // Clear Overflow

// --- Registre Y ---

void ins_LDY(CPU *cpu) {
    cpu->Y = cpu->fetched;
    cpu_set_flag(cpu, FLAG_Z, cpu->Y == 0);
    cpu_set_flag(cpu, FLAG_N, (cpu->Y & 0x80) != 0);
}

void ins_STY(CPU *cpu) {
    mem_write(cpu->mem, cpu->addr_abs, cpu->Y);
}

void ins_INY(CPU *cpu) {
    cpu->Y++;
    cpu_set_flag(cpu, FLAG_Z, cpu->Y == 0);
    cpu_set_flag(cpu, FLAG_N, (cpu->Y & 0x80) != 0);
}

void ins_DEY(CPU *cpu) {
    cpu->Y--;
    cpu_set_flag(cpu, FLAG_Z, cpu->Y == 0);
    cpu_set_flag(cpu, FLAG_N, (cpu->Y & 0x80) != 0);
}

// --- Mémoire INC/DEC ---
// C'est spécial : on lit la valeur, on modifie, et on réécrit dans addr_abs

void ins_INC(CPU *cpu) {
    u8 val = cpu->fetched;
    val++;
    cpu_set_flag(cpu, FLAG_Z, val == 0);
    cpu_set_flag(cpu, FLAG_N, (val & 0x80) != 0);
    
    // Réécrire en mémoire
    mem_write(cpu->mem, cpu->addr_abs, val);
}

void ins_DEC(CPU *cpu) {
    u8 val = cpu->fetched;
    val--;
    cpu_set_flag(cpu, FLAG_Z, val == 0);
    cpu_set_flag(cpu, FLAG_N, (val & 0x80) != 0);
    
    mem_write(cpu->mem, cpu->addr_abs, val);
}

// --- Bits (Shifts) ---// 1. ASL Accumulator (pour le registre A)
void ins_ASL_ACC(CPU *cpu) {
    cpu_set_flag(cpu, FLAG_C, (cpu->A & 0x80) != 0);
    cpu->A = cpu->A << 1;
    cpu_set_flag(cpu, FLAG_Z, cpu->A == 0);
    cpu_set_flag(cpu, FLAG_N, (cpu->A & 0x80) != 0);
}

// 2. ASL Mémoire (pour une adresse)
void ins_ASL(CPU *cpu) {
    u8 val = cpu->fetched;
    cpu_set_flag(cpu, FLAG_C, (val & 0x80) != 0);
    val = val << 1;
    cpu_set_flag(cpu, FLAG_Z, val == 0);
    cpu_set_flag(cpu, FLAG_N, (val & 0x80) != 0);
    mem_write(cpu->mem, cpu->addr_abs, val);
}

// 3. LSR Accumulator
void ins_LSR_ACC(CPU *cpu) {
    cpu_set_flag(cpu, FLAG_C, (cpu->A & 0x01) != 0); 
    cpu->A = cpu->A >> 1;
    cpu_set_flag(cpu, FLAG_Z, cpu->A == 0);
    cpu_set_flag(cpu, FLAG_N, 0); 
}

// 4. LSR Mémoire
void ins_LSR(CPU *cpu) {
    u8 val = cpu->fetched;
    cpu_set_flag(cpu, FLAG_C, (val & 0x01) != 0);
    val = val >> 1;
    cpu_set_flag(cpu, FLAG_Z, val == 0);
    cpu_set_flag(cpu, FLAG_N, 0);
    mem_write(cpu->mem, cpu->addr_abs, val);
}

// BRK : Interruption logicielle (Opcode 0x00)
void ins_BRK(CPU *cpu) {
u16 pc_to_save = cpu->PC + 1;
cpu_push_byte(cpu, (pc_to_save >> 8) & 0xFF);
cpu_push_byte(cpu, pc_to_save & 0xFF);
cpu_push_byte(cpu, cpu->P | 0x30);
cpu_set_flag(cpu, FLAG_I, 1);
u16 lo = mem_read(cpu->mem, 0xFFFE);
u16 hi = mem_read(cpu->mem, 0xFFFF);
cpu->PC = (hi << 8) | lo;
}

void ins_RTI(CPU *cpu) {
u8 status = cpu_pull_byte(cpu);
cpu->P = (status & 0xEF) | 0x20;
u8 lo = cpu_pull_byte(cpu);
u8 hi = cpu_pull_byte(cpu);
cpu->PC = (hi << 8) | lo;
}

void ins_TXS(CPU *cpu) {
    cpu->SP = cpu->X;
}

void ins_TSX(CPU *cpu) {
    cpu->X = cpu->SP;
    cpu_set_flag(cpu, FLAG_Z, cpu->X == 0);
    cpu_set_flag(cpu, FLAG_N, (cpu->X & 0x80) != 0);
}

// TYA : Transfer Y to Accumulator
void ins_TYA(CPU *cpu) {
    cpu->A = cpu->Y;
    cpu_set_flag(cpu, FLAG_Z, cpu->A == 0);
    cpu_set_flag(cpu, FLAG_N, (cpu->A & 0x80) != 0);
}

// TAY : Transfer Accumulator to Y
void ins_TAY(CPU *cpu) {
    cpu->Y = cpu->A;
    cpu_set_flag(cpu, FLAG_Z, cpu->Y == 0);
    cpu_set_flag(cpu, FLAG_N, (cpu->Y & 0x80) != 0);
}

// --- Branchements Conditionnels (Suite) ---

// BPL (10) : Branch if Plus (N == 0)
void ins_BPL(CPU *cpu) {
    if (!cpu_get_flag(cpu, FLAG_N)) {
        cpu->PC = cpu->addr_abs;
        cpu->cycles++;
    }
}

// BMI (30) : Branch if Minus (N == 1)
void ins_BMI(CPU *cpu) {
    if (cpu_get_flag(cpu, FLAG_N)) {
        cpu->PC = cpu->addr_abs;
        cpu->cycles++;
    }
}

// BCS (B0) : Branch if Carry Set (C == 1)
void ins_BCS(CPU *cpu) {
    if (cpu_get_flag(cpu, FLAG_C)) {
        cpu->PC = cpu->addr_abs;
        cpu->cycles++;
    }
}

// BCC (90) : Branch if Carry Clear (C == 0)
void ins_BCC(CPU *cpu) {
    if (!cpu_get_flag(cpu, FLAG_C)) {
        cpu->PC = cpu->addr_abs;
        cpu->cycles++;
    }
}

// BVS (70) : Branch if Overflow Set (V == 1)
void ins_BVS(CPU *cpu) {
    if (cpu_get_flag(cpu, FLAG_V)) {
        cpu->PC = cpu->addr_abs;
        cpu->cycles++;
    }
}

// BVC (50) : Branch if Overflow Clear (V == 0)
void ins_BVC(CPU *cpu) {
    if (!cpu_get_flag(cpu, FLAG_V)) {
        cpu->PC = cpu->addr_abs;
        cpu->cycles++;
    }
}

// PLP : Pull Processor Status (Restaure les flags depuis la pile)
void ins_PLP(CPU *cpu) {
u8 val = cpu_pull_byte(cpu);
cpu->P = (val & 0xEF) | 0x20;
}
// PHP : Push Processor Status (Sauvegarde les flags sur la pile)
void ins_PHP(CPU *cpu) {
cpu_push_byte(cpu, cpu->P | 0x30);
}

// --- BIT (Bit Test) ---
void ins_BIT(CPU *cpu) {
    u8 val = cpu->fetched;
    
    // Le test BIT met à jour N et V selon les bits 7 et 6 de la mémoire lue
    cpu_set_flag(cpu, FLAG_N, (val & 0x80) != 0); // Bit 7 -> N
    cpu_set_flag(cpu, FLAG_V, (val & 0x40) != 0); // Bit 6 -> V
    
    // Le flag Z est mis si A AND Mémoire == 0
    cpu_set_flag(cpu, FLAG_Z, (cpu->A & val) == 0);
}

// --- ROL (Rotate Left) ---
// Décalage à gauche, le bit 7 va dans Carry, Carry va dans le bit 0
void ins_ROL_ACC(CPU *cpu) {
    u8 val = cpu->A;
    u8 new_carry = (val & 0x80) != 0;
    val = (val << 1) | cpu_get_flag(cpu, FLAG_C); // Insère l'ancien carry
    
    cpu_set_flag(cpu, FLAG_C, new_carry);
    cpu_set_flag(cpu, FLAG_Z, val == 0);
    cpu_set_flag(cpu, FLAG_N, (val & 0x80) != 0);
    cpu->A = val;
}

void ins_ROL(CPU *cpu) {
    u8 val = cpu->fetched;
    u8 new_carry = (val & 0x80) != 0;
    val = (val << 1) | cpu_get_flag(cpu, FLAG_C);
    
    cpu_set_flag(cpu, FLAG_C, new_carry);
    cpu_set_flag(cpu, FLAG_Z, val == 0);
    cpu_set_flag(cpu, FLAG_N, (val & 0x80) != 0);
    mem_write(cpu->mem, cpu->addr_abs, val);
}

// --- ROR (Rotate Right) ---
// Décalage à droite, le bit 0 va dans Carry, Carry va dans le bit 7
void ins_ROR_ACC(CPU *cpu) {
    u8 val = cpu->A;
    u8 new_carry = val & 0x01;
    val = (val >> 1) | (cpu_get_flag(cpu, FLAG_C) << 7); // Insère l'ancien carry au bit 7
    
    cpu_set_flag(cpu, FLAG_C, new_carry);
    cpu_set_flag(cpu, FLAG_Z, val == 0);
    cpu_set_flag(cpu, FLAG_N, (val & 0x80) != 0);
    cpu->A = val;
}
// STX : Store X Register
void ins_STX(CPU *cpu) {
    mem_write(cpu->mem, cpu->addr_abs, cpu->X);
}
void ins_ROR(CPU *cpu) {
    u8 val = cpu->fetched;
    u8 new_carry = val & 0x01;
    val = (val >> 1) | (cpu_get_flag(cpu, FLAG_C) << 7);
    
    cpu_set_flag(cpu, FLAG_C, new_carry);
    cpu_set_flag(cpu, FLAG_Z, val == 0);
    cpu_set_flag(cpu, FLAG_N, (val & 0x80) != 0);
    mem_write(cpu->mem, cpu->addr_abs, val);
}