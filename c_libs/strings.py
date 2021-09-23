from hobo.validation import validate_string, validate_type


NUL_CHAR = bytes(chr(0), 'ascii')  # Nul character


def bytelen(string: bytes) -> int:
    """Replicate strlen() for bytes.

    Replicates strlen() behavior for a bytearray.

    Args:
        string: A nul-terminated bytearray

    Returns:
        The number of non-nul bytes preceding the nul character

    Raises:
        TypeError: Bad argument type
        ValueError: No nul-character found in string
    """
    # LOCAL VARIABLES
    length = 0                         # Return value

    # INPUT VALIDATION
    validate_type(string, 'string', bytes)
    if NUL_CHAR not in string:
        raise ValueError('This is not a nul-terminated bytearray')

    # MEASURE IT
    for char in string:
        # print(f'CHAR: {char}')  # DEBUGGING
        if 0 == char:
            # print('FOUND A NUL CHAR!')  # DEBUGGING
            break  # Found it
        else:
            length += 1
    if length == len(string):
        # print(f'\nLENGTH: {length}\nlen(string): {len(string)}')  # DEBUGGING
        # print(f'bytelen({string}')  # DEBUGGING
        raise RuntimeError('string passed validation but no nul character was found?!')

    # DONE
    return length


def strstr(haystack: bytes, needle: bytes) -> bool:
    """Replicate strstr() for bytes.

    Replicates strstr() behavior for bytearrays in a Pythonic way.  We're not testing
    strstr() here so there's no concern over nul-termination.  If stsrstr() were
    called with one or more non-nul teriminated buffers, they would likely not match
    anyway.

    Args:
        haystack: A bytearray in which to search for the needle
        needle: A bytearray to find in the haystack

    Returns:
        True if needle is found in haystack, False otherwise

    Raises:
        TypeError: Bad argument type
        ValueError: needle is empty
    """
    # INPUT VALIDATION
    validate_type(haystack, 'haystack', bytes)
    validate_type(needle, 'needle', bytes)
    if not needle:
        raise ValueError('needle can not be empty')

    # DO IT
    return needle in haystack
