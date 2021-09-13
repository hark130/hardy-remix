/*
 *	Moving HARE function declarations into a library for the purposes of testing and dynamic linking.
 */

#ifndef __HARE_LIBRARY__
#define __HARE_LIBRARY__

#include <stdio.h>      // NULL
#include <sys/types.h>  // off_t
#include <syslog.h>     // syslog(), LOG_* macros

/*
 * Stolen from https://opensource.apple.com/source/xnu/xnu-344/bsd/sys/syslog.h.auto.html
 */
typedef struct _Code {
    char *name;
    int  value;
} CODE;

typedef struct _Message
{
    char *buffer;  // Message Contents
    size_t size;   // Message Size
} Message;

typedef struct _INotifyMessage
{
    Message message;    // Returned message contents from getWatcherData
    // void *privateData;  // Private internal data
} INotifyMessage;

// Settings for INotify folder watcher thread
typedef struct _INotifySettings
{
    char *watched;       // Dir(s) to watch
    char *process;     // Directory (rel to watch) to move processed files into
} INotifySettings;

// Holds the configuration data
typedef struct _Configuration
{
    INotifySettings inotify_config;  // INotify folder watcher settings
    INotifyMessage inotify_message;  // INotify message
} Configuration;

// MACROs to help properly access int array indices
#define PIPE_READ 0
#define PIPE_WRITE 1
#define INVALID_FD -1
extern int pipe_fds[2];  // Pipe used to send data from the test harness to the daemon as if it was inotify


/*
 * Mirrors SURE's main() in that it acts as Linux-style daemon loader by fork()ing and exiting, thus releasing control
 */
void be_sure(Configuration *config);


/*
 *  Verify, read, and print contents of filename
 */
int do_it(char *filename);


/*
 * Loosely based on SURE's execute() in that it executes the main process loop
 */
void execute_order(Configuration *config);


/*
 *  Return the filename argument
 */
char *get_filename(int argc, char *argv[]);


/*
 * Loosely based on SURE's getINotifyData(), represents the test harnesses replacement
 *      as a injection point for the test case data.
 * Returns 0 on success, -1 on error, and errnum on failure
 */
int getINotifyData(Configuration *config);


/*
 *  Append a newline-terminated entry to "log_filename"
 */
void log_it(char *log_entry, char *log_filename);


/*
 *  Make plumbing easy
 *  Arguments
 *      empty_pipes - Integer array of dimension 2 to store the resulting file descriptors
 *      flags - Flags to pass to pipe2() if appropriate
 *  Returns 0 on success, -1 on bad input, errno on failure;
 *  Notes
 *      Based on MACROS and the value of flags, make_pipes() will call pipe2() instead
 *          of pipe() in the right circumstances
 *      This function will overwrite empty_pipes values with INVALID_FD before calling pipe()/pipe2().
 */
int make_pipes(int empty_pipes[2], int flags);


/*
 *  Validate arguments.  Return argument 1 (as appropriate).
 */
char *parse_args(int argc, char *argv[]);


/*
 *  Print usage instructions for this binary; Define the BINARY_NAME macro or we'll define it for you!
 */
void print_usage(void);


/*
 *  Reads characters from read_fd one byte at a time into a heap-allocated buffer
 *  Arguments
 *      read_fd - File descriptor to read from
 *      msg_len - Out parameter to store the number of bytes read into the return value
 *      errnum - Out parameter to store errno in the event of an error
 */
char *read_a_pipe(int read_fd, int *msg_len, int *errnum);


/*
 *  Read filename into a custom-sized, heap-allocated buffer
 */
char *read_file(char *filename);


/*
 *  Get the size of a file: size on success, -1 on error
 */
off_t size_file(char *filename);


/*
 *  Minimally mirrors logIt() from SURE_logging.h
 *  logLevels:
 *      LOG_EMERG      system is unusable
 *      LOG_ALERT      action must be taken immediately
 *      LOG_CRIT       critical conditions
 *      LOG_ERR        error conditions
 *      LOG_WARNING    warning conditions
 *      LOG_NOTICE     normal, but significant, condition
 *      LOG_INFO       informational message
 *      LOG_DEBUG      debug-level message
 */
void syslog_it(int logLevel, char *msg);


/*
 *  Minimally mirrors logIt2() from SURE_logging.h
 *  logLevels:
 *      LOG_EMERG      system is unusable
 *      LOG_ALERT      action must be taken immediately
 *      LOG_CRIT       critical conditions
 *      LOG_ERR        error conditions
 *      LOG_WARNING    warning conditions
 *      LOG_NOTICE     normal, but significant, condition
 *      LOG_INFO       informational message
 *      LOG_DEBUG      debug-level message
 */
void syslog_it2(int logLevel, char *msg, ...);


/*
 *  Minimally mirrors logErrno() from SURE_logging.h
 */
void syslog_errno(int errNum, char *msg, ...);


/*
 *  Prints usage instructions on option match or returns argOne
 */
char *validate_arg(char *argOne);


/*
 *  Verify filename exists: 1 exists, 0 missing, -1 error
 */
int verify_filename(char *filename);


#endif  // __HARE_LIBRARY__
