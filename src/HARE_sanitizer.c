/*
 *  Defines HARE_sanitizer functionality.
 */

#include "HARE_library.h"    // syslog_*()
#include "HARE_sanitizer.h"


/*
 *  Populates san_logs->asan_log with Memwatch log filename, if found.  Does not validate input.
 */
void get_asan_log(SanitizerLogs *san_logs);


/*
 *  Populates san_logs->memwatch_log with Memwatch log filename, if found.  Does not validate input.
 */
void get_memwatch_log(SanitizerLogs *san_logs);


char *get_env_var(char *var_name)
{
    // LOCAL VARIABLES
    char *var_value = NULL;  // Environment variable value for var_name

    // INPUT VALIDATION
    if (var_name && *var_name)
    {
        // DO IT
        var_value = getenv(var_name);
        if (var_value)
        {
            syslog_it2(LOG_DEBUG, "Found %s: %s", var_name, var_value);  // DEBUGGING
        }
        else
        {
            syslog_it2(LOG_DEBUG, "Unable to find environment variable %s", var_name);  // DEBUGGING
        }
    }

    // DONE
    return var_value;
}


int fill_sanitizer_logs(SanitizerLogs *san_logs)
{
    // LOCAL VARIABLES
    int success = -1;     // 0 success, -1 bad input, -2 no envs, errno on error

    // INPUT VALIDATION
    if (san_logs)
    {
        if (NULL == san_logs->asan_log && NULL == san_logs->memwatch_log)
        {
            success = 0;
        }
    }

    // READ SANITIZER LOG FILES
    // ASAN
    if (0 == success)
    {
        // Read ASAN
        get_asan_log(san_logs);
    }
    // Memwatch
    if (0 == success)
    {
        // Read Memwatch
        get_memwatch_log(san_logs);
    }

    // DETERMINE SUCCESS
    if (0 == success && NULL == san_logs->asan_log && NULL == san_logs->memwatch_log)
    {
        success = -2;
    }

    // DONE
    return success;
}


/*************************************************************************************************/
/**************************************** LOCAL FUNCTIONS ****************************************/
/*************************************************************************************************/


void get_asan_log(SanitizerLogs *san_logs)
{
    // LOCAL VARIABLES
    char asan_env_var[] = "ASAN_OPTIONS"  // Address Sanitizer (ASAN) env variable
    char log_path_val[] = "log_path="     // Needle to search for within env var value
    char *temp_value = NULL;              // ASAN env variable value
    char *log_file = NULL;                // Address to log_path
    size_t filename_len = 0;              // Length of the log_path_val value

    // DO IT
    temp_value = get_env_var(asan_env_var);
    if (temp_value)
    {
        log_file = strstr(temp_value, log_path_val);
        if (log_file)
        {
            log_file = log_file + strlen(log_path_val);  // Fast forward past the needle
            while (0x0 != *log_file && ' ' != *log_file)
            {
                filename_len++;
                log_file++;
            }
            if (*log_file && filename_len > 0)
            {
                san_logs->asan_log = calloc(filename_len + 1, sizeof(char));
                if (san_logs->asan_log)
                {
                    if (san_logs->asan_log != strncpy(san_logs->asan_log, log_file, filename_len))
                    {
                        syslog_it2(LOG_ERR, "Failed to copy log_file into %p", san_logs->asan_log);
                        free(san_logs->asan_log);
                        san_logs->asan_log = NULL;
                    }
                    else
                    {
                        syslog_it2(LOG_DEBUG, "Successfuly extricated the ASAN log as %s", san_logs->asan_log);
                    }
                }
                else
                {
                    syslog_errno(errno, "Call to calloc() failed");
                }
            }
        }
    }

    // DONE
    return;
}


void get_memwatch_log(SanitizerLogs *san_logs)
{
    // LOCAL VARIABLES
    char memwatch_env_var[] = "";  // Memwatch environment variable
    char *temp_value = NULL;       // Memwatch env variable value

    // DO IT
    temp_value = get_env_var(memwatch_env_var);
    if (temp_value)
    {
        syslog_it2(LOG_DEBUG, "TO DO: DON'T DO NOW... Parse %s for the Mewatch log", temp_value);  // DEBUGGING
    }
    else
    {
        syslog_it(LOG_DEBUG, "TO DO: DON'T DO NOW... Configure get_memwatch_log() with the Memwatch environment variable");  // DEBUGING
    }

    // DONE
    return;
}

// ASAN NOTE: Then parse *that* for "log_path"
