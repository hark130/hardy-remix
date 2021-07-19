# CC = gcc
CC = afl-gcc-fast
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

source04:
# 	$(CC) $(CFLAGS) -Wno-implicit-function-declaration -o $(DIST)source04_bad.o -c $(CODE)source04_bad.c
# 	$(CC) $(CFLAGS) -o $(DIST)source04_better.o -c $(CODE)source04_better.c
	$(CC) $(CFLAGS) -o $(DIST)source04_best.o -c $(CODE)source04_best.c
# 	$(CC) $(CFLAGS) -o $(DIST)source04_bad.bin $(DIST)source04_bad.o
# 	$(CC) $(CFLAGS) -o $(DIST)source04_better.bin $(DIST)source04_better.o
	$(CC) $(CFLAGS) -o $(DIST)source04_best.bin $(DIST)source04_best.o

source05:
# 	$(CC) $(CFLAGS) -Wno-implicit-function-declaration -o $(DIST)source05_bad.o -c $(CODE)source05_bad.c
# 	$(CC) $(CFLAGS) -o $(DIST)source05_better.o -c $(CODE)source05_better.c
	$(CC) $(CFLAGS) -o $(DIST)source05_best.o -c $(CODE)source05_best.c
# 	$(CC) $(CFLAGS) -o $(DIST)source05_bad.bin $(DIST)source05_bad.o
# 	$(CC) $(CFLAGS) -o $(DIST)source05_better.bin $(DIST)source05_better.o
	$(CC) $(CFLAGS) -o $(DIST)source05_best.bin $(DIST)source05_best.o

all_source:
	$(MAKE) source01
	$(MAKE) source04
	$(MAKE) source05

all:
	$(MAKE) clean
	$(MAKE) all_source

clean:
	@rm -f $(DIST)*.o $(DIST)*.exe $(DIST)*.bin $(DIST)*.lib $(DIST)*.so
