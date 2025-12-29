# Compiler and flags
CC      = gcc
CFLAGS  = -Wall -Wextra -Werror -g
LDFLAGS =

# Project name
TARGET = radishdb

# Source files
SRC = \
	src/main.c \
	src/hashtable.c \
	src/persistence.c

# Object files (generated automatically)
OBJ = $(SRC:.c=.o)

# Default target
all: $(TARGET)

# Link step
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

# Compile each .c into .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJ) $(TARGET)

# Deep clean (optional)
distclean: clean
	rm -f *.rdbx

# Run with AddressSanitizer (debug target)
asan:
	$(CC) -fsanitize=address -fno-omit-frame-pointer $(SRC) -o $(TARGET)

# Phony targets
.PHONY: all clean distclean asan

