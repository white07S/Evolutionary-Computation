# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2 -fopenmp

# Directories
SRCDIR = src
INCDIR = include
BINDIR = bin

# Source files
SOURCES = $(SRCDIR)/main.c $(SRCDIR)/algorithms.c $(SRCDIR)/utils.c

# Executable name
EXECUTABLE = $(BINDIR)/greedy_heuristics

# Rule to build the executable
$(EXECUTABLE): $(SOURCES)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $(SOURCES) -I$(INCDIR) -lm

# Clean up generated files
clean:
	rm -rf $(BINDIR)

.PHONY: clean
