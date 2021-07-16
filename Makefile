CC = gcc
CFLAGS=-Wall
DIST = ./dist/
CODE = ./src/

source01:
	$(CC) $(CFLAGS) -Wno-implicit-function-declaration -o $(DIST)source01_bad.o -c $(CODE)source01_bad.c
	$(CC) $(CFLAGS) -o $(DIST)source01_better.o -c $(CODE)source01_better.c
	$(CC) $(CFLAGS) -o $(DIST)source01_best.o -c $(CODE)source01_best.c
	$(CC) $(CFLAGS) -o $(DIST)source01_bad.bin $(DIST)source01_bad.o
	$(CC) $(CFLAGS) -o $(DIST)source01_better.bin $(DIST)source01_better.o
	$(CC) $(CFLAGS) -o $(DIST)source01_best.bin $(DIST)source01_best.o

all_source:
	$(MAKE) source01

all:
	$(MAKE) clean
	$(MAKE) all_source

clean:
	@rm -f $(DIST)*.o $(DIST)*.exe $(DIST)*.bin $(DIST)*.lib $(DIST)*.so
