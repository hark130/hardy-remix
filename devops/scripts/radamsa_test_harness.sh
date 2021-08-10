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
CRASHES_FILE_DIR="$TEST_FILE_DIR/crashes"
LOG_FILENAME="radamsa_log_"$(date +"%Y%m%d_%H%M%S")".log"
NUM_INPUTS=$1        # Number of test inputs to generate
TEMP_ERROR=""        # Capture stderr for each execution
TEMP_RET=0           # Temporary exit code var
INPUT_NUM=0          # Counter for test input
NUM_FILES_CREATED=0  # Number of files successfully created
FILE_EXISTS=0        # Current $TEST_FILE_DIR/$INPUT file was created


#
# PURPOSE: Join a filename with its directory
# ARGUMENTS:
#   directory: Directory in which to create the file
#   filename: Filename to create in directory
# NOTES:
#   Prints directory/filename on success
#   Prints "" on failure
# RETURN: None
#
join_file()
{
    # LOCAL VARIABLES
    DIRECTORY=$1     # Filename's directory
    FILENAME=$2      # Filename
    ABS_FILENAME=""  # Absolute filename constructed with $DIRECTORY

    # INPUT VALIDATION
    if [[ -n "$FILENAME" ]]
    then
        # Form Absolute Filename
        if [[ -z "$DIRECTORY" ]]
        then
            ABS_FILENAME=$FILENAME
        else
            ABS_FILENAME=$DIRECTORY"/"$FILENAME
        fi
    fi

    # DONE
    echo "$ABS_FILENAME"
}


#
# PURPOSE: Copy a file to a directory
# ARGUMENTS:
#   source_file: Relative or absolute filename to the file to copy
#   destination_dir: Destination directory
# NOTE:
#   Will not overwrite an existing file
#   Failing to overwrite an existing file will appear to succeed
# RETURN:
#   0 on success
#   1 for bad input
#   2 on error
#
copy_file()
{
    # LOCAL VARIABLES
    SOURCE_FILE=$1      # File, absolute or relative, to copy
    DESTINATION_DIR=$2  # Destination directory
    RET_VAL=0           # Function return value

    # INPUT VALIDATION
    if ([ -n $SOURCE_FILE ] && [ -n $DESTINATION_DIR ])
    then
        # DO IT
        cp -n "$SOURCE_FILE" "$DESTINATION_DIR"
        RET_VAL=$?
        if [[ $RET_VAL -ne 0 ]]
        then
            RET_VAL=2
        fi
    else
        RET_VAL=1
    fi

    # DONE
    return $RET_VAL
}


#
# PURPOSE: Copy a file from $TEST_FILE_DIR to $CRASHES_FILE_DIR
# ARGUMENTS:
#   filename: File to copy from $TEST_FILE_DIR to $CRASHES_FILE_DIR
# NOTE:
#   Will not overwrite an existing file
#   Failing to overwrite an existing file will appear to succeed
#   Function will automatically join the filename with the source dir
#   Calls copy_file() under the hood
# RETURN:
#   0 on success
#   1 for bad input
#   2 on error
#
save_crash_file()
{
    # LOCAL VARIABLES
    FILENAME=$1      # Filename to save in the crashes directory
    RET_VAL=0           # Function return value
    ABS_SOURCE_FILE=""  # $TEST_FILE_DIR/$FILENAME

    # JOIN IT
    ABS_SOURCE_FILE=$(join_file $TEST_FILE_DIR $FILENAME)

    # COPY IT
    copy_file "$ABS_SOURCE_FILE" "$CRASHES_FILE_DIR"
    RET_VAL=$?

    # DONE
    return $RET_VAL
}


