CC = gcc
CFLAGS=-Wall
DIST = ./dist/
CODE = ./src/

####################
# FUZZER VARIABLES #
####################

# 1. AMERICAN FUZZY LOP++ (AFL++)
AFLCC = afl-gcc-fast

# 2. HONGGFUZZ
HGFUZZCC = hfuzz-gcc
HONGFLAGS = -fsanitize-coverage=trace-pc -O3 -fno-omit-frame-pointer -ggdb -Wno-error


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

# HARE variables
HARE_BIN_NAME = "\"default_bin_name\""
HARE_FLAGS = -DBINARY_NAME=$(HARE_BIN_NAME)

hare:
	$(CC) $(CFLAGS) $(HARE_FLAGS) -o $(DIST)HARE_library_bad.o -c $(CODE)HARE_library_bad.c
	$(CC) $(CFLAGS) $(HARE_FLAGS) -o $(DIST)HARE_library_best.o -c $(CODE)HARE_library_best.c

library:
	$(MAKE) hare
	$(MAKE) memwatch

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
	$(AFLCC) $(CFLAGS) -I$(MEMWATCH_DIR) $(ASANFLAGS) -o $(DIST)source06_bad_AFL_ASAN.bin $(CODE)source06_bad.c $(CODE)HARE_memwatch.c

source07:
	$(CC) $(CFLAGS) -DBINARY_NAME="\"source07_bad.bin\"" -o $(DIST)HARE_library_bad.o -c $(CODE)HARE_library_bad.c
	$(CC) $(CFLAGS) -DBINARY_NAME="\"source07_best.bin\"" -o $(DIST)HARE_library_best.o -c $(CODE)HARE_library_best.c
	$(CC) $(CFLAGS) -o $(DIST)source07_bad.o -c $(CODE)source07_bad.c
	$(CC) $(CFLAGS) -o $(DIST)source07_best.o -c $(CODE)source07_best.c
	$(CC) $(CFLAGS) -o $(DIST)source07_test_harness.o -c $(CODE)source07_test_harness.c
	$(CC) $(CFLAGS) -o $(DIST)source07_bad.bin $(DIST)source07_bad.o $(DIST)HARE_library_bad.o
	$(CC) $(CFLAGS) -o $(DIST)source07_best.bin $(DIST)source07_best.o $(DIST)HARE_library_best.o
	$(CC) $(CFLAGS) -o $(DIST)source07_test_harness_bad.bin $(DIST)source07_test_harness.o $(DIST)HARE_library_bad.o
	$(CC) $(CFLAGS) -o $(DIST)source07_test_harness_best.bin $(DIST)source07_test_harness.o $(DIST)HARE_library_best.o
	$(CC) $(CFLAGS) $(ASANFLAGS) -o $(DIST)source07_test_harness_bad_ASAN.bin $(CODE)source07_test_harness.c $(CODE)HARE_library_bad.c
	$(CC) $(CFLAGS) $(ASANFLAGS) -o $(DIST)source07_test_harness_best_ASAN.bin $(CODE)source07_test_harness.c $(CODE)HARE_library_best.c
	$(MAKE) source07_afl
	$(MAKE) source07_honggfuzz

# This rule was created to facilitate making an AFL++ test harness
source07_afl:
	$(AFLCC) $(CFLAGS) -DBINARY_NAME="\"source07_bad.bin\"" -o $(DIST)source07_test_harness_bad_AFL.bin $(CODE)HARE_library_bad.c $(CODE)source07_test_harness.c
	$(AFLCC) $(CFLAGS) -DBINARY_NAME="\"source07_bad.bin\"" $(ASANFLAGS) -o $(DIST)source07_test_harness_bad_AFL_ASAN.bin $(CODE)HARE_library_bad.c $(CODE)source07_test_harness.c
	$(AFLCC) $(CFLAGS) -DBINARY_NAME="\"source07_best.bin\"" -o $(DIST)source07_test_harness_best_AFL.bin $(CODE)HARE_library_best.c $(CODE)source07_test_harness.c
	$(AFLCC) $(CFLAGS) -DBINARY_NAME="\"source07_best.bin\"" $(ASANFLAGS) -o $(DIST)source07_test_harness_best_AFL_ASAN.bin $(CODE)HARE_library_best.c $(CODE)source07_test_harness.c

# This rule was created to facilitate making Honggfuzz test harnesses
source07_honggfuzz:
	$(HGFUZZCC) $(CFLAGS) -DBINARY_NAME="\"source07_bad.bin\"" -g $(HONGFLAGS) -o $(DIST)source07_test_harness_bad_HGFUZZ.bin $(CODE)HARE_library_bad.c $(CODE)source07_test_harness.c
	$(HGFUZZCC) $(CFLAGS) -DBINARY_NAME="\"source07_bad.bin\"" $(ASANFLAGS) -o $(DIST)source07_test_harness_bad_HGFUZZ_ASAN.bin $(CODE)HARE_library_bad.c $(CODE)source07_test_harness.c
	$(HGFUZZCC) $(CFLAGS) -DBINARY_NAME="\"source07_best.bin\"" -g $(HONGFLAGS) -o $(DIST)source07_test_harness_best_HGFUZZ.bin $(CODE)HARE_library_best.c $(CODE)source07_test_harness.c
	$(HGFUZZCC) $(CFLAGS) -DBINARY_NAME="\"source07_best.bin\"" $(ASANFLAGS) -o $(DIST)source07_test_harness_best_HGFUZZ_ASAN.bin $(CODE)HARE_library_best.c $(CODE)source07_test_harness.c

# This rule was created to replicate the behavior of a basic Linux daemon
source08:
	$(CC) $(CFLAGS) -DBINARY_NAME="\"source08_bad.bin\"" -o $(DIST)source08_test_harness_bad.bin $(CODE)HARE_library_bad.c $(CODE)HARE_library.c $(CODE)source08_test_harness.c
	$(CC) $(CFLAGS) -DBINARY_NAME="\"source08_best.bin\"" -o $(DIST)source08_test_harness_best.bin $(CODE)HARE_library_best.c $(CODE)HARE_library.c $(CODE)source08_test_harness.c

waiting:
	$(CC) $(CFLAGS) -o $(DIST)waiting.o -c $(CODE)waiting.c
	$(CC) $(CFLAGS) -o $(DIST)waiting.bin $(DIST)waiting.o

all_source:
	$(MAKE) source01
	$(MAKE) source04
	$(MAKE) source05
	$(MAKE) source06
	$(MAKE) source07
	$(MAKE) source07_afl
	$(MAKE) source07_honggfuzz
	$(MAKE) waiting

all:
	$(MAKE) clean
	$(MAKE) library
	$(MAKE) all_source

clean:
	@rm -f $(DIST)*.o $(DIST)*.exe $(DIST)*.bin $(DIST)*.lib $(DIST)*.so
