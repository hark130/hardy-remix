CC = gcc
CFLAGS=-Wall
DIST = ./dist/
CODE = ./src/

####################
# FUZZER VARIABLES #
####################

# 1. AMERICAN FUZZY LOP++ (AFL++)
AFLCC = afl-gcc-fast


#######################
# SANITIZER VARIABLES #
#######################

# 1. ADDRESS SANITIZER (ASAN)
ASANFLAGS=-fsanitize=address -g

# 2. MEMWATCH variables
# Directory containing the memwatch source file and header
MEMWATCH_DIR = $(CODE)Memwatch/
# Source file for the Memwatch tool
MEMWATCH_SOURCE = memwatch.c
# Necessary flags to "activate" Memwatch functionality
MEMWATCH_FLAGS = -DMEMWATCH -DMW_STDIO -g

memwatch:
	$(CC) $(CFLAGS) $(MEMWATCH_FLAGS) -o $(DIST)memwatch.o -c $(MEMWATCH_DIR)$(MEMWATCH_SOURCE)

source01:
	$(CC) $(CFLAGS) -Wno-implicit-function-declaration -o $(DIST)source01_bad.o -c $(CODE)source01_bad.c
	$(CC) $(CFLAGS) -o $(DIST)source01_better.o -c $(CODE)source01_better.c
	$(CC) $(CFLAGS) -o $(DIST)source01_best.o -c $(CODE)source01_best.c
	$(CC) $(CFLAGS) -o $(DIST)source01_bad.bin $(DIST)source01_bad.o
	$(CC) $(CFLAGS) -o $(DIST)source01_better.bin $(DIST)source01_better.o
	$(CC) $(CFLAGS) -o $(DIST)source01_best.bin $(DIST)source01_best.o

source04:
	$(CC) $(CFLAGS) -Wno-implicit-function-declaration -o $(DIST)source04_bad.o -c $(CODE)source04_bad.c
# 	$(CC) $(CFLAGS) -o $(DIST)source04_better.o -c $(CODE)source04_better.c
	$(CC) $(CFLAGS) -o $(DIST)source04_best.o -c $(CODE)source04_best.c
	$(CC) $(CFLAGS) -o $(DIST)source04_bad.bin $(DIST)source04_bad.o
# 	$(CC) $(CFLAGS) -o $(DIST)source04_better.bin $(DIST)source04_better.o
	$(CC) $(CFLAGS) -o $(DIST)source04_best.bin $(DIST)source04_best.o

source05:
	$(CC) $(CFLAGS) -o $(DIST)source05_bad.o -c $(CODE)source05_bad.c
# 	$(CC) $(CFLAGS) -o $(DIST)source05_better.o -c $(CODE)source05_better.c
	$(CC) $(CFLAGS) -o $(DIST)source05_best.o -c $(CODE)source05_best.c
	$(CC) $(CFLAGS) -o $(DIST)source05_bad.bin $(DIST)source05_bad.o
# 	$(CC) $(CFLAGS) -o $(DIST)source05_better.bin $(DIST)source05_better.o
	$(CC) $(CFLAGS) -o $(DIST)source05_best.bin $(DIST)source05_best.o

# This rule was created to compare fuzzzers, sanitizers and fuzzer/sanitizer combinations
source06:
	$(MAKE) memwatch
	$(CC) $(CFLAGS) -I$(MEMWATCH_DIR) -o $(DIST)source06_bad_base.bin $(CODE)source06_bad.c $(CODE)HARE_memwatch.c
	$(CC) $(CFLAGS) -I$(MEMWATCH_DIR) $(ASANFLAGS) -o $(DIST)source06_bad_ASAN.bin $(CODE)source06_bad.c $(CODE)HARE_memwatch.c
	$(CC) $(CFLAGS) -I$(MEMWATCH_DIR) $(MEMWATCH_FLAGS) -o $(DIST)source06_bad_Memwatch.bin $(CODE)source06_bad.c $(CODE)HARE_memwatch.c $(DIST)memwatch.o
	$(AFLCC) $(CFLAGS) -I$(MEMWATCH_DIR) -o $(DIST)source06_bad_AFL.bin $(CODE)source06_bad.c $(CODE)HARE_memwatch.c
	$(AFLCC) $(CFLAGS) -I$(MEMWATCH_DIR) -fsanitize=address -o $(DIST)source06_bad_AFL_ASAN.bin $(CODE)source06_bad.c $(CODE)HARE_memwatch.c

waiting:
	$(CC) $(CFLAGS) -o $(DIST)waiting.o -c $(CODE)waiting.c
	$(CC) $(CFLAGS) -o $(DIST)waiting.bin $(DIST)waiting.o

all_source:
	$(MAKE) source01
	$(MAKE) source04
	$(MAKE) source05
	$(MAKE) source06
	$(MAKE) waiting

all:
	$(MAKE) clean
	$(MAKE) memwatch
	$(MAKE) all_source

clean:
	@rm -f $(DIST)*.o $(DIST)*.exe $(DIST)*.bin $(DIST)*.lib $(DIST)*.so
