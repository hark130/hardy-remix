/*
 *  Implements HARE_library.h functions in a standardized way.
 */

#define _XOPEN_SOURCE 500  // Let's use nftw()
#include <errno.h>         // errno
#include <fcntl.h>         // fcntl(), F_GETFL, F_SETFL
#include <ftw.h>           // nftw(), FTW macros
#include <libgen.h>        // basename()
#include <linux/limits.h>  // PATH_MAX
#include <stdarg.h>        // va_end(), va_start()
#include <stdio.h>         // rename(), remove()
#include <stdlib.h>        // calloc(), free()
#include <string.h>        // strlen(), strstr()
#include <sys/types.h>
#include <sys/stat.h>      // stat()
#include <time.h>          // localtime(), time_t
#include <unistd.h>        // close(), read()
#include <sys/wait.h>      // waitpid(), W* macros
#include "HARE_library.h"  // be_sure(), Configuration

// An arbitrarily large maximum log message size has been chosen in an attempt to accommodate
//  calls to logging functions that take variable length arguments and accept printf()-family
//  "format strings".  These cases make it difficult to determine the ultimate string length
//  at runtime.
#define MAX_LOG_SIZE 32768

#define PIPE_BUFF_SIZE 2048  // Size of the local buffer in read_a_pipe()

/*
 * Updated version of code grabbed from bsd syslog header. Reflects SURE values.
 * For reference (from: `man syslog`):
 *  LOG_EMERG      system is unusable
 *  LOG_ALERT      action must be taken immediately
 *  LOG_CRIT       critical conditions
 *  LOG_ERR        error conditions
 *  LOG_WARNING    warning conditions
 *  LOG_NOTICE     normal, but significant, condition
 *  LOG_INFO       informational message
 *  LOG_DEBUG      debug-level message
 */
CODE priorityNames[] =
{
   {"UNKNOWN",   -2},
   {"ALERT",     LOG_ALERT},
   {"CRITICAL",  LOG_CRIT},
   {"DEBUG",     LOG_DEBUG},
   {"EMERGENCY", LOG_EMERG},
   {"ERROR",     LOG_ERR},
   {"INFO",      LOG_INFO},
   {"NOTICE",    LOG_NOTICE},
   {"WARNING",   LOG_WARNING},
   {NULL,        -1},
};

int pipe_fds[2] = {INVALID_FD, INVALID_FD};  // Intializing global externed variable
char *base_filename = NULL;                  // Name of the file-based test case created by the test harness
size_t base_filename_len = 0;                // Length of the base_filename
char *processed_filename = NULL;             // Absolute filename of a file that matches on base_filename

/*************************************************************************************************/
/**************************************** LOCAL FUNCTIONS ****************************************/
/*************************************************************************************************/


/*
 * Closes and redirects the standard file streams (stdin, stdout, and stderr) by
 *      overwriting them with FILE pointers to /dev/null
 * Copy/paste from SURE
 */
int redirectStdStreams();


/*
 * Closes the standard streams (stdin, stdout, and stderr)
 * Copy/paste from SURE
 */
int closeStdStreams()
{
    int status = 0;  // Return value

    if (EOF == fclose(stdin))
    {
        syslog_errno(errno, "Failed to close stdin file stream");
        status = -1;
    }
    if (EOF == fclose(stdout))
    {
        syslog_errno(errno, "Failed to close stdout file stream");
        status = -1;
    }
    if (EOF == fclose(stderr))
    {
        syslog_errno(errno, "Failed to close stderr file stream");
        status = -1;
    }

    return status;
}


/*
 * function: getPriorityString
 * description: Returns the descriptive string for the integer priority
 *
 * priority: The logging priority
 *
 * return: String containing priority level
 */
char *getPriorityString(int priority)
{
    bool found = false;
    CODE *priorities = priorityNames;
    char *name  = priorities->name;

    while (NULL != priorities->name && !found)
    {
        if (priorities->value == priority)
        {
            name = priorities->name;
            found = true;
        }
        priorities++;
    }
    return name;
}


/*
 *  Compares fpath to base_filename.  Sets processed_filename if a match is found.
 *  Returns 1 if a file match is found (stop), 0 on no match (continue), and -1 on error
 *  Notes
 *      base_filename and processed_filename are both externed globals found in HARE_library.h.
 *          They are both initialized and utilized in source08_test_harness.c.
 */
// static int match_file(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf)
// {
//     // LOCAL VARIABLES
//     int continue = 0;                            // 0 to continue, 1 to stop, -1 on error
//     char *base_fpath = NULL;                     // Address to the base filename of fpath
//     char *needle_file = base_filename;           // Filename to match on
//     size_t needle_file_len = base_filename_len;  // Length of needle_file
//     bool nul_in_needle = false;                  // A nul character in the needle_file is a game changer

//     // INPUT VALIDATION
//     if (FTW_F != tflag)
//     {
//         syslog_it2(LOG_DEBUG, "Not a file so we're skipping %s", fpath);  // DEBUGGING
//     }
//     else if (fpath && sb && ftwbuf && needle_file && 0 < needle_file_len)
//     {
//         syslog_it2(LOG_DEBUG, "CURRENT STATUS... fpath: %s needle: %s needle length: %zu", fpath, needle_file, needle_file_len);  // DEBUGGING
//         // MATCH FILE NAMES
//         // 1. Does the needle have a nul character?
//         if (strlen(needle_file) != needle_file_len)
//         {
//             nul_in_needle = true;
//         }
//         // 2. Matching strategies
//         if (true == nul_in_needle)
//         {
//             _nul_file_matching
//         }
//     }
//     else
//     {
//         continue = -1;
//     }

