make filename_test
make source08

WATCH_DIR=/tmp/watch/
PROCESS_DIR=/tmp/watch/processed/
TEST_NUM=1

# SETUP ENVIRONMENT
# Watch
if [ -d $WATCH_DIR ]
then
    rm -f /tmp/watch/*
else
	mkdir $WATCH_DIR
fi
# Processed
if [ -d $PROCESS_DIR ]
then
    rm -f /tmp/watch/processed/*
else
	mkdir $PROCESS_DIR
fi

# TEST CODE
while [ `ls /tmp/watch/processed | wc -l` -eq 0 ] && [ `ls /tmp/watch | wc -l` -eq 1 ]
do
	echo "Test: $TEST_NUM"
	echo -n "some_filename.txt" | radamsa > filename_test.txt && sudo $1 filename_test.txt
	TEST_NUM=$((TEST_NUM+1))
done
ls /tmp/watch/
ls /tmp/watch/processed/

#echo -n "some_filename.txt" | radamsa > filename_test.txt && sudo ./dist/filename_test_best.bin filename_test.txt
