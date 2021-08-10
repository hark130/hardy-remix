#!/bin/bash
#
# USAGE: radamsa_test_harness.sh <number of inputs to generate>
#

# Reserved Exit Codes
# https://tldp.org/LDP/abs/html/exitcodes.html

LOG_FILENAME="radamsa_log_"$(date +"%Y%m%d_%H%M%S")".log"
NUM_INPUTS=$1
TEMP_RET=0
INPUT_NUM=0

#
# PURPOSE: Append an entry to a log file
# ARGUMENTS:
#   filename: Filename, relative or otherwise, to append to
#   message: Log entry to append to filename
# NOTES: Prepends datetime stamps and appends a newline to log entries
#
log_it()
{
    FILENAME=$1
    MESSAGE=$2
    DT_STAMP=$(date +"%Y%m%d_%H%M%S")
    printf "%s : %s\n" $DT_STAMP "$MESSAGE" >> $FILENAME
}


#
#
#
log_to_file()
{
    MESSAGE=$1
    log_it $LOG_FILENAME "$MESSAGE"
}

log_to_file "EXECUTION START"
log_to_file "EXECUTION: $0 $1 $2 $3 $4 $5 $6 $7 $8 $9"
log_to_file "NUMBER OF INPUTS: $NUM_INPUTS"
#./dist/waiting.bin


# while INPUT=$(echo "some_file.txt" | radamsa --count $NUM_INPUTS --delay 100);
for INPUT in $(echo "some_file.txt" | radamsa --count $NUM_INPUTS --delay 100 | tr -d '\0');
do
    INPUT_NUM=$(($INPUT_NUM + 1))
    if [[ $(($INPUT_NUM % 100)) -eq 0 ]]
    then
        log_to_file "$INPUT_NUM INPUTS"
    fi

    # log_to_file "INPUT: $INPUT"
    ./dist/source05_bad.bin $INPUT
    TEMP_RET=$?
    # log_to_file "RETURN VALUE: $TEMP_RET"
    if [[ $TEMP_RET -ne 0 ]]
    then
        log_to_file "INPUT #$INPUT_NUM: $INPUT ...caused... RETURN VALUE: $TEMP_RET"
        exit $TEMP_RET
    fi
done


log_to_file "EXECUTION FINISH"