//     return continue;
// }


/*
 *  Perform input validation on behalf of the _*nul_file_match() functions
 *  Returns -1 on error, 0 otherwise
 */
int _validate_nftw_callback(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf)
{
    // LOCAL VARIABLES
    int results = -1;

    // INPUT VALIDATION
    if (fpath && *fpath && sb && ftwbuf)
    {
        results = 0;
    }

    // DONE
    return results;
}


/*
 *  Implements a nftw() callback that deletes all files
 *  Returns -1 on error, errnum on failure, 0 otherwise (tells nftw() to continue)
 */
static int _delete_file(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf)
{
    // LOCAL VARIABLES
    int results = 0;  // Return value

    // INPUT VALIDATION
    // Arguments
    results = _validate_nftw_callback(fpath, sb, tflag, ftwbuf);

    // DO IT
    if (0 == results)
    {
        if (FTW_F == tflag)
        {
            results = delete_file((char *)fpath);  // Returns 0 on success, -1 on error, and errnum on failure
        }
    }

    // DONE
    return results;
}


/*
 *  Implements and utilizes the non-nul terminator filename matching algorithm as the
 *      nftw() callback
 *  Returns 1 on a match, -1 on error, 0 otherwise (tells nftw() to continue)
 */
static int _non_nul_file_match(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf)
{
    // LOCAL VARIABLES
    int results = 0;                  // 1 on a match, -1 on error, 0 otherwise (tells nftw() to continue)
    const char *fpath_base = NULL;    // Base filename from fpath
    size_t fpath_base_len = 0;        // Length of fpath's base filename
    size_t fpath_len = 0;             // Length of fpath

    // INPUT VALIDATION
    // Arguments
    results = _validate_nftw_callback(fpath, sb, tflag, ftwbuf);
    // Globals
    if (!base_filename || 0 >= base_filename_len)
    {
        results = -1;
    }

    // DO IT
    if (-1 != results)
    {
        if (FTW_F != tflag)
        {
            // syslog_it2(LOG_DEBUG, "Not a file so we're skipping %s", fpath);  // DEBUGGING
        }
        else
        {
            fpath_base = fpath + ftwbuf->base;  // Just the filename
            fpath_base_len = strlen(fpath_base);  // Length of the filename
            // syslog_it2(LOG_DEBUG, "fpath: %s", fpath);  // DEBUGGING
            // syslog_it2(LOG_DEBUG, "fpath_base: %s", fpath_base);  // DEBUGGING
            // syslog_it2(LOG_DEBUG, "fpath_base_len: %zu", fpath_base_len);  // DEBUGGING
            // syslog_it2(LOG_DEBUG, "base_filename: %s", base_filename);

            if (fpath_base_len < base_filename_len)
            {
                results = 0;  // There's not enough room for a match
            }
            else if (!memcmp(fpath_base + (fpath_base_len - base_filename_len), base_filename, base_filename_len))
            {
                fpath_len = strlen(fpath);
                processed_filename = calloc(fpath_len + 1, sizeof(char));
                if (processed_filename)
                {
                    if (processed_filename != memcpy(processed_filename, fpath, fpath_len))
                    {
                        results = -1;
                    }
                    else
                    {
                        // syslog_it2(LOG_DEBUG, "NON-'nul' processed_filename is %s", processed_filename);  // DEBUGGING
                        results = 1;
                    }
                }
                else
                {
                    results = -1;
                }
            }
        }
    }

    // CLEANUP
    if (-1 == results)
    {
        if (processed_filename)
        {
            free(processed_filename);
            processed_filename = NULL;
        }
    }

    // DONE
    return results;
}


/*
 *  Implements and utilizes the nul terminator filename matching algorithm as the nftw() callback
 *  Returns 1 on a match, -1 on error, 0 otherwise (tells nftw() to continue)
 */
static int _nul_file_match(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf)
{
    // LOCAL VARIABLES
    int results = 0;                // 1 on a match, -1 on error, 0 otherwise (tells nftw() to continue)
    const char *fpath_base = NULL;  // Base filename from fpath
    size_t fpath_base_len = 0;      // Length of fpath's base filename
    size_t base_filename_nul = 0;   // Find the nul-terminator in the base_filename
    size_t new_buff_len = 0;        // Length of the heap-allocated buffer to provide the caller

    // INPUT VALIDATION
    // Arguments
    results = _validate_nftw_callback(fpath, sb, tflag, ftwbuf);
    // Globals
    if (!base_filename || 0 >= base_filename_len)
    {
        results = -1;
    }

    // DO IT
    if (-1 != results)
    {
        if (FTW_F != tflag)
        {
            // syslog_it2(LOG_DEBUG, "Not a file so we're skipping %s", fpath);  // DEBUGGING
        }
        else
        {
            fpath_base = fpath + ftwbuf->base;  // Just the filename
            fpath_base_len = strlen(fpath_base);  // Length of the filename
            base_filename_nul = strlen(base_filename);  // Index of the base_filename nul character

            if (fpath_base_len < base_filename_nul)
            {
                results = 0;  // There's not enough room for a match
            }
            else if (!memcmp(fpath_base + (fpath_base_len - base_filename_nul), base_filename, base_filename_len))
            {
                // NOTE: The *real* buffer size would be:
                //  strlen(fpath) + 1 + base_filename_len - strlen(base_file_len) + 1, sizeof(char)
                //  ...but, when it comes to memory, better to overshoot than undershoot.
                new_buff_len = strlen(fpath) + 1 + base_filename_len;
                processed_filename = calloc(new_buff_len + 1, sizeof(char));
                if (processed_filename)
                {
                    if (processed_filename != memcpy(processed_filename, fpath, new_buff_len))
                    {
                        results = -1;
                    }
                    else
                    {
                        // syslog_it2(LOG_DEBUG, "'nul' processed_filename is %s", processed_filename);  // DEBUGGING
                        results = 1;
                    }
                }
                else
                {
                    results = -1;
                }
            }
        }
    }

    // CLEANUP
    if (-1 == results)
    {
        if (processed_filename)
        {
            free(processed_filename);
            processed_filename = NULL;
        }
    }

    // DONE
    return results;
}


