/*
 *
 * 1. Take a filename as a command line argument
 * 2. Read stdin
 * 3. Read file contents
 * 4. Use user input to deobfuscate file contents
 * 5. Print the deobfuscated file contents
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define BINARY_NAME "source04_bad.bin"
#define PASSWORD "fuzzy"
#define BUF_LEN 65


int authenticate(char *secret);
char *get_filename(int argc, char *argv[]);
char *get_password(void);
char *parse_args(int argc, char *argv[]);
void print_usage(void);
char *validate_arg(char *argOne);
int verify_filename(char *filename);
void truncate_newlines(char *string);


int main(int argc, char *argv[])
{
    // LOCAL VARIABLES
    char *filename = NULL;    // Flow control
    // int file_exists = 0;      // Return value from verify_filename(): 1 exists, 0 missing, -1 error
    int success = 0;          // 0 Success, -1 Bad input, -2 Error

    // DO IT
    // 1. PARSE ARGUMENTS
    // filename = get_filename(argc, argv);
    filename = argv[1];

    // 2. VERIFY ENVIRONMENT
    // if (filename)
    // {
    //     file_exists = verify_filename(filename);
    //     if (0 == file_exists)
    //     {
    //         fprintf(stderr, "Unable to find %s\n", filename);
    //         print_usage();
    //         filename = NULL;  // Used to control flow
    //         success = -1;
    //     }
    //     else if (-1 == file_exists)
    //     {
    //         fprintf(stderr, "Error verifying %s\n", filename);
    //         filename = NULL;  // Used to control flow
    //         success = -2;
    //     }
    // }

    // 3. READ FILE
    if (filename)
    {
        if(!authenticate(PASSWORD))
        {
            // 4. Read file
            // 5. Print the file contents
            fprintf(stderr, "%s\n", filename);  // DEBUGGING            
        }
        else
        {
            success = -1;  // Bad password
        }
    }

    // DONE
    return success;
}


/*
 *  Read a password from stdin and authenticate.  0 for success, 1 for failure.
 */
int authenticate(char *secret)
{
    // LOCAL VARIABLES
    char *user_input = NULL;  // Heap-allocated buffer with the user's input
    int success = 1;          // Default the return value to failure

    // INPUT VALIDATION
    if (secret && *secret)
    {
        user_input = get_password();

        if (user_input && !strcmp(user_input, secret))
        {
            success = 0;
        }
    }

    // DONE
    free(user_input);
    user_input = NULL;
    return success;
}


/*
 *  Return the filename argument
 */
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


/*
 *  Reads user input from stdin into a heap-allocated buffer
 */
char *get_password(void)
{
    // LOCAL VARIABLES
    char *input_ptr = malloc(BUF_LEN * sizeof(char));

    // INPUT VALIDATION
    if (input_ptr)
    {
        printf("Enter your password:\n");
        if (!fgets(input_ptr, BUF_LEN, stdin))
        {
            free(input_ptr);
            input_ptr = NULL;
        }
        else
        {
            truncate_newlines(input_ptr);
        }
    }
    
    // DONE
    return input_ptr;
}


/*
 *  Validate arguments.  Return argument 1 (as appropriate).
 */
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
    fprintf(stderr, "usage: app_name [options] obfuscated_file\n");
    fprintf(stderr, "\toptions:\n");
    fprintf(stderr, "\t\t-h, --help\t\tprint usage\n");
    fprintf(stderr, "\n");
}


void truncate_newlines(char *string)
{
    // LOCAL VARIABLES
    char *tmp_ptr = string;

    // INPUT VALIDATION
    if (tmp_ptr && *tmp_ptr)
    {
        while(*tmp_ptr)
        {
            if ('\n' == *tmp_ptr)
            {
                *tmp_ptr = '\0';
                break;
            }
            else
            {
                tmp_ptr++;
            }
        }
    }
}


/*
 *  Prints usage instructions on option match or returns argOne
 */
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


/*
 *  Verify filename exists: 1 exists, 0 missing, -1 error
 */
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
