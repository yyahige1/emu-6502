# Le compilateur
CC=gcc

# Les drapeaux (flags)
CFLAGS=-Wall -Wextra -Iinclude -g

# Les sources (ON AJOUTE memory.c ICI)
SRC=src/main.c src/memory.c

# La cible par défaut : créer l'exécutable 'emu-6502'
TARGET=emu-6502

# Cible par défaut (si on tape juste 'make')
all: $(TARGET)

# Règle de compilation
 $(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Règle pour exécuter
run: $(TARGET)
	./$(TARGET)

# Nettoyer les fichiers générés
clean:
	rm -f $(TARGET)

.PHONY: all run clean