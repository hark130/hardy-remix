"""memstr() unit test module.

This module contains all necessary functionality to utilize existing test functionality to
execute unit tests on the memstr() function.

    Usage:
    `python3 -m unittest` to run *all* test cases
    `python3 -m unittest -k test_memstr` to run 'memstr' test cases
    `python3 -m unittest -k normal` to run *all* 'normal' test cases
    `python3 -m test.unit_tests.test_memstr` to run just these test cases
    `python3 -m test.unit_tests.test_memstr -k normal` to run just these normal test cases
"""

# Standard Imports
from typing import Any
import sys
import unittest
# Third Party Imports
# Local Imports
from test.unit_tests.helper_functions import append_noise, byteify_a_string, randomize_bytes
from test.unit_tests.unittestbase import execute_test_cases
from test.unit_tests.unittestcallable import UnitTestCallable
from c_libs.memory import memstr
from c_libs.strings import NUL_CHAR


class TestMemstr(UnitTestCallable):
    """TestMemstr unit test class.

    This class provides base functionality to run NEBS unit tests for memstr().
    """

    # CORE CLASS METHODS
    # Methods listed in call order
    def __init__(self, *args, **kwargs) -> None:
        """TestMemstr ctor.

        TestMemstr constructor.  Initializes attributes after constructing the parent
        object.

        Args:
            args: Arguments to pass to the parent class ctor
            kwargs: Keyword arguments to pass to the parent class ctor

        Returns:
            None

        Raises:
            None
        """
        super().__init__(*args, **kwargs)

        # ATTRIBUTES
        self.max_int_size = sys.maxsize  # https://docs.python.org/3.1/library/sys.html#sys.maxsize

    def setUp(self) -> None:
        """Prepares Test Case.

        Automate any preparation necessary before each Test Case executes.

        Args:
            None

        Returns:
            None

        Raises:
            None
        """
        super(TestMemstr, self).setUp()

        # INPUT VALIDATION
        self._validate_type(self.max_int_size, 'self.max_int_size', int)
        if 0 >= self.max_int_size:
            raise ValueError(f'Invalid max_int_size of {self.max_int_size}')

    def call_callable(self) -> Any:
        """Calls memstr.

        Overrides the parent method.  Defines the way to call memstr.

        Args:
            None

        Returns:
            Return value of memstr

        Raises:
            Exceptions raised by memstr are bubbled up and handled by UnitTestCallable
        """
        return memstr(*self._args, **self._kwargs)

    def validate_return_value(self, return_value: Any) -> None:
        """Validate memstr return value.

        Overrides the parent method.  Defines how the test framework validates the return value
        of a completed call.  Calls self._validate_return_value() method under the hood.

        Args:
            return_value: The data to check against what the test author defined as the expected
                return value.  The intended practice is to use the return value of the
                call_callable() method.

        Returns:
            None

        Raises:
            None
        """
        self._validate_return_value(return_value=return_value)


class NormalTestMemstr(TestMemstr):
    """Normal Test Cases.

    Organize the Normal Test Cases
    """

    def test_normal_01(self):
        """Basic string matching."""
        haystack = byteify_a_string('This is my string.  '
            'There are many like it but this one is mine.')
        needle = b'my string'
        self.set_test_input(haystack, needle, len(needle))
        self.expect_return(True)
        self.run_test()

    def test_normal_02(self):
        """Basic string matching with kwargs."""
        haystack = byteify_a_string('Now is the time for all good men to come to the aid '
                                    'of their country')
        needle = b'good men'
        self.set_test_input(haystack=haystack, needle=needle, needle_len=len(needle))
        self.expect_return(expected_result=True)
        self.run_test()

    def test_normal_03(self):
        """Basic input, no match."""
        haystack = byteify_a_string('Your time is up, my time is now')
        needle = b"You can't see me"
        self.set_test_input(haystack, needle, len(needle))
        self.expect_return(False)
        self.run_test()

    def test_normal_04(self):
        """Basic input, no match."""
        haystack = byteify_a_string('Your time is up, my time is now')
        needle = b"You can't see me"
        self.set_test_input(haystack=haystack, needle=needle, needle_len=len(needle))
        self.expect_return(False)
        self.run_test()


