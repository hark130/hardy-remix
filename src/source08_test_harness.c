/*
 *  Defines the test harness for "source08" functionality.
 *  BASIC USAGE:
 *      1. Manual
 *          - make source08
 *          - echo -n "some_file.txt" > source08_test_input.txt
 *          - sudo ./dist/source08_test_harness_<choose one>.bin source08_test_input.txt
 *      2. Script
 *          - sudo ./devops/scripts/filename_test.sh ./dist/source08_test_harness_<choose one>.bin
 *      3. Fuzzer
 *          A. AFL++
 *              - see: README.md
 *          B. Honggfuzz
 *              - TO DO: DON'T DO NOW...
 *          C. Radamsa
 *              - make source08
 *              - echo -n "some_file.txt" | radamsa > source08_test_input.txt
 *              - sudo ./dist/source08_test_harness_<choose one>.bin source08_test_input.txt
 */

#include <errno.h>           // errno
#include <fcntl.h>           // open()
// #include <signal.h>        // raise(), signal(), sa_handler
// #include <stdio.h>         // fprintf(), remove(), snprintf()
#include <stdint.h>          // SIZE_MAX
#include <stdlib.h>          // calloc(), free()
#include <string.h>          // strerror()
#include <sys/stat.h>        // stat(), S_xxxx
#include <unistd.h>          // close(), write()
#include "HARE_library.h"    // be_sure()
#include "HARE_sanitizer.h"  // fill_sanitizer_logs(), SanitizerLogs


/*
 *  Check to see if dirname exists: Returns 1 if exists, 0 if not, -1 on error
 */
int check_dir(char *path);


/*
 *  Add filename to a radamsa command to file filename with fuzzed contents
 */
// char *create_system_cmd(char *filename);


/*
 *  Uses radasma to fuzz original, read the output, store it in heap-allocated memory
 */
char *get_fuzzed_contents(char *original, size_t *buff_size);


/*
 *  Append a log message to LOG_FILENAME
 */
void log_external(char *log_entry);


/*
 *  Prepend the contents of filename with the string found in prepend
 *  If prepend if NULL or empty, returns unaltered contents of filename
 *  Contents of total_size is zeroized.  Upon success, total_size contains
 *      the full length of the data contained in the return value.
 */
char *prepend_test_input(char *filename, char *prepend, size_t *total_size);


/*
 *  Use popen() to execute command, read the results, and return the heap-allocated buffer
 */
char *read_from_process(char *command, size_t *buff_size);


/*
 *  Read test input from filename into a custom-sized, heap-allocated buffer
 */
char *read_test_file(char *filename, off_t *buff_size);


/*
 *  Allocate a buffer of size curr_size + add_space + 1 and read old_buff into it
 */
char *reallocate(char *old_buff, size_t curr_size, size_t add_space);


/*
 *  Get the size of a test file: size on success, -1 on error
 */
off_t size_test_file(char *filename);


/*
 *  Test harness.
 *  1. Read and prepend test case filename
 *  2. Setup environment (e.g., watch dir, process dir)
 *  3. Prepare to "hook" inotify
 *  4. Attempt to create the test case file (with fuzzed contents)
 *  5. Tell the "daemon" about the test case
 *  6. Start the "daemon"
 *  7. Test results(?)
 *  8. Delete the test case file
 */
