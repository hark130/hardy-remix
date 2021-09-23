"""IRON unit test callable class module.

Iron Onyx (IRON) unit test callable class.  Implements common functionality to unit test functions
and basic class implementations.

    Typical usage example:

    1. Inherit from UnitTestCallable
    2. Define the call_callable() method
    3. Define the validate_return_value() method
    4. Define additional functionality if necessary
    5. Define unittest Test Cases that:
        5.1. Set Test Input
        self.set_test_input()
        5.2. Set Expected Results
        self.expect_return()
        -or-
        self.expect_exception()
        5.3. Run Test
        self.run_test()
    6. Call execute_test_cases() (see: unittestbase.py)
"""

# Standard Imports
from typing import Any
# Third Party Imports
# Local Imports
from test.unit_tests.unittestbase import ExceptionData, UnitTestBase


class UnitTestCallable(UnitTestBase):
    """IRON unit test callable class.

    This class defines necessary functionality to unit test IRON functions and basic classes.

        General usage:
        1. Inherit from this class
        2. Look for necessary functionality among the 'sibling' classes (and move it up a level)
        3. Define the functionality you need
    """

    # CORE CLASS METHODS
    # Methods listed in call order
    def __init__(self, *args, **kwargs) -> None:
        """UnitTestCallable ctor.

        UnitTestCallable constructor.  Initializes attributes after constructing the parent
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

        self._args = None                       # *args from set_test_input()
        self._expected_exception_data = None    # Defined by expect_exception()
        self._defined_expected_results = False  # Set True by expect_*()
        self._defined_test_input = False        # Set True by set_test_input()
        self._exp_return = None                 # Defined by expect_return()
        self._kwargs = None                     # **kwargs from set_test_input()

    def call_callable(self) -> Any:
        """Child class defines test case callable.

        This method must be overridden by the child class.  This method must define how the
        test framework calls the code being tested.  If any input above and beyond self._args
        and self._kwargs is necessary, feel free to define additional input methods for the
        test author to utilize and then implement it here.  This method will be utilized by
        self._run_test_fail() and self._run_test_pass() to execute the code being tested.

        Args:
            None

        Returns:
            None

        Raises:
            NotImplementedError: The child class hasn't overridden this method.
        """
        # Example Usage:
        # return the_function_you_are_testing(*self._args, **self._kwargs)
        raise NotImplementedError(
            self._test_error.format('The child class must override the call_callable method'))

    def validate_return_value(self, return_value: Any) -> None:
        """Child class defines test case return validation.

        This method must be overridden by the child class.  This method must define how the
        test framework validates the return value of a completed call.  Consider checking the
        type, value, and order as applicable.  Utilize the _validate_return_value() method to
        check data type and data value.  This method will be utilized by self._run_test_return()
        to validate the return value.

        Args:
            return_value: The data to check against what the test author defined as the expected
                return value.  The intended practice is to use the return value of the
                call_callable() method.

        Returns:
            None

        Raises:
            NotImplementedError: The child class hasn't overridden this method.
        """
        # Example Usage:
        # self._validate_return_value(return_value=return_value)
        raise NotImplementedError(
            self._test_error.format(
                'The child class must override the validate_return_value method'))

    # TEST AUTHOR METHODS
    # Methods listed in "suggested" call order
    # 1. Set Test Input
    def set_test_input(self, *args, **kwargs) -> None:
        """Sets test case input.

        Receive and store a variable number of arguments to pass to the 'callable'.  This method
        does not validate input.  If you want the input from the test author validated, then
        wrap this method in an 'input' method for the test author to utilize.

        Args:
            args: Arguments to pass to the function called in self.call_callable()
            kwargs: Keyword arguments to pass to the function called in self.call_callable()

        Returns:
            None

        Raises:
            None
        """
        self._args = args
        self._kwargs = kwargs
        self._defined_test_input = True

    # 2. Set Expected Results
    # Execution Returns
    def expect_return(self, expected_result: Any) -> None:
        """Define expected return values.

        Allow the test author to define the expected return value.  This method
        does not validate input.  If you want the input from the test author validated, then
        wrap this method in an 'input' method for the test author to utilize.

        Args:
            expected_result: The expected return value from execution that returns a value.

        Returns:
            None

        Raises:
            None.  Calls self.fail() instead.
        """
        if self._defined_expected_results:
            self.fail(self._test_error.format('Expected results were already specified'))

        # STORE IT
        self._exp_return = expected_result
        self._defined_expected_results = True

    # Exception Raised
    def expect_exception(self, exception_type: Exception, exception_msg: str) -> None:
        """Define expected failure Exception.

        Allow the test author to define an expected exception and message.

        Args:
            exception_type: An Exception type to expect (e.g., ValueError)
            exception_msg: A sub-string, empty or not, to look for in the raised Exception

        Returns:
            None

        Raises:
            None.  Calls self.fail() instead.
        """
        if self._defined_expected_results:
            self.fail(self._test_error.format('Expected results were already specified'))

        # INPUT VALIDATION
        # exception_type
        self._validate_type(exception_type, 'exception_type', type)
        # exception_msg
        self._validate_string(exception_msg, 'exception_msg', can_be_empty=True)

        # STORE IT
        self._expected_exception_data = ExceptionData(exception_type, exception_msg)
        self._defined_expected_results = True

    # 3. Run Test
    def run_test(self) -> None:
        """Execute the test.

        Execute self.call_callable() and validate the results accordingly.
        This method:
        1. Validates the test author's usage
        2. Executes self.call_callable() and validates the results
        3. Presents all test failures

        Args:
            None

        Returns:
            None

        Raises:
            None.  Calls self.fail() or self._add_test_failure() instead.
        """
        # 1. CONTEXT VALIDATION
        self._validate_usage()

        # 2. RUN TEST
        if self._expected_exception_data:
            self._run_test_exception()
        else:
            self._run_test_return()

        # 3. REPORT
        self._present_test_failures()

    # CLASS HELPER METHODS
    # Methods listed in alphabetical order

    def _run_test_exception(self) -> None:
        """Execute a test expected to fail.

        Execute the self.call_callable() with the assumption it will fail.

        Args:
            None

        Returns:
            None

        Raises:
            None.  Calls self.fail() or self._add_test_failure() instead.
        """
        try:
            self.call_callable()
        # pylint: disable=broad-except
        except Exception as err:
            if not isinstance(err, self._expected_exception_data.exception_type):
                self._add_test_failure('Expected Exception of type '
                                       f'{self._expected_exception_data.exception_type} but '
                                       f'caught an Exception of type {type(err)}')
            elif self._expected_exception_data.exception_msg.lower() not in str(err).lower():
                self._add_test_failure(
                    'Expected the message '
                    f'"{self._expected_exception_data.exception_msg}" in {str(err)}')
        # pylint: enable=broad-except
        else:
            self._add_test_failure('Expected Exception of type '
                                   f'{self._expected_exception_data.exception_type} but '
                                   'no Exception was raised')

    def _run_test_return(self) -> None:
        """Execute a test expected to pass.

        Execute the self.call_callable() with the assumption it will succeed.

        Args:
            None

        Returns:
            None

        Raises:
            None.  Calls self.fail() or self._add_test_failure() instead.
        """
        # LOCAL VARIABLES
        result = None  # Return value self.call_callable()

        # RUN IT
        try:
            result = self.call_callable()
        # pylint: disable=broad-except
        except Exception as err:
            self._add_test_failure(f'Execution failed unexpectedly with {str(err)}')
        else:
            self.validate_return_value(result)
        # pylint: enable=broad-except

    def _validate_return_value(self, return_value: Any) -> None:
        """Validate the return value.

        Validate a return value's type and value against the expected return value provided by
        the test author.

        Args:
            return_value: The data to check against what the test author defined as the expected
                return value.  The intended practice is to use the return value of the
                call_callable() method.

        Returns:
            None

        Raises:
            None.  Calls self._add_test_failure() instead.
        """
        # Type
        if not isinstance(return_value, type(self._exp_return)):
            self._add_test_failure(f'Expected type {type(self._exp_return)} '
                                   f'but it was of type {type(return_value)}')
        # Value
        elif return_value != self._exp_return:
            self._add_test_failure(f'Expected value "{self._exp_return}" '
                                   f'but received "{return_value}" instead')

    def _validate_usage(self) -> None:
        """Validate test author's usage.

        Verify the test author hasn't neglected to make a mandatory function call.

        Args:
            None

        Returns:
            None

        Raises:
            None.  Calls self.fail() instead.
        """
        if not self._defined_test_input:
            self.fail(self._test_error.format('No test input was found'))
        if not self._defined_expected_results:
            self.fail(self._test_error.format('Expected results were not specified'))
