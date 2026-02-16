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

#include <stdio.h>
// ... autres includes

int mem_load(Memory *mem, const char *filename, u16 offset) {
    FILE *f = fopen(filename, "rb"); // "rb" = read binary
    if (f == NULL) {
        printf("Erreur : Impossible d'ouvrir le fichier %s\n", filename);
        return 0;
    }

    // Chercher la taille du fichier
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    // Vérifier si ça rentre dans la mémoire
    if (offset + size > 0x10000) {
        printf("Erreur : Fichier trop grand pour la mémoire\n");
        fclose(f);
        return 0;
    }

    // Lire le fichier et le mettre directement dans notre tableau data
    fread(&mem->data[offset], 1, size, f);
    
    fclose(f);
    return (int)size;
}