int main(int argc, char *argv[])
{
    // LOCAL VARIABLES
    int success = 0;                 // Return value
    char *filename = argv[1];        // CLI argument: filename
    char *test_filename = NULL;      // Contents of filename: use as test input
    size_t test_filename_len = 0;    // Total readable length of test_filename
    char *test_content = NULL;       // Fuzzed test file contents
    size_t content_size = 0;         // Readable size of test_content
    int fd = 0;                      // Filename's file descriptor
    int file_exists = 0;             // Makeshift boolean to track file creation
    int errnum = 0;                  // Store errno values
    Configuration config = { 0 };    // Pass to be_sure()
    SanitizerLogs san_logs = { 0 };  // Saniziter log file names
    mode_t old_umask = 0;            // Store umask() value here and restore it
    pid_t daemon = 0;                // PID if parent, 0 if child, -1 on failure
    int process_san_logs = 0;        // 0 for no sanitizer logs, otherwise 1

    // DO IT
    // 1. Read file containing test input
    if (1 == check_dir("/ramdisk"))
    {
        test_filename = prepend_test_input(filename, "/ramdisk/watch/", &test_filename_len);
        config.inotify_config.watched = "/ramdisk/watch/";
        config.inotify_config.process = "/ramdisk/watch/processed/";
    }
    else if (1 == check_dir("/tmp"))
    {
        test_filename = prepend_test_input(filename, "/tmp/watch/", &test_filename_len);
        config.inotify_config.watched = "/tmp/watch/";
        config.inotify_config.process = "/tmp/watch/processed/";
    }
    else
    {
        test_filename = prepend_test_input(filename, "./watch/", &test_filename_len);
        config.inotify_config.watched = "./watch/";
        config.inotify_config.process = "./watch/processed/";
    }
    if (!test_filename)
    {
        syslog_it(LOG_ERR, "Call to prepend_test_input() failed with an unspecified error");
        success = -1;
    }

    // Read sanitizer log files
    if (0 == success)
    {
        success = fill_sanitizer_logs(&san_logs);
        if (0 == success)
        {
            if (san_logs.asan_log)
            {
                syslog_it2(LOG_DEBUG, "(TEST HARNESS) ASAN LOG: %s", san_logs.asan_log);  // DEBUGGING
                process_san_logs = 1;  // We'll need to process these logs later
            }
            if (san_logs.memwatch_log)
            {
                syslog_it2(LOG_DEBUG, "(TEST HARNESS) MEMWATCH LOG: %s", san_logs.memwatch_log);  // DEBUGGING
                process_san_logs = 1;  // We'll need to process these logs later
            }
        }
        else if (-1 == success)
        {
            syslog_it2(LOG_ERR, "(TEST HARNESS) fill_sanitizer_logs() reported an unspecified error");
        }
        else if (-2 == success)
        {
            syslog_it2(LOG_NOTICE, "(TEST HARNESS) No sanitizer logs found among the environment variables");
            success = 0;  // This is not an error so we'll continue
        }
        else
        {
            syslog_it2(LOG_DEBUG, "(TEST HARNESS) fill_sanitizer_logs() returned an unspecified value: %d", success);  // DEBUGGING
        }
    }

    // 2. Setup environment
    // Watch dir
    if (0 == success)
    {
        old_umask = umask(0);
        if (0 == check_dir(config.inotify_config.watched))
        {
            if (0 != mkdir(config.inotify_config.watched, S_IRWXU | S_IRWXG | S_IRWXO))
            {
                errnum = errno;
                syslog_errno(errnum, "(TEST HARNESS) Failed to create watch directory");
                if (errnum)
                {
                    success = errnum;
                }
                else
                {
                    success = -1;
                }
            }
        }
    }
    // Processed dir
    if (0 == success)
    {
        if (0 == check_dir(config.inotify_config.process))
        {
            if (0 != mkdir(config.inotify_config.process, S_IRWXU | S_IRWXG | S_IRWXO))
            {
                errnum = errno;
                syslog_errno(errnum, "(TEST HARNESS) Failed to create process directory");
                if (errnum)
                {
                    success = errnum;
                }
                else
                {
                    success = -1;
                }
            }
        }
    }
    // Sanitizer logs
    if (0 == success && 1 == process_san_logs)
    {
        success = prepare_log_dirs(&san_logs);
    }

    // 3. Prepare the "hook"
    if (0 == success)
    {
        success = make_pipes(pipe_fds, O_NONBLOCK);

        if (-1 == success)
        {
            syslog_it(LOG_ERR, "(TEST HARNESS) Call to make_pipes() failed with bad input");
        }
        else if (0 != success)
        {
            syslog_errno(success, "(TEST HARNESS) Failed to make the pipes");
        }
    }

    // 4. Attempt file creation
    // syslog_it2(LOG_DEBUG, "Current status is... success: %d, test_filename: %s, daemon: %d", success, test_filename, daemon);  // DEBUGGING
    if (0 == success)
    {
        // Create
        // log_external("(PARENT) About to create the file");  // DEBUGGING
        // fprintf(stderr, "TEST FILENAME: %s\n", test_filename);  // DEBUGGING
        fd = open(test_filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
        if (fd > -1)
        {
            file_exists = 1;
            // log_external("File exists");  // DEBUGGING
            // syslog_it2(LOG_DEBUG, "File %s exists on file descriptor %d", test_filename, fd);  // DEBUGGING

            // Get fuzzed content
            test_content = get_fuzzed_contents("This is my file.\nThere are many like it but this one is mine.\n", &content_size);
            // test_content = get_fuzzed_contents("This is my file.\n", &content_size);
            // test_content = get_fuzzed_contents("file", &content_size);

            if (test_content && content_size > 0)
            {
                // syslog_it2(LOG_DEBUG, "Current status is... fd: %d, test_content: (%p) %s, content size: %zu", fd, test_content, test_content, content_size);  // DEBUGGING
                // Write
                if (-1 == write(fd, test_content, content_size))
                {
                    errnum = errno;
                    syslog_errno(errnum, "(TEST HARNESS) Unable to write to %s", test_filename);
                    // log_external("(PARENT) Unable to write file");  // DEBUGGING
                    // syslog_it2(LOG_DEBUG, "Current status is... fd: %d, test_content: (%p) %s, content size: %zu", fd, test_content, test_content, content_size);  // DEBUGGING
                    if (errnum)
                    {
                        success = errnum;
                    }
                    else
                    {
                        success = -1;
                    }
                }
                else
                {
                    // syslog_it2(LOG_DEBUG, "(TEST HARNESS) Successfully created and wrote to file %s", test_filename);  // DEBUGGING
                }
                if (test_content)
                {
                    // Clear it here since we moved the call to fork() *after* test case prep
                    free(test_content);
                    test_content = NULL;
                }
            }
            else
            {
                // log_external("Failed to fuzz content.");  // DEBUGGING
                // log_external("(PARENT) Failed to fuzz content");  // DEBUGGING
                syslog_it(LOG_ERR, "(TEST HARNESS) Failed to fuzz content.");
            }

            // Close
            if (fsync(fd))
            {
                errnum = errno;
                syslog_errno(errnum, "(TEST HARNESS) Unable to synchronize %s", test_filename);
                // log_external("(PARENT) Unable to sychronize");  // DEBUGGING
                if (errnum)
                {
                    success = errnum;
                }
                else
                {
                    success = -1;
                }
            }
            else
            {
                // syslog_it2(LOG_DEBUG, "(TEST HARNESS) Synched file descriptor: %d", fd);  // DEBUGGING
            }
            if (close(fd))
            {
                errnum = errno;
                syslog_errno(errnum, "(TEST HARNESS) Unable to close %s", test_filename);
                // log_external("(PARENT) Unable to close");  // DEBUGGING
                if (errnum)
                {
                    success = errnum;
                }
                else
                {
                    success = -1;
                }
            }
            else
            {
                // syslog_it2(LOG_DEBUG, "(TEST HARNESS) Closed file descriptor: %d", fd);  // DEBUGGING
            }
            fd = -1;
        }
        else
        {
            errnum = errno;
            syslog_errno(errnum, "(TEST HARNESS) Unable to make file %s", test_filename);
            // log_external(test_filename);  // DEBUGGING
            // log_external("Failed to create file");  // DEBUGGING
            if (errnum)
            {
                success = errnum;
            }
            else
            {
                success = -1;
            }
        }
    }
    // Handle odd edge cases (e.g., BAD_ADDRESS)
    if (0 != success && 1 == file_exists)
    {
        // syslog_it(LOG_DEBUG, "Here we are, handling some weird edge case...");  // DEBUGGING
        if (1 == verify_filename(test_filename))
        {
            if (-1 == remove(test_filename))
            {
                errnum = errno;
                // fprintf(stderr, "Unable to delete %s.\nERROR: %s\n", test_filename, strerror(errnum));
                // log_external("Failed to delete file");  // DEBUGGING
                syslog_errno(errnum, "(TEST HARNESS) Test file was created, there was an error, but now unable to delete %s", test_filename);
            }
            else
            {
                syslog_it2(LOG_NOTICE, "(TEST HARNESS) An error occurred when writing to %s.  It has been deleted.", test_filename);
            }
        }
        else
        {
            syslog_it(LOG_ERR, "(TEST HARNESS) Test file was created, there was an error, but now we can't find it");
        }
    }

    // 5. Tell the daemon
    if (0 == success)
    {
        errnum = write_a_pipe(pipe_fds[PIPE_WRITE], test_filename, test_filename_len);

        if (errnum)
        {
            // fprintf(stderr, "Unable to write to pipe.\nERROR: %s\n", strerror(errnum));
            // log_external("Failed to write to pipe");  // DEBUGGING
            syslog_errno(errnum, "(TEST HARNESS) Unable to write to pipe.");
            success = errnum;
        }
        else
        {
            // log_external("The call to write_a_pipe() succeeded");  // DEBUGGING
        }
    }

    // 6. Start the "daemon"
    if (0 == success)
    {
        // log_external("Successfully created the pipes");  // DEBUGGING
        // syslog_it2(LOG_DEBUG, "pipe_fds[PIPE_READ] == %d and pipe_fds[PIPE_WRITE] == %d", pipe_fds[PIPE_READ], pipe_fds[PIPE_WRITE]);  // DEBUGGING
        daemon = be_sure(&config);
        // log_external("The call to be_sure() returned");  // DEBUGGING
        // syslog_it2(LOG_DEBUG, "The call to be_sure() returned %d", daemon);  // DEBUGGING
        if (-1 == daemon)
        {
            syslog_it(LOG_ERR, "(TEST HARNESS) The call to be_sure() failed");
            success = -1;
        }
    }

    // 7. Test results
    if (0 == success && 0 < daemon)
    {
        // WAIT FOR THE DAEMON TO EXIT
        errnum = wait_daemon(daemon, &success);
        if (-1 == errnum)
        {
            syslog_it2(LOG_ERR, "(TEST HARNESS) Call to wait_daemon(%ld) failed with an unspecified error", daemon);
        }
        else if (0 < errnum)
        {
            syslog_errno(errnum, "(TEST HARNESS) Call to wait_daemon(%ld) failed", daemon);
        }
        else
        {
            syslog_it2(LOG_INFO, "(TEST HARNESS) Call to wait_daemon(%ld) succeeded.  Daemon exited with %d.", daemon, success);
        }
        // Was it processed?
        // Any errors detected among the syslog entries
        // Did the "daemon" crash
        // Did this filename show up in the fuzzer's "output"?  (If so, maybe save it?)
    }

    // 8. Delete files
    if (0 < daemon && 1 == file_exists)
    {
        if (1 == verify_filename(test_filename))
        {
            if (-1 == remove(test_filename))
            {
                errnum = errno;
                syslog_errno(errnum, "(TEST HARNESS) Unable to delete %s", test_filename);
            }
        }
        else
        {
            errnum = delete_matching_file(config.inotify_config.process, base_filename, base_filename_len);
            // Returns 0 on success, -1 on error, -2 if no match found, and errnum on failure
            if (-2 == errnum)
            {
                syslog_it2(LOG_ERR, "(TEST HARNESS) No match for %s found in %s to cleanup", base_filename, config.inotify_config.watched);
            }
            else if (-1 == errnum)
            {
                syslog_it2(LOG_ERR, "(TEST HARNESS) delete_matching_file(%s, %s, %zu) encountered an unspecified error", config.inotify_config.watched, base_filename, base_filename_len);
            }
            else if (0 == errnum)
            {
                syslog_it2(LOG_INFO, "(TEST HARNESS) Successfully deleted a file matching %s from within %s", base_filename, config.inotify_config.watched);
            }
            else
            {
                syslog_errno(errnum, "(TEST HARNESS) delete_matching_file(%s, %s, %zu) encountered an error", config.inotify_config.watched, base_filename, base_filename_len);
            }
        }
    }

    // CLEANUP
    // Parent Cleanup
    if (0 < daemon)
    {
        // Restore umask
        old_umask = umask(old_umask);
        // Close the pipes
        if (INVALID_FD != pipe_fds[PIPE_READ])
        {
            close(pipe_fds[PIPE_READ]);
            pipe_fds[PIPE_READ] = INVALID_FD;
        }
        if (INVALID_FD != pipe_fds[PIPE_WRITE])
        {
            close(pipe_fds[PIPE_WRITE]);
            pipe_fds[PIPE_WRITE] = INVALID_FD;
        }
        // processed_filename
        if (processed_filename)
        {
            free(processed_filename);
            processed_filename = NULL;
        }
    }
    // EVERYBODY
    if (san_logs.asan_log)
    {
        free(san_logs.asan_log);
        san_logs.asan_log = NULL;
    }
    if (san_logs.memwatch_log)
    {
        free(san_logs.memwatch_log);
        san_logs.memwatch_log = NULL;
    }
    // PRO TIP: Since test_filename gets allocated *before* the call to fork(), the child process
    //  gets a *copy* of the heap memory... not *access* to the parent processes memory.
    //  That means that both the parent and the child process need to free this address.
    //  Don't like it?  Use the exec family of calls instead.
    // test_filename
    if (test_filename)
    {
        free(test_filename);
        test_filename = NULL;
    }

    // DONE
    if (0 != daemon)  // Parent or fork() failed
    {
        syslog_it(LOG_NOTICE, "(TEST HARNESS) Exiting");
    }

    return success;
}


int check_dir(char *path)
{
    // LOCAL VARIABLES
    struct stat sb;  // Out paramter for stat()
    int retval = 0;  // 1 if it exists, 0 if not, -1 on error

    // INPUT VALIDATION
    if (path && *path)
    {
        // DO IT
        if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode))
        {
            retval = 1;  // Directory exists
        }
    }
    else
    {
        retval = -1;  // Error
    }

    // DONE
    return retval;
}


