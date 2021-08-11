"""Checks a specified Memwatch log file for errors.

Checks the specified Memwatch log file for detected errors.
Exit Codes:
    0 - No errors detected
    1 - Memwatch errors detected
    2 - Bad input: missing file

    Typical usage example:

    #!/bin/bash
    python3 check_memwatch.py --log <memwatch.log>
    RET_VAL=$?
    if [[ $RET_VAL -ne 0 ]]
    then
        echo "Something went wrong?!"
    else
        echo "All clear!"
    fi
"""

import argparse
import sys
from hobo.disk_operations import validate_file
from hobo.memwatch_automation import parse_memwatch_log


def parse_args() -> str:
    """Returns Memwatch log filename."""
    # LOCAL VARIABLES
    parser = argparse.ArgumentParser()  # Parser
    args = None                         # Arguments
    log_name = ''                       # Memwatch log filename

    # DO IT
    parser.add_argument('-l', '--log', help='Memwatch log filename', default='memwatch.log')
    args = parser.parse_args()
    try:
        log_name = args.log
    except AttributeError:
        pass

    # DONE
    return log_name


def main() -> None:
    """Checks Memwtch log."""
    # LOCAL VARIABLES
    log_name = parse_args()  # Memwatch log filename provided in the arguments
    exit_code = 0            # Exit code: 0 No errors, 1 Memwatch errors, 2 Bad input

    # INPUT VALIDATION
    if log_name:
        try:
            validate_file(log_name, 'Memwatch log filename')
        except (FileNotFoundError, OSError, ValueError, TypeError):
            exit_code = 2
        else:
            if parse_memwatch_log(log_name):
                exit_code = 1
    else:
        exit_code = 2

    # DONE
    sys.exit(exit_code)


if __name__ == '__main__':
    main()
