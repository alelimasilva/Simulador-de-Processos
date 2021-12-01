all: main

main: tpmain.c fat.o util.c
	gcc -O3 fat.o util.c  tpmain.c -o main  -w

util.o: util.c tp.h
	gcc -O3 -c  util.c  -w

fat.o: fat.c tp.h
	gcc -O3 -c  fat.c  -w

clear:
	rm *.o