#
# PURPOSE: Create a file in a directory with given content
# ARGUMENTS:
#   directory: Directory in which to create the file
#   filename: Filename to create in directory
#   contents: Contents to add to the filename created in directory
# NOTE:
#   Will not overwrite an existing file
#   Appends a newline to contents
#   Echos stderr from file creation
# RETURN:
#   0 on success
#   1 for bad input
#   2 if file already exists
#   3 on error
#
create_file()
{
    # LOCAL VARIABLES
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
        # if [[ -z "$DIRECTORY" ]]
        # then
        #     ABS_FILENAME=$FILENAME
        # else
        #     ABS_FILENAME=$DIRECTORY"/"$FILENAME
        # fi
        ABS_FILENAME=$(join_file "$DIRECTORY" "$FILENAME")

        # DO IT
        if [[ ! -f $ABS_FILENAME ]]
        then
            TEMP_ERROR=$(printf "%s\n" "$CONTENTS" 2>&1 > "$ABS_FILENAME")
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
    if [[ -n "$TEMP_ERROR" ]]
    then
        echo "$TEMP_ERROR"
    else
        TEMP_ERROR=""
    fi
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
#   Echos stderr from file creation
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
    TEMP_ERROR=$(create_file "$TEST_FILE_DIR" "$FILENAME" "$CONTENTS")
    RET_VAL=$?

    # DONE
    if [[ -n "$TEMP_ERROR" ]]
    then
        echo "$TEMP_ERROR"
    else
        TEMP_ERROR=""
    fi
    return $RET_VAL
}


#
# PURPOSE: Delete a file from a directory
# ARGUMENTS:
#   directory: Directory in which to create the file
#   filename: Filename to create in directory
# RETURN:
#   0 on success
#   1 for bad input
#   2 on error
#
delete_file()
{
    # LOCAL VARIABLES
    DIRECTORY=$1     # Directory that contains the filename
    FILENAME=$2      # Filename to delete
    ABS_FILENAME=""  # Concatenated $DIRECTORY and $FILENAME
    RET_VAL=0        # rm return value; Also function return value

    # CONCATENATE DIR AND FILE
    ABS_FILENAME=$(join_file "$DIRECTORY" "$FILENAME")

    # DO IT
    if [[ -n "$ABS_FILENAME" ]]
    then
        if [[ -f "$ABS_FILENAME" ]]
        then
            rm -f "$ABS_FILENAME"
            RET_VAL=$?
            if [[ $RET_VAL -ne 0 ]]
            then
                RET_VAL=2
            fi
        fi
    else
        RET_VAL=1
    fi

    # DONE
    return $RET_VAL
}


#
# PURPOSE: Delete a file from the $TEST_FILE_DIR directory
# ARGUMENTS:
#   filename: Filename to create in directory
# NOTE:
#   Calls delete_file() under the hood
# RETURN:
#   0 on success
#   1 for bad input
#   2 on error
#
delete_test_file()
{
    # LOCAL VARIABLES
    FILENAME=$1      # Filename to delete
    RET_VAL=0        # delete_file() return value; Also function return value

    # DO IT
    delete_file "$TEST_FILE_DIR" "$FILENAME"
    RET_VAL=$?

    # DONE
    return $RET_VAL
}


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
    TEMP_ERROR=$(create_test_file "$INPUT")
    TEMP_RET=$?
    if [[ $TEMP_RET -eq 0 ]]
    then
        # log_to_file "CREATED FILE: $INPUT"  # DEBUGGING
        NUM_FILES_CREATED=$(($NUM_FILES_CREATED + 1))
        FILE_EXISTS=1
    else
        # log_to_file "FAILED TO CREATE FILE: $INPUT"  # DEBUGGING
        # log_to_file "FAILED TO CREATE FILE INPUT #$INPUT_NUM WITH ERROR MESSAGE: $TEMP_ERROR"
        TEMP_ERROR=""
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
        save_crash_file "$INPUT"
        TEMP_RET=$?
        if [[ $TEMP_RET -ne 0 ]]
        then
            log_to_file "FAILED TO SAVE CRASH FILE FOR INPUT #$INPUT_NUM"
        else
            log_to_file "SAVE #$INPUT_NUM CRASH FILE"
        fi
    fi

    # Cleanup
    # "Cleanup, cleanup, cleanup this mess for me for me!"
    if [[ $FILE_EXISTS -eq 1 ]]
    then
        delete_test_file "$INPUT"
        TEMP_RET=$?
        if [[ $TEMP_RET -ne 0 ]]
        then
            log_to_file "UNABLE TO DELETE: $INPUT ...returned... RETURN VALUE: $TEMP_RET"
        else
            FILE_EXISTS=0
        fi
    fi

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
log_to_file "CREATED $NUM_FILES_CREATED FILES FOR $INPUT_NUM INPUTS"
log_to_file "EXECUTION FINISH"