class ErrorTestMemstr(TestMemstr):
    """Error Test Cases.

    Organize the Error Test Cases
    """

    def test_error_01(self):
        """Error input that's expected to fail."""
        self.set_test_input('This is not a bytearray', b'This is', 7)
        self.expect_exception(TypeError, '')
        self.run_test()

    def test_error_02(self):
        """Error input, using keywords, that's expected to fail."""
        self.set_test_input(haystack=b'This is a bytearray',
                            needle='This is not',
                            needle_len=11)
        self.expect_exception(TypeError, '')
        self.run_test()

    def test_error_03(self):
        """Error input that's expected to fail."""
        self.set_test_input(b'This is a bytearray', b'This is too', ['Not', 'an', 'int'])
        self.expect_exception(TypeError, '')
        self.run_test()

    def test_error_04(self):
        """Error input that's expected to fail."""
        self.set_test_input(b'This is a bytearray', b'This is too', -2)
        self.expect_exception(ValueError, '')
        self.run_test()

    def test_error_05(self):
        """needle is empty"""
        needle = b''
        haystack = byteify_a_string('What will happen?!')
        self.set_test_input(haystack=haystack, needle=needle, needle_len=len(needle))
        self.expect_exception(RuntimeError, 'A C buffer would never be empty')
        self.run_test()

    def test_error_06(self):
        """haystack is *all the way* empty"""
        needle = b'Lost and found'
        haystack = b''
        self.set_test_input(haystack=haystack, needle=needle, needle_len=len(needle))
        self.expect_exception(ValueError, 'This is not a nul-terminated bytearray')
        self.run_test()


class BoundaryTestMemstr(TestMemstr):
    """Boundary Test Cases.

    Organize the Boundary Test Cases
    """

    def test_boundary_01(self):
        """One-off test: non-nul needle is barely not a match (beginning)"""
        haystack = byteify_a_string('You better not find this')
        needle = b'you'
        self.set_test_input(haystack, needle, len(needle))
        self.expect_return(False)
        self.run_test()

    def test_boundary_02(self):
        """One-off test: non-nul needle is barely not a match (end)"""
        haystack = byteify_a_string('You better not find this')
        needle = b'hiS'
        self.set_test_input(haystack, needle, len(needle))
        self.expect_return(False)
        self.run_test()

    def test_boundary_03(self):
        """haystack match is at the end"""
        haystack = byteify_a_string('You better find this')
        needle = b'You'
        self.set_test_input(haystack, needle, len(needle))
        self.expect_return(True)
        self.run_test()

    def test_boundary_04(self):
        """haystack match is at the beginning"""
        haystack = byteify_a_string('You better find this')
        needle = b'this'
        self.set_test_input(haystack, needle, len(needle))
        self.expect_return(True)
        self.run_test()

    def test_boundary_05(self):
        """haystack and needle are an exact match"""
        test_input = b'You better find this'
        self.set_test_input(haystack=test_input + NUL_CHAR, needle=test_input,
                            needle_len=len(test_input))
        self.expect_return(True)
        self.run_test()

    def test_boundary_06(self):
        """haystack and needle are an exact match precending haystack's nul char"""
        test_input = 'You better find this'
        needle = bytes(test_input, 'ascii')
        haystack = byteify_a_string(test_input)
        self.set_test_input(haystack=haystack, needle=needle, needle_len=len(needle))
        self.expect_return(True)
        self.run_test()

    def test_boundary_08(self):
        """haystack is empty"""
        needle = b'Lost and found'
        haystack = NUL_CHAR + randomize_bytes()
        self.set_test_input(haystack=haystack, needle=needle, needle_len=len(needle))
        self.expect_return(False)
        self.run_test()

    def test_boundary_09(self):
        """haystack is just a nul character without noise"""
        needle = b'Lost and found'
        haystack = NUL_CHAR
        self.set_test_input(haystack=haystack, needle=needle, needle_len=len(needle))
        self.expect_return(False)
        self.run_test()

    @unittest.skip('This test case raises a MemoryError exception')
    def test_boundary_10(self):
        """Needle exists; needle length is max int - 1"""
        needle = b'?' * (self.max_int_size - 1)
        haystack = byteify_a_string('This is my string.  I found it here: ' + needle +
                                    'Can you see it?')
        self.set_test_input(haystack, needle, len(needle))
        self.expect_return(True)
        self.run_test()

    @unittest.skip('This test case raises a MemoryError exception')
    def test_boundary_11(self):
        """Needle missing; needle length is max int - 1"""
        needle = b'?' * (self.max_int_size - 1)
        haystack = byteify_a_string('!' * (self.max_int_size + 10))
        self.set_test_input(haystack, needle, len(needle))
        self.expect_return(False)
        self.run_test()

    @unittest.skip('This test case raises a MemoryError exception')
    def test_boundary_12(self):
        """Needle exists; needle length is max int - 1"""
        needle = b'?' * self.max_int_size
        haystack = byteify_a_string('This is my string.  I found it here: ' + needle +
                                    'Can you see it?')
        self.set_test_input(haystack, needle, len(needle))
        self.expect_return(True)
        self.run_test()

    @unittest.skip('This test case raises a MemoryError exception')
    def test_boundary_13(self):
        """Needle missing; needle length is max int - 1"""
        needle = b'?' * self.max_int_size
        haystack = byteify_a_string('!' * (self.max_int_size + 10))
        self.set_test_input(haystack, needle, len(needle))
        self.expect_return(False)
        self.run_test()


