##  MakeFile: Basic compilation for C/C++ programs  ##
######################################################

## Compiler Options (change accordingly for C/C++)
CC = gcc
SUFFIX = c

## Add compiler flags as needed
CCFLAGS = -g -Wall
LDFLAGS = -g -Wall

## Project Paths
PROJECT_ROOT=.
SRCDIR = $(PROJECT_ROOT)
OBJDIR = $(PROJECT_ROOT)
BINDIR = $(PROJECT_ROOT)

## Name of target file
TARGET = jash

SRCS := $(wildcard $(SRCDIR)/*.$(SUFFIX))
OBJS := $(SRCS:$(SRCDIR)/%.$(SUFFIX)=$(OBJDIR)/%.o)

.PHONY: all setup clean
all: setup $(BINDIR)/$(TARGET)

## Setting Up
setup:
	@mkdir -p $(OBJDIR)
	@mkdir -p $(BINDIR)

## Compiling
$(OBJS): $(OBJDIR)/%.o : $(SRCDIR)/%.$(SUFFIX)
	$(CC) -c $< -o $@ $(CCFLAGS) -MD

## Including dependencies
-include $(OBJS:.o=.d)

## Building executable
$(BINDIR)/$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

## CleanUp
clean:
	rm -rf $(OBJS) $(OBJS:.o=.d) $(BINDIR)/$(TARGET) 
