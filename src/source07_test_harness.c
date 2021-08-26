#include <errno.h>         // errno
#include <fcntl.h>         // open()
#include <stdio.h>         // fprintf(), remove(), snprintf()
#include <stdint.h>        // SIZE_MAX
#include <stdlib.h>        // calloc()
#include <string.h>        // strerror()
#include <sys/stat.h>      // S_xxxx
#include <unistd.h>        // close()
#include "HARE_library.h"  // do_it()


#ifndef ENOERR
#define ENOERR 0
#endif  // ENOERR


/*
 *  Check to see if dirname exists: Returns 1 if exists, 0 if not, -1 on error
 */
int check_dir(char *path);


/*
 *  Add filename to a radamsa command to file filename with fuzzed contents
 */
char *create_system_cmd(char *filename);


/*
 *  Uses radasma to fuzz original, read the output, store it in heap-allocated memory
 */
char *get_fuzzed_contents(char *original, size_t *buff_size);


/*
 *  Prepend the contents of filename with the string found in prepend
 *  If prepend if NULL or empty, returns unaltered contents of filename
 */
char *prepend_test_input(char *filename, char *prepend);


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


int main(int argc, char *argv[])
{
    // LOCAL VARIABLES
    int success = 0;             // Return value
    char *filename = argv[1];    // CLI argument: filename
    char *test_filename = NULL;  // Contents of filename: use as test input
    char *test_content = NULL;   // Fuzzed test file contents
    size_t content_size = 0;     // Readable size of test_content
    int fd = 0;                  // Filename's file descriptor
    int file_exists = 0;         // Makeshift boolean to track file creation
    int errnum = 0;              // Store errno values

    // DO IT
    // 1. Read file containing test input
    // fprintf(stderr, "ARGV[1]: %s\n", filename);  // DEBUGGING
    if (1 == check_dir("/ramdisk"))
    {
        test_filename = prepend_test_input(filename, "/ramdisk/");
    }
    else if (1 == check_dir("/tmp"))
    {
        test_filename = prepend_test_input(filename, "/tmp/");
    }
    else
    {
        test_filename = prepend_test_input(filename, "./");
    }

    // 2. Attempt file creation
    if (test_filename)
    {
        // Create
        // fprintf(stderr, "TEST FILENAME: %s\n", test_filename);  // DEBUGGING
        fd = open(test_filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
        if (fd > -1)
        {
            file_exists = 1;

            // Get fuzzed content
            test_content = get_fuzzed_contents("This is my file.\nThere are many like it but this one is mine.\n", &content_size);

            if (test_content && content_size > 0)
            {
                // Write
                if (-1 == write(fd, test_content, content_size))
                {
                    errnum = errno;
                    fprintf(stderr, "Unable to write to %s.\nERROR: %s\n", test_filename, strerror(errnum));
                }
            }
            else
            {
                fprintf(stderr, "Failed to fuzz content.\n");
            }

            // Close
            if (fsync(fd))
            {
                errnum = errno;
                fprintf(stderr, "Unable to synchronize %s.\nERROR: %s\n", test_filename, strerror(errnum));
            }
            if (close(fd))
            {
                errnum = errno;
                fprintf(stderr, "Unable to close %s.\nERROR: %s\n", test_filename, strerror(errnum));
            }
            fd = -1;
        }
        else
        {
            errnum = errno;
            fprintf(stderr, "Unable to make file %s.\nERROR: %s\n", test_filename, strerror(errnum));
        }
    }
    // 3. Execute
    success = do_it(test_filename);  // Test
    // 4. Delete file
    if (1 == file_exists)
    {
        if (-1 == remove(test_filename))
        {
            errnum = errno;
            fprintf(stderr, "Unable to delete %s.\nERROR: %s\n", test_filename, strerror(errnum));
        }
    }

    // DONE
    // test_filename
    if (test_filename)
    {
        free(test_filename);
        test_filename = NULL;
    }
    // test_content
    if (test_content)
    {
        free(test_content);
        test_content = NULL;
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


char *create_system_cmd(char *filename)
{
    // LOCAL VARIABLES
    // Template string used to create the system command
    char template[] = { "echo \"This is my file.\nThere are many like it but this one is mine.\" | radamsa > %s" };
    // char template[] = { "echo \"This is my file.\n\" | radamsa > %s" };
    char *system_cmd = NULL;  // Full system command
    size_t buff_size = 0;     // Number of bytes to allocate
    int errnum = 0;           // Store errno values

    // INPUT VALIDATION
    if (filename && *filename)
    {
        buff_size = sizeof(template) + strlen(filename) + 1;
        system_cmd = (char *)calloc(buff_size, sizeof(char));
        if (system_cmd)
        {
            if (snprintf(system_cmd, buff_size, template, filename) < 0)
            {
                errnum = errno;
                fprintf(stderr, "Unable to form the system command.\nERROR: %s\n", strerror(errnum));
                free(system_cmd);
                system_cmd = NULL;
            }
        }
        else
        {
            errnum = errno;
            fprintf(stderr, "Unable to allocate memory.\nERROR: %s\n", strerror(errnum));
        }
    }

    // DONE
    return system_cmd;
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


char *prepend_test_input(char *filename, char *prepend)
{
    // LOCAL VARIABLES
    off_t buff_size = 0;           // Readable size of buffer allocated by read_test_file()
    int error = 0;                 // Manual boolean to track errors
    size_t prepend_len = 0;        // Length of prepend
    char *old_test_input = NULL;   // Test input read from filename
    char *new_test_input = NULL;   // Allocate mem and return
    char *new_test_offset = NULL;  // Starting address of the old_test_input inside the new_test_input

    // INPUT VALIDATION
    if (filename && *filename)
    {
        old_test_input = read_test_file(filename, &buff_size);
        // fprintf(stderr, "OLD TEST INPUT: %s\n", old_test_input);  // DEBUGGING
        if (old_test_input && *old_test_input && prepend && *prepend)
        {
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
            }
            else
            {
                // fprintf(stderr, "ERROR: calloc failed with %s\n", strerror(errno));  // DEBUGGING
                error++;
            }
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
