#include <stdio.h>
#include <assert.h>
#include "types.h"
#include "memory.h"
#include "cpu.h"

int main() {
    printf("=== Emulateur 6502 ===\n");
    printf("Phase 5 & 6 : Addressing Modes & Table Lookup.\n\n");

    Memory mem;
    mem_init(&mem);

    // Adresse de départ : 0x8000
    mem_write(&mem, 0xFFFC, 0x00);
    mem_write(&mem, 0xFFFD, 0x80);

    // Programme
    u16 start = 0x8000;
    mem_write(&mem, start++, 0xA9); // LDA #$05
    mem_write(&mem, start++, 0x05);
    
    mem_write(&mem, start++, 0x85); // STA $00 (Zero Page)
    mem_write(&mem, start++, 0x00);
    
    mem_write(&mem, start++, 0xA9); // LDA #$00 (Reset A)
    mem_write(&mem, start++, 0x00);

    mem_write(&mem, start++, 0xA5); // LDA $00 (Zero Page Load)
    mem_write(&mem, start++, 0x00);

    mem_write(&mem, start++, 0x8D); // STA $1234 (Absolute Store)
    mem_write(&mem, start++, 0x34); // Low byte
    mem_write(&mem, start++, 0x12); // High byte

    CPU cpu;
    cpu_reset(&cpu, &mem);

    // Execution pas à pas
    cpu_step(&cpu); // LDA #$05
    assert(cpu.A == 5);
    printf("[OK] LDA Immediate : A = 5\n");

    cpu_step(&cpu); // STA $00
    assert(mem_read(&mem, 0x0000) == 5);
    printf("[OK] STA Zero Page : Mem[0x00] = 5\n");

    cpu_step(&cpu); // LDA #$00
    assert(cpu.A == 0);
    printf("[OK] LDA Immediate : A = 0\n");

    cpu_step(&cpu); // LDA $00
    assert(cpu.A == 5); // On doit retrouver 5
    printf("[OK] LDA Zero Page : A = 5 (lu depuis la RAM)\n");

    cpu_step(&cpu); // STA $1234
    assert(mem_read(&mem, 0x1234) == 5);
    printf("[OK] STA Absolute : Mem[0x1234] = 5\n");

    printf("\nSUCCES : Architecture modulaire operationnelle !\n");
    return 0;
}