char *get_fuzzed_contents(char *original, size_t *buff_size)
{
    // LOCAL VARIABLES
    char *fuzz_buff = NULL;  // Current storage buffer
    char *cmnd_buff = NULL;  // Heap memory with the popen command string
    size_t fuzz_size = 0;    // Size of the readable fuzz_buff
    size_t cmnd_size = 0;    // Size of the readable cmnd_buff, including nul character
    int error = 1;           // Controls flow as a boolean
    int errnum = 0;          // Store errno values
    char template[] = { "echo \"%s\" | radamsa" };
    // char template[] = { "echo \"%s\"" };

    // INPUT VALIDATION
    if (original && *original && buff_size)
    {
        *buff_size = 0;
        error = 0;
        // fprintf(stderr, "ORIGINAL: %s\n", original);  // DEBUGGING
    }

    // BUILD COMMAND
    // Allocate
    if (!error)
    {
        cmnd_size = sizeof(template) + strlen(original) + 1;
        cmnd_buff = (char *)calloc(cmnd_size, sizeof(char));
        if (!cmnd_buff)
        {
            error++;
            errnum = errno;
            fprintf(stderr, "Unable to allocate memory.\nERROR: %s\n", strerror(errnum));
        }
    }
    // Build command
    if (!error)
    {
        if (snprintf(cmnd_buff, cmnd_size, template, original) < 0)
        {
            error++;
            errnum = errno;
            fprintf(stderr, "Call to snprintf() failed for %s and %s.\nERROR: %s\n", template, original, strerror(errnum));
        }
    }

    // GET OUTPUT
    if (!error)
    {
        fuzz_buff = read_from_process(cmnd_buff, &fuzz_size);
        if (!fuzz_buff || fuzz_size <= 0)
        {
            error++;
            errnum = errno;
            fprintf(stderr, "Failed to read from process command: %s.\nERROR: %s\n", cmnd_buff, strerror(errnum));
        }
        else
        {
            *buff_size = fuzz_size;
        }
    }

    // CLEANUP
    // cmnd_buff
    if (cmnd_buff)
    {
        free(cmnd_buff);
        cmnd_buff = NULL;
    }

    // DONE
    // Errors
    if (error)
    {
        if (fuzz_buff)
        {
            free(fuzz_buff);
            fuzz_buff = NULL;
        }
    }
    return fuzz_buff;
}


