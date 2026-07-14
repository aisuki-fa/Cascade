# compiler
CC      = gcc 
# c99 standard, all warnings, optimization level 2, tells compiler where to find raylib, include raygui
CFLAGS  = -std=c99 -Wall -O2 $(shell pkg-config --cflags raylib) -I./src -I./lib/raygui 
# linker flags, tells linker where to find raylib, link math library
LDFLAGS = $(shell pkg-config --libs raylib) -lm

# finds all source files and creates space-separated list of c files
SRC = $(wildcard src/*.c)
# creates list of object files by replacing .c with .o
OBJ = $(SRC:.c=.o)

# gcc output files and linker flags
cascade: $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

# pattern rule to compile .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# delete all object files and the executable and forces recompilation from scratch
clean:
	rm -f src/*.o cascade

# one liner that does clean + rebuild + run
run: clean cascade
	./cascade
