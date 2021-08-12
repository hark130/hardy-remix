#!/bin/bash
#
# PURPOSE: Executes Radamsa on the source_06_bad binaries instrumented with AddressSanitizer,
# 	Memwatch, and no sanitizer using source06_test_harness.sh.  This script was specifically
#	created to compare the performance/success-rate between different sanitizers.
#
# USAGE: source06.sh
#
# NOTE: Consider using a tmpfs/ramdisk, as repeated writes may damage your hard disk or SSD.
#
NUM_INPUTS=100
clear

# BASE
devops/scripts/source06_test_harness.sh base $NUM_INPUTS
grep "EXECUTION: " `ls radamsa*.log | tail -n 1`
grep -B 1 -A 3 STATISTICS `ls radamsa*.log | tail -n 1`
# rm `ls radamsa*.log | tail -n1`
echo

# ASAN
devops/scripts/source06_test_harness.sh ASAN $NUM_INPUTS
grep "EXECUTION: " `ls radamsa*.log | tail -n 1`
grep -B 1 -A 3 STATISTICS `ls radamsa*.log | tail -n 1`
# rm `ls radamsa*.log | tail -n1`
echo

# MEMWATCH
devops/scripts/source06_test_harness.sh Memwatch $NUM_INPUTS
grep "EXECUTION: " `ls radamsa*.log | tail -n 1`
grep -B 1 -A 3 STATISTICS `ls radamsa*.log | tail -n 1`
# rm `ls radamsa*.log | tail -n1`
echo
