#include <stdio.h>
/* This file tests gets.  Force it to be declared.  */
#include <features.h>

#undef __GLIBC_USE_DEPRECATED_GETS
#define __GLIBC_USE_DEPRECATED_GETS 1
#define BUF_LEN 64


int main(int argc, char *argv[])
{
	// LOCAL VARIABLES
	char buffer[BUF_LEN + 1];
	int retVal;

	// READ AND WRITE
	if (!gets(buffer))
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
