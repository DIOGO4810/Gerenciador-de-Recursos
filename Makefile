# Nome do executável
TARGET = manage

# Arquivos fonte
SOURCES = manage.c

# Comando do compilador
CC = gcc

# Flags do compilador
CFLAGS = -O2 -Wall -Wextra -std=c99

# Regra padrão: compilar e executar
all: $(TARGET)

$(TARGET): $(SOURCES)
	@$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

run: $(TARGET)
	@./$(TARGET)

# Limpar os arquivos compilados
clean:
	rm -f $(TARGET)

.PHONY: all run clean
