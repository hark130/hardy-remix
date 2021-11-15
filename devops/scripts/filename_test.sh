make filename_test
make source08

rm -f /tmp/watch/*
rm -f /tmp/watch/processed/*
TEST_NUM=1
while [[ `ls /tmp/watch/processed | wc -l` -eq 0 ]] && [[ `ls /tmp/watch | wc -l` -eq 1 ]]
do
	echo "Test: $TEST_NUM"
	echo -n "some_filename.txt" | radamsa > filename_test.txt && sudo $1 filename_test.txt
	((TEST_NUM=TEST_NUM+1))
done
ls /tmp/watch/
ls /tmp/watch/processed/

#echo -n "some_filename.txt" | radamsa > filename_test.txt && sudo ./dist/filename_test_best.bin filename_test.txt
