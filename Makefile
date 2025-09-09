CC = gcc
CFLAGS = -Wall -Wextra -std=c17 -Iinclude
TARGET = fat12
SRC = src/fat12.c
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
LIB_DIR = $(BUILD_DIR)/lib
OBJ = $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(SRC))
LIB = $(LIB_DIR)/lib$(TARGET).a

all: $(LIB)

$(LIB): $(OBJ) | $(LIB_DIR)
	ar rcs $(LIB) $(OBJ)

$(OBJ_DIR)/%.o: src/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(LIB_DIR):
	mkdir -p $(LIB_DIR)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean