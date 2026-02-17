#include <stdio.h>
#include <string.h>
#include "types.h"
#include "memory.h"
#include "cpu.h"

void run_builtin_test() {
    printf("=== Mode Test Interne ===\n");
    
    Memory mem;
    mem_init(&mem);

    mem_write(&mem, 0xFFFC, 0x00);
    mem_write(&mem, 0xFFFD, 0x80);

    u16 start = 0x8000;
    mem_write(&mem, start++, 0xA2); // LDX #$00
    mem_write(&mem, start++, 0x00);
    mem_write(&mem, start++, 0xE8); // INX
    mem_write(&mem, start++, 0xE0); // CPX #$05
    mem_write(&mem, start++, 0x05);
    mem_write(&mem, start++, 0xD0); // BNE (retour au INX)
    mem_write(&mem, start++, 0xFB); // Offset -5
    mem_write(&mem, start++, 0xEA); // NOP

    CPU cpu;
    cpu_reset(&cpu, &mem);

    printf("Lancement du test interne...\n");
    int max_steps = 50;
    while (max_steps > 0) {
        cpu_step(&cpu);
        max_steps--;
    }
    
    printf("X final : %d (Attendu : 5)\n", cpu.X);
}

int main(int argc, char **argv) {
    if (argc > 1) {
        printf("=== Emulateur 6502 ===\n");
        printf("Chargement du fichier : %s\n\n", argv[1]);

        Memory mem;
        mem_init(&mem);

        if (!mem_load(&mem, argv[1], 0x0000)) {
            return 1;
        }

        CPU cpu;
        
        // 1. Initialisation (UNE SEULE FOIS)
        cpu_reset(&cpu, &mem);
        
        // 2. Forçage du démarrage (UNE SEULE FOIS)
        //printf("Forcage du demarrage a 0x0400...\n");
        cpu.PC = 0x0400; 

        printf("Execution...\n");
        
        // 3. Boucle d'exécution
        while (1) {

           
            cpu_step(&cpu);
// Détection du succès ou de l'échec
            // 1. Détection du SUCCÈS
            // Si le PC arrive à l'adresse 0x37A3, le test est fini et réussi
            if (cpu.PC == 0x37A3) {
                printf("\n========================================\n");
                printf("   TEST SUITE PASSED WITH SUCCESS !\n");
                printf("   (Le programme a bouclé sur l'adresse de succès)\n");
                printf("========================================\n");
                break; // IMPORTANT : Arrête la boucle ici !
            }
            
            // Sécurité
                        // Sécurité Timeout
            // Sécurité Timeout
            if (cpu.cycles > 10000000) {
                printf("\nTimeout ! Le CPU semble bloque.\n");
                printf("Adresse de blocage : 0x%04X\n", cpu.PC);
                
                // Lire le numéro du test en cours (adresse standard $0210 pour ce test ROM)
                u8 test_num = mem_read(&mem, 0x0210);
                printf("Numero du test en cours : %d\n", test_num);
                
                // Afficher l'etat des registres
                printf("Registre A : 0x%02X\n", cpu.A);
                printf("Valeur attendue en mem[$0F] : 0x%02X\n", mem_read(&mem, 0x0F));
                printf("Etat bit-a-bit de P : ");
for(int i=7; i>=0; i--) printf("%d", (cpu.P >> i) & 1);
printf("\n");
                break;
            }
        }
        
    } else {
        run_builtin_test();
    }

    return 0;
}