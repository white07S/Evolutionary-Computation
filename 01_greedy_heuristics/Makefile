# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2

# Directories
SRCDIR = src
INCDIR = include
BINDIR = bin

# Source files
SOURCES = $(SRCDIR)/main.c \
          $(SRCDIR)/algorithms.c \
          $(SRCDIR)/utils.c \
          $(SRCDIR)/local_search.c \
          $(SRCDIR)/cm_local_search.c \
          $(SRCDIR)/delta_local_search.c \
          $(SRCDIR)/msls.c \
          $(SRCDIR)/ils.c

# Header files (optional, for dependencies)
HEADERS = $(INCDIR)/algorithms.h \
          $(INCDIR)/utils.h \
          $(INCDIR)/local_search.h \
          $(INCDIR)/cm_local_search.h \
          $(INCDIR)/delta_local_search.h \
          $(INCDIR)/msls.h \
          $(INCDIR)/ils.h

# Executable name
EXECUTABLE = $(BINDIR)/greedy_heuristics

# Rule to build the executable
$(EXECUTABLE): $(SOURCES) $(HEADERS)
	# Ensure the bin directory exists
	mkdir -p $(BINDIR)
	# Compile the program
	$(CC) $(CFLAGS) -o $@ $(SOURCES) -I$(INCDIR) -lm

# Clean up generated files
clean:
	rm -rf $(BINDIR)

.PHONY: clean
