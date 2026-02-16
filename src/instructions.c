#include "instructions.h"

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
    if (!cpu_get_flag(cpu, FLAG_Z)) { // Si Z est à 0
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
    cpu_set_flag(cpu, FLAG_V, (~(cpu->A ^ value) & (cpu->A ^ sum) & 0x80));

    cpu->A = sum & 0xFF; // On garde l'octet bas
}

void ins_SBC(CPU *cpu) {
    u8 value = cpu->fetched;
    u16 sub = (u16)cpu->A - (u16)value - (1 - (u16)cpu_get_flag(cpu, FLAG_C));

    cpu_set_flag(cpu, FLAG_C, sub < 0x100); // Carry si pas d'emprunt (inversé en soustraction)
    cpu_set_flag(cpu, FLAG_Z, (sub & 0x00FF) == 0);
    cpu_set_flag(cpu, FLAG_N, sub & 0x80);
    cpu_set_flag(cpu, FLAG_V, ((cpu->A ^ value) & (cpu->A ^ sub) & 0x80));

    cpu->A = sub & 0xFF;
}

// --- Comparaison ---
// Compare un registre avec une valeur. Le registre n'est pas modifié.
// Flags : Z (égalité), C (Registre >= Valeur), N (Signe du résultat)

void ins_CMP(CPU *cpu) {
    u8 value = cpu->fetched;
    u16 result = (u16)cpu->A - (u16)value;

    cpu_set_flag(cpu, FLAG_C, cpu->A >= value); // Carry si A >= valeur
    cpu_set_flag(cpu, FLAG_Z, result == 0);     // Zero si A == valeur
    cpu_set_flag(cpu, FLAG_N, result & 0x80);   // Négatif
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