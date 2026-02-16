#include "cpu.h"
#include "addressing.h"
#include "instructions.h"
#include <stdio.h>
#include <string.h>

// Structure pour une entrée de la table
typedef struct {
    InstructionFunc instruction;
    AddrModeFunc addrmode;
    const char *name;
    u8 cycles;
} OpcodeEntry;
// ... (Includes existants)

// --- Fonctions Privées pour la Pile ---

// Écrire un octet sur la pile
 void cpu_push_byte(CPU *cpu, u8 value) {
    // L'adresse de la pile est 0x0100 + SP
    mem_write(cpu->mem, 0x0100 + cpu->SP, value);
    cpu->SP--; // La pile descend
}

// Lire un octet depuis la pile
 u8 cpu_pull_byte(CPU *cpu) {
    cpu->SP++; // La pile remonte
    return mem_read(cpu->mem, 0x0100 + cpu->SP);
}

// Écrire une adresse (16 bits) sur la pile (pour JSR)
 void cpu_push_word(CPU *cpu, u16 value) {
    // On pousse l'octet haut puis l'octet bas
    cpu_push_byte(cpu, (value >> 8) & 0xFF); // High byte
    cpu_push_byte(cpu, value & 0xFF);        // Low byte
}

// Lire une adresse (16 bits) depuis la pile (pour RTS)
 u16 cpu_pull_word(CPU *cpu) {
    u16 lo = cpu_pull_byte(cpu);
    u16 hi = cpu_pull_byte(cpu);
    return (hi << 8) | lo;
}
// LA TABLE DES OPCODES (Look-up Table)
// On ne remplit que ceux qu'on a codé pour l'instant
static OpcodeEntry lookup[256];

