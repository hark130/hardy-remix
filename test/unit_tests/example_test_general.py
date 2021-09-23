"""Template general unit test module.

Use this module as an example/template to write unit tests for general (see: non-core) features
of Iron Onyx (IRON).
If you decide to separate your test class and Test Cases into separate modules within this
package, ensure the Test Case module filename begins with `test_`.  The class's module
may be named IAW PEP8 naming standards.

    Usage:
    1. Copy this file and rename it to `test_something.py`
    2. Rename the internal 'SOMETHING' placeholders accordingly
    3. Source-control discretely named file-based test input (if applicable)
    4. Define numbered Test Cases within the NEBS classes whose name begins with `test_<NEBS>`
    5. `python3 -m test.unit_tests.test_SOMETHING` to run your test cases

    Troubleshooting:
    Q1. Why didn't my test cases run when I executed `python3 -m unittest`?
    A1a. Ensure your test case module's filename start with `test_`?
    A1b. Is there at least one method name that starts with `test_`?
    Q2. Why didn't my test cases execute with `python3 -m test.unit_tests.test_SOMETHING`?
    A2. Consider the following:
        - Did you replace the command's 'test_SOMETHING' with the actual name of your module?
        - Did you remove or comment out `main()` or the code block that executes `main()`?
        - Did you remove, comment, or modify `main()`'s behavior?
        - Is there at least one Test Case whose name starts with `test_`?
        - Try `python3 -m unittest -k SOMETHING` but replace SOMETHING with a substring related
            to your test cases and/or module
"""
# Standard Imports
# Third Party Imports
# Local Imports
from test.unit_tests.unittestbase import execute_test_cases, UnitTestBase


class TestSOMETHING(UnitTestBase):
    """IRON SOMETHING unit test class.

    This class provides base functionality to run NEBS unit tests for IRON's general SOMETHING
    feature.
    """

    def __init__(self, *args, **kwargs) -> None:
        """TestSOMETHING ctor.

        Class constructor.
        """
        super().__init__(*args, **kwargs)
        self._placeholder = True  # TD: DDN... Remove


class NormalTestSOMETHING(TestSOMETHING):
    """Normal Test Cases.

    Organize the Normal Test Cases
    """

    def test_normal_01(self):
        """
        Normal input that's expected to pass
        """
        self.assertTrue(self._placeholder)  # TD: DDN

    def test_normal_02(self):
        """
        Valid input that's expected to successfully identify a failed 'check operation'
        """
        self.assertTrue(self._placeholder is not False)  # TD: DDN


class ErrorTestSOMETHING(TestSOMETHING):
    """Error Test Cases.

    Organize the Error Test Cases
    """

    def test_error_01(self):
        """
        Error input that's expected to raise an exception
        """
        self.assertFalse(2+2 == 5)  # TD: DDN


class BoundaryTestSOMETHING(TestSOMETHING):
    """Boundary Test Cases.

    Organize the Boundary Test Cases
    """

    def test_boundary_01(self):
        """
        Boundary input that that tests the lower and upper limits of both good and bad input
        """
        self.assertNotAlmostEqual(1, 100)  # TD: DDN


class SpecialTestSOMETHING(TestSOMETHING):
    """Special Test Cases.

    Organize the Special Test Cases
    """

    def test_special_01(self):
        """
        Special input that that tests an edge case
        """
        self.assertIsNot(True, False)  # TD: DDN


if __name__ == '__main__':
    execute_test_cases()
