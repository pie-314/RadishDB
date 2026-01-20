# Compiler and flags
CC      = gcc
CFLAGS  = -Wall -Wextra -Werror -g

# Project
TARGET  = radishdb
BUILD   = build

# Source files
SRC = \
	src/main.c \
	src/hashtable.c \
	src/persistence.c \
	src/aof.c \
	src/expires.c \
	src/utils.c \
	src/engine.c \
	src/repl.c \
	src/result.c \
	src/server.c

# Convert src/foo.c -> build/foo.o
OBJ = $(patsubst src/%.c,$(BUILD)/%.o,$(SRC))

# Default target
all: $(TARGET)

# Link
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

# Compile rule
$(BUILD)/%.o: src/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure build dir exists
$(BUILD):
	mkdir -p $(BUILD)

# Clean
clean:
	rm -rf $(BUILD) $(TARGET)

# ASAN build
asan:
	$(CC) -fsanitize=address -fno-omit-frame-pointer $(SRC) -o $(TARGET)

.PHONY: all clean asan