/*
 *  Perform input validation on behalf of the *_nul_file_matching() functions
 *  Returns -1 on error, 0 otherwise
 */
int _validate_file_matching(char *dirname, char *filename, size_t filename_len)
{
    // LOCAL VARIABLES
    int results = 0;  // -1 on error, 0 otherwise

    // INPUT VALIDATION
    // dirname
    if (!dirname)
    {
        results = -1;
    }
    else if (1 != verify_directory(dirname))
    {
        results = -1;
    }
    // filename
    else if (!filename)
    {
        results = -1;
    }
    // filename_len
    else if (0 >= filename_len)
    {
        results = -1;
    }

    // DONE
    return results;
}


/*
 *  Executes a recursive dirwalk on dirname attempting to match filename if filename
 *      contains a premature nul character.  Does not validate input.
 *  Returns 1 on a match, -1 on error, 0 otherwise
 */
int _nul_file_matching(char *dirname, char *filename, size_t filename_len)
{
    // LOCAL VARIABLES
    int results = 0;                   // 1 on a match, -1 on error, 0 otherwise
    int flags = FTW_DEPTH | FTW_PHYS;  // See: man nftw

    // DIRWALK
    results = nftw(dirname, _nul_file_match, 4, flags);

    // VERIFY RESULTS
    if (processed_filename)
    {
        if (NULL != strstr(processed_filename, filename))
        {
            // syslog_it2(LOG_DEBUG, "_nul_file_matching() matched %s in %s with %s",
            //            filename, dirname, processed_filename);  // DEBUGGING
        }
        else
        {
            syslog_it2(LOG_ERR, "Odd that _nul_file_matching() matched %s in %s with %s",
                       filename, dirname, processed_filename);
        }
    }
    else if (1 == results)
    {
        syslog_it(LOG_ERR, "How can the processed_filename pointer be NULL but nftw() claims a match?!");
        results = -1;  // Can't have a match if the pointer is NULL!
    }

    // DONE
    return results;
}


/*
 *  Executes a recursive dirwalk on dirname attempting to match filename if filename
 *      does not contains a premature nul character.  Does not validate input.
 *  Returns 1 on a match, -1 on error, 0 otherwise
 */
int _non_nul_file_matching(char *dirname, char *filename, size_t filename_len)
{
    // LOCAL VARIABLES
    int results = 0;                   // 1 on a match, -1 on error, 0 otherwise
    int flags = FTW_DEPTH | FTW_PHYS;  // See: man nftw

    // DIRWALK
    results = nftw(dirname, _non_nul_file_match, 4, flags);

    // VERIFY RESULTS
    if (processed_filename)
    {
        if (NULL != strstr(processed_filename, filename))
        {
            // syslog_it2(LOG_DEBUG, "_non_nul_file_matching() matched %s in %s with %s",
            //            filename, dirname, processed_filename);  // DEBUGGING
        }
        else
        {
            syslog_it2(LOG_ERR, "Odd that _non_nul_file_matching() matched %s in %s with %s",
                       filename, dirname, processed_filename);
        }
    }
    else if (1 == results)
    {
        syslog_it(LOG_ERR, "How can the processed_filename pointer be NULL but nftw() claims a match?!");
        results = -1;  // Can't have a match if the pointer is NULL!
    }

    // DONE
    return results;
}


int redirectStdStreams()
{
    int status = 0;                  // Return value
    const char *path = "/dev/null";  // Null path

    // Close
    closeStdStreams();  // Continue execution on error

    // Redirect
    stdin = fopen(path, "r");
    if (NULL == stdin)
    {
        syslog_errno(errno, "Failed to open stdin file stream");
        status = -1;
    }
    stdout = fopen(path, "w+");
    if (NULL == stdout)
    {
        syslog_errno(errno, "Failed to open stdout file stream");
        status = -1;
    }
    stderr = fopen(path, "w+");
    if (NULL == stderr)
    {
        syslog_errno(errno, "Failed to open stderr file stream");
        status = -1;
    }

    return status;
}


/*************************************************************************************************/
/*************************************** LIBRARY FUNCTIONS ***************************************/
/*************************************************************************************************/


void cleanupDaemon()
{
    // Ignore any errors that might occur
    closeStdStreams();
}