// Fonction d'initialisation de la table (appelée une fois)
static void init_lookup_table() {
    // On met des NOP partout par défaut pour éviter les crashs
    for (int i = 0; i < 256; i++) {
        lookup[i].instruction = ins_NOP;
        lookup[i].addrmode = addr_immediate;
        lookup[i].name = "???";
        lookup[i].cycles = 2;
    }

    // --- Instructions Implementees ---

    // LDA Immediate (A9)
    lookup[0xA9].instruction = ins_LDA;
    lookup[0xA9].addrmode = addr_immediate;
    lookup[0xA9].name = "LDA IMM";
    lookup[0xA9].cycles = 2;

    // LDA Zero Page (A5)
    lookup[0xA5].instruction = ins_LDA;
    lookup[0xA5].addrmode = addr_zero_page;
    lookup[0xA5].name = "LDA ZP";
    lookup[0xA5].cycles = 3;

    // LDA Absolute (AD)
    lookup[0xAD].instruction = ins_LDA;
    lookup[0xAD].addrmode = addr_absolute;
    lookup[0xAD].name = "LDA ABS";
    lookup[0xAD].cycles = 4;

    // LDX Immediate (A2)
    lookup[0xA2].instruction = ins_LDX;
    lookup[0xA2].addrmode = addr_immediate;
    lookup[0xA2].name = "LDX IMM";
    lookup[0xA2].cycles = 2;

    // STA Zero Page (85)
    lookup[0x85].instruction = ins_STA;
    lookup[0x85].addrmode = addr_zero_page;
    lookup[0x85].name = "STA ZP";
    lookup[0x85].cycles = 3;

    // STA Absolute (8D)
    lookup[0x8D].instruction = ins_STA;
    lookup[0x8D].addrmode = addr_absolute;
    lookup[0x8D].name = "STA ABS";
    lookup[0x8D].cycles = 4;

    // NOP (EA)
    lookup[0xEA].instruction = ins_NOP;
    lookup[0xEA].addrmode = addr_implied; // NOP n'a pas d'adressage, mais on met un défaut
    lookup[0xEA].name = "NOP";
    lookup[0xEA].cycles = 2;

        // --- Nouveaux Opcodes ---

    // TAX (AA) - Implied
    lookup[0xAA].instruction = ins_TAX;
    lookup[0xAA].addrmode = addr_implied;
    lookup[0xAA].name = "TAX";
    lookup[0xAA].cycles = 2;

    // TXA (8A) - Implied
    lookup[0x8A].instruction = ins_TXA;
    lookup[0x8A].addrmode = addr_implied;
    lookup[0x8A].name = "TXA";
    lookup[0x8A].cycles = 2;

    // INX (E8) - Implied
    lookup[0xE8].instruction = ins_INX;
    lookup[0xE8].addrmode = addr_implied;
    lookup[0xE8].name = "INX";
    lookup[0xE8].cycles = 2;

    // DEX (CA) - Implied
    lookup[0xCA].instruction = ins_DEX;
    lookup[0xCA].addrmode = addr_implied;
    lookup[0xCA].name = "DEX";
    lookup[0xCA].cycles = 2;

    // BEQ (F0) - Relative
    lookup[0xF0].instruction = ins_BEQ;
    lookup[0xF0].addrmode = addr_relative;
    lookup[0xF0].name = "BEQ";
    lookup[0xF0].cycles = 2; // Base cycles (3 si saut pris, géré dans l'instruction)

    // BNE (D0) - Relative
    lookup[0xD0].instruction = ins_BNE;
    lookup[0xD0].addrmode = addr_relative;
    lookup[0xD0].name = "BNE";
    lookup[0xD0].cycles = 2;

    // JMP Absolute (4C)
    lookup[0x4C].instruction = ins_JMP;
    lookup[0x4C].addrmode = addr_absolute;
    lookup[0x4C].name = "JMP ABS";
    lookup[0x4C].cycles = 3;

        // --- Pile ---
    
    // PHA (48)
    lookup[0x48].instruction = ins_PHA;
    lookup[0x48].addrmode = addr_implied;
    lookup[0x48].name = "PHA";
    lookup[0x48].cycles = 3;

    // PLA (68)
    lookup[0x68].instruction = ins_PLA;
    lookup[0x68].addrmode = addr_implied;
    lookup[0x68].name = "PLA";
    lookup[0x68].cycles = 4;

    // --- Sous-Programmes ---

    // JSR Absolute (20) - Appel de fonction
    lookup[0x20].instruction = ins_JSR;
    lookup[0x20].addrmode = addr_absolute;
    lookup[0x20].name = "JSR";
    lookup[0x20].cycles = 6;

    // RTS (60) - Retour de fonction
    lookup[0x60].instruction = ins_RTS;
    lookup[0x60].addrmode = addr_implied;
    lookup[0x60].name = "RTS";
    lookup[0x60].cycles = 6;

        // --- Arithmétique ---

    // ADC (Add with Carry)
    lookup[0x69].instruction = ins_ADC; lookup[0x69].addrmode = addr_immediate; lookup[0x69].name = "ADC IMM"; lookup[0x69].cycles = 2;
    lookup[0x65].instruction = ins_ADC; lookup[0x65].addrmode = addr_zero_page; lookup[0x65].name = "ADC ZP"; lookup[0x65].cycles = 3;
    lookup[0x6D].instruction = ins_ADC; lookup[0x6D].addrmode = addr_absolute; lookup[0x6D].name = "ADC ABS"; lookup[0x6D].cycles = 4;

    // SBC (Subtract with Carry)
    lookup[0xE9].instruction = ins_SBC; lookup[0xE9].addrmode = addr_immediate; lookup[0xE9].name = "SBC IMM"; lookup[0xE9].cycles = 2;
    lookup[0xE5].instruction = ins_SBC; lookup[0xE5].addrmode = addr_zero_page; lookup[0xE5].name = "SBC ZP"; lookup[0xE5].cycles = 3;
    lookup[0xED].instruction = ins_SBC; lookup[0xED].addrmode = addr_absolute; lookup[0xED].name = "SBC ABS"; lookup[0xED].cycles = 4;

    // --- Comparaison ---

    // CMP (Compare A)
    lookup[0xC9].instruction = ins_CMP; lookup[0xC9].addrmode = addr_immediate; lookup[0xC9].name = "CMP IMM"; lookup[0xC9].cycles = 2;
    lookup[0xC5].instruction = ins_CMP; lookup[0xC5].addrmode = addr_zero_page; lookup[0xC5].name = "CMP ZP"; lookup[0xC5].cycles = 3;
    lookup[0xCD].instruction = ins_CMP; lookup[0xCD].addrmode = addr_absolute; lookup[0xCD].name = "CMP ABS"; lookup[0xCD].cycles = 4;

    // CPX (Compare X)
    lookup[0xE0].instruction = ins_CPX; lookup[0xE0].addrmode = addr_immediate; lookup[0xE0].name = "CPX IMM"; lookup[0xE0].cycles = 2;
    
    // CPY (Compare Y)
    lookup[0xC0].instruction = ins_CPY; lookup[0xC0].addrmode = addr_immediate; lookup[0xC0].name = "CPY IMM"; lookup[0xC0].cycles = 2;

    // --- Logique ---

    // AND
    lookup[0x29].instruction = ins_AND; lookup[0x29].addrmode = addr_immediate; lookup[0x29].name = "AND IMM"; lookup[0x29].cycles = 2;
    
    // ORA
    lookup[0x09].instruction = ins_ORA; lookup[0x09].addrmode = addr_immediate; lookup[0x09].name = "ORA IMM"; lookup[0x09].cycles = 2;
    
    // EOR
    lookup[0x49].instruction = ins_EOR; lookup[0x49].addrmode = addr_immediate; lookup[0x49].name = "EOR IMM"; lookup[0x49].cycles = 2;

    // --- Drapeaux ---
    lookup[0x18].instruction = ins_CLC; lookup[0x18].addrmode = addr_implied; lookup[0x18].name = "CLC"; lookup[0x18].cycles = 2;
    lookup[0x38].instruction = ins_SEC; lookup[0x38].addrmode = addr_implied; lookup[0x38].name = "SEC"; lookup[0x38].cycles = 2;
    lookup[0xD8].instruction = ins_CLD; lookup[0xD8].addrmode = addr_implied; lookup[0xD8].name = "CLD"; lookup[0xD8].cycles = 2;
    lookup[0xF8].instruction = ins_SED; lookup[0xF8].addrmode = addr_implied; lookup[0xF8].name = "SED"; lookup[0xF8].cycles = 2;
    lookup[0x58].instruction = ins_CLI; lookup[0x58].addrmode = addr_implied; lookup[0x58].name = "CLI"; lookup[0x58].cycles = 2;
    lookup[0x78].instruction = ins_SEI; lookup[0x78].addrmode = addr_implied; lookup[0x78].name = "SEI"; lookup[0x78].cycles = 2;
    lookup[0xB8].instruction = ins_CLV; lookup[0xB8].addrmode = addr_implied; lookup[0xB8].name = "CLV"; lookup[0xB8].cycles = 2;

}

