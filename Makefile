# Le compilateur
CC=gcc

# Les drapeaux (flags)
CFLAGS=-Wall -Wextra -Iinclude -g

# Les sources (AJOUT DE src/cpu.c ICI)
SRC=src/main.c src/memory.c src/cpu.c src/instructions.c src/addressing.c
# La cible par défaut
TARGET=emu-6502

all: $(TARGET)

 $(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean