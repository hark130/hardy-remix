from c_libs.strings import bytelen, NUL_CHAR, strstr
from hobo.validation import validate_string, validate_type


def memcmp(first_buff: bytes, second_buff: bytes, second_len: int) -> bool:
    """Replicate memcmp() for bytes.

    Replicates memcmp() behavior for two bytearrays.

    Args:
        first_buff: A bytearray
        second_buff: A bytearray
        second_len: The length of second_buff

    Returns:
        True if the two bytearrays match for the first "second_len" bytes

    Raises:
        TypeError: Bad argument type
        ValueError: Invalid length
        RuntimeError: second_buff is shorter than second_len
    """
    # LOCAL VARIABLES
    same = True  # Set this to false if a difference is detected

    # INPUT VALIDATION
    validate_type(first_buff, 'first_buff', bytes)
    validate_type(second_buff, 'second_buff', bytes)
    validate_type(second_len, 'second_len', int)
    if second_len < 0:
        raise ValueError('second_len can not be negative')
    if second_len > len(second_buff):
        raise RuntimeError('Invalid usage for this test')

    # DO IT
    try:
        for index in range(0, second_len):
            if first_buff[index] != second_buff[index]:
                same = False
                break  # We can stop looking now
    except IndexError:
        same = False  # Apparently, first_buff wasn't long enough

    # DONE
    return same


def memstr(haystack: bytes, needle: bytes, needle_len: int) -> bool:
    """Match needle in haystack.

    Attempts to find an occurrence of needle, up to needle_len in size, inside haystack.
    If needle contains a nul-terminator, this function will investigate memory past
    haystack's nul-terminator.

    Args:
        haystack: A nul-terminated bytearray, of indeterminate size, to search for needle
        needle: A raw buffer to search haystack for
        needle_len: The length of needle

    Returns:
        True if needle is found in haystack

    Raises:
        TypeError: Bad argument type
        ValueError: Bad values
        RuntimeError: needle is empty
    """
    # LOCAL VARIABLES
    nul_char_found = False  # Life is different if needle contains a nul character
    match = False           # Return value

    # INPUT VALIDATION
    validate_type(haystack, 'haystack', bytes)
    validate_type(needle, 'needle', bytes)
    validate_type(needle_len, 'needle_len', int)
    if needle_len < 0:
        raise ValueError('needle_len can not be negative')
    if 0 == len(needle):
        raise RuntimeError('A C buffer would never be empty')

    # DO IT
    # 1. Is there a nul character in needle within needle_len range?
    try:
        if needle_len > bytelen(needle):
            nul_char_found = True
    except ValueError:
        nul_char_found = False

    # 2. Search for a match
    if nul_char_found:
        match = _nul_search(haystack, needle, needle_len)
    else:
        match = _non_nul_search(haystack, needle, needle_len)

    # DONE
    return match


def _non_nul_search(haystack: bytes, needle: bytes, needle_len: int) -> bool:
    """Match needle in haystack.

    Attempts to find an occurrence of needle, up to needle_len in size, inside haystack.
    This function does not validate type or value above what is covered under Raises.

    Args:
        haystack: A nul-terminated bytearray, of indeterminate size, to search for needle
        needle: A raw buffer, non-nul terminated, to search haystack for
        needle_len: The length of needle

    Returns:
        True if needle is found in haystack, False otherwise

    Raises:
        RuntimeError: needle contains a nul character within needle_len
    """
    # LOCAL VARIABLES
    found_it = False                  # Needle was found in haystack
    haystack_len = bytelen(haystack)  # Length of haystack

    # INPUT VALIDATION
    if strstr(needle[:needle_len], NUL_CHAR):
        raise RuntimeError('needle contains a nul character.  '
                           'Call _nul_search() instead.')

    # DO IT
    if haystack_len >= needle_len:
        for offset in range(0, haystack_len - needle_len + 1):
            found_it = memcmp(haystack[offset:], needle, needle_len)
            if found_it:
                break

    # DONE
    return found_it


def _nul_search(haystack: bytes, needle: bytes, needle_len: int) -> bool:
    """Match needle, containing a nul character, in haystack.

    Attempts to find an occurrence of needle (which contains at least one nul
    character), up to needle_len in size, inside haystack.
    This function does not validate type or value above what is covered under Raises.

    Args:
        haystack: A nul-terminated bytearray, of indeterminate size, to search for needle
        needle: A raw buffer, non-nul terminated, to search haystack for
        needle_len: The length of needle

    Returns:
        True if needle is found in haystack, False otherwise

    Raises:
        RuntimeError: needle does not contain a nul character within needle_len
    """
    # LOCAL VARIABLES
    found_it = False                  # Needle was found in haystack
    haystack_len = bytelen(haystack)  # Length of haystack
    haystack_nul_index = 0            # Index of the nul character in haystack
    needle_nul_index = 0              # Index of the first nul character in needle

    # INPUT VALIDATION
    if not strstr(needle[:needle_len], NUL_CHAR):
        raise RuntimeError('needle does not contain a nul character.  '
                           'Call _non_nul_search() instead.')

    # DO IT
    # Line up the nul characters
    haystack_nul_index = haystack_len
    needle_nul_index = bytelen(needle)
    # Go looking
    if haystack_len >= needle_nul_index:
        found_it = memcmp(haystack[haystack_nul_index - needle_nul_index:], needle, needle_len)

    # DONE
    return found_it
