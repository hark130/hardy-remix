#include <errno.h>         // errno
#include <fcntl.h>         // open()
#include <stdio.h>         // fprintf(), remove(), snprintf()
#include <stdlib.h>        // calloc()
#include <string.h>        // strerror()
#include <sys/stat.h>      // S_xxxx
#include <unistd.h>        // close()
#include "HARE_library.h"  // do_it()


char *create_system_cmd(char *filename);


int main(int argc, char *argv[])
{
    // LOCAL VARIABLES
    int success = 0;           // Return value
    char *filename = argv[1];  // CLI argument: filename
    char *system_cmd = NULL;   // Formed system command
    int fd = 0;                // Filename's file descriptor
    int file_exists = 0;       // Makeshift boolean to track file creation
    int errnum = 0;            // Store errno values

    // DO IT
    // 1. Attempt file creation
    if (filename && *filename)
    {
        fd = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
        if (fd > -1)
        {
            file_exists = 1;
            close(fd);
            // 2. Fuzz file contents
            system_cmd = create_system_cmd(filename);
            if (system(system_cmd) < 0)
            {
                errnum = errno;
                fprintf(stderr, "Unable to execute %s.\nERROR: %s\n", filename, strerror(errnum));
            }
        }
    }
    // 3. Execute
    success = do_it(argv[1]);  // Test
    // 4. Delete file
    if (1 == file_exists)
    {
        if (-1 == remove(filename))
        {
            errnum = errno;
            fprintf(stderr, "Unable to delete %s.\nERROR: %s\n", filename, strerror(errnum));
        }
    }

    // DONE
    if (system_cmd)
    {
        free(system_cmd);
        system_cmd = NULL;
    }
    return success;
}


char *create_system_cmd(char *filename)
{
    // LOCAL VARIABLES
    // Template string used to create the system command
    char template[] = { "echo \"This is my file.\nThere are many like it but this one is mine.\" | radamsa > %s" };
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
