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
        # TD: DDN... Put them here

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
        # TD: DDN... validate attributes

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


class BoundaryTestMemstr(TestMemstr):
    """Boundary Test Cases.

    Organize the Boundary Test Cases
    """

    @unittest.skip('TD: DDN... Implement later')
    def test_boundary_01(self):
        """Boundary input: max int"""
        # self.set_test_input(MAX_INT_VALUE - 90, 90)
        # self.expect_return(MAX_INT_VALUE)
        # self.run_test()

    @unittest.skip('TD: DDN... Implement later')
    def test_boundary_02(self):
        """Boundary input: max int (negative)"""
        # self.set_test_input(-MAX_INT_VALUE + 90, -90)
        # self.expect_return(-MAX_INT_VALUE)
        # self.run_test()


class SpecialTestMemstr(TestMemstr):
    """Special Test Cases.

    Organize the Special Test Cases
    """

    @unittest.skip('TD: DDN... Implement later')
    def test_special_01(self):
        """Boundary input: max int"""
        # self.set_test_input(0, 0)
        # self.expect_return(0)
        # self.run_test()


if __name__ == '__main__':
    execute_test_cases()
