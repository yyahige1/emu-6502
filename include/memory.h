#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

// 64 Ko de RAM (0x0000 à 0xFFFF)
#define MAX_MEMORY 0x10000

// Structure représentant la mémoire de l'ordinateur
typedef struct {
    u8 data[MAX_MEMORY];
} Memory;

// Prototypes des fonctions
void mem_init(Memory *mem);
u8 mem_read(Memory *mem, u16 address);
void mem_write(Memory *mem, u16 address, u8 value);
// Charge un fichier binaire en mémoire à partir d'une adresse donnée
// Retourne la taille du fichier chargé, ou 0 si erreur
int mem_load(Memory *mem, const char *filename, u16 offset);
#endif