void cpu_set_flag(CPU *cpu, u8 flag, int value) {
    if (value) cpu->P |= flag; else cpu->P &= ~flag;
}
int cpu_get_flag(CPU *cpu, u8 flag) {
    return (cpu->P & flag) != 0;
}

void cpu_reset(CPU *cpu, Memory *mem) {
    cpu->A = 0; cpu->X = 0; cpu->Y = 0;
    cpu->SP = 0xFD;
    cpu->P = 0x24;
    cpu->cycles = 0;
    cpu->mem = mem;

    u16 lo = mem_read(mem, 0xFFFC);
    u16 hi = mem_read(mem, 0xFFFD);
    cpu->PC = (hi << 8) | lo;

    // Initialiser la table des opcodes
    init_lookup_table();
}

void cpu_step(CPU *cpu) {
    // 1. Sauvegarder l'état avant l'action
    u16 pc_before = cpu->PC;
    u8 sp_before = cpu->SP;

    // 2. FETCH
    u8 opcode = mem_read(cpu->mem, cpu->PC);
    cpu->PC++;

    // 3. DECODE
    OpcodeEntry entry = lookup[opcode];

    // 4. DEBUG : Afficher ce qui va se passer
    // %04X = adresse en hexadécimal 4 chiffres
    // %02X = nombre en hexadécimal 2 chiffres
    printf("[TRACE] PC: %04X | Opcode: %02X | Instruction: %-8s | SP: %02X -> ", 
           pc_before, opcode, entry.name, sp_before);

    // 5. ADDRESSING & EXECUTE
    entry.addrmode(cpu);
    entry.instruction(cpu);

    // 6. Afficher l'état APRÈS l'action
    printf("A: %02X, X: %02X, SP: %02X, PC: %04X\n", 
           cpu->A, cpu->X, cpu->SP, cpu->PC);

    // 7. CYCLES
    cpu->cycles += entry.cycles;
}