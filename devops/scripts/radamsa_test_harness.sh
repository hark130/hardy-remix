#!/bin/bash
#
# USAGE: radamsa_test_harness.sh <number of inputs to generate>
#
# NOTE: Consider using a tmpfs/ramdisk, as repeated writes may damage your hard disk or SSD.
#
# Reserved Exit Codes
# https://tldp.org/LDP/abs/html/exitcodes.html

# Filename to log execution
TEST_FILE_DIR="test/radamsa"
LOG_FILENAME="radamsa_log_"$(date +"%Y%m%d_%H%M%S")".log"
NUM_INPUTS=$1        # Number of test inputs to generate
TEMP_ERROR=""        # Capture stderr for each execution
TEMP_RET=0           # Temporary exit code var
INPUT_NUM=0          # Counter for test input
NUM_FILES_CREATED=0  # Number of files successfully created
FILE_EXISTS=0        # Current $TEST_FILE_DIR/$INPUT file was created


#
# PURPOSE: Create a file in a directory with given content
# ARGUMENTS:
#   directory: Directory in which to create the file
#   filename: Filename to create in directory
#   contents: Contents to add to the filename created in directory
# NOTE:
#   Will not overwrite an existing file
#   Appends a newline to contents
# RETURN:
#   0 on success
#   1 for bad input
#   2 if file already exists
#   3 on error
#
create_file()
{
    DIRECTORY=$1     # Filename's directory
    FILENAME=$2      # Filename
    CONTENTS=$3      # File contents
    RET_VAL=0        # create_file() return value
    PRINT_SUCCESS=0  # printf return value

    # INPUT VALIDATION
    if [[ -z "$FILENAME" ]]
    then
        RET_VAL=1
    else
        # Form Absolute Filename
        if [[ -z "$DIRECTORY" ]]
        then
            ABS_FILENAME=$FILENAME
        else
            ABS_FILENAME=$DIRECTORY"/"$FILENAME
        fi

        # DO IT
        if [[ ! -f $ABS_FILENAME ]]
        then
            printf "%s\n" "$CONTENTS" > $ABS_FILENAME
            PRINT_SUCCESS=$?
            if [[ $PRINT_SUCCESS -ne 0 ]]
            then
                RET_VAL=3
            fi
        else
            RET_VAL=2
        fi
    fi

    # DONE
    return $RET_VAL
}


#
# PURPOSE: Create a file in $TEST_FILE_DIR with the radasma content
# ARGUMENTS:
#   filename: Filename to create in directory
# NOTE:
#   Will not overwrite an existing file
#   Appends a newline to contents
#   Calls create_file() under the hood
# RETURN:
#   0 on success
#   1 for bad input
#   2 if file already exists
#   3 on error
#
create_test_file()
{
    # LOCAL VARIABLES
    FILENAME=$1  # Filename
    RET_VAL=0    # create_file() return value
    CONTENTS=$(echo "This is my file.  There are many like it but this one is mine." | radamsa | tr -d '\0')

    # DO IT
    # create_file "$TEST_FILE_DIR" "$FILENAME" "$(date)"
    create_file "$TEST_FILE_DIR" "$FILENAME" "$CONTENTS"
    RET_VAL=$?

    # DONE
    return $RET_VAL
}


# delete_file()
# {
#     # LOCAL VARIABLES
#     DIRECTORY=$1  # Directory that contains the filename
#     FILENAME=$2   # Filename to delete
#     RET_VAL=0     # rm return value; Also function return value


# }

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
# PURPOSE: Append an entry to $LOG_FILENAME
# ARGUMENTS:
#   message: Log entry to append to filename
# NOTES:
#   Prepends datetime stamps and appends a newline to log entries
#   Calls log_it() under the hood
#
log_to_file()
{
    MESSAGE=$1
    log_it $LOG_FILENAME "$MESSAGE"
}


# PREAMBLE
log_to_file "EXECUTION START"
log_to_file "EXECUTION: $0 $1 $2 $3 $4 $5 $6 $7 $8 $9"
log_to_file "NUMBER OF INPUTS: $NUM_INPUTS"

# DO_IT()
while true;
do
    # Generate Input
    INPUT=$(echo "some_file.txt" | radamsa --delay 100 | tr -d '\0')

    # Incremement Counter
    INPUT_NUM=$(($INPUT_NUM + 1))
    if [[ $(($INPUT_NUM % 100)) -eq 0 ]]
    then
        log_to_file "$INPUT_NUM INPUTS"
    fi

    # Create File
    create_test_file $INPUT
    TEMP_RET=$?
    if [[ $TEMP_RET -eq 0 ]]
    then
        log_to_file "CREATED FILE: $INPUT"  # DEBUGGING
        NUM_FILES_CREATED=$(($NUM_FILES_CREATED + 1))
    else
        log_to_file "FAILED TO CREATE FILE: $INPUT WITH RETURN VALUE: $TEMP_RET"  # DEBUGGING
    fi

    # Execute
    TEMP_ERROR=$(./dist/source05_bad.bin $INPUT 2>&1 > /dev/null)
    TEMP_RET=$?
    if [[ -n "$TEMP_ERROR" ]]
    then
        log_to_file "EXECUTION #$INPUT_NUM ...produced... ERROR: $TEMP_ERROR"
        TEMP_ERROR=""
    fi
    if [[ $TEMP_RET -ne 0 ]]
    then
        log_to_file "INPUT #$INPUT_NUM: $INPUT ...caused... RETURN VALUE: $TEMP_RET"
        # exit $TEMP_RET
    fi

    # Cleanup
    # "Cleanup, cleanup, cleanup this mess for me for me!"

    # Check Loop
    if [[ $INPUT_NUM -ge NUM_INPUTS ]]
    then
        log_to_file "$INPUT_NUM INPUTS COMPLETED"
        break
    else
        # Reset Temp Variables
        TEMP_RET=0
        TEMP_ERROR=""
        FILE_EXISTS=0
    fi
done

# FINALE
log_to_file "EXECUTION FINISH"