pid_t daemonize()
{
    pid_t pid = 0;  // Process IDentifier
    pid_t sid = 0;  // Child process session ID

    // Fork off the parent process
    pid = fork();
    // Error
    if (pid < 0)
    {
        syslog_errno(errno, "Failure value from fork()");
        syslog_it(LOG_EMERG, "Failed to fork");
        exit(EXIT_FAILURE);  // Error
    }
    // Parent (test harness)
    else if (pid > 0)
    {
        syslog_it(LOG_NOTICE, "(PARENT) Fork successful");
        // syslog_it(LOG_INFO, "(PARENT) Test harness will continue");
        // exit(EXIT_SUCCESS);  // Child process successfully forked
    }
    // Child process
    else
    {
        // Child process continues here
        syslog_it(LOG_NOTICE, "(CHILD) Starting daemon");

        // Change the file mode creation mask
        // NOTE: This system call always succeeds
        umask(0);

        // Create a new Session ID for the child process
        sid = setsid();
        if (sid < 0)
        {
            syslog_errno(errno, "(CHILD) Failure value from setsid()");
            syslog_it(LOG_EMERG, "(CHILD) Failed to acquire Session ID");
        }
        // Change the current working directory
        else if ((chdir("/")) < 0)
        {
            syslog_errno(errno, "(CHILD) Failure value from chdir()");
            syslog_it(LOG_EMERG, "(CHILD) Failed to change directory");
        }
        // Close out the standard file descriptors. A gentleman's agreement
        // was reached that this function can error and that the main
        // function will soldier on since it is not a critical error.
        redirectStdStreams();  // Redirect the standard streams to /dev/null

        // initSignalHandlers();
    }

    // DONE
    return pid;
}


int delete_file(char *filename)
{
    // LOCAL VARIABLES
    int results = -1;  // 0 on success, -1 on error, and errnum on failure
    int errnum = 0;    // Store errno here

    // INPUT VALIDATION
    results = verify_filename(filename);

    // DELETE IT
    if (1 == results)
    {
        results = remove(filename);
        if (results)
        {
            errnum = errno;
            if (errnum)
            {
                results = errnum;
            }
            else
            {
                results = -1;
            }
        }
    }
    else
    {
        results = -1;
    }

    // DONE
    return results;
}


int delete_matching_file(char *dirname, char *filename, size_t filename_len)
{
    // LOCAL VARIABLES
    int results = -1;                 // 0 on success, -1 on error, -2 if no match found, and errnum on failure
    char *matched_file = NULL;  // Filename to delete

    // INPUT VALIDATION
    results = _validate_file_matching(dirname, filename, filename_len);

    // FIND IT
    if (0 == results)
    {
        matched_file = search_dir(dirname, filename, filename_len);
        if (!matched_file)
        {
            results = -2;  // Validation passed but no match found
        }
    }

    // DELETE IT
    if (0 == results)
    {
        // syslog_it2(LOG_DEBUG, "We're about to delete %s because it matched!", matched_file);  // DEBUGGING
        results = delete_file(matched_file);
    }

    // DONE
    return results;
}


