"""IRON unit test base class module.

Iron Onyx (IRON) unit test base class.  Implements the core functionality of the unit test
framework.

    Typical usage example:

    1. Inherit from UnitTestBase
    2. Define additional functionality if necessary
    3. Utilize class functionality as appropriate:
        self._add_test_failure('Present this failure later')  # Use when a Test Case fails
        self.fail(self._test_error.format('Present this failure NOW'))  # Immediate error
        self.validate_*()  # Validate test author input
        self._present_test_failures()  # Call this after test execution to present test failures
    4. Define unittest Test Cases
    5. Call execute_test_cases()
"""
# Standard Imports
from collections import namedtuple
from typing import Any
import unittest
# Third Party Imports
from hobo.disk_operations import validate_directory, validate_file
from hobo.validation import validate_list, validate_string, validate_type
# Local Imports


# Stores test author's expected Exception data
# pylint:disable=undefined-variable
ExceptionData = namedtuple('ExceptionData', ['exception_type', 'exception_msg'])
# pylint:enable=undefined-variable


def execute_test_cases():
    """Execute Test Cases.

    Call this within a module to execute its Test Cases as a stand-alone collection.
    """
    unittest.main(verbosity=2, exit=False)


class UnitTestBase(unittest.TestCase):
    """IRON unit test base class.

    This class defines necessary functionality to unit test IRON.

        General usage:
        1. Inherit from this class
        2. Look for necessary functionality among the 'sibling' classes (and move it up a level)
        3. Define the functionality you need
    """

    _test_error = 'TEST ERROR: {}'  # Pre-formatted test error string

    # CORE CLASS METHODS
    # Methods listed in call order
    def __init__(self, *args, **kwargs) -> None:
        """UnitTestBase ctor.

        UnitTestBase constructor.  Initializes attributes after constructing the parent
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

        self._test_failure_list = []  # List of failures presented at the end

    # TEST AUTHOR METHODS
    # Methods listed in "suggested" call order

    # CLASS HELPER METHODS
    # Methods listed in alphabetical order
    def _add_test_failure(self, failure_msg: str) -> None:
        """Add a Test Case failure message.

        Adds Test Case failures to an internal list.

        Args:
            failure_msg: Non-empty string containing a failure message.

        Returns:
            None

        Raises:
            None
        """
        # INPUT VALIDATION
        # failure_msg
        self._validate_string(failure_msg, 'failure_msg')

        # ADD IT
        self._test_failure_list.append(failure_msg)

    def _present_test_failures(self) -> None:
        """Present test failures.

        Present all of the collated test failures as self.fail().

        Args:
            None

        Returns:
            None

        Raises:
            None.  Calls self.fail() instead.
        """
        # INPUT VALIDATION
        # self._test_failure_list
        self._validate_list(validate_this=self._test_failure_list,
                            param_name='UnitTestBase._test_failure_list',
                            can_be_empty=True)

        # LOCAL VARIABLES
        test_fail_str = '\n' + '\n'.join(self._test_failure_list) + '\n'

        # SHOULD WE FAIL?
        if self._test_failure_list:
            self.fail(test_fail_str)

    def _validate_directory(self, dirname: str, param_name: str, must_exist: bool = True) -> None:
        """Validate a directory.

        Wraps hobo.disk_operations' validate_directory() in a class method to utilize self.fail().
        Input validation is handled by validate_directory().

        Args:
            dirname: The absolute or relative directory to validate
            param_name: The name of the parameter (used in exception messages)
            must_exist: [OPTIONAL] Verifies the directory exists

        Returns:
            None

        Raises:
            None.  Calls self.fail() instead.
        """
        # LOCAL VARIABLES
        error_message = ''  # Store the error message here

        # VALIDATION
        try:
            validate_directory(validate_dir=dirname, param_name=param_name, must_exist=must_exist)
        except (TypeError, ValueError, FileNotFoundError, OSError) as err:
            error_message = self._test_error.format(err.args[0])

        # REPORTING
        if error_message:
            self.fail(error_message)

    def _validate_file(self, filename: str, param_name: str, must_exist: bool = True) -> None:
        """Validate a test file.

        Wraps hobo.disk_operations' validate_file() in a class method to utilize self.fail().
        Input validation is handled by validate_file().

        Args:
            filename: The absolute or relative filename to validate
            param_name: The name of the parameter (used in exception messages)
            must_exist: [OPTIONAL] Verifies the file exists

        Returns:
            None

        Raises:
            None.  Calls self.fail() instead.
        """
        # LOCAL VARIABLES
        error_message = ''  # Store the error message here

        # VALIDATION
        try:
            validate_file(filename=filename, param_name=param_name, must_exist=must_exist)
        except (TypeError, ValueError, FileNotFoundError, OSError) as err:
            error_message = self._test_error.format(err.args[0])

        # REPORTING
        if error_message:
            self.fail(error_message)

    def _validate_list(self, validate_this: list, param_name: str,
                       can_be_empty: bool = True) -> None:
        """Validate a list.

        Wraps hobo.validation's validate_list() in a class method to utilize self.fail().
        Input validation is handled by validate_list().

        Args:
            validate_this: The parameter to validate
            param_name: The name of the parameter (used in failure messages)
            can_be_empty: [OPTIONAL] If False, this function will verify that validate_this is
                *not* empty.

        Returns:
            None

        Raises:
            None.  Calls self.fail() instead.
        """
        # LOCAL VARIABLES
        error_message = ''  # Store the error message here

        # VALIDATION
        try:
            validate_list(validate_this=validate_this, param_name=param_name,
                          can_be_empty=can_be_empty)
        except (TypeError, ValueError) as err:
            error_message = self._test_error.format(err.args[0])

        # REPORTING
        if error_message:
            self.fail(error_message)

    def _validate_string(self, validate_this: str, param_name: str,
                         can_be_empty: bool = False) -> None:
        """Validate a string.

        Wraps hobo.validation's validate_string() in a class method to utilize self.fail().
        Input validation is handled by validate_string().

        Args:
            validate_this: The parameter to validate
            param_name: The name of the parameter (used in failure messages)
            can_be_empty: [OPTIONAL] If False, this function will verify that validate_this is
                *not* empty.

        Returns:
            None

        Raises:
            None.  Calls self.fail() instead.
        """
        # LOCAL VARIABLES
        error_message = ''  # Store the error message here

        # VALIDATION
        try:
            validate_string(validate_this=validate_this, param_name=param_name,
                            can_be_empty=can_be_empty)
        except (TypeError, ValueError) as err:
            error_message = self._test_error.format(err.args[0])

        # REPORTING
        if error_message:
            self.fail(error_message)

    def _validate_type(self, validate_this: Any, param_name: str, param_type: type) -> None:
        """Validate a type.

        Wraps hobo.validation's validate_type() in a class method to utilize self.fail().
        Input validation is handled by validate_type().

        Args:
            validate_this: The parameter to validate
            param_name: The name of the parameter (used in failure messages)
            param_type: The Python 'type' to verify validate_this against

        Returns:
            None

        Raises:
            None.  Calls self.fail() instead.
        """
        # LOCAL VARIABLES
        error_message = ''  # Store the error message here

        # VALIDATION
        try:
            validate_type(var=validate_this, var_name=param_name, var_type=param_type)
        except TypeError as err:
            error_message = self._test_error.format(err.args[0])

        # REPORTING
        if error_message:
            self.fail(error_message)
