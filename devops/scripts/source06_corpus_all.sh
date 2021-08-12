#!/bin/bash
#
# PURPOSE: Executes the source_06_bad binaries, which have been instrumented with AddressSanitizer,
#   Memwatch, and no sanitizer, on the corpus files using source06_corpus_test_harness.sh.  This
#   script was specifically created to compare the performance/success-rate between different
#   sanitizers on a static set of test inputs.
#
# USAGE: source06_corpus_all.sh
#
# NOTE: Consider using a tmpfs/ramdisk, as repeated writes may damage your hard disk or SSD.
#

# Directory to store log files
TEST_FILE_DIR="test/radamsa"
# Directory to store files that caused detectable crashes
CRASHES_FILE_DIR="$TEST_FILE_DIR/crashes"
NUM_INPUTS=$(ls "$CRASHES_FILE_DIR" | wc -l)  # Number of inputs to execute
clear

# BASE
devops/scripts/source06_corpus_test_harness.sh base $NUM_INPUTS
grep "EXECUTION: " `ls corpus*.log | tail -n 1`
grep -B 1 -A 3 STATISTICS `ls corpus*.log | tail -n 1`
echo

# ASAN
devops/scripts/source06_corpus_test_harness.sh ASAN $NUM_INPUTS
grep "EXECUTION: " `ls corpus*.log | tail -n 1`
grep -B 1 -A 3 STATISTICS `ls corpus*.log | tail -n 1`
echo

# MEMWATCH
devops/scripts/source06_corpus_test_harness.sh Memwatch $NUM_INPUTS
grep "EXECUTION: " `ls corpus*.log | tail -n 1`
grep -B 1 -A 3 STATISTICS `ls corpus*.log | tail -n 1`
echo
