#!/bin/bash
NUM_INPUTS=10000
clear

# BASE
devops/scripts/source06_test_harness.sh base $NUM_INPUTS
grep "EXECUTION: " `ls radamsa*.log | tail -n 1`
grep -B 1 -A 3 STATISTICS `ls radamsa*.log | tail -n 1`
rm `ls radamsa*.log | tail -n1`
echo

# ASAN
devops/scripts/source06_test_harness.sh ASAN $NUM_INPUTS
grep "EXECUTION: " `ls radamsa*.log | tail -n 1`
grep -B 1 -A 3 STATISTICS `ls radamsa*.log | tail -n 1`
rm `ls radamsa*.log | tail -n1`
echo

# MEMWATCH
devops/scripts/source06_test_harness.sh Memwatch $NUM_INPUTS
grep "EXECUTION: " `ls radamsa*.log | tail -n 1`
grep -B 1 -A 3 STATISTICS `ls radamsa*.log | tail -n 1`
rm `ls radamsa*.log | tail -n1`
echo