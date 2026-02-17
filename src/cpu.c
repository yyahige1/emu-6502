#include "cpu.h"
#include "addressing.h"
#include "instructions.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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


     // --- Instructions Indexées par X ---

    // STA Zero Page,X (95)
    lookup[0x95].instruction = ins_STA;
    lookup[0x95].addrmode = addr_zero_page_x;
    lookup[0x95].name = "STA ZP,X";
    lookup[0x95].cycles = 4;

    // LDA Zero Page,X (B5)
    lookup[0xB5].instruction = ins_LDA;
    lookup[0xB5].addrmode = addr_zero_page_x;
    lookup[0xB5].name = "LDA ZP,X";
    lookup[0xB5].cycles = 4;

    // LDA Absolute,X (BD)
    lookup[0xBD].instruction = ins_LDA;
    lookup[0xBD].addrmode = addr_absolute_x;
    lookup[0xBD].name = "LDA ABS,X";
    lookup[0xBD].cycles = 4; // (+1 si page traverse, on simplifie pour l'instant)

    // ADC Absolute,X (7D)
    lookup[0x7D].instruction = ins_ADC;
    lookup[0x7D].addrmode = addr_absolute_x;
    lookup[0x7D].name = "ADC ABS,X";
    lookup[0x7D].cycles = 4;

    // --- Instructions Indexées par Y ---

    // LDA Absolute,Y (B9)
    lookup[0xB9].instruction = ins_LDA;
    lookup[0xB9].addrmode = addr_absolute_y;
    lookup[0xB9].name = "LDA ABS,Y";
    lookup[0xB9].cycles = 4;

        // --- LDY (Load Y) ---
    lookup[0xA0].instruction = ins_LDY; lookup[0xA0].addrmode = addr_immediate; lookup[0xA0].name = "LDY IMM"; lookup[0xA0].cycles = 2;
    lookup[0xA4].instruction = ins_LDY; lookup[0xA4].addrmode = addr_zero_page; lookup[0xA4].name = "LDY ZP"; lookup[0xA4].cycles = 3;
    lookup[0xAC].instruction = ins_LDY; lookup[0xAC].addrmode = addr_absolute; lookup[0xAC].name = "LDY ABS"; lookup[0xAC].cycles = 4;
    lookup[0xB4].instruction = ins_LDY; lookup[0xB4].addrmode = addr_zero_page_x; lookup[0xB4].name = "LDY ZP,X"; lookup[0xB4].cycles = 4;
    lookup[0xBC].instruction = ins_LDY; lookup[0xBC].addrmode = addr_absolute_x; lookup[0xBC].name = "LDY ABS,X"; lookup[0xBC].cycles = 4;

    // --- STY (Store Y) ---
    lookup[0x84].instruction = ins_STY; lookup[0x84].addrmode = addr_zero_page; lookup[0x84].name = "STY ZP"; lookup[0x84].cycles = 3;
    lookup[0x8C].instruction = ins_STY; lookup[0x8C].addrmode = addr_absolute; lookup[0x8C].name = "STY ABS"; lookup[0x8C].cycles = 4;
    lookup[0x94].instruction = ins_STY; lookup[0x94].addrmode = addr_zero_page_x; lookup[0x94].name = "STY ZP,X"; lookup[0x94].cycles = 4;

    // --- INY / DEY ---
    lookup[0xC8].instruction = ins_INY; lookup[0xC8].addrmode = addr_implied; lookup[0xC8].name = "INY"; lookup[0xC8].cycles = 2;
    lookup[0x88].instruction = ins_DEY; lookup[0x88].addrmode = addr_implied; lookup[0x88].name = "DEY"; lookup[0x88].cycles = 2;

    // --- INC / DEC (Mémoire) ---
    lookup[0xE6].instruction = ins_INC; lookup[0xE6].addrmode = addr_zero_page; lookup[0xE6].name = "INC ZP"; lookup[0xE6].cycles = 5;
    lookup[0xF6].instruction = ins_INC; lookup[0xF6].addrmode = addr_zero_page_x; lookup[0xF6].name = "INC ZP,X"; lookup[0xF6].cycles = 6;
    lookup[0xEE].instruction = ins_INC; lookup[0xEE].addrmode = addr_absolute; lookup[0xEE].name = "INC ABS"; lookup[0xEE].cycles = 6;
    
    lookup[0xC6].instruction = ins_DEC; lookup[0xC6].addrmode = addr_zero_page; lookup[0xC6].name = "DEC ZP"; lookup[0xC6].cycles = 5;
    lookup[0xD6].instruction = ins_DEC; lookup[0xD6].addrmode = addr_zero_page_x; lookup[0xD6].name = "DEC ZP,X"; lookup[0xD6].cycles = 6;
    lookup[0xCE].instruction = ins_DEC; lookup[0xCE].addrmode = addr_absolute; lookup[0xCE].name = "DEC ABS"; lookup[0xCE].cycles = 6;

    // --- ASL / LSR ---
    lookup[0x0A].instruction = ins_ASL_ACC; lookup[0x0A].addrmode = addr_implied; lookup[0x0A].name = "ASL A"; lookup[0x0A].cycles = 2;
    lookup[0x06].instruction = ins_ASL; lookup[0x06].addrmode = addr_zero_page; lookup[0x06].name = "ASL ZP"; lookup[0x06].cycles = 5;
    lookup[0x0E].instruction = ins_ASL; lookup[0x0E].addrmode = addr_absolute; lookup[0x0E].name = "ASL ABS"; lookup[0x0E].cycles = 6;

    lookup[0x4A].instruction = ins_LSR_ACC; lookup[0x4A].addrmode = addr_implied; lookup[0x4A].name = "LSR A"; lookup[0x4A].cycles = 2;
    lookup[0x46].instruction = ins_LSR; lookup[0x46].addrmode = addr_zero_page; lookup[0x46].name = "LSR ZP"; lookup[0x46].cycles = 5;
    lookup[0x4E].instruction = ins_LSR; lookup[0x4E].addrmode = addr_absolute; lookup[0x4E].name = "LSR ABS"; lookup[0x4E].cycles = 6;
    // --- Interruptions ---
    lookup[0x00].instruction = ins_BRK; lookup[0x00].addrmode = addr_implied; lookup[0x00].name = "BRK"; lookup[0x00].cycles = 7;
    lookup[0x40].instruction = ins_RTI; lookup[0x40].addrmode = addr_implied; lookup[0x40].name = "RTI"; lookup[0x40].cycles = 6;
    // TXS (9A) - Transfert X vers Stack Pointer
    lookup[0x9A].instruction = ins_TXS;
    lookup[0x9A].addrmode = addr_implied;
    lookup[0x9A].name = "TXS";
    lookup[0x9A].cycles = 2;

    // TSX (BA) - Transfert Stack Pointer vers X
    lookup[0xBA].instruction = ins_TSX;
    lookup[0xBA].addrmode = addr_implied;
    lookup[0xBA].name = "TSX";
    lookup[0xBA].cycles = 2;

    // TYA (98)
    lookup[0x98].instruction = ins_TYA;
    lookup[0x98].addrmode = addr_implied;
    lookup[0x98].name = "TYA";
    lookup[0x98].cycles = 2;

    // TAY (A8)
    lookup[0xA8].instruction = ins_TAY;
    lookup[0xA8].addrmode = addr_implied;
    lookup[0xA8].name = "TAY";
    lookup[0xA8].cycles = 2;

        // BPL (10) - Relative
    lookup[0x10].instruction = ins_BPL; lookup[0x10].addrmode = addr_relative; lookup[0x10].name = "BPL"; lookup[0x10].cycles = 2;
    
    // BMI (30) - Relative
    lookup[0x30].instruction = ins_BMI; lookup[0x30].addrmode = addr_relative; lookup[0x30].name = "BMI"; lookup[0x30].cycles = 2;

    // BVC (50) - Relative
    lookup[0x50].instruction = ins_BVC; lookup[0x50].addrmode = addr_relative; lookup[0x50].name = "BVC"; lookup[0x50].cycles = 2;

    // BVS (70) - Relative
    lookup[0x70].instruction = ins_BVS; lookup[0x70].addrmode = addr_relative; lookup[0x70].name = "BVS"; lookup[0x70].cycles = 2;

    // BCC (90) - Relative
    lookup[0x90].instruction = ins_BCC; lookup[0x90].addrmode = addr_relative; lookup[0x90].name = "BCC"; lookup[0x90].cycles = 2;

    // BCS (B0) - Relative
    lookup[0xB0].instruction = ins_BCS; lookup[0xB0].addrmode = addr_relative; lookup[0xB0].name = "BCS"; lookup[0xB0].cycles = 2;

        // PLP (28)
    lookup[0x28].instruction = ins_PLP; lookup[0x28].addrmode = addr_implied; lookup[0x28].name = "PLP"; lookup[0x28].cycles = 4;

    // PHP (08)
    lookup[0x08].instruction = ins_PHP; lookup[0x08].addrmode = addr_implied; lookup[0x08].name = "PHP"; lookup[0x08].cycles = 3;
    // JMP Indirect (6C)
    lookup[0x6C].instruction = ins_JMP; lookup[0x6C].addrmode = addr_indirect; lookup[0x6C].name = "JMP IND"; lookup[0x6C].cycles = 5;

        // --- BIT ---
    lookup[0x24].instruction = ins_BIT; lookup[0x24].addrmode = addr_zero_page; lookup[0x24].name = "BIT ZP"; lookup[0x24].cycles = 3;
    lookup[0x2C].instruction = ins_BIT; lookup[0x2C].addrmode = addr_absolute; lookup[0x2C].name = "BIT ABS"; lookup[0x2C].cycles = 4;

    // --- ROL ---
    lookup[0x2A].instruction = ins_ROL_ACC; lookup[0x2A].addrmode = addr_implied; lookup[0x2A].name = "ROL A"; lookup[0x2A].cycles = 2;
    lookup[0x26].instruction = ins_ROL; lookup[0x26].addrmode = addr_zero_page; lookup[0x26].name = "ROL ZP"; lookup[0x26].cycles = 5;
    lookup[0x2E].instruction = ins_ROL; lookup[0x2E].addrmode = addr_absolute; lookup[0x2E].name = "ROL ABS"; lookup[0x2E].cycles = 6;
    // Ajoute aussi les modes X si tu veux être complet (36, 3E)

    // --- ROR ---
    lookup[0x6A].instruction = ins_ROR_ACC; lookup[0x6A].addrmode = addr_implied; lookup[0x6A].name = "ROR A"; lookup[0x6A].cycles = 2;
    lookup[0x66].instruction = ins_ROR; lookup[0x66].addrmode = addr_zero_page; lookup[0x66].name = "ROR ZP"; lookup[0x66].cycles = 5;
    lookup[0x6E].instruction = ins_ROR; lookup[0x6E].addrmode = addr_absolute; lookup[0x6E].name = "ROR ABS"; lookup[0x6E].cycles = 6;
    // Ajoute aussi les modes X si tu veux être complet (76, 7E)

        // --- STX (Store X Register) ---
    lookup[0x86].instruction = ins_STX; lookup[0x86].addrmode = addr_zero_page; lookup[0x86].name = "STX ZP"; lookup[0x86].cycles = 3;
    lookup[0x96].instruction = ins_STX; lookup[0x96].addrmode = addr_zero_page_y; lookup[0x96].name = "STX ZP,Y"; lookup[0x96].cycles = 4;
    lookup[0x8E].instruction = ins_STX; lookup[0x8E].addrmode = addr_absolute; lookup[0x8E].name = "STX ABS"; lookup[0x8E].cycles = 4;
    // --- LDX (Load X Register) - Complément ---
    lookup[0xA6].instruction = ins_LDX; lookup[0xA6].addrmode = addr_zero_page; lookup[0xA6].name = "LDX ZP"; lookup[0xA6].cycles = 3;
    lookup[0xB6].instruction = ins_LDX; lookup[0xB6].addrmode = addr_zero_page_y; lookup[0xB6].name = "LDX ZP,Y"; lookup[0xB6].cycles = 4;
    lookup[0xAE].instruction = ins_LDX; lookup[0xAE].addrmode = addr_absolute; lookup[0xAE].name = "LDX ABS"; lookup[0xAE].cycles = 4;
    lookup[0xBE].instruction = ins_LDX; lookup[0xBE].addrmode = addr_absolute_y; lookup[0xBE].name = "LDX ABS,Y"; lookup[0xBE].cycles = 4;
    // STA Absolute,Y (99)
    lookup[0x99].instruction = ins_STA; lookup[0x99].addrmode = addr_absolute_y; lookup[0x99].name = "STA ABS,Y"; lookup[0x99].cycles = 5;
    lookup[0xD9].instruction = ins_CMP; lookup[0xD9].addrmode = addr_absolute_y; lookup[0xD9].name = "CMP ABS,Y"; lookup[0xD9].cycles = 4;
    // STA Absolute,X (9D)
    lookup[0x9D].instruction = ins_STA; lookup[0x9D].addrmode = addr_absolute_x; lookup[0x9D].name = "STA ABS,X"; lookup[0x9D].cycles = 5;
        // CMP Absolute,X (DD)
    lookup[0xDD].instruction = ins_CMP; lookup[0xDD].addrmode = addr_absolute_x; lookup[0xDD].name = "CMP ABS,X"; lookup[0xDD].cycles = 4;
    // CMP Zero Page,X (D5)
    lookup[0xD5].instruction = ins_CMP; lookup[0xD5].addrmode = addr_zero_page_x; lookup[0xD5].name = "CMP ZP,X"; lookup[0xD5].cycles = 4;
    // CPX Zero Page (E4)
    lookup[0xE4].instruction = ins_CPX; lookup[0xE4].addrmode = addr_zero_page; lookup[0xE4].name = "CPX ZP"; lookup[0xE4].cycles = 3;
    // CPX Absolute (EC)
    lookup[0xEC].instruction = ins_CPX; lookup[0xEC].addrmode = addr_absolute; lookup[0xEC].name = "CPX ABS"; lookup[0xEC].cycles = 4;
    // CPY Zero Page (C4)
    lookup[0xC4].instruction = ins_CPY; lookup[0xC4].addrmode = addr_zero_page; lookup[0xC4].name = "CPY ZP"; lookup[0xC4].cycles = 3;
    // --- AND (Manquants) ---
    lookup[0x25].instruction = ins_AND; lookup[0x25].addrmode = addr_zero_page; lookup[0x25].name = "AND ZP"; lookup[0x25].cycles = 3;
    lookup[0x35].instruction = ins_AND; lookup[0x35].addrmode = addr_zero_page_x; lookup[0x35].name = "AND ZP,X"; lookup[0x35].cycles = 4;
    lookup[0x2D].instruction = ins_AND; lookup[0x2D].addrmode = addr_absolute; lookup[0x2D].name = "AND ABS"; lookup[0x2D].cycles = 4;
    lookup[0x3D].instruction = ins_AND; lookup[0x3D].addrmode = addr_absolute_x; lookup[0x3D].name = "AND ABS,X"; lookup[0x3D].cycles = 4;
    lookup[0x39].instruction = ins_AND; lookup[0x39].addrmode = addr_absolute_y; lookup[0x39].name = "AND ABS,Y"; lookup[0x39].cycles = 4;
    lookup[0x21].instruction = ins_AND; lookup[0x21].addrmode = addr_indirect_x; lookup[0x21].name = "AND (ZP,X)"; lookup[0x21].cycles = 6;
    lookup[0x31].instruction = ins_AND; lookup[0x31].addrmode = addr_indirect_y; lookup[0x31].name = "AND (ZP),Y"; lookup[0x31].cycles = 5;

    // --- ORA (Manquants) ---
    lookup[0x05].instruction = ins_ORA; lookup[0x05].addrmode = addr_zero_page; lookup[0x05].name = "ORA ZP"; lookup[0x05].cycles = 3;
    lookup[0x15].instruction = ins_ORA; lookup[0x15].addrmode = addr_zero_page_x; lookup[0x15].name = "ORA ZP,X"; lookup[0x15].cycles = 4;
    lookup[0x0D].instruction = ins_ORA; lookup[0x0D].addrmode = addr_absolute; lookup[0x0D].name = "ORA ABS"; lookup[0x0D].cycles = 4;
    lookup[0x1D].instruction = ins_ORA; lookup[0x1D].addrmode = addr_absolute_x; lookup[0x1D].name = "ORA ABS,X"; lookup[0x1D].cycles = 4;
    lookup[0x19].instruction = ins_ORA; lookup[0x19].addrmode = addr_absolute_y; lookup[0x19].name = "ORA ABS,Y"; lookup[0x19].cycles = 4;
    lookup[0x01].instruction = ins_ORA; lookup[0x01].addrmode = addr_indirect_x; lookup[0x01].name = "ORA (ZP,X)"; lookup[0x01].cycles = 6;
    lookup[0x11].instruction = ins_ORA; lookup[0x11].addrmode = addr_indirect_y; lookup[0x11].name = "ORA (ZP),Y"; lookup[0x11].cycles = 5;

    // --- EOR (Manquants) ---
    lookup[0x45].instruction = ins_EOR; lookup[0x45].addrmode = addr_zero_page; lookup[0x45].name = "EOR ZP"; lookup[0x45].cycles = 3;
    lookup[0x55].instruction = ins_EOR; lookup[0x55].addrmode = addr_zero_page_x; lookup[0x55].name = "EOR ZP,X"; lookup[0x55].cycles = 4;
    lookup[0x4D].instruction = ins_EOR; lookup[0x4D].addrmode = addr_absolute; lookup[0x4D].name = "EOR ABS"; lookup[0x4D].cycles = 4;
    lookup[0x5D].instruction = ins_EOR; lookup[0x5D].addrmode = addr_absolute_x; lookup[0x5D].name = "EOR ABS,X"; lookup[0x5D].cycles = 4;
    lookup[0x59].instruction = ins_EOR; lookup[0x59].addrmode = addr_absolute_y; lookup[0x59].name = "EOR ABS,Y"; lookup[0x59].cycles = 4;
    lookup[0x41].instruction = ins_EOR; lookup[0x41].addrmode = addr_indirect_x; lookup[0x41].name = "EOR (ZP,X)"; lookup[0x41].cycles = 6;
    lookup[0x51].instruction = ins_EOR; lookup[0x51].addrmode = addr_indirect_y; lookup[0x51].name = "EOR (ZP),Y"; lookup[0x51].cycles = 5;

    // --- ASL Indexés ---
    lookup[0x16].instruction = ins_ASL; lookup[0x16].addrmode = addr_zero_page_x; lookup[0x16].name = "ASL ZP,X"; lookup[0x16].cycles = 6;
    lookup[0x1E].instruction = ins_ASL; lookup[0x1E].addrmode = addr_absolute_x; lookup[0x1E].name = "ASL ABS,X"; lookup[0x1E].cycles = 7;

    // --- LSR Indexés ---
    lookup[0x56].instruction = ins_LSR; lookup[0x56].addrmode = addr_zero_page_x; lookup[0x56].name = "LSR ZP,X"; lookup[0x56].cycles = 6;
    lookup[0x5E].instruction = ins_LSR; lookup[0x5E].addrmode = addr_absolute_x; lookup[0x5E].name = "LSR ABS,X"; lookup[0x5E].cycles = 7;

    // --- ROL Indexés ---
    lookup[0x36].instruction = ins_ROL; lookup[0x36].addrmode = addr_zero_page_x; lookup[0x36].name = "ROL ZP,X"; lookup[0x36].cycles = 6;
    lookup[0x3E].instruction = ins_ROL; lookup[0x3E].addrmode = addr_absolute_x; lookup[0x3E].name = "ROL ABS,X"; lookup[0x3E].cycles = 7;

    // --- ROR Indexés ---
    lookup[0x76].instruction = ins_ROR; lookup[0x76].addrmode = addr_zero_page_x; lookup[0x76].name = "ROR ZP,X"; lookup[0x76].cycles = 6;
    lookup[0x7E].instruction = ins_ROR; lookup[0x7E].addrmode = addr_absolute_x; lookup[0x7E].name = "ROR ABS,X"; lookup[0x7E].cycles = 7;

    // --- ADC Indirects ---
    lookup[0x61].instruction = ins_ADC; lookup[0x61].addrmode = addr_indirect_x; lookup[0x61].name = "ADC (ZP,X)"; lookup[0x61].cycles = 6;
    lookup[0x71].instruction = ins_ADC; lookup[0x71].addrmode = addr_indirect_y; lookup[0x71].name = "ADC (ZP),Y"; lookup[0x71].cycles = 5;

    // --- SBC Indirects ---
    lookup[0xE1].instruction = ins_SBC; lookup[0xE1].addrmode = addr_indirect_x; lookup[0xE1].name = "SBC (ZP,X)"; lookup[0xE1].cycles = 6;
    lookup[0xF1].instruction = ins_SBC; lookup[0xF1].addrmode = addr_indirect_y; lookup[0xF1].name = "SBC (ZP),Y"; lookup[0xF1].cycles = 5;

    // --- CMP Indirects ---
    lookup[0xC1].instruction = ins_CMP; lookup[0xC1].addrmode = addr_indirect_x; lookup[0xC1].name = "CMP (ZP,X)"; lookup[0xC1].cycles = 6;
    lookup[0xD1].instruction = ins_CMP; lookup[0xD1].addrmode = addr_indirect_y; lookup[0xD1].name = "CMP (ZP),Y"; lookup[0xD1].cycles = 5;

    // --- LDA Indirects ---
    lookup[0xA1].instruction = ins_LDA; lookup[0xA1].addrmode = addr_indirect_x; lookup[0xA1].name = "LDA (ZP,X)"; lookup[0xA1].cycles = 6;
    lookup[0xB1].instruction = ins_LDA; lookup[0xB1].addrmode = addr_indirect_y; lookup[0xB1].name = "LDA (ZP),Y"; lookup[0xB1].cycles = 5;

    // --- STA Indirects ---
    lookup[0x81].instruction = ins_STA; lookup[0x81].addrmode = addr_indirect_x; lookup[0x81].name = "STA (ZP,X)"; lookup[0x81].cycles = 6;
    lookup[0x91].instruction = ins_STA; lookup[0x91].addrmode = addr_indirect_y; lookup[0x91].name = "STA (ZP),Y"; lookup[0x91].cycles = 6;
    // CPY Absolute (CC)
    lookup[0xCC].instruction = ins_CPY; lookup[0xCC].addrmode = addr_absolute; lookup[0xCC].name = "CPY ABS"; lookup[0xCC].cycles = 4;