int empty_dir(char *dirname)
{
    // LOCAL VARIABLES
    int success = verify_directory(dirname);  // 0 on success, -1 on error, and errnum on failure
    int flags = FTW_DEPTH | FTW_PHYS;         // See: man nftw

    // INPUT VALIDATION
    // verify_directory() return values: 1 exists, 0 missing, -1 error
    if (0 == success)
    {
        success = -1;  // Missing dirname counts as an error
    }
    else if (1 == success)
    {
        success = 0;  // Existing dirname counts as success (so far)
    }
    // An error while verifying dirname counts as an error so the obvious else condition falls through

    // EMPTY DIR
    if (0 == success)
    {
        success = nftw(dirname, _delete_file, 1, flags);
    }

    // DONE
    return success;
}


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
                // SEARCH FILE
                if (true == search_a_file(config->inotify_message.message.buffer, NEEDLE))
                {
                    syslog_it2(LOG_INFO, "Found the %s needle in the file %s", NEEDLE, config->inotify_message.message.buffer);
                }

                // STAMP FILE
                // syslog_it2(LOG_DEBUG, "Main: Received %s", config->inotify_message.message.buffer);  // DEBUGGING
                // Received data, now add it to the jobs queue for the threadpool
                // thpool_add_work(threadPool, execRunner, allocContext(config, context));
                success = stamp_a_file(config->inotify_message.message.buffer, config->inotify_config.process);
                // syslog_it2(LOG_DEBUG, "The call to stamp_a_file() returned %d.", success);  // DEBUGGING
                if (0 != success)
                {
                    syslog_errno(success, "The call to stamp_a_file() failed");
                }
                // syslog_it(LOG_DEBUG, "The call to stamp_a_file() returned");  // DEBUGGING

                // Cleanup
                free(config->inotify_message.message.buffer);
                config->inotify_message.message.buffer = NULL;
                config->inotify_message.message.size = 0;
                break;
            }
            else
            {
                // No data available. Sleep for a brief moment and try again.
                syslog_it(LOG_DEBUG, "Call to getINotifyData() provided no data.");  // DEBUGGING
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


char *get_filename(int argc, char *argv[])
{
    // LOCAL VARIABLES
    char *argOne = NULL;    // Argument 1 from argv
    char *filename = NULL;  // Filename, if it exists

    // PARSE ARGS
    argOne = parse_args(argc, argv);

    // CHECK ARGS
    filename = validate_arg(argOne);

    // DONE
    return filename;
}


char *get_datetime_stamp(int *errnum)
{
    // LOCAL VARIABLES
    char *stamp = NULL;                              // YYYYMMDD_HHMMSS_
    char format[] = { "%04d%02d%02d_%02d%02d%02d_"}; // snprintf() format
    size_t stamp_len = strlen("YYYYMMDD_HHMMSS_");   // Readable length of stamp
    time_t T = time(NULL);                           // Number of sec since Epoch
    struct tm time = *localtime(&T);                 // Transform datetime

    // INPUT VALIDATION
    if (errnum)
    {
        *errnum = ENOERR;  // Initialize

        // STAMP IT
        // Allocate
        stamp = calloc(stamp_len + 1, sizeof(char));

        if (!stamp)
        {
            *errnum = errno;
        }
        else if (stamp_len != snprintf(stamp, stamp_len + 1, format,
                                       time.tm_year + 1900, time.tm_mon, time.tm_mday,
                                       time.tm_hour, time.tm_min, time.tm_sec))
        {
            *errnum = errno;
            free(stamp);
            stamp = NULL;
        }
    }

    // DONE
    return stamp;
}


int getINotifyData(Configuration *config)
{
    // LOCAL VARIABLES
    int success = -1;   // 0 on success, -1 on error, and errnum on failure
    int errnum = 0;     // Errno value from call to read_a_pipe()
    char *data = NULL;  // Data read from read_a_pipe()
    int msg_len = 0;    // Length of data buffer

    // INPUT VALIDATION
    if (config)
    {
        success = 0;
    }

    // GET IT
    if (0 == success)
    {
        // syslog_it(LOG_DEBUG, "About to call read_a_pipe()...");  // DEBUGGING
        data = read_a_pipe(pipe_fds[PIPE_READ], &msg_len, &errnum);
        // syslog_it(LOG_DEBUG, "The call to read_a_pipe() completed.");  // DEBUGGING
        // syslog_it2(LOG_DEBUG, "The untested call to read_a_pipe() returned %s", data);  // DEBUGGING

        if (errnum)
        {
            success = errnum;
            syslog_errno(errnum, "The call to read_a_pipe() failed.");
        }
        else if (!data)
        {
            // success = EIO;  // Input/output error
            success = ENOERR;  // Apparently, there was nothing to read
            // syslog_it(LOG_INFO, "The call to read_a_pipe() returned NULL without an errno value.");  // DEBUGGING
        }
        else if (0 >= msg_len)
        {
            success = ENODATA;  // No data available
            syslog_it2(LOG_ERR, "The call to read_a_pipe() returned a buffer pointer of length %d", msg_len);
        }
        else
        {
            // syslog_it2(LOG_DEBUG, "The call to read_a_pipe() returned %s", data);  // DEBUGGING
            config->inotify_message.message.buffer = data;
            config->inotify_message.message.size = msg_len;
        }
    }

    // DONE
    return success;
}


bool isRootUser()
{
    return (0 == geteuid());  // This function does not fail
}


void log_it(char *log_entry, char *log_filename)
{
    // LOCAL VARIABLES
    FILE *fp = NULL;

    // INPUT VALIDATION
    if (log_filename && *log_filename)
    {
        fp = fopen("source07_log.txt", "a");
        
        // WRITE IT
        if (fp && log_entry)
        {
            fputs(log_entry, fp);
            fputs("\n", fp);  // Add a newline
        }
    }
    
    // DONE
    if (fp)
    {
        fclose(fp);
        fp = NULL;
    }
    return;
}


int make_pipes(int empty_pipes[2], int flags)
{
    // LOCAL VARIABLES
    int success = -1;         // Returns 0 on success, -1 on bad input, errno on failure
    int retval = 0;           // Return value from pipe()/pipe2()
    bool call_fcntl = false;  // If true, call fcntl() with flags
    int old_flags = 0;        // Current flags

    // INPUT VALIDATION
    if (empty_pipes)
    {
        empty_pipes[PIPE_READ] = INVALID_FD;
        empty_pipes[PIPE_WRITE] = INVALID_FD;
        success = 0;
    }

    // DO IT
    if (0 == success)
    {
        // pipe()
        if (flags)
        {
            #if defined _GNU_SOURCE && defined __USE_GNU
            // syslog_it(LOG_INFO, "Calling pipe2()");
            retval = pipe2(empty_pipes, flags);
            #else
            // syslog_it(LOG_INFO, "Calling pipe()");
            retval = pipe(empty_pipes);
            call_fcntl = true;  // There are flags but we're not using pipe2()
            #endif  // _GNU_SOURCE && __USE_GNU
        }
        else
        {
            // syslog_it(LOG_INFO, "No flags, calling pipe()");
            retval = pipe(empty_pipes);
        }

        // Respond
        if (retval)
        {
            success = errno;
            syslog_errno(success, "The call to pipe()/pipe2() failed.");
        }
        else if (true == call_fcntl)
        {
            old_flags = fcntl(empty_pipes[PIPE_READ], F_GETFL);

            if (-1 == old_flags)
            {
                success = errno;
                syslog_errno(success, "The call to fcntl(get_file_flags) failed.");
            }
            else if (0 != fcntl(empty_pipes[PIPE_READ], F_SETFL, old_flags | flags))
            {
                success = errno;
                syslog_errno(success, "The call to fcntl(set_file_flags) failed.");
            }
        }
    }

    // CLEAN UP
    if (0 != success)
    {
        if (INVALID_FD != empty_pipes[PIPE_READ])
        {
            close(empty_pipes[PIPE_READ]);  // Best effort.  Ignore errors.
            empty_pipes[PIPE_READ] = INVALID_FD;
        }
        if (INVALID_FD != empty_pipes[PIPE_WRITE])
        {
            close(empty_pipes[PIPE_WRITE]);  // Best effort.  Ignore errors.
            empty_pipes[PIPE_WRITE] = INVALID_FD;
        }
    }

    // DONE
    return success;
}


char *parse_args(int argc, char *argv[])
{
    // LOCAL VARIABLES
    char *argOne = NULL;

    // VALIDATE ARGUMENTS

    // Verify argc
    if (argc != 2)
    {
        fprintf(stderr, "Invalid number of arguments!\n");
    }
    else if (argv[0] && argv[1])
    {
        // Verify arg 0
        if (strstr(argv[0], BINARY_NAME) != (strlen(argv[0]) - strlen(BINARY_NAME) + argv[0]))
        {
            fprintf(stderr, "Invalid binary name!\n");
        }
        else
        {
            argOne = argv[1];
        }
    }

    // ERROR
    if (!argOne)
    {
        print_usage();
    }

    // DONE
    return argOne;
}


void print_usage(void)
{
    fprintf(stderr, "usage: %s [options] input_file\n", BINARY_NAME);
    fprintf(stderr, "\toptions:\n");
    fprintf(stderr, "\t\t-h, --help\t\tprint usage\n");
    fprintf(stderr, "\n");
}


char *read_a_pipe(int read_fd, int *msg_len, int *errnum)
{
    // LOCAL VARIABLES
    char local_buff[PIPE_BUFF_SIZE + 1] = { 0 };  // Temporary storage
    char *tmp_ptr = local_buff;                   // Iterating variable
    char *retval = NULL;                          // Function return value
    int read_count = 0;                           // Count the bytes read
    ssize_t read_retval = 0;                      // Return value from read()
    size_t num_bytes = 1;                         // Number of bytes to read at once
    bool success = false;                         // Flow control

    // INPUT VALIDATION
    if (-1 < read_fd && msg_len && errnum)
    {
        *msg_len = 0;
        *errnum = 0;
        success = true;
    }

    // READ IT
    while (true == success && read_count < PIPE_BUFF_SIZE)
    {
        // syslog_it2(LOG_DEBUG, "Calling read()... read_count: %d", read_count);  // DEBUGGING
        // syslog_it2(LOG_DEBUG, "Calling read(%d, %p, %zu)", read_fd, tmp_ptr, num_bytes);  // DEBUGGING
        read_retval = read(read_fd, tmp_ptr, num_bytes);
        // syslog_it(LOG_DEBUG, "The call to read() has returned");  // DEBUGGING

        if (-1 == read_retval)
        {
            *errnum = errno;
            if (EAGAIN == *errnum)
            {
                *errnum = 0;  // We're ignoring this particular error
                if (0 == read_count)
                {
                    success = false;
                }
                break;  // Observed behavior indicates there's nothing to read right now.
            }
            else
            {
                success = false;
                // syslog_it2(LOG_DEBUG, "So far we've read: %s", local_buff);  // DEBUGGING
            }
        }
        else if (0 == read_retval)
        {
            break;  // End of file
        }
        else
        {
            read_count += num_bytes;  // Advance the count
            tmp_ptr += num_bytes;  // Advance the iterator
        }
    }

    // COPY DATA
    if (true == success)
    {
        retval = calloc(read_count + 1, sizeof(char));

        if (!retval)
        {
            *errnum = errno;
            success = false;
        }
        else if (retval != memcpy(retval, local_buff, read_count))
        {
            *errnum = errno;
            success = false;
        }
        else
        {
            *msg_len = read_count;
        }
    }

    // CLEANUP
    if (false == success)
    {
        if (retval)
        {
            free(retval);
            retval = NULL;
        }
        if (msg_len)
        {
            *msg_len = 0;
        }
    }

    // DONE
    return retval;
}


bool search_a_file(char *haystack_file, char *needle)
{
    // LOCAL VARIABLES
    bool found_it = false;       // Return value: true if found, false otherwise (or on error)
    bool keep_going = false;     // Flow control
    char *file_contents = NULL;  // Return value from call to read_file()

    // INPUT VALIDATION
    if (haystack_file && *haystack_file && needle && *needle)
    {
        keep_going = true;
    }

    // DO IT
    // Read the file
    if (true == keep_going)
    {
        file_contents = read_file(haystack_file);
        if (!file_contents)
        {
            keep_going = false;
        }
    }
    // Search the contents
    if (true == keep_going)
    {
        if (strstr(file_contents, needle))
        {
            found_it = true;
            syslog_it2(LOG_DEBUG, "Found the needle %s in %s", needle, haystack_file);  // DEBUGGING
        }
        else
        {
            syslog_it2(LOG_DEBUG, "Failed to find the needle %s in %s", needle, haystack_file);  // DEBUGGING
        }
    }

    // CLEANUP
    if (file_contents)
    {
        free(file_contents);
        file_contents = NULL;
    }

    // DONE
    return found_it;
}


char *search_dir(char *haystack_dir, char *needle_file, size_t needle_file_len)
{
    // LOCAL VARIABLES
    char *matching_file = NULL;  // Filename that matches needle_file
    bool nul_in_needle = false;        // Different function calls based on nul characters
    int results = 0;                   // Return value from internal function calls

    // INPUT VALIDATION
    // haystack_dir
    if (!haystack_dir || 0x0 == *haystack_dir)
    {
        // syslog_it(LOG_DEBUG, "Problem with haystack_dir");  // DEBUGGING
    }
    else if (1 != verify_directory(haystack_dir))
    {
        // syslog_it2(LOG_DEBUG, "Unable to verify directory: %s", haystack_dir);  // DEBUGGING
    }
    // needle_file
    else if (!needle_file || 0x0 == *needle_file)
    {
        // syslog_it(LOG_DEBUG, "Problem with needle_file");  // DEBUGGING
    }
    // needle_file_len
    else if (0 >= needle_file_len)
    {
        // syslog_it2(LOG_DEBUG, "Invalid needle_file_len of %zu", needle_file_len);  // DEBUGGING
    }
    // DIR WALK
    else
    {
        // Is there a premature nul-character in needle_file?
        // syslog_it2(LOG_DEBUG, "needle_file_len is %zu and strlen(needle_file) is %zu", needle_file_len, strlen(needle_file));  // DEBUGGING
        if (needle_file_len != strlen(needle_file))
        {
            nul_in_needle = true;
        }
        // Find it!
        if (true == nul_in_needle)
        {
            results = _nul_file_matching(haystack_dir, needle_file, needle_file_len);
        }
        else
        {
            results = _non_nul_file_matching(haystack_dir, needle_file, needle_file_len);
        }
        // What happened?
        if (1 == results)
        {
            matching_file = processed_filename;
        }
    }

    // DONE
    return matching_file;
}


off_t size_file(char *filename)
{
    // LOCAL VARIABLES
    struct stat response;  // Used by stat()
    off_t size = -1;       // Return value

    // INPUT VALIDATION
    if (filename && *filename)
    {
        if (0 == stat(filename, &response))
        {
            size = response.st_size;
        }        
    }

    // DONE
    return size;
}


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


void syslog_it(int logLevel, char *msg)
{
    openlog(BINARY_NAME, LOG_PID, LOG_DAEMON);                        // System call returns void
    // Log formatted msg
    syslog(logLevel, "[%s] %s", getPriorityString(logLevel), msg);  // System call returns void
    closelog();
}


void syslog_it2(int logLevel, char *msg, ...)
{
    // LOCAL VARIABLES
    char message[MAX_LOG_SIZE] = {0};  // Holds the output version of the message
    va_list args;                      // Needed for variable length arguments

    // DO IT
    va_start(args, msg);
    // Use n version to prevent buffer overflow
    vsnprintf(message, sizeof(message), msg, args);
    syslog_it(logLevel, message);
    va_end(args);
}


void syslog_errno(int errNum, char *msg, ...)
{
    // LOCAL VARIABLES
    char tempMsg[256] = {0};           // Holds the temporary errno string
    char message[MAX_LOG_SIZE] = {0};  // Holds the output version of the message
    char *buffer = NULL;               // Malloced buffer if message is exceeded
    size_t msgLen = 0;                 // Holds temporary string length
    size_t tmpLen = 0;                 // Holds the message length
    va_list args;                      // Needed for variable length arguments

    // DO IT
    va_start(args, msg);

    // Use n version to prevent buffer overflow
    vsnprintf(message, sizeof(message), msg, args);
    snprintf(tempMsg, sizeof(tempMsg), " ERRNO: %d Reason: %s", errNum, strerror(errNum));
    tmpLen = strlen(tempMsg);
    msgLen = strlen(message);
    if (sizeof(message) > (msgLen + tmpLen))
    {
        strcat(message, tempMsg);
        syslog_it(LOG_ERR, message);
    }
    else
    {
        buffer = calloc(msgLen + tmpLen + 1, sizeof(char));
        if (buffer)
        {
            strcpy(buffer, message);
            strcat(buffer, tempMsg);
            syslog_it(LOG_ERR, buffer);
            free(buffer);
            buffer = NULL;
        }
        else
        {
            syslog_it(LOG_ERR, message);
            syslog_it(LOG_ERR, tempMsg);
        }
    }

    // CLEANUP
    va_end(args);
}


char *validate_arg(char *argOne)
{
    // LOCAL VARIABLES
    char *filename = argOne;
    char *options[] = { "-h", "--help", NULL };
    char **tmp_option = options;

    // CHECK IT
    while(tmp_option && *tmp_option)
    {
        if (strstr(argOne, *tmp_option) == argOne)
        {
            filename = NULL;  // It was an option, not the filename
            print_usage();
            break;
        }
        else
        {
            tmp_option++;
        }
    }

    // DONE
    return filename;
}


int verify_directory(char *directory)
{
    // LOCAL VARIABLES
    struct stat response;  // Used by stat()
    int exists = 0;        // 1 exists, 0 missing, -1 error

    // INPUT VALIDATION
    if (!directory || !(*directory))
    {
        exists = -1;
    }
    else
    {
        exists = verify_pathname(directory);

        if (1 == exists && 0 == stat(directory, &response))
        {
            if (S_IFDIR != (response.st_mode & S_IFMT))
            {
                exists = 0;
            }
        }
    }

    // DONE
    return exists;
}


int verify_filename(char *filename)
{
    // LOCAL VARIABLES
    struct stat response;  // Used by stat()
    int exists = 0;        // 1 exists, 0 missing, -1 error

    // INPUT VALIDATION
    if (!filename || !(*filename))
    {
        exists = -1;
    }
    else
    {
        exists = verify_pathname(filename);

        if (1 == exists && 0 == stat(filename, &response))
        {
            if (S_IFREG != (response.st_mode & S_IFMT))
            {
                exists = 0;
            }
        }
    }

    // DONE
    return exists;
}


int wait_daemon(pid_t daemon_pid, int *daemon_exit)
{
    // LOCAL VARIABLES
    int success = 0;        // 0 on success, -1 on error, or errno
    int wait_status = 0;    // Out parameter for the call to waitpid()
    int wait_options = 0;   // Argument to waitpid()
    pid_t wait_retval = 0;  // Return value from the call to waitpid()

    // WAIT
    if (daemon_exit)
    {
        wait_options = WUNTRACED | WCONTINUED;
        *daemon_exit = 0;
        do
        {
            wait_retval = waitpid(daemon_pid, &wait_status, wait_options);

            if (-1 == wait_retval)
            {
                if (errno)
                {
                    success = errno;
                    syslog_errno(success, "The call to waitpid(%ld) failed", daemon_pid);
                }
                else
                {
                    success = -1;
                    syslog_errno(success, "The call to waitpid(%ld) failed with an unspecified error", daemon_pid);
                }
                break;  // Error encountered.  Time to exit.
            }
            else
            {
                // Test return value
                if (0 == wait_retval)
                {
                    syslog_it2(LOG_DEBUG, "PID %ld has not yet changed state", daemon_pid);  // DEBUGGING
                }
                else
                {
                    syslog_it2(LOG_DEBUG, "PID %ld has changed state", daemon_pid);  // DEBUGGING
                }

                // Test status
                // WIFEXITED
                if (WIFEXITED(wait_status))
                {
                    *daemon_exit = WEXITSTATUS(wait_status);
                    syslog_it2(LOG_DEBUG, "PID %ld has exited with status %d", daemon_pid, *daemon_exit);  // DEBUGGING
                }
                // WIFSIGNALED
                if (WIFSIGNALED(wait_status))
                {
                    syslog_it2(LOG_DEBUG, "PID %ld was killed by signal %d", daemon_pid, WTERMSIG(wait_status));  // DEBUGGING
                }
                // WIFSTOPPED
                if (WIFSTOPPED(wait_status))
                {
                    syslog_it2(LOG_DEBUG, "PID %ld was stopped by signal %d", daemon_pid, WSTOPSIG(wait_status));  // DEBUGGING
                }
                // WIFCONTINUED
                if (WIFCONTINUED(wait_status))
                {
                    syslog_it2(LOG_DEBUG, "PID %ld continued", daemon_pid);  // DEBUGGING
                }
            }
        } while (!WIFEXITED(wait_status) && !WIFSIGNALED(wait_status));
    }
    else
    {
        success = -1;  // NULL Pointer
    }

    // DONE
    return success;
}


int verify_pathname(char *pathname)
{
    // LOCAL VARIABLES
    struct stat response;  // Used by stat()
    int exists = 0;        // 1 exists, 0 missing, -1 error

    // INPUT VALIDATION
    if (!pathname || !(*pathname))
    {
        exists = -1;
    }
    else if (0 == stat(pathname, &response))
    {
        exists = 1;
    }

    // DONE
    return exists;
}


int write_a_pipe(int write_fd, void *write_buff, size_t num_bytes)
{
    // LOCAL VARIABLES
    int errnum = 0;            // Here to capture errno if anything fails
    bool success = false;      // Set this to false if anything fails
    ssize_t write_retval = 0;  // Return value from read function call

    // INPUT VALIDATION
    if (write_fd < 0)
    {
        errnum = EBADF;  // Bad file descriptor
    }
    else if (!write_buff)
    {
        errnum = EFAULT;  // Bad address
    }
    else if (num_bytes <= 0)
    {
        errnum = EINVAL;  // Invalid argument
    }
    else
    {
        success = true;
    }

    // WRITE
    if (true == success)
    {
        // syslog_it(LOG_DEBUG, "About to call write()");  // DEBUGGING
        write_retval = write(write_fd, write_buff, num_bytes);
        // syslog_it(LOG_DEBUG, "The call to write() returned");  // DEBUGGING

        if (-1 == write_retval)
        {
            errnum = errno;
            success = false;
        }
        else if (0 == write_retval)
        {
            errnum = errno;
            success = false;
        }
        else
        {
            // syslog_it(LOG_DEBUG, "The call to write() succeeded");  // DEBUGGING
        }
    }

    // DONE
    if (false == success)
    {
        // syslog_errno(errnum, "Failed to write %zu bytes of %p to file descriptor %d", num_bytes, write_buff, write_fd);  // DEBUGGING
    }
    return errnum;
}
