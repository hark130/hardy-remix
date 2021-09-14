/*
 *  Implements HARE_library.h functions in a standardized way.
 */

#include <errno.h>     // errno
#include <fcntl.h>     // fcntl(), F_GETFL, F_SETFL
#include <stdarg.h>    // va_end(), va_start()
#include <stdlib.h>    // calloc(), free()
#include <string.h>    // strlen(), strstr()
#include <sys/types.h>
#include <sys/stat.h>  // stat()
#include <unistd.h>    // close(), read()
#include "HARE_library.h"  // be_sure(), Configuration

// An arbitrarily large maximum log message size has been chosen in an attempt to accommodate
//  calls to logging functions that take variable length arguments and accept printf()-family
//  "format strings".  These cases make it difficult to determine the ultimate string length
//  at runtime.
#define MAX_LOG_SIZE 32768

#define PIPE_BUFF_SIZE 2048  // Size of the local buffer in read_a_pipe()

/*
 * Updated version of code grabbed from bsd syslog header. Reflects SURE values.
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
 * Closes all opened streams from daemonize()
 * Copy/paste from SURE
 */
void cleanupDaemon()
{
    // Ignore any errors that might occur
    closeStdStreams();
}


/*
 * SURE implementation of a standard Linux daemon loader
 * Copy/paste/refactor from SURE
 * Returns PID if parent, 0 if child, -1 on failure
 */
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
        syslog_it(LOG_INFO, "Fork successful");
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
 * Tests euid for a value of 0
 * Copy/paste from SURE
 */
bool isRootUser()
{
    return (0 == geteuid());  // This function does not fail
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
            syslog_it(LOG_DEBUG, "(CHILD) The call to daemonize() returned");  // DEBUGGING
            syslog_it(LOG_DEBUG, "(CHILD) About to call execute_order()");  // DEBUGGING
            execute_order(config);
            syslog_it(LOG_DEBUG, "(CHILD) The call to execute_order() returned");  // DEBUGGING
            syslog_it(LOG_DEBUG, "(CHILD) About to call cleanupDaemon()");  // DEBUGGING
            cleanupDaemon();
            syslog_it(LOG_DEBUG, "(CHILD) The call to cleanupDaemon() returned");  // DEBUGGING
        }
        else if (daemon < 0)
        {
            syslog_it(LOG_ERR, "Call to daemonize() failed");
        }
        else
        {
            syslog_it(LOG_INFO, "(PARENT) Test harness will continue on");
        }
    }

    // DONE
    return daemon;
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
        syslog_it2(LOG_DEBUG, "Call to getINotifyData() returned %d", success);  // DEBUGGING
        if (0 == success)
        {
            if (config->inotify_message.message.buffer && config->inotify_message.message.size > 0)
            {
                syslog_it2(LOG_DEBUG, "Main: Received %s", config->inotify_message.message.buffer);
                // Received data, now add it to the jobs queue for the threadpool
                // thpool_add_work(threadPool, execRunner, allocContext(config, context));
                break;
            }
            else
            {
                // No data available. Sleep for a brief moment and try again.
                syslog_it(LOG_DEBUG, "Call to getINotifyData() provided no data.");
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


char *get_datetime_stamp(void)
{
    // LOCAL VARIABLES
    char *stamp = NULL;

    // STAMP IT
    // TD: DDN... Properly implement this function

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
        syslog_it(LOG_DEBUG, "About to call read_a_pipe()...");  // DEBUGGING
        data = read_a_pipe(pipe_fds[PIPE_READ], &msg_len, &errnum);
        // syslog_it(LOG_DEBUG, "The call to read_a_pipe() completed.");  // DEBUGGING

        if (errnum)
        {
            success = errnum;
            syslog_errno(errnum, "The call to read_a_pipe() failed.");
        }
        else if (!data)
        {
            // success = EIO;  // Input/output error
            success = ENOERR;  // Apparently, there was nothing to read
            syslog_it(LOG_INFO, "The call to read_a_pipe() returned NULL without an errno value.");
        }
        else if (0 >= msg_len)
        {
            success = ENODATA;  // No data available
            syslog_it2(LOG_ERR, "The call to read_a_pipe() returned a buffer pointer of length %d", msg_len);
        }
        else
        {
            syslog_it2(LOG_DEBUG, "The call to read_a_pipe() returned %s", data);  // DEBUGGING
            config->inotify_message.message.buffer = data;
            config->inotify_message.message.size = msg_len;
        }
    }

    // DONE
    return success;
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
            syslog_it(LOG_DEBUG, "Calling pipe2()");
            retval = pipe2(empty_pipes, flags);
            #else
            syslog_it(LOG_DEBUG, "Calling pipe()");
            retval = pipe(empty_pipes);
            call_fcntl = true;  // There are flags but we're not using pipe2()
            #endif  // _GNU_SOURCE && __USE_GNU
        }
        else
        {
            syslog_it(LOG_DEBUG, "No flags, calling pipe()");
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


int move_a_file(char *filename, char *dest, bool prepend)
{
    // LOCAL VARIABLES
    int errnum = -1;  // 0 on success, -1 on bad input, errno on failure

    // INPUT VALIDATION
    // Arguments
    if (filename && *filename && dest && *dest)
    {
        errnum = 0;

        // Environment
        if (1 != verify_filename(filename))
        {
            errnum = -1;
        }
        else if (1 != verify_directory(dest))
        {
            errnum = -1;
        }
    }

    // DO IT
    if (0 == errnum)
    {
        errnum = -1;  // TD: DDN... Implement this function properly
    }

    // DONE
    return errnum;
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
        syslog_it(LOG_DEBUG, "About to call write()");  // DEBUGGING
        write_retval = write(write_fd, write_buff, num_bytes);
        syslog_it(LOG_DEBUG, "The call to write() returned");  // DEBUGGING

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
            syslog_it(LOG_DEBUG, "The call to write() succeeded");  // DEBUGGING
        }
    }

    // DONE
    if (false == success)
    {
        syslog_errno(errnum, "Failed to write %zu bytes of %p to file descriptor %d", num_bytes, write_buff, write_fd);  // DEBUGGING
    }
    return errnum;
}
