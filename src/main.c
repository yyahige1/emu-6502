#include <stdio.h>
#include "types.h"

int main(int argc, char **argv) {
    printf("=== Emulateur 6502 ===\n");
    printf("Phase 1 : Infrastructure OK.\n");
    
    // Test des types
    u8 test_byte = 0xFF;
    u16 test_addr = 0xFFFF;

    printf("Taille u8 : %zu octets (attendu : 1)\n", sizeof(test_byte));
    printf("Taille u16 : %zu octets (attendu : 2)\n", sizeof(test_addr));
    
    return 0;
}