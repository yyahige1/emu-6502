# Le compilateur
CC=gcc

# Les drapeaux (flags)
CFLAGS=-Wall -Wextra -Iinclude -g

# Les sources
SRC=src/main.c

# La cible par défaut : créer l'exécutable 'emu-6502'
TARGET=emu-6502

# Cible par défaut (si on tape juste 'make')
all: $(TARGET)

# Règle de compilation
 $(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# NOUVELLE RÈGLE : Pour exécuter le programme
# Elle dépend de $(TARGET), donc elle recompile automatiquement si le code a changé !
run: $(TARGET)
	./$(TARGET)

# Nettoyer les fichiers générés
clean:
	rm -f $(TARGET)

# Indique que ces cibles ne sont pas des fichiers
.PHONY: all run clean