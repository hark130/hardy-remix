#include <stdio.h>

#define BUF_LEN 65


int main(int argc, char *argv[])
{
	// LOCAL VARIABLES
	char buffer[BUF_LEN] = {0};
	int retVal = 0;

	// READ AND WRITE
	if (!fgets(buffer, BUF_LEN, stdin))
	{
		retVal = 1;
	}
	else if (EOF == puts(buffer))
	{
		retVal = 2;
	}
	else
	{
		retVal = 0;
	}	

	// DONE
	return retVal;
}
