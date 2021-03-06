/*
 *	Implements HARE_library.h functions in the best way.
 */

#ifndef BINARY_NAME
#define BINARY_NAME "<this_best_binary>"  // You didn't define it so we defined it for you!
#endif  // BINARY_NAME

#include <sys/types.h>
#include <sys/stat.h>      // stat()
#include <errno.h>         // errno
#include <fcntl.h>
#include <libgen.h>        // basename()
#include <linux/limits.h>  // PATH_MAX
#include <stdarg.h>        // va_end(), va_start()
#include <stdbool.h>       // bool
#include <stdlib.h>        // calloc(), free()
#include <string.h>        // strlen(), strstr()
#include <unistd.h>        // close(), read()
#include "HARE_library.h"

#define BAD_MAX 128  // Buffer size macro

/*************************************************************************************************/
/*************************************** LIBRARY FUNCTIONS ***************************************/
/*************************************************************************************************/


// Different than the "good" version
pid_t be_sure(Configuration *config)
{
    // LOCAL VARIABLES
    pid_t daemon = -1;    // Return value from daemonize()

    // BE SURE
    daemon = daemonize();
    if (0 == daemon)
    {
        // syslog_it(LOG_DEBUG, "(CHILD) The call to daemonize() returned");  // DEBUGGING
        // syslog_it(LOG_DEBUG, "(CHILD) About to call execute_order()");  // DEBUGGING
        execute_order(config);
        // syslog_it(LOG_DEBUG, "(CHILD) The call to execute_order() returned");  // DEBUGGING
        // syslog_it(LOG_DEBUG, "(CHILD) About to call cleanupDaemon()");  // DEBUGGING
        cleanupDaemon();
        // syslog_it(LOG_DEBUG, "(CHILD) The call to cleanupDaemon() returned");  // DEBUGGING
    }
    else if (daemon < 0)
    {
        syslog_it(LOG_ERR, "Call to daemonize() failed");
    }
    else
    {
        // syslog_it(LOG_INFO, "(PARENT) Test harness will continue on");
    }

    // DONE
    return daemon;
}


// Different than the "good" version
int do_it(char *filename)
{
    // LOCAL VARIABLES
    char *file_contents = NULL;  // Head-allocated member holding the contents of filename
    int success = 0;             // 0 Success, -1 Bad input, -2 Error

    // 2. READ FILE CONTENTS
    if (filename)
    {
        // Read file
        file_contents = read_file(filename);
        // 3. PRINT FILE CONTENTS
        if (file_contents)
        {
            fprintf(stdout, "%s\n", file_contents);
        }
        else
        {
            success = -2;
        }
    }

    // DONE
    if (file_contents)
    {
        free(file_contents);
        file_contents = NULL;
    }
    return success;
}


// Different than the "good" version
int move_file(char *source, char *destination)
{
    // LOCAL VARIABLES
    int errnum = ENOERR;  // 0 on success, -1 on bad input, errno on failure

    // MOVE IT
    if (0 == errnum)
    {
        errnum = rename(source, destination);

        if (-1 == errnum)
        {
            errnum = errno;
        }
    }

    // DONE
    return errnum;
}


// Different than the "good" version
char *read_file(char *filename)
{
    // LOCAL VARIABLES
    char *file_contents = NULL;  // Allocate mem and return
    off_t file_size = BAD_MAX;   // Size of buffer in bytes
    FILE *fp = NULL;             // File pointer to filename
    char *curr_char_ptr = NULL;  // Temporary pointer
    char curr_char = 0x0;        // Temporary read character
    
    // INPUT VALIDATION
    if (filename && *filename)
    {
        // Size it
        // file_size = size_file(filename);
        if (file_size > -1)
        {
            // Allocate it
            file_contents = calloc(file_size + 1, 1);
            // Read it
            if (file_contents)
            {
                fp = fopen(filename, "r");
                if (fp)
                {
                    curr_char_ptr = file_contents;
                    while(1)
                    {
                        curr_char = getc(fp);
                        if (EOF == curr_char)
                        {
                            break;
                        }
                        else
                        {
                            *curr_char_ptr = curr_char;
                            curr_char_ptr++;
                            if (0x0 == curr_char)
                            {
                                break;
                            }
                        }
                    }
                }
            }            
        }
    }
    
    // DONE
    if (fp)
    {
        fclose(fp);
        fp = NULL;
    }
    return file_contents;
}