void log_external(char *log_entry)
{
    syslog_it(LOG_DEBUG, log_entry);
}


char *prepend_test_input(char *filename, char *prepend, size_t *total_size)
{
    // LOCAL VARIABLES
    off_t buff_size = 0;           // Readable size of buffer allocated by read_test_file()
    int error = 0;                 // Manual boolean to track errors
    size_t prepend_len = 0;        // Length of prepend
    char *old_test_input = NULL;   // Test input read from filename
    char *new_test_input = NULL;   // Allocate mem and return
    char *new_test_offset = NULL;  // Starting address of the old_test_input inside the new_test_input

    // INPUT VALIDATION
    if (filename && *filename && prepend && total_size)
    {
        *total_size = 0;  // Initialize out parameter
        old_test_input = read_test_file(filename, &buff_size);
        // fprintf(stderr, "OLD TEST INPUT: %s\n", old_test_input);  // DEBUGGING
        if (old_test_input && *old_test_input && prepend && *prepend)
        {
            base_filename = old_test_input;
            base_filename_len = buff_size;
            prepend_len = strlen(prepend);
            new_test_input = calloc(prepend_len + buff_size + 1, sizeof(char));
            if (new_test_input)
            {
                new_test_offset = new_test_input + prepend_len;
                if (new_test_input != strncpy(new_test_input, prepend, prepend_len))
                {
                    // fprintf(stderr, "ERROR: strncpy failed with %s\n", strerror(errno));  // DEBUGGING
                    error++;
                }
                else if (new_test_offset != memcpy(new_test_offset, old_test_input, buff_size))
                {
                    // fprintf(stderr, "ERROR: memcpy failed with %s\n", strerror(errno));  // DEBUGGING
                    error++;
                }
                else
                {
                    *total_size = prepend_len + buff_size;
                }
            }
            else
            {
                // fprintf(stderr, "ERROR: calloc failed with %s\n", strerror(errno));  // DEBUGGING
                error++;
            }
            // free(old_test_input);  // Don't free it here.  free(base_filename) at exit-time.
            old_test_input = NULL;
        }
        else
        {
            new_test_input = old_test_input;
            old_test_input = NULL;
        }
    }

    // DONE
    if (error)
    {
        if (new_test_input == old_test_input)
        {
            old_test_input = NULL;
        }
        if (old_test_input)
        {
            free(old_test_input);
            old_test_input = NULL;
        }
        if (new_test_input)
        {
            free(new_test_input);
            new_test_input = NULL;
        }
        base_filename = NULL;
    }
    // fprintf(stderr, "NEW TEST INPUT: %s\n", new_test_input);  // DEBUGGING
    return new_test_input;
}


