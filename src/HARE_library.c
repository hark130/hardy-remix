/*
 *	Implements HARE_library.h functions in a standardized way.
 */

#include <errno.h>     // errno
#include <stdarg.h>    // va_end(), va_start()
#include <stdbool.h>   // bool
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
 * Copy/paste from SURE
 */
void daemonize()
{
    pid_t pid = 0;  // Process IDentifier
    pid_t sid = 0;  // Child process session ID

    // Fork off the parent process
    pid = fork();
    if (pid < 0)
    {
        syslog_errno(errno, "Failure value from fork()");
        syslog_it(LOG_EMERG, "Failed to fork");
        exit(EXIT_FAILURE);  // Error
    }
    else if (pid > 0)
    {
        syslog_it(LOG_INFO, "Fork successful");
        syslog_it(LOG_INFO, "Daemon loader is exiting");
        exit(EXIT_SUCCESS);  // Child process successfully forked
    }

    // Child process continues here
    syslog_it(LOG_NOTICE, "Starting daemon");

    // Change the file mode creation mask
    // NOTE: This system call always succeeds
    umask(0);

    // Create a new Session ID for the child process
    sid = setsid();
    if (sid < 0)
    {
        syslog_errno(errno, "Failure value from setsid()");
        syslog_it(LOG_EMERG, "Failed to acquire Session ID");
    }
    // Change the current working directory
    else if ((chdir("/")) < 0)
    {
        syslog_errno(errno, "Failure value from chdir()");
        syslog_it(LOG_EMERG, "Failed to change directory");
    }
    // Close out the standard file descriptors. A gentleman's agreement
    // was reached that this function can error and that the main
    // function will soldier on since it is not a critical error.
    redirectStdStreams();  // Redirect the standard streams to /dev/null

    // initSignalHandlers();
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


void be_sure(Configuration *config)
{
    // LOCAL VARIABLES
    int success = true;  // Flow control

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
        daemonize();

        execute_order(config);

        cleanupDaemon();
    }
}


void execute_order(Configuration *config)
{
    // LOCAL VARIABLES
    int success = 0;  // Holds return value from getInotifyData()

    // EXECUTE ORDER 66
    while(1)
    {
        // Retrieve the latest data from the message queue
        // success = getINotifyData(config);  // TD: DDN... Implement this function with shared pipes between the test harness
        if (1 == success)
        {
            syslog_it2(LOG_DEBUG, "Main: Received %s", config->inotify_message.message.buffer);
            // Received data, now add it to the jobs queue for the threadpool
            // thpool_add_work(threadPool, execRunner, allocContext(config, context));
        }
        else if (0 == success)
        {
            // No data available. Sleep for a brief moment and try again.
            syslog_it(LOG_DEBUG, "Call to getINotifyData() provided no data.");
            sleep(1);
            syslog_it(LOG_DEBUG, "Exiting (until the test harness' getINotifyData() is implemented).");  // TD: DDN... remove once getINotifyData() is implemented
            break;  // TD: DDN... remove once getINotifyData() is implemented
        }
        else
        {
            // Got error. Should we exit?
            syslog_it(LOG_ERR, "Call to getINotifyData() failed.  Exiting.");
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


int getINotifyData(Configuration *config)
{
    // LOCAL VARIABLES
    int success = -1;  // 0 on success, -1 on error, and errnum on failure

    // INPUT VALIDATION
    if (config)
    {
        success = 0;
    }

    // GET IT

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
    int success = -1;  // Returns 0 on success, -1 on bad input, errno on failure
    int retval = 0;    // Return value from pipe()/pipe2()

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
            retval = pipe2(empty_pipes, flags);
            #else
            retval = pipe(empty_pipes);
            #endif  // _GNU_SOURCE && __USE_GNU
        }
        else
        {
            retval = pipe(empty_pipes);
        }

        // Respond
        if (retval)
        {
            success = errno;
            syslog_errno(success, "The call to pipe()/pipe2() failed.");
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
        read_retval = read(read_fd, tmp_ptr, num_bytes);

        if (-1 == read_retval)
        {
            *errnum = errno;
            success = false;
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


int verify_filename(char *filename)
{
    // LOCAL VARIABLES
    struct stat response;  // Used by stat()
    int exists = 0;       // Return value

    // INPUT VALIDATION
    if (!filename || !(*filename))
    {
        exists = -1;
    }
    else
    {
        if (0 == stat(filename, &response))
        {
            exists = 1;
        }        
    }

    // DONE
    return exists;
}
