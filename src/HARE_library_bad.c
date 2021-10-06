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


// TD: DDN... Make this different from the "good" version
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


// TD: DDN... Make this different from the "good" version
void execute_order(Configuration *config)
{
    // LOCAL VARIABLES
    int success = 0;  // Holds return value from getInotifyData()

    // EXECUTE ORDER 66
    // syslog_it(LOG_DEBUG, "Starting execute_order() while loop...");  // DEBUGGING
    while(1)
    {
        // syslog_it(LOG_DEBUG, "Top of the execute_order() while loop...");  // DEBUGGING
        // Retrieve the latest data from the message queue
        success = getINotifyData(config);  // TD: DDN... Implement this function with shared pipes between the test harness
        // Returns 0 on success, -1 on error, and errnum on failure
        // syslog_it2(LOG_DEBUG, "Call to getINotifyData() returned %d", success);  // DEBUGGING
        if (0 == success)
        {
            if (config->inotify_message.message.buffer && config->inotify_message.message.size > 0)
            {
                // syslog_it2(LOG_DEBUG, "Main: Received %s", config->inotify_message.message.buffer);  // DEBUGGING
                // Received data, now add it to the jobs queue for the threadpool
                // thpool_add_work(threadPool, execRunner, allocContext(config, context));
                success = stamp_a_file(config->inotify_message.message.buffer, config->inotify_config.process);
                if (0 != success)
                {
                    syslog_errno(success, "The call to stamp_a_file() failed");
                }
                break;
            }
            else
            {
                // No data available. Sleep for a brief moment and try again.
                // syslog_it(LOG_DEBUG, "Call to getINotifyData() provided no data.");  // DEBUGGING
                sleep(1);
                // syslog_it(LOG_DEBUG, "Exiting (until the test harness' getINotifyData() is implemented).");  // TD: DDN... remove once getINotifyData() is implemented
                // break;  // TD: DDN... remove once getINotifyData() is implemented
            }
        }
        else
        {
            if (0 < success)
            {
                syslog_errno(success, "Call to getINotifyData() failed");
            }
            else
            {
                syslog_it2(LOG_ERR, "Call to getINotifyData() failed with %d.  Exiting.", success);
            }
            // Got error. Should we exit?
            break;  // Yes
        }
    }
}


// TD: DDN... Make this different from the "good" version
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


// Different than the "good" version
char *read_file(char *filename)
{
    // LOCAL VARIABLES
    char *file_contents = NULL;  // Allocate mem and return
    off_t file_size = 512;       // Size of buffer in bytes
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


// TD: DDN... Make this different from the "good" version
int stamp_a_file(char *source_file, char *dest_dir)
{
    // LOCAL VARIABLES
    int errnum = -1;                              // 0 on success, -1 on bad input, errno on failure
    char *datetime_stamp = NULL;                  // Store the datetime stamp here
    char new_filename[FILE_MAX + 1] = { 0 };      // New stamped filename
    char new_abs_filename[PATH_MAX + 1] = { 0 };  // Concatenated dest_dir and new_filename
    size_t stamp_len = 0;                         // Length of datetime_stamp
    size_t dest_len = 0;                          // Length of dest_dir

    // INPUT VALIDATION
    // Arguments
    if (source_file && *source_file && dest_dir && *dest_dir)
    {
        errnum = 0;

        // Environment
        if (1 != verify_filename(source_file))
        {
            errnum = -1;
            // syslog_it2(LOG_ERR, "Unable to verify source_file: %s", source_file);  // DEBUGGING
        }
        else if (1 != verify_directory(dest_dir))
        {
            errnum = -1;
            // syslog_it2(LOG_ERR, "Unable to verify dest_dir: %s", dest_dir);  // DEBUGGING
        }
    }

    // DO IT
    if (0 == errnum)
    {
        // Get datetime stamp
        datetime_stamp = get_datetime_stamp(&errnum);

        if (!datetime_stamp || ENOERR != errnum)
        {
            syslog_errno(errnum, "Call to get_datetime_stamp() failed");
        }
        else
        {
            // Form new destination filename
            stamp_len = strlen(datetime_stamp);
            dest_len = strlen(dest_dir);
            memcpy(new_filename, datetime_stamp, stamp_len);
            strncat(new_filename, basename(source_file), FILE_MAX - stamp_len);
            // syslog_it2(LOG_DEBUG, "new_filename == %s", new_filename);  // DEBUGGING
            memcpy(new_abs_filename, dest_dir, dest_len);
            if ('/' != new_abs_filename[strlen(new_abs_filename) - 1])
            {
                strncat(new_abs_filename, "/", 2);
            }
            strncat(new_abs_filename, new_filename, FILE_MAX);
            // syslog_it2(LOG_DEBUG, "new_abs_filename == %s", new_abs_filename);  // DEBUGGING

        }
        // errnum = -1;  // TD: DDN... Implement this function properly
    }
    // Move the file
    if (0 == errnum)
    {
        errnum = move_file(source_file, new_abs_filename);
        if (0 == errnum)
        {
            syslog_it2(LOG_INFO, "Successfully renamed %s to %s", source_file, new_abs_filename);
        }
        else if (-1 == errnum)
        {
            syslog_it(LOG_ERR, "The call to move_file() failed with bad input");
        }
        else
        {
            syslog_errno(errnum, "The call to move_file() failed");
        }
    }

    // CLEANUP
    if (datetime_stamp)
    {
        free(datetime_stamp);
        datetime_stamp = NULL;
    }

    // DONE
    return errnum;
}
