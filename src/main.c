#include <stdio.h>
#include <assert.h>
#include "types.h"
#include "memory.h"
#include "cpu.h"

int main() {
    printf("=== Emulateur 6502 ===\n");
    printf("Phase 8 : La Pile et les Fonctions (JSR/RTS).\n\n");

    Memory mem;
    mem_init(&mem);

    // Vecteur de Reset
    mem_write(&mem, 0xFFFC, 0x00);
    mem_write(&mem, 0xFFFD, 0x80);

    // PROGRAMME PRINCIPAL (0x8000)
    // 1. Charge 5 dans A
    mem_write(&mem, 0x8000, 0xA9); // LDA #$05
    mem_write(&mem, 0x8001, 0x05);
    
    // 2. Appelle la fonction à l'adresse 0x9000
    mem_write(&mem, 0x8002, 0x20); // JSR $9000
    mem_write(&mem, 0x8003, 0x00); // Low byte
    mem_write(&mem, 0x8004, 0x90); // High byte
    
    // 5. Après retour, NOP (fin)
    mem_write(&mem, 0x8005, 0xEA); // NOP

    // SOUS-PROGRAMME / FONCTION (0x9000)
    // 3. Ajoute 10 à A (5 + 10 = 15)
    // On utilise ADC (Add with Carry). Pour simplifier, on va faire: A = A + 10 manuellement
    // Ou plus simple : LDA #$0F (on écrase A pour tester le retour)
    mem_write(&mem, 0x9000, 0xA9); // LDA #$0F (15)
    mem_write(&mem, 0x9001, 0x0F);
    
    // 4. Retour
    mem_write(&mem, 0x9002, 0x60); // RTS

    CPU cpu;
    cpu_reset(&cpu, &mem);

    printf("Execution du programme principal et de la fonction...\n");
    
    // On exécute tout
    int max_steps = 100;
    while (cpu.PC != 0x8006 && max_steps > 0) { // On s'arrête quand on revient au NOP final
        cpu_step(&cpu);
        max_steps--;
    }

    printf("Valeur finale de A : %d (Attendu : 15)\n", cpu.A);
    printf("PC final : 0x%04X (Attendu : 0x8006)\n", cpu.PC);

    assert(cpu.A == 15); // La fonction a bien modifié A
    assert(cpu.PC == 0x8006); // On est bien revenu après le JSR

    printf("\nSUCCES : Le CPU peut appeler des fonctions et revenir !\n");
    return 0;
}