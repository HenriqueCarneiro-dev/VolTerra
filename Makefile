CC      = gcc
CFLAGS  = -Wall -Wextra -g -Ilib/include -Iinclude
TARGET  = volterra

# Seus .c (src/) + .c da cli-lib (lib/src/), exceto o main.c de exemplo dela
SRC = $(wildcard src/*.c) \
      $(filter-out lib/src/main.c, $(wildcard lib/src/*.c))
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	./$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all run clean
