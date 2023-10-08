# Compiler flags
CFLAGS = -Wall -Wno-strict-aliasing -std=gnu11 -g -I. -O0

# Directory for object files
ODIR = build

# Object files generated from source files
OBJS = $(patsubst %.c, $(ODIR)/%.o, $(wildcard *.c))

# Path to the 8cc compiler executable
ECC = ./8cc

# header files, currently only 8cc.h
H_FILES := 8cc.h

# Additional CFLAGS override to define the BUILD_DIR
override CFLAGS += -DBUILD_DIR='"$(shell pwd)"'


8cc: $(H_FILES) $(OBJS)
	@echo "Compiling 8cc..."
	@cc -o $@ $(OBJS) $(LDFLAGS)
	@echo "8CC Compiled Successfully"

# compile individual .c source files into .o object files
$(ODIR)/%.o: %.c | $(ODIR)
	@$(CC) $(CFLAGS) -o $@ -c $<
	@echo "Successfully compiled $<"

# Create the output dir if it doesn't exist
$(ODIR): 
	@mkdir -p $(ODIR)

# dependencies for object files
$(OBJS): $(H_FILES) keyword.inc

clean:
	@echo "Cleaning Up"
	@rm -f 8cc
	@rm -rf $(ODIR)
	@echo "Cleaned!"

# Default target build the 8cc compiler
all: 8cc

.PHONY: clean all
