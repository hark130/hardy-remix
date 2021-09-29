/*
 *  Moving HARE function declarations into a library for the purposes of testing and dynamic linking.
 */

#ifndef __HARE_LIBRARY__
#define __HARE_LIBRARY__

#include <stdbool.h>   // bool
#include <stdio.h>      // NULL
#include <sys/types.h>  // off_t
#include <syslog.h>     // syslog(), LOG_* macros

#ifndef ENOERR
#define ENOERR 0
#endif  // ENOERR

#define FILE_MAX 255  // Maximum length for a Linux filename

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
extern int pipe_fds[2];           // Pipe used to send data from the test harness to the daemon as if it was inotify
extern char *base_filename;       // Name of the file-based test case created by the test harness
extern size_t base_filename_len;  // Length of the base_filename
extern char *processed_filename;  // Absolute filename of a file that matches on base_filename


/*
 * Mirrors SURE's main() in that it acts as Linux-style daemon loader by fork()ing and exiting, thus releasing control
 * Returns PID if parent, 0 if child, -1 on failure
 */
pid_t be_sure(Configuration *config);


/*
 *  Delete filename
 *  Returns 0 on success, -1 on error, and errnum on failure
 */
int delete_file(char *filename);


/*
 *  Search dirname for a file that matches filename and delete it
 *  Returns 0 on success, -1 on error, -2 if no match found, and errnum on failure
 */
int delete_matching_file(char *dirname, char *filename, size_t filename_len);


/*
 *  Verify, read, and print contents of filename
 */
int do_it(char *filename);


/*
 * Loosely based on SURE's execute() in that it executes the main process loop
 */
void execute_order(Configuration *config);


/*
 *  Return a YYYYMMDD_HHMMSS_ string in a heap-allocated buffer
 */
char *get_datetime_stamp(int *errnum);


/*
 *  Return the filename argument
 */
char *get_filename(int argc, char *argv[]);


/*
 * Loosely based on SURE's getINotifyData(), represents the test harnesses replacement
 *      as a injection point for the test case data.
 * Returns 0 on success, -1 on error, and errnum on failure
 * Notes
 *      Returns 0 even if there's no data to read
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
 *  Move a file from source to destination
 *  Returns 0 on success, -1 on bad input, errno on failure
 */
int move_file(char *source, char *destination);


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
 *  Recursively searches haystack_dir for a filename whose ending matches needle_file
 *  Returns absolute filename on success, NULL on failure or "no match"
 */
char *search_dir(char *haystack_dir, char *needle_file, size_t needle_file_len);


/*
 *  Get the size of a file: size on success, -1 on error
 */
off_t size_file(char *filename);


/*
 *  Move filename to dest and prepend the filename with a datetime stamp
 *  Arguments
 *      source_file - Filename to move
 *      dest_dir - Directory to move filename to
 *  Returns 0 on success, -1 on bad input, errno on failure
 */
int stamp_a_file(char *source_file, char *dest_dir);


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
 *  Verify directory exists: 1 exists, 0 missing, -1 error
 */
int verify_directory(char *directory);


/*
 *  Verify filename exists as a regular file: 1 exists, 0 missing, -1 error
 */
int verify_filename(char *filename);


/*
 *  Verify pathname exists: 1 exists, 0 missing, -1 error
 */
int verify_pathname(char *pathname);


/*
 *  Write "num_bytes" worth of "write_buff" into the "write_fd" file descriptor
 *  Arguments
 *      write_fd - The pipe's write file descriptor
 *      write_buff - Buffer to write to writeFD
 *      num_bytes - Amount of data to copy from writeStr into writeFD
 *  Returns
 *      On success, 0
 *      On failure, errno
 *  Notes
 *      This function will not close the file descriptor
 */
int write_a_pipe(int write_fd, void *write_buff, size_t num_bytes);


#endif  // __HARE_LIBRARY__
