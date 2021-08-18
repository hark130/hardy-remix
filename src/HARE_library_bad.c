/*
 *	Implements HARE_library.h functions in the best way.
 */

#ifndef BINARY_NAME
#define BINARY_NAME "<this_best_binary>"  // You didn't define it so we defined it for you!
#endif  // BINARY_NAME

#include <sys/types.h>
#include <sys/stat.h>  // stat()
#include <fcntl.h>
#include <stdlib.h>    // calloc(), free()
#include <string.h>    // strlen(), strstr()
#include <unistd.h>    // close(), read()
#include "HARE_library.h"


// Different than the "good" version
int do_it(char *filename)
{
    // LOCAL VARIABLES
    char *file_contents = NULL;  // Head-allocated member holding the contents of filename
    int success = 0;             // 0 Success, -1 Bad input, -2 Error

    // 2. READ FILE CONTENTS
    if (filename)
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


// Different than the "good" version
char *read_file(char *filename)
{
    // LOCAL VARIABLES
    char *file_contents = NULL;  // Allocate mem and return
    off_t file_size = 64;        // Size of buffer in bytes
    FILE *fp = NULL;             // File pointer to filename
    char *curr_char_ptr = NULL;  // Temporary pointer
    char curr_char = 0x0;        // Temporary read character
    
    // INPUT VALIDATION
    if (filename && *filename)
    {
        // Size it
        // file_size = size_file(filename);
        if (file_size > -1)
        {
            // Allocate it
            file_contents = calloc(file_size + 1, 1);
            // Read it
            if (file_contents)
            {
                fp = fopen(filename, "r");
                if (fp)
                {
                    curr_char_ptr = file_contents;
                    while(1)
                    {
                        curr_char = getc(fp);
                        if (EOF == curr_char)
                        {
                            break;
                        }
                        else
                        {
                            *curr_char_ptr = curr_char;
                            curr_char_ptr++;
                            if (0x0 == curr_char)
                            {
                                break;
                            }
                        }
                    }
                }
            }            
        }
    }
    
    // DONE
    if (fp)
    {
        fclose(fp);
        fp = NULL;
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
