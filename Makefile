CC := gcc

EXEC := manage

CFLAGS := -Wall -O3 -g -flto -std=gnu99 -Wextra -pedantic -Iincludes

SRC := $(shell find src/ -name "*.c")
OBJ := $(SRC:src/%.c=build/%.o)


.PHONY: all
all: $(EXEC) 

$(EXEC): $(OBJ)
	@$(CC) $(OBJ) $(EXTERNAL_OBJ)  -o $@ ; echo "[Linking] $@"


build/%.o: src/%.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@ ; echo "[Compiling] $@"


run: $(EXEC)
	@./$(EXEC)



.PHONY: clean
clean:
	rm -rf build
	rm -f $(EXEC)
