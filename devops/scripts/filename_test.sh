make filename_test

rm -f /tmp/watch/processed/*
while [[ `ls /tmp/watch/processed | wc -c` -eq 0 ]]
do
	echo -n "some_filename.txt" | radamsa > filename_test.txt && sudo ./dist/filename_test_best.bin filename_test.txt
done
ls /tmp/watch/processed/

#echo -n "some_filename.txt" | radamsa > filename_test.txt && sudo ./dist/filename_test_best.bin filename_test.txt