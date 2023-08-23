CXX ?= clang++
CXXFLAGS_BASE := -Wall -Wextra -std=c++23 -O2
CXXFLAGS += $(CXXFLAGS_BASE)
LINKDEPS := -lmipc

all: Makefile main loadelf

main: main.s
	nasm main.s -f elf64 && ld main.o -o main

loadelf: loadelf.cc
	$(CXX) loadelf.cc -o loadelf $(LINKDEPS) $(CXXFLAGS)

clean:
	rm -f main.o main loadelf

.PHONY: clean
