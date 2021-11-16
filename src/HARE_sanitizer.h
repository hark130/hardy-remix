/*
 *  Functionality to handle external sanitizer logs on behalf of HARE.
 *  fill_sanitizer_logs() in the entry point.  All other functions are merely useful.
 */

#ifndef __HARE_SANITIZER__
#define __HARE_SANITIZER__

// Holds the sanitizer log filenames
typedef struct _SanitizerLogs
{
    char *asan_log;      // Adderess Sanitizer (ASAN) log file
    char *memwatch_log;  // Memwatch log file
} SanitizerLogs;


/*
 *  Searches the environment list to find the variable var_name.
 *  Do *NOT* free the return value.
 */
char *get_env_var(char *var_name);


/*
 *  Parses environment variables for external sanitizer environment variables.  Populates
 *  san_logs with what it finds.  Function will error if the struct contains anything.  Caller is
 *  responsible for freeing the heap-allocated pointers in the struct's members.
 *      Address Sanitizer (ASAN) = "ASAN_OPTIONS"
 *      Memwatch = 
 *  Returns 0 on success, -1 on bad input, -2 if no envs were found.
 */
int fill_sanitizer_logs(SanitizerLogs *san_logs);


#endif  // __HARE_SANITIZER__