char *read_from_process(char *command, size_t *buff_size)
{
    // LOCAL VARIABLES
    FILE *stream_ptr = NULL; // Stream pointer to radasma call
    char *curr_buff = NULL;  // Current storage buffer
    char *new_buff = NULL;   // For when curr_buff isn't big enough
    size_t curr_size = 0;    // Store this in buff_size on success
    size_t base_size = 512;  // Increment memory using this as the base
    int error = 1;           // Controls flow as a boolean
    char letter = 0;         // One character read from the stream
    int errnum = 0;          // Store errno values

    // INPUT VALIDATION
    if (command && *command && buff_size)
    {
        *buff_size = 0;
        error = 0;
    }

    // DO IT
    // Allocate memory
    if (!error)
    {
        curr_buff = calloc(base_size + 1, sizeof(char));
        if (!curr_buff)
        {
            error++;
            errnum = errno;
            fprintf(stderr, "Failed to allocate memory.\nERROR: %s\n", strerror(errnum));
        }
    }

    // Start process
    if (!error)
    {
        stream_ptr = popen(command, "r");
        if (stream_ptr)
        {
            while (1)
            {
                // Read a character
                letter = fgetc(stream_ptr);
                // putchar(letter);  // DEBUGGING
                // Done?
                if (EOF == letter)
                {
                    // fprintf(stderr, "DONE READING\n");  // DEBUGGING
                    *buff_size = curr_size - 1;
                    break;
                }
                // Store the character
                (*(curr_buff + curr_size)) = letter;
                // fprintf(stderr, "CURRENT BUFF: %s\n", curr_buff);  // DEBUGGING
                // Increment safely
                if (SIZE_MAX - 1 > curr_size)
                {
                    curr_size++;
                }
                else
                {
                    error++;  // We're about to overflow curr_size
                    fprintf(stderr, "Overflow error.  Exiting loop.");
                    break;
                }
                // Check length
                if (curr_size >= base_size)
                {
                    // fprintf(stderr, "ALLOCATING");  // DEBUGGING
                    new_buff = reallocate(curr_buff, curr_size, base_size);
                    if (new_buff)
                    {
                        free(curr_buff);
                        curr_buff = new_buff;
                        new_buff = NULL;
                        base_size += base_size;
                    }
                }
            }
        }
        else
        {
            error++;
            errnum = errno;
            fprintf(stderr, "Failed to open a process with command: %s.\nERROR: %s\n", command, strerror(errnum));
        }
    }

    // CLEANUP
    if (stream_ptr)
    {
        pclose(stream_ptr);
        stream_ptr = NULL;
    }

    // DONE
    if (error)
    {
        if (curr_buff == new_buff)
        {
            new_buff = NULL;
        }
        if (curr_buff)
        {
            free(curr_buff);
            curr_buff = NULL;
        }
        if (new_buff)
        {
            free(new_buff);
            new_buff = NULL;
        }
    }
    return curr_buff;
}


