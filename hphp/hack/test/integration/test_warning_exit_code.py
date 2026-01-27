# pyre-strict
"""
Tests that the server exit code correctly distinguishes errors from warnings.

When there are only warnings (no errors with severity=Err), the
server should exit with code 0. When there are actual errors, it should exit
with code 2.
"""

from __future__ import absolute_import, division, print_function, unicode_literals

import os
from typing import List, Optional, Tuple

import hphp.hack.test.integration.common_tests as common_tests
import hphp.hack.test.integration.test_case as test_case


class TestWarningExitCode(test_case.TestCase[common_tests.CommonTestDriver]):
    @classmethod
    def get_template_repo(cls) -> str:
        return "hphp/hack/test/integration/data/warning_exit_code_repo/"

    @classmethod
    def get_test_driver(cls) -> common_tests.CommonTestDriver:
        return common_tests.CommonTestDriver()

    def test_warning_only_exit_code_zero(self) -> None:
        """
        Test that files with only warnings produce exit code 0.

        When there are only warnings (no actual type errors), hh_client check
        should exit with code 0 to indicate success.
        """
        with open(
            os.path.join(self.test_driver.repo_dir, "warning_only.php"), "w"
        ) as f:
            f.write(
                """<?hh
function test_warnings_only(vec<int> $arg): void {
  if (null !== $arg) {
    // Warn[12007]: This comparison will always return true
  }
}
"""
            )

        self.test_driver.start_hh_server(changed_files=["warning_only.php"])

        (output, err, retcode) = self.test_driver.run_check()

        self.assertIn(
            "12007",
            output,
            "Test precondition failed: warning_only.php must produce Warn[12007]. "
            "If Hack warnings have changed, please update the test code.",
        )

        self.assertEqual(
            retcode,
            0,
            f"Expected exit code 0 for warning-only file, but got {retcode}. "
            "Warnings should not cause non-zero exit codes.",
        )

    def test_error_exit_code_two(self) -> None:
        """
        Test that files with actual type errors produce exit code 2.

        When there are actual type errors, hh_client check should exit with
        code 2 to indicate type errors were found.
        """
        # Create a file that produces an actual type error
        with open(os.path.join(self.test_driver.repo_dir, "error.php"), "w") as f:
            f.write(
                """<?hh
function test_error(): int {
  return "not an int";
}
"""
            )

        self.test_driver.start_hh_server(changed_files=["error.php"])

        (output, err, retcode) = self.test_driver.run_check()

        self.assertIn(
            "4110",
            output,
            "Test precondition failed: error.php must produce Typing[4110].",
        )

        self.assertEqual(
            retcode,
            2,
            f"Expected exit code 2 for file with type error, but got {retcode}. "
            "Type errors should cause exit code 2.",
        )

    def test_mixed_warnings_and_errors_exit_code_two(self) -> None:
        """
        Test that files with both warnings and errors produce exit code 2.

        When there are both warnings and actual type errors, hh_client check
        should exit with code 2 (the error takes precedence).
        """
        with open(
            os.path.join(self.test_driver.repo_dir, "warning_file.php"), "w"
        ) as f:
            f.write(
                """<?hh
function has_warning(vec<int> $arg): void {
  if (null !== $arg) {
    // Warn[12007]
  }
}
"""
            )

        with open(os.path.join(self.test_driver.repo_dir, "error_file.php"), "w") as f:
            f.write(
                """<?hh
function has_error(): int {
  return "not an int";
}
"""
            )

        self.test_driver.start_hh_server(
            changed_files=["warning_file.php", "error_file.php"]
        )

        (output, err, retcode) = self.test_driver.run_check()

        self.assertIn("12007", output, "Expected warning Warn[12007] in output")
        self.assertIn("4110", output, "Expected error Typing[4110] in output")

        self.assertEqual(
            retcode,
            2,
            f"Expected exit code 2 when both warnings and errors present, "
            f"but got {retcode}.",
        )
