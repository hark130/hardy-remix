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


/*************************************************************************************************/
/*************************************** LIBRARY FUNCTIONS ***************************************/
/*************************************************************************************************/


pid_t be_sure(Configuration *config)
{
    // LOCAL VARIABLES
    int success = true;   // Flow control
    pid_t daemon = -1;    // Return value from daemonize()

    // INPUT VALIDATION
    // config
    if (!config)
    {
        fprintf(stderr, "NULL Configuration pointer.\n");
        success = false;
    }
    // Configuration.inotify_config
    else if (NULL == config->inotify_config.watched)
    {
        fprintf(stderr, "NULL watched directory.\n");
        success = false;
    }
    else if (NULL == config->inotify_config.process)
    {
        fprintf(stderr, "NULL process directory.\n");
        success = false;
    }

    // ENVIRONMENT VALIDATION
    if (false == isRootUser())
    {
        fprintf(stderr, "Daemon must be run with root privileges.\n");
        success = false;
    }

    // BE SURE
    if (true == success)
    {
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
    }

    // DONE
    return daemon;
}


int do_it(char *filename)
{
    // LOCAL VARIABLES
    char *file_contents = NULL;  // Head-allocated member holding the contents of filename
    int file_exists = 0;         // Return value from verify_filename(): 1 exists, 0 missing, -1 error
    int success = 0;             // 0 Success, -1 Bad input, -2 Error

    // VERIFY ENVIRONMENT
    if (filename)
    {
        file_exists = verify_filename(filename);
        if (0 == file_exists)
        {
            fprintf(stderr, "Unable to find %s\n", filename);
            print_usage();
            filename = NULL;  // Used to control flow
            success = -1;
        }
        else if (-1 == file_exists)
        {
            fprintf(stderr, "Error verifying %s\n", filename);
            filename = NULL;  // Used to control flow
            success = -2;
        }
    }

    // 2. READ FILE CONTENTS
    if (1 == file_exists)
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


int move_file(char *source, char *destination)
{
    // LOCAL VARIABLES
    int errnum = -1;  // 0 on success, -1 on bad input, errno on failure

    // INPUT VALIDATION
    if (source && *source && destination && *destination)
    {
        if (1 == verify_filename(source) && 0 == verify_directory(destination))
        {
            errnum = ENOERR;
        }
    }

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


char *read_file(char *filename)
{
    // LOCAL VARIABLES
    char *file_contents = NULL;  // Allocate mem and return
    off_t file_size = 0;         // Size of filename in bytes
    int fd = 0;                  // File descriptor of filename
    size_t read_bytes = 0;       // Number of bytes read by read()
    
    // INPUT VALIDATION
    if (filename && *filename)
    {
        // Size it
        file_size = size_file(filename);
        if (file_size > -1)
        {
            // Allocate it
            file_contents = calloc(file_size + 1, 1);
            // Read it
            if (file_contents)
            {
                fd = open(filename, O_RDONLY);
                if (fd > -1)
                {
                    read_bytes = read(fd, file_contents, file_size);
                    if (-1 == read_bytes)
                    {
                        // Error
                        free(file_contents);
                        file_contents = NULL;
                    }
                }
            }
        }
    }
    
    // DONE
    if (fd)
    {
        close(fd);
        fd = 0;
    }
    return file_contents;
}