class SpecialTestMemstr(TestMemstr):
    """Special Test Cases.

    Organize the Special Test Cases
    """

    def test_special_01(self):
        """Existing needle contains a nul: leading"""
        needle = NUL_CHAR + b'My string'
        haystack = append_noise(
            b'Have you seen my string?' + needle + b' is around here somewhere')
        # print(f'NEEDLE: {needle}\nHAYSTACK: {haystack}')  # DEBUGGING
        self.set_test_input(haystack, needle, len(needle))
        self.expect_return(True)
        self.run_test()

    def test_special_02(self):
        """Missing needle contains a nul: leading"""
        needle = NUL_CHAR + b'My missing string'
        haystack = byteify_a_string('Have you seen my string?  I thought I left it here.')
        self.set_test_input(haystack, needle, len(needle))
        self.expect_return(False)
        self.run_test()

    def test_special_03(self):
        """Existing needle contains a nul: trailing"""
        needle = b'My string' + NUL_CHAR
        haystack = append_noise(
            b'Have you seen my string?  ' + needle + b' is around here somewhere')
        self.set_test_input(haystack, needle, len(needle))
        self.expect_return(True)
        self.run_test()

    def test_special_04(self):
        """Missing needle contains a nul: trailing"""
        needle = b'My missing string' + NUL_CHAR
        haystack = byteify_a_string('Have you seen my string?  I thought I left it here.')
        self.set_test_input(haystack, needle, len(needle))
        self.expect_return(False)
        self.run_test()

    def test_special_05(self):
        """Existing needle contains a nul: middle"""
        needle = b'My string' + NUL_CHAR + b' is still missing'
        haystack = append_noise(
            b'Have you seen my string?  ' + needle + b' and is around here somewhere')
        self.set_test_input(haystack, needle, len(needle))
        self.expect_return(True)
        self.run_test()

    def test_special_06(self):
        """Missing needle contains a nul: middle"""
        needle = b'My missing string' + NUL_CHAR + b' still has not been found'
        haystack = byteify_a_string('Have you seen my string?  I thought I left it here.'
                                    '  Apparently, it has gone missing!  Let me know if you'
                                    ' see it.')
        self.set_test_input(haystack, needle, len(needle))
        self.expect_return(False)
        self.run_test()

    def test_special_07(self):
        """Existing needle contains a nul: leading and middle"""
        needle = b'My string' + NUL_CHAR + b' is still missing'
        haystack = append_noise(
            b'Have you seen my string?  ' + needle + b' and is around here somewhere')
        self.set_test_input(haystack, needle, len(needle))
        self.expect_return(True)
        self.run_test()

    def test_special_08(self):
        """Missing needle contains a nul: leading and middle"""
        needle = b'My missing string' + NUL_CHAR + b' still has not been found'
        haystack = byteify_a_string('Have you seen my string?  I thought I left it here.'
                                    '  Apparently, it has gone missing!  Let me know if you'
                                    ' see it.')
        self.set_test_input(haystack, needle, len(needle))
        self.expect_return(False)
        self.run_test()

    def test_special_09(self):
        """Existing needle contains a nul: middle and trailing"""
        needle = b'My string' + NUL_CHAR + b' is still missing'
        haystack = append_noise(
            b'Have you seen my string?  ' + needle + b' and is around here somewhere')
        self.set_test_input(haystack, needle, len(needle))
        self.expect_return(True)
        self.run_test()

    def test_special_10(self):
        """Missing needle contains a nul: middle and trailing"""
        needle = b'My missing string' + NUL_CHAR + b' still has not been found'
        haystack = byteify_a_string('Have you seen my string?  I thought I left it here.'
                                    '  Apparently, it has gone missing!  Let me know if you'
                                    ' see it.')
        self.set_test_input(haystack, needle, len(needle))
        self.expect_return(False)
        self.run_test()

    def test_special_11(self):
        """Existing needle contains a nul: leading and trailing"""
        needle = b'My string' + NUL_CHAR + b' is still missing'
        haystack = append_noise(
            b'Have you seen my string?  ' + needle + b' and is around here somewhere')
        self.set_test_input(haystack, needle, len(needle))
        self.expect_return(True)
        self.run_test()

    def test_special_12(self):
        """Missing needle contains a nul: leading and trailing"""
        needle = b'My missing string' + NUL_CHAR + b' still has not been found'
        haystack = byteify_a_string('Have you seen my string?  I thought I left it here.'
                                    '  Apparently, it has gone missing!  Let me know if you'
                                    ' see it.')
        self.set_test_input(haystack, needle, len(needle))
        self.expect_return(False)
        self.run_test()

    def test_special_13(self):
        """Existing needle contains a nul: leading, middle, and trailing"""
        needle = b'My string' + NUL_CHAR + b' is still missing'
        haystack = append_noise(
            b'Have you seen my string?  ' + needle + b' and is around here somewhere')
        self.set_test_input(haystack, needle, len(needle))
        self.expect_return(True)
        self.run_test()

    def test_special_14(self):
        """Missing needle contains a nul: leading, middle, and trailing"""
        needle = b'My missing string' + NUL_CHAR + b' still has not been found'
        haystack = byteify_a_string('Have you seen my string?  I thought I left it here.'
                                    '  Apparently, it has gone missing!  Let me know if you'
                                    ' see it.')
        self.set_test_input(haystack, needle, len(needle))
        self.expect_return(False)
        self.run_test()

    def test_special_15(self):
        """Existing needle contains a nul: numerous"""
        needle = b'My string' + NUL_CHAR + b' is still missing'
        haystack = append_noise(
            b'Have you seen my string?  ' + needle + b' and is around here somewhere')
        self.set_test_input(haystack, needle, len(needle))
        self.expect_return(True)
        self.run_test()

    def test_special_16(self):
        """Missing needle contains a nul: numerous"""
        needle = b'My missing string' + NUL_CHAR + b' still has not been found'
        haystack = byteify_a_string('Have you seen my string?  I thought I left it here.'
                                    '  Apparently, it has gone missing!  Let me know if you'
                                    ' see it.')
        self.set_test_input(haystack, needle, len(needle))
        self.expect_return(False)
        self.run_test()

    def test_special_17(self):
        """Existing needle contains a nul: all"""
        needle = NUL_CHAR * 10
        haystack = append_noise(
            b'Have you seen my string?  ' + needle + b'  I think I left it here.')
        self.set_test_input(haystack, needle, len(needle))
        self.expect_return(True)
        self.run_test()

    def test_special_18(self):
        """Missing needle contains a nul: all"""
        needle = NUL_CHAR * 10
        haystack = append_noise(
            b'Have you seen my string?  ' + NUL_CHAR * 9 + b'  I think I left it here.')
        self.set_test_input(haystack, needle, len(needle))
        self.expect_return(False)
        self.run_test()

    def test_special_19(self):
        """Non-prepended haystack, existing needle, no nul character in needle"""
        needle = b'This is the needle'
        haystack = append_noise(needle)
        self.set_test_input(haystack, needle, len(needle))
        self.expect_return(True)
        self.run_test()

    def test_special_20(self):
        """Non-prepended haystack, missing needle, nul character in needle"""
        needle = b'Can you find ' + NUL_CHAR + b' THE RING?!'
        haystack = append_noise(needle)
        self.set_test_input(haystack, needle, len(needle))
        self.expect_return(True)
        self.run_test()

    def test_special_21(self):
        """Needle is nul-terminated but nul character outside needle_len range"""
        test_input = b'Ignore this nul character'
        needle = test_input + NUL_CHAR
        haystack = append_noise(b'What did I tell you?!  ' + needle + b' or else!')
        self.set_test_input(haystack, needle, len(test_input))
        self.expect_return(True)
        self.run_test()

    def test_special_22(self):
        """Needle is nul-terminated but nul character outside needle_len range and missing"""
        test_input = b'Ignore this nul character'
        needle = test_input + NUL_CHAR
        haystack = append_noise(b'What did I tell you?!  ' + test_input + b' or else!')
        self.set_test_input(haystack, needle, len(test_input))
        self.expect_return(True)
        self.run_test()

    def test_special_23(self):
        """Needle contains non-matching noise outside needle_len range"""
        test_input = b'Ignore this noise'
        needle = test_input + randomize_bytes()
        haystack = append_noise(b'What did I tell you?!  ' + test_input + b' or else!')
        self.set_test_input(haystack, needle, len(test_input))
        self.expect_return(True)
        self.run_test()

    def test_special_24(self):
        """Ensure non-nul needle doesn't match after the haystack nul character"""
        haystack = byteify_a_string('This is my string.')
        haystack += NUL_CHAR + b'There are many like it but this one is mine.'
        needle = b'this one is mine'
        self.set_test_input(haystack, needle, len(needle))
        self.expect_return(False)
        self.run_test()


if __name__ == '__main__':
    execute_test_cases()
