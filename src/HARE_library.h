/*
 *	Moving HARE function declarations into a library for the purposes of testing and dynamic linking.
 */

#ifndef __HARE_LIBRARY__
#define __HARE_LIBRARY__

#include <stdio.h>         // NULL
#include <sys/types.h>     // off_t


/*
 *  Verify, read, and print contents of filename
 */
int do_it(char *filename);


/*
 *  Return the filename argument
 */
char *get_filename(int argc, char *argv[]);


/*
 *  Append a newline-terminated entry to "log_filename"
 */
void log_it(char *log_entry, char *log_filename);


/*
 *  Validate arguments.  Return argument 1 (as appropriate).
 */
char *parse_args(int argc, char *argv[]);


/*
 *  Print usage instructions for this binary; Define the BINARY_NAME macro or we'll define it for you!
 */
void print_usage(void);


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
