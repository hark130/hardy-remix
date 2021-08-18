/*
 * 1. Take a filename as a command line argument
 * 2. Read file contents
 * 3. Print the file contents
 */
#include <stdio.h>         // NULL
#include "HARE_library.h"  // Good and bad implementations of local function calls

#define LOGGING 0  // 1 for on
#define LOG_IT(msg) do { if (LOGGING) log_it(msg, "source07_log.txt"); } while (0)


int main(int argc, char *argv[])
{
    // LOCAL VARIABLES
    char *filename = NULL;  // Flow control; Stack mem
    int success = 0;        // 0 Success, -1 Bad input, -2 Error

    // DO IT
    LOG_IT(argv[0]);  // AFL DEBUGGING
    LOG_IT(argv[1]);  // AFL DEBUGGING
    // 1. FILE CLI ARG
    filename = get_filename(argc, argv);
    // 2 & 3. READ AND PRINT FILE
    success = do_it(filename);

    // DONE
    if (filename)
    {
        filename = NULL;
    }
    LOG_IT("\n");  // AFL DEBUGGING
    return success;
}
