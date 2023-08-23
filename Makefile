CXX := g++
CXXFLAGS_BASE := -Wall -Wextra -std=gnu++2b -O2
CXXFLAGS += $(CXXFLAGS_BASE)
LINKDEPS := -lmipc

all: Makefile depend main loadelf

depend: .depend

.depend: loadelf.cc
	rm -f "$@"
	$(CXX) $(CXXFLAGS) -MM $^ -MF "$@"

include .depend

main: main.s
	nasm main.s -f elf64 && ld main.o -o main

loadelf.o: loadelf.cc
	$(CXX) loadelf.cc -c -o loadelf.o $(CXXFLAGS)

loadelf: loadelf.o
	$(CXX) loadelf.o -o loadelf $(LINKDEPS) $(CXXFLAGS)

clean:
	rm -f main.o main loadelf .depend

.PHONY: clean
