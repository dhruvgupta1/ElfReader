main: elfreader.o
	gcc -o elfreader -g main.o

elfreader.o: main.c elf.h
	gcc -g -c main.c

clean:
	rm -f *.o elfreader