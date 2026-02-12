#include "memory.h"
#include <string.h> // Pour memset

// Initialise la mémoire à 0
void mem_init(Memory *mem) {
    memset(mem->data, 0, sizeof(mem->data));
}

// Lit un octet à une adresse donnée
u8 mem_read(Memory *mem, u16 address) {
    // L'adresse ne peut pas dépasser 0xFFFF car u16 va jusqu'à 65535
    return mem->data[address];
}

// Écrit un octet à une adresse donnée
void mem_write(Memory *mem, u16 address, u8 value) {
    mem->data[address] = value;
}