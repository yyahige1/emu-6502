```markdown
# Roadmap : Émulateur CPU 6502 en C

## 1. Objectif du Projet
Créer un émulateur fonctionnel du processeur **MOS Technology 6502** en langage C.
Le but est de comprendre le fonctionnement interne d'un CPU (Fetch, Decode, Execute), la gestion de la mémoire, et les interruptions.

## 2. Architecture Matérielle (Modules)

Le projet sera divisé en fichiers sources (`src/`) et en-têtes (`include/`) pour une meilleure organisation.

*   **`types.h`** : Définition des types de données (`u8`, `u16`, `s8`) pour assurer la portabilité.
*   **`memory.c/h`** : Gestion du bus mémoire (Lecture/Écriture dans le tableau RAM).
*   **`cpu.c/h`** : Cœur du processeur (Registres, Flags, Cycle d'horloge).
*   **`instructions.c/h`** : Implémentation du jeu d'instructions (ISA) et des modes d'adressage.

## 3. Feuille de Route (To-Do List)

### Phase 1 : Infrastructure de base ✅
- [x] Initialiser le dépôt GitHub.
- [x] Configurer l'environnement WSL et VS Code (Token, Extensions).
- [x] Configurer le `Makefile` (compilation et règle `run`).
- [x] Créer les fichiers de base (`main.c`, `types.h`).
- [x] Définir les types personnalisés (`u8`, `u16`) dans `types.h`.
- [x] Configurer IntelliSense (`.vscode/c_cpp_properties.json`).

### Phase 2 : Le Sous-système Mémoire
- [ ] Créer `memory.c` et `memory.h`.
- [ ] Allouer un tableau de 64 Ko (65536 octets) pour la RAM.
- [ ] Implémenter `mem_read(u16 address)`.
- [ ] Implémenter `mem_write(u16 address, u8 value)`.
- [ ] Tester : Écrire une valeur en mémoire et la relire pour vérifier l'intégrité.

### Phase 3 : Structure du CPU
- [ ] Créer `cpu.c` et `cpu.h`.
- [ ] Créer la structure `CPU` contenant les registres :
    - `A` (Accumulateur)
    - `X`, `Y` (Registres d'index)
    - `PC` (Program Counter - 16 bits)
    - `SP` (Stack Pointer)
- [ ] Créer la structure ou l'énumération pour les Flags (`N`, `V`, `B`, `D`, `I`, `Z`, `C`).
- [ ] Implémenter `cpu_reset()` : Initialiser les registres et charger le vecteur de reset à l'adresse `0xFFFC`.

### Phase 4 : Le Cœur (Fetch, Decode, Execute)
- [ ] Implémenter le cycle principal dans `cpu_step()` :
    1. Lire l'octet pointé par `PC` (Opcode).
    2. Incrémenter `PC`.
    3. Décoder l'instruction via une table de correspondance.
    4. Exécuter l'instruction.
    5. Mettre à jour le compteur de cycles.

### Phase 5 : Modes d'Adressage (Crucial)
*Le 6502 accède aux données de différentes manières. Il faut implémenter ces fonctions avant les instructions.*
- [ ] **Immediate** : L'opérande est la valeur suivant l'instruction.
- [ ] **Zero Page** : Adresse sur 1 octet (0x00 à 0xFF).
- [ ] **Zero Page,X / Y** : Adresse + Registre X ou Y.
- [ ] **Absolute** : Adresse complète sur 2 octets.
- [ ] **Absolute,X / Y** : Adresse absolue + Registre X ou Y.
- [ ] **Indirect** : Utilisé principalement pour le JMP.
- [ ] **Indexed Indirect (X)** : `(Indirect,X)`
- [ ] **Indirect Indexed (Y)** : `(Indirect),Y`

### Phase 6 : Implémentation des Instructions (ISA)

#### A. Transfert de Données
- [ ] `LDA`, `LDX`, `LDY` (Load)
- [ ] `STA`, `STX`, `STY` (Store)
- [ ] `TAX`, `TAY`, `TXA`, `TYA`, `TSX`, `TXS` (Transferts registres)

#### B. Arithmétique et Logique
- [ ] `ADC` (Add with Carry)
- [ ] `SBC` (Subtract with Carry)
- [ ] `AND`, `ORA`, `EOR` (Opérations bit-à-bit)
- [ ] `INC`, `DEC`, `INX`, `DEX`, `INY`, `DEY`
- [ ] `CMP`, `CPX`, `CPY` (Comparaisons)

#### C. Sauts et Branchements
- [ ] `JMP` (Jump absolute/indirect)
- [ ] `JSR`, `RTS` (Jump to Subroutine / Return)
- [ ] Branchements conditionnels (`BEQ`, `BNE`, etc.).
- [ ] `BRK`, `RTI` (Break / Return from Interrupt).

#### D. Gestion de la Pile (Stack)
- [ ] `PHA`, `PHP` (Push)
- [ ] `PLA`, `PLP` (Pull)

#### E. Drapeaux (Flags)
- [ ] `CLC`, `SEC`, `CLD`, `SED`, `CLI`, `SEI`, `CLV`.

### Phase 7 : Interruptions (ISR)
- [ ] Implémenter le signal **IRQ**.
- [ ] Implémenter le signal **NMI**.
- [ ] Sauvegarder le contexte sur la pile.
- [ ] Charger le vecteur d'interruption approprié.

### Phase 8 : Tests et Validation
- [ ] Test unitaire simple : LDA #$05.
- [ ] Test de la pile (JSR/RTS).
- [ ] Intégrer une suite de tests ROM (Klaus Dormann).
```