// ADC (Add with Carry)
lookup[0x75].instruction = ins_ADC; lookup[0x75].addrmode = addr_zero_page_x; lookup[0x75].name = "ADC ZP,X";  lookup[0x75].cycles = 4;
lookup[0x7D].instruction = ins_ADC; lookup[0x7D].addrmode = addr_absolute_x;  lookup[0x7D].name = "ADC ABS,X"; lookup[0x7D].cycles = 4;
lookup[0x79].instruction = ins_ADC; lookup[0x79].addrmode = addr_absolute_y;  lookup[0x79].name = "ADC ABS,Y"; lookup[0x79].cycles = 4;

// SBC (Subtract with Carry) - Tu avais déjà mis F5, FD, F9, vérifie juste s'ils y sont tous
lookup[0xF5].instruction = ins_SBC; lookup[0xF5].addrmode = addr_zero_page_x; lookup[0xF5].name = "SBC ZP,X";  lookup[0xF5].cycles = 4;
lookup[0xFD].instruction = ins_SBC; lookup[0xFD].addrmode = addr_absolute_x;  lookup[0xFD].name = "SBC ABS,X"; lookup[0xFD].cycles = 4;
lookup[0xF9].instruction = ins_SBC; lookup[0xF9].addrmode = addr_absolute_y;  lookup[0xF9].name = "SBC ABS,Y"; lookup[0xF9].cycles = 4;