char *read_test_file(char *filename, off_t *buff_size)
{
    // LOCAL VARIABLES
    char *file_contents = NULL;  // Allocate mem and return
    off_t file_size = 0;         // Size of filename in bytes
    int fd = 0;                  // File descriptor of filename
    size_t read_bytes = 0;       // Number of bytes read by read()

    // INPUT VALIDATION
    if (filename && *filename && buff_size)
    {
        // Zeroize out-parameter
        *buff_size = 0;
        // Size it
        file_size = size_test_file(filename);
        if (file_size > -1)
        {
            // Allocate it
            file_contents = calloc(file_size + 1, sizeof(char));
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
                    else if (buff_size)
                    {
                        *buff_size = file_size;
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


char *reallocate(char *old_buff, size_t curr_size, size_t add_space)
{
    // LOCAL VARIABLES
    char *new_buff = NULL;

    // INPUT VALIDATION
    if (old_buff && curr_size > 0 && add_space > 0)
    {
        new_buff = calloc(curr_size + add_space + 1, sizeof(char));
        if (new_buff)
        {
            if (new_buff != memcpy(new_buff, old_buff, curr_size))
            {
                free(new_buff);
                new_buff = NULL;
            }
        }
    }

    // DONE
    return new_buff;
}


off_t size_test_file(char *filename)
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
