from hobo.validation import validate_string, validate_type
import random


def append_noise(source: bytes, lower_limit: int = 64, upper_limit: int = 1024) -> bytes:
    """Transforms a bytearray into a C-like buffer.

    Add a terminating nul character, chr(0), and a pseudorandom amount of trailing "noise".

    Args:
        source: A bytesarray, empty or not, to append to
        lower_limit: Optional; Minimum num of random bytes, inclusive, to trail the nul char
        upper_limit: Optional; Maximum num of random bytes, inclusive, to trail the nul char

    Returns:
        A byte array containing source, terminated by a nul character (encoded as ascii), and
        appended with a psuedorandom number of psuedorandom bytes.

    Raises:
        TypeError: Bad argument type
        ValueError: Bad limit values... e.g., negative limits, lower is larger than upper
    """
    # LOCAL VARIABLES
    byte_string = source  # Return value
    noise_len = 0         # Number of "noise" bytes to randomize

    # INPUT VALIDATION
    validate_type(var=source, var_name='source', var_type=bytes)
    # Remaing arguments are validated by call to randomize_bytes()

    # APPEND NOISE
    # Add nul character
    byte_string += bytes(chr(0), 'ascii')
    # Add random bytes
    byte_string += randomize_bytes(lower_limit=lower_limit, upper_limit=upper_limit)

    # DONE
    return byte_string


def byteify_a_string(source: str, lower_limit: int = 64, upper_limit: int = 1024) -> bytes:
    """Transforms a string into a C-like buffer.

    Encodes source into a bytearray with a terminating nul character, chr(0), and a pseudorandom
    amount of trailing "noise".

    Args:
        source: A string, empty or not, to translate into a bytearray
        lower_limit: Optional; Minimum num of random bytes, inclusive, to trail the nul char
        upper_limit: Optional; Maximum num of random bytes, inclusive, to trail the nul char

    Returns:
        A byte array containing source, encoded as ascii, terminated by a nul character, and
        appended with a psuedorandom number of psuedorandom bytes.

    Raises:
        TypeError: Bad argument type
        ValueError: Bad limit values... e.g., negative limits, lower is larger than upper
    """
    # LOCAL VARIABLES
    byte_string = b''  # Return value

    # INPUT VALIDATION
    validate_string(validate_this=source, param_name='source', can_be_empty=True)
    # Remaing arguments are validated by call to append_noise()

    # BYTEIFY IT
    # Encode
    byte_string = bytes(source, 'ascii')
    byte_string = append_noise(byte_string, lower_limit=lower_limit, upper_limit=upper_limit)

    # DONE
    return byte_string


def randomize_bytes(lower_limit: int = 64, upper_limit: int = 1024) -> bytes:
    """Create random bytes.

    Create a bytearray with a psuedorandom number of psuedorandomized bytes.

    Args:
        lower_limit: Optional; Minimum num of random bytes, inclusive, to trail the nul char
        upper_limit: Optional; Maximum num of random bytes, inclusive, to trail the nul char

    Returns:
        A byte array containing a psuedorandom number of psuedorandom bytes.

    Raises:
        TypeError: Bad argument type
        ValueError: Bad limit values... e.g., negative limits, lower is larger than upper
    """
    # LOCAL VARIABLES
    byte_string = b''  # Return value
    num_bytes = 0      # Number of bytes to randomize

    # INPUT VALIDATION
    validate_type(var=lower_limit, var_name='lower_limit', var_type=int)
    validate_type(var=upper_limit, var_name='upper_limit', var_type=int)
    if lower_limit > upper_limit:
        raise ValueError('lower_limit can not be greater than upper_limit')
    if 0 > lower_limit:
        raise ValueError('lower_limit can not be negative')
    if 0 > upper_limit:
        raise ValueError('upper_limit can not be negative')

    # RANDOMIZE BYTES
    # Determine noise length
    if lower_limit == upper_limit:
        num_bytes = lower_limit
    else:
        num_bytes = random.randint(lower_limit, upper_limit)
    # Randomize bytes
    byte_string += bytes([random.randint(0, 255) for _ in range(0, num_bytes)])

    # DONE
    return byte_string