// CMP (Compare A)
lookup[0xD5].instruction = ins_CMP; lookup[0xD5].addrmode = addr_zero_page_x; lookup[0xD5].name = "CMP ZP,X";  lookup[0xD5].cycles = 4;
lookup[0xDD].instruction = ins_CMP; lookup[0xDD].addrmode = addr_absolute_x;  lookup[0xDD].name = "CMP ABS,X"; lookup[0xDD].cycles = 4;
lookup[0xD9].instruction = ins_CMP; lookup[0xD9].addrmode = addr_absolute_y;  lookup[0xD9].name = "CMP ABS,Y"; lookup[0xD9].cycles = 4;

// ASL Absolute,X et ROR Absolute,X
lookup[0x1E].instruction = ins_ASL; lookup[0x1E].addrmode = addr_absolute_x; lookup[0x1E].name = "ASL ABS,X"; lookup[0x1E].cycles = 7;
lookup[0x7E].instruction = ins_ROR; lookup[0x7E].addrmode = addr_absolute_x; lookup[0x7E].name = "ROR ABS,X"; lookup[0x7E].cycles = 7;

    // --- INC Indexés (Manquants) ---
    lookup[0xF6].instruction = ins_INC; lookup[0xF6].addrmode = addr_zero_page_x; lookup[0xF6].name = "INC ZP,X"; lookup[0xF6].cycles = 6;
    lookup[0xFE].instruction = ins_INC; lookup[0xFE].addrmode = addr_absolute_x; lookup[0xFE].name = "INC ABS,X"; lookup[0xFE].cycles = 7;

    // --- DEC Indexés (Manquants) ---
    lookup[0xD6].instruction = ins_DEC; lookup[0xD6].addrmode = addr_zero_page_x; lookup[0xD6].name = "DEC ZP,X"; lookup[0xD6].cycles = 6;
    lookup[0xDE].instruction = ins_DEC; lookup[0xDE].addrmode = addr_absolute_x; lookup[0xDE].name = "DEC ABS,X"; lookup[0xDE].cycles = 7;
}        // CMP Absolute,Y (D9)

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
    cpu->irq_pending = 0;
    cpu->nmi_pending = 0;
    // Initialiser la table des opcodes
    init_lookup_table();
}
void cpu_nmi(CPU *cpu) {
    cpu->nmi_pending = 1;
}

