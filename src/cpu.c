#include "cpu.h"
#include "instructions.h"
#include <stdio.h>

// Aide: Lire un octet et avancer le PC
static u8 cpu_fetch_byte(CPU *cpu) {
    u8 data = mem_read(cpu->mem, cpu->PC);
    cpu->PC++;
    return data;
}

// Aide: Définir un flag
void cpu_set_flag(CPU *cpu, u8 flag, int value) {
    if (value) {
        cpu->P |= flag;
    } else {
        cpu->P &= ~flag;
    }
}

// Aide: Lire un flag
int cpu_get_flag(CPU *cpu, u8 flag) {
    return (cpu->P & flag) != 0;
}

void cpu_reset(CPU *cpu, Memory *mem) {
    cpu->A = 0;
    cpu->X = 0;
    cpu->Y = 0;
    cpu->SP = 0xFD;
    cpu->P = 0x24;
    cpu->cycles = 0;
    
    cpu->mem = mem; // Lier la mémoire au CPU

    // Lecture du vecteur de reset
    u16 lo = mem_read(mem, 0xFFFC);
    u16 hi = mem_read(mem, 0xFFFD);
    cpu->PC = (hi << 8) | lo;
}

void cpu_step(CPU *cpu) {
    // 1. FETCH : Lire l'opcode à l'adresse PC
    u8 opcode = cpu_fetch_byte(cpu);

    // 2. DECODE & EXECUTE
    // Pour l'instant, on gère manuellement les opcodes qu'on a codés.
    // Plus tard, on utilisera un tableau de pointeurs de fonction.
    
    switch (opcode) {
        case 0xA9: // LDA Immediate
            ins_LDA_immediate(cpu);
            cpu->cycles += 2; // LDA immediate prend 2 cycles
            break;
        
        case 0xA2: // LDX Immediate
            ins_LDX_immediate(cpu);
            cpu->cycles += 2;
            break;

        case 0xEA: // NOP
            ins_NOP(cpu);
            cpu->cycles += 2;
            break;

        default:
            printf("Opcode inconnu : 0x%02X à l'adresse 0x%04X\n", opcode, cpu->PC - 1);
            break;
    }
}