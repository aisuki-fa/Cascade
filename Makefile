CC      = gcc
CFLAGS  = -std=c99 -Wall -O2 $(shell pkg-config --cflags raylib) -I./src -I./lib/raygui
LDFLAGS = $(shell pkg-config --libs raylib) -lm

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)

cascade: $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o cascade

run: clean cascade
	./cascade