// Fonction interne pour exécuter une interruption
static void cpu_handle_interrupt(CPU *cpu, u16 vector_addr) {
    // Sauvegarder PC
    cpu_push_byte(cpu, (cpu->PC >> 8) & 0xFF);
    cpu_push_byte(cpu, cpu->PC & 0xFF);

    // Sauvegarder Status (P). Contrairement à BRK, le flag B est à 0 pour les interruptions matérielles.
    u8 status = cpu->P | FLAG_U; // Flag U toujours à 1
    cpu_push_byte(cpu, status);

    // Désactiver interruptions
    cpu_set_flag(cpu, FLAG_I, 1);

    // Sauter au vecteur
    u16 lo = mem_read(cpu->mem, vector_addr);
    u16 hi = mem_read(cpu->mem, vector_addr + 1);
    cpu->PC = (hi << 8) | lo;
    cpu->cycles += 7; // Les interruptions prennent du temps
}
void cpu_step(CPU *cpu) {
    // 1. Sauvegarder l'état avant l'action
    u16 pc_before = cpu->PC;
    u8 sp_before = cpu->SP;
// 1. NMI (Non-Maskable) - Toujours exécutée si demandée
    if (cpu->nmi_pending) {
        cpu->nmi_pending = 0;
        cpu_handle_interrupt(cpu, 0xFFFA); // Vecteur NMI à $FFFA
        return; // On saute l'instruction normale
    }

    // 2. IRQ (Interrupt Request) - Seulement si le flag I est à 0
    if (cpu->irq_pending && !cpu_get_flag(cpu, FLAG_I)) {
        cpu->irq_pending = 0;
        cpu_handle_interrupt(cpu, 0xFFFE); // Vecteur IRQ à $FFFE
        return;
    }

    // 2. FETCH
    u8 opcode = mem_read(cpu->mem, cpu->PC);
    cpu->PC++;

    // 3. DECODE
    OpcodeEntry entry = lookup[opcode];

    // 4. DEBUG : Afficher ce qui va se passer
    // %04X = adresse en hexadécimal 4 chiffres
    // %02X = nombre en hexadécimal 2 chiffres
    //printf("[TRACE] PC: %04X | Opcode: %02X | Instruction: %-8s | SP: %02X -> ", 
      //     pc_before, opcode, entry.name, sp_before);
// DEBUG : Détecter les instructions manquantes
    if (strcmp(entry.name, "???") == 0) {
        printf("\n[ERREUR] OPCODE NON IMPLEMENTE : 0x%02X à l'adresse 0x%04X\n", opcode, pc_before);
            exit(1); // Quitte le programme immédiatement (nécessite <stdlib.h>)

    }
    // 5. ADDRESSING & EXECUTE
    entry.addrmode(cpu);
    entry.instruction(cpu);
//printf("TRACE PC: 0x%04X A: 0x%02X X: 0x%02X Y: 0x%02X P: 0x%02X\n", 
  //     pc_before, cpu->A, cpu->X, cpu->Y, cpu->P);
    // 6. Afficher l'état APRÈS l'action
    //printf("A: %02X, X: %02X, SP: %02X, PC: %04X\n", 
      //    cpu->A, cpu->X, cpu->SP, cpu->PC);

    // 7. CYCLES
    cpu->cycles += entry.cycles;
}