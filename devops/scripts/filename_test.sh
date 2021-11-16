make filename_test
make source08

LOG_DIR=/tmp/log/
LOG_FILENAME=asan_log
WATCH_DIR=/tmp/watch/
PROCESS_DIR=/tmp/watch/processed/
TEST_NUM=1

# SETUP ENVIRONMENT
# Log
if [ -d $LOG_DIR ]
then
    rm -f $LOG_DIR*
else
	mkdir $WATCH_DIR
fi
# Watch
if [ -d $WATCH_DIR ]
then
    rm -f $WATCH_DIR*
else
	mkdir $WATCH_DIR
fi
# Processed
if [ -d $PROCESS_DIR ]
then
    rm -f $PROCESS_DIR*
else
	mkdir $PROCESS_DIR
fi

# TEST CODE
while [ `ls $LOG_DIR | wc -l` -eq 0 ] && [ `ls $WATCH_DIR | wc -l` -eq 1 ] && [ `ls $PROCESS_DIR | wc -l` -eq 0 ]
do
	echo "Test: $TEST_NUM"
	echo -n "some_filename.txt" | radamsa > filename_test.txt && sudo ASAN_OPTIONS="log_path=$LOG_DIR$LOG_FILENAME" $1 filename_test.txt
	TEST_NUM=$((TEST_NUM+1))
done
echo "$LOG_DIR"
ls $LOG_DIR
echo
echo "$WATCH_DIR"
ls $WATCH_DIR
echo
echo "$PROCESS_DIR"
ls $PROCESS_DIR
echo

#echo -n "some_filename.txt" | radamsa > filename_test.txt && sudo ./dist/filename_test_best.bin filename_test.txt
