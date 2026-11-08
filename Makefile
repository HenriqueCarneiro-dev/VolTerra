CC = gcc
CFLAGS = -Wall -std=c99
LIBS = -lraylib -lopengl32 -lgdi32 -lwinmm

all:
    $(CC) src/main.c -o volterra.exe $(CFLAGS) $(LIBS)