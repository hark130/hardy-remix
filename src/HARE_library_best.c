/*
 *	Implements HARE_library.h functions in the best way.
 */

#ifndef BINARY_NAME
#define BINARY_NAME "<this_best_binary>"  // You didn't define it so we defined it for you!
#endif  // BINARY_NAME

#include <sys/types.h>
#include <sys/stat.h>  // stat()
#include <fcntl.h>
#include <stdarg.h>    // va_end(), va_start()
#include <stdbool.h>   // bool
#include <stdlib.h>    // calloc(), free()
#include <string.h>    // strlen(), strstr()
#include <syslog.h>    // syslog(), LOG_* macros
#include <unistd.h>    // close(), read()
#include "HARE_library.h"

/*
 * Stolen from https://opensource.apple.com/source/xnu/xnu-344/bsd/sys/syslog.h.auto.html
 */
typedef struct _code {
    char *name;
    int  value;
} CODE;

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

// An arbitrarily large maximum log message size has been chosen in an attempt to accommodate
//  calls to logging functions that take variable length arguments and accept printf()-family
//  "format strings".  These cases make it difficult to determine the ultimate string length
//  at runtime.
#define MAX_LOG_SIZE 32768


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
