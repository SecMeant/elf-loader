all: main

main: main.s
	nasm main.s -f elf64 && ld main.o -o main

clean:
	rm -f main.o main

.PHONY: clean
