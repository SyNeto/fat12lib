CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude
TARGET = fat12
SRC = src/fat12.c
OBJ = $(SRC:.c=.o)

all: lib$(TARGET).a

lib$(TARGET).a: $(OBJ)
	ar rcs lib$(TARGET).a $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) lib$(TARGET).a

.PHONY: all clean