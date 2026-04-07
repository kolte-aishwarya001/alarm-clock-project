# Compiler and flags
CC = gcc
CFLAGS = $(shell pkg-config --cflags gtk+-3.0)  # Correctly call pkg-config to fetch flags for GTK
LDFLAGS = $(shell pkg-config --libs gtk+-3.0)   # Correctly call pkg-config for GTK libraries

# Windows-specific flags for PlaySound (if using MinGW or MSYS)
LIBS = -lwinmm  # PlaySound library

# Define the source file and output
SRC = alarm_clock.c
OBJ = alarm_clock.o
OUT = alarm_clock.exe

# Default target to build the executable
all: $(OUT)

# Build the object file
$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $(SRC) -o $(OBJ)

# Link the object file to create the executable
$(OUT): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) $(LIBS) -o $(OUT)

# Clean up generated files
clean:
	rm -f $(OBJ) $(OUT)

# Run the program (optional)
run: $(OUT)
	./$(OUT)
