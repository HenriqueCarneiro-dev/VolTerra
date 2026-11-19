CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -O2
LDFLAGS = -lraylib -lopengl32 -lgdi32 -lwinmm
SRC     = $(wildcard src/*.c)
OBJ     = $(SRC:.c=.o)
TARGET  = volterra.exe

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del /Q src\*.o $(TARGET)

.PHONY: all clean