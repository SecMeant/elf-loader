CXX := g++
CXXFLAGS_BASE := -Wall -Wextra -std=gnu++2b -O2
CXXFLAGS += $(CXXFLAGS_BASE)
LINKDEPS := -lmipc

all: Makefile depend main main_pie loadelf

depend: .depend

.depend: loadelf.cc
	rm -f "$@"
	$(CXX) $(CXXFLAGS) -MM $^ -MF "$@"

include .depend

main.o: main.s
	nasm main.s -f elf64 -o main.o

main: main.o
	ld main.o -o main

main_pie: main.o
	ld -pie main.o -o main_pie

loadelf.o: loadelf.cc
	$(CXX) loadelf.cc -c -o loadelf.o $(CXXFLAGS)

loadelf: loadelf.o
	$(CXX) loadelf.o -o loadelf $(LINKDEPS) $(CXXFLAGS)

clean:
	rm -f main.o main loadelf .depend

.PHONY: clean
