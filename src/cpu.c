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
    lookup[0xEA].addrmode = addr_immediate; // NOP n'a pas d'adressage, mais on met un défaut
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
    // 1. FETCH
    u8 opcode = mem_read(cpu->mem, cpu->PC);
    cpu->PC++;

    // 2. DECODE : Regarder dans la table
    OpcodeEntry entry = lookup[opcode];

    // Debug (Optionnel : commenter pour avoir une sortie propre)
    // printf("Executing: %s (0x%02X) at PC 0x%04X\n", entry.name, opcode, cpu->PC-1);

    // 3. ADDRESSING : Calculer où sont les données
    // Cela remplit cpu->fetched ou cpu->addr_abs
    entry.addrmode(cpu);

    // 4. EXECUTE
    entry.instruction(cpu);

    // 5. CYCLES
    cpu->cycles += entry.cycles;
}