# pyre-strict

from __future__ import absolute_import, division, print_function, unicode_literals

import os

import hphp.hack.test.integration.common_tests as common_tests


class TestStatusSingle(common_tests.CommonTests):
    @classmethod
    def get_test_driver(cls) -> common_tests.CommonTestDriver:
        return common_tests.CommonTestDriver()

    def test_status_single(self) -> None:
        """
        Test hh_client check --single
        """
        self.test_driver.start_hh_server()

        # Test with a single file
        with open(
            os.path.join(self.test_driver.repo_dir, "typing_error.php"), "w"
        ) as f:
            f.write("<?hh //strict\n function aaaa(): int { return h(); }")

        self.test_driver.check_cmd(
            [
                "ERROR: {root}typing_error.php:2:32,34: Invalid return type (Typing[4110])",
                "  {root}typing_error.php:2:19,21: Expected `int`",
                "  {root}foo_3.php:3:23,28: But got `string`",
            ],
            options=["--single", "{root}typing_error.php"],
            stdin="",
        )

        self.test_driver.check_cmd(
            [
                "ERROR: :2:11,14: Name already bound: `aaaa` (Naming[2012])",
                "  {root}typing_error.php:2:11,14: Previous definition is here",
                "ERROR: :2:32,34: Invalid return type (Typing[4110])",
                "  :2:19,21: Expected `int`",
                "  {root}foo_3.php:3:23,28: But got `string`",
            ],
            options=["--single", "-"],
            stdin="<?hh //strict\n function aaaa(): int { return h(); }",
        )

        # Test with 3 files (should use sequential processing)
        with open(
            os.path.join(self.test_driver.repo_dir, "typing_error1.php"), "w"
        ) as f:
            f.write("<?hh //strict\n function error1(): int { return h(); }")

        with open(
            os.path.join(self.test_driver.repo_dir, "typing_error2.php"), "w"
        ) as f:
            f.write("<?hh //strict\n function error2(): int { return h(); }")

        with open(
            os.path.join(self.test_driver.repo_dir, "typing_error3.php"), "w"
        ) as f:
            f.write("<?hh //strict\n function error3(): int { return h(); }")

        self.test_driver.check_cmd(
            [
                "ERROR: {root}typing_error1.php:2:34,36: Invalid return type (Typing[4110])",
                "  {root}typing_error1.php:2:21,23: Expected `int`",
                "  {root}foo_3.php:3:23,28: But got `string`",
                "ERROR: {root}typing_error2.php:2:34,36: Invalid return type (Typing[4110])",
                "  {root}typing_error2.php:2:21,23: Expected `int`",
                "  {root}foo_3.php:3:23,28: But got `string`",
                "ERROR: {root}typing_error3.php:2:34,36: Invalid return type (Typing[4110])",
                "  {root}typing_error3.php:2:21,23: Expected `int`",
                "  {root}foo_3.php:3:23,28: But got `string`",
            ],
            options=[
                "--single",
                "{root}typing_error1.php",
                "--single",
                "{root}typing_error2.php",
                "--single",
                "{root}typing_error3.php",
            ],
            stdin="",
        )

        # Test with 15 files (should use parallel processing)
        for i in range(1, 16):
            with open(
                os.path.join(self.test_driver.repo_dir, f"typing_error_multi{i}.php"),
                "w",
            ) as f:
                f.write(
                    f"<?hh //strict\n function error_multi{i}(): int {{ return h(); }}"
                )

        options = []
        for i in range(1, 16):
            options.extend(["--single", f"{{root}}typing_error_multi{i}.php"])

        # We're not checking the exact error messages here because the order might vary due to
        # parallel processing. Instead, we'll check that we get the expected number of errors.
        output, _ = self.test_driver.check_cmd(None, options=options, stdin="")
        error_lines = [
            line for line in output.splitlines() if line.startswith("ERROR:")
        ]

        # We should have 15 type errors (one for each file)
        self.assertEqual(len(error_lines), 15)

        # Verify that each file has an error
        for i in range(1, 16):
            file_errors = [
                line for line in error_lines if f"typing_error_multi{i}.php" in line
            ]
            self.assertTrue(
                len(file_errors) > 0, f"No errors found for typing_error_multi{i}.php"
            )
