# Émulateur MOS 6502

Un émulateur simple et éducatif du processeur MOS Technology 6502 écrit en C.

## À propos
Ce projet a pour but de comprendre le fonctionnement interne d'un processeur 8-bit classique. Il implémente le cycle "Fetch, Decode, Execute" et tente de reproduire le comportement du légendaire CPU 6502 (utilisé dans la NES, l'Apple II, le Commodore 64, etc.).

## Fonctionnalités Actuelles
* Architecture Modulaire : Séparation claire entre le CPU, la Mémoire et les Instructions.
* Mémoire : Gestion d'un bus mémoire de 64 Ko.
* CPU :
    * Registres principaux (A, X, Y, SP, PC).
    * Registre de statut (Flags N, V, B, D, I, Z, C).
    * Vecteur de Reset (démarrage à l'adresse 0xFFFC).
* Jeu d'instructions (En cours) :
    * Transferts : LDA, LDX, STA, STX.
    * Contrôle : NOP.
* Modes d'adressage (En cours) :
    * Immediate (#$val)
    * Zero Page ($addr)
    * Absolute ($addr)

## Installation et Compilation

### Prérequis
* Un compilateur C (GCC ou Clang).
* make.

### Compiler
Clonez le dépôt et utilisez la commande make :
```bash
git clone [https://github.com/yyahige1/emu-6502.git](https://github.com/yyahige1/emu-6502.git)
cd emu-6502
make run
```

### Lancer
Pour exécuter le programme de test actuel :

make run

## Feuille de route (Roadmap)
* Phase 1 : Infrastructure de base (Makefile, Types).
* Phase 2 : Gestion de la Mémoire (RAM 64Ko).
* Phase 3 : Structure du CPU et Reset.
* Phase 4 : Fetch, Decode, Execute.
* Phase 5 & 6 : Architecture modulaire et Table d'Opcodes.
* Phase 7 : Instructions de contrôle (Branchements, Pile).
* Phase 8 : Instructions Arithmétiques et Logiques.
* Phase 9 : Gestion des Interruptions (IRQ, NMI).
* Phase 10 : Validation avec suite de tests (Klaus Dormann).

## Architecture
Le projet est divisé en plusieurs modules :

* src/cpu.c : Le cœur du processeur, la boucle principale et la table de décodage.
* src/memory.c : Simulation de la RAM et du bus.
* src/instructions.c : Implémentation des opcodes (LDA, STA, etc.).
* src/addressing.c : Calcul des adresses effective (Immediate, Absolute, etc.).

## Licence
Ce projet est open-source à but éducatif.