# pyre-strict

from __future__ import absolute_import, division, print_function, unicode_literals

import os
from typing import Sequence

import hphp.hack.test.integration.common_tests as common_tests


class TestStatusSingle(common_tests.CommonTests):
    @classmethod
    def get_test_driver(cls) -> common_tests.CommonTestDriver:
        return common_tests.CommonTestDriver()

    def write_cached_status_single_fixtures(self) -> None:
        with open(
            os.path.join(self.test_driver.repo_dir, "cached_error1.php"), "w"
        ) as f:
            f.write("<?hh //strict\n function error1(): int { return h(); }")

        with open(
            os.path.join(self.test_driver.repo_dir, "unrelated_error.php"), "w"
        ) as f:
            f.write('<?hh //strict\n function unrelated_error(): int { return "x"; }')

    def check_cached_status_single_outputs(self) -> None:
        self.test_driver.check_cmd(
            [
                "ERROR: {root}cached_error1.php:2:34,36: Invalid return type (Typing[4110])",
                "  {root}cached_error1.php:2:21,23: Expected `int`",
                "  {root}foo_3.php:3:23,28: But got `string`",
            ],
            options=["--single", "{root}cached_error1.php"],
            stdin="",
        )

        self.test_driver.check_cmd(
            ["No errors!"],
            options=["--single", "{root}foo_3.php"],
            stdin="",
        )

    def write_status_single_error_fixture(
        self,
        file_name: str,
        function_name: str,
    ) -> None:
        with open(os.path.join(self.test_driver.repo_dir, file_name), "w") as f:
            f.write(f"<?hh //strict\nfunction {function_name}(): int {{ return h(); }}")

    def check_status_single_reports_errors_for_files(
        self,
        file_names: Sequence[str],
    ) -> None:
        options: list[str] = []
        for file_name in file_names:
            options.extend(["--single", f"{{root}}{file_name}"])

        output, _ = self.test_driver.check_cmd(None, options=options, stdin="")
        error_lines = [
            line for line in output.splitlines() if line.startswith("ERROR:")
        ]

        self.assertEqual(len(file_names), len(error_lines), output)
        for file_name in file_names:
            self.assertTrue(
                any(file_name in line for line in error_lines),
                f"No error found for {file_name}\n{output}",
            )

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

    def test_status_single_cached_diagnostics_disabled_by_default(self) -> None:
        self.write_cached_status_single_fixtures()
        self.test_driver.start_hh_server()
        self.check_cached_status_single_outputs()

    def test_status_single_cached_diagnostics_enabled_preserves_behavior(
        self,
    ) -> None:
        self.write_cached_status_single_fixtures()
        self.test_driver.start_hh_server(
            args=["--config", "status_single_use_cached_diagnostics=true"]
        )
        self.check_cached_status_single_outputs()

    def test_status_single_cached_diagnostics_rechecks_changed_file(self) -> None:
        file_name = "cached_clean_then_error.php"
        with open(os.path.join(self.test_driver.repo_dir, file_name), "w") as f:
            f.write(
                "<?hh //strict\nfunction cached_clean_then_error(): string { return h(); }"
            )

        self.test_driver.start_hh_server(
            args=["--config", "status_single_use_cached_diagnostics=true"]
        )
        self.test_driver.check_cmd(
            ["No errors!"],
            options=["--single", f"{{root}}{file_name}"],
            stdin="",
        )

        self.write_status_single_error_fixture(file_name, "cached_clean_then_error")
        self.check_status_single_reports_errors_for_files([file_name])

    def test_status_single_cached_diagnostics_rechecks_changed_dependency(self) -> None:
        dependency_file_name = "cached_dependency.php"
        dependent_file_name = "cached_dependent.php"
        with open(
            os.path.join(self.test_driver.repo_dir, dependency_file_name), "w"
        ) as f:
            f.write("<?hh //strict\nfunction cached_dependency(): int { return 1; }")
        with open(
            os.path.join(self.test_driver.repo_dir, dependent_file_name), "w"
        ) as f:
            f.write(
                "<?hh //strict\nfunction cached_dependent(): int { return cached_dependency(); }"
            )

        self.test_driver.start_hh_server(
            args=["--config", "status_single_use_cached_diagnostics=true"]
        )
        self.test_driver.check_cmd(
            ["No errors!"],
            options=["--single", f"{{root}}{dependent_file_name}"],
            stdin="",
        )

        with open(
            os.path.join(self.test_driver.repo_dir, dependency_file_name), "w"
        ) as f:
            f.write(
                '<?hh //strict\nfunction cached_dependency(): string { return "x"; }'
            )
        output, _ = self.test_driver.check_cmd(
            None,
            options=["--single", f"{{root}}{dependent_file_name}"],
            stdin="",
        )

        self.assertIn("Invalid return type", output)
        self.assertIn(dependent_file_name, output)
        self.assertIn(dependency_file_name, output)

    def test_status_single_cached_diagnostics_reports_partial_global_errors(
        self,
    ) -> None:
        file_names = []
        for i in range(20):
            file_name = f"cached_partial_error{i}.php"
            self.write_status_single_error_fixture(
                file_name,
                f"cached_partial_error{i}",
            )
            file_names.append(file_name)

        self.test_driver.start_hh_server(
            args=[
                "--config",
                "status_single_use_cached_diagnostics=true",
                "--config",
                "workload_quantile=0,2",
            ]
        )

        self.check_status_single_reports_errors_for_files(file_names)

    def test_status_multi(self) -> None:
        """
        Test hh_client check --multi
        """
        self.test_driver.start_hh_server()

        # Also create the typing_error_multi files for the large file test
        for i in range(1, 16):
            with open(
                os.path.join(self.test_driver.repo_dir, f"typing_error_multi{i}.php"),
                "w",
            ) as f:
                f.write(
                    f"<?hh //strict\n function error_multi{i}(): int {{ return h(); }}"
                )

        # Create a file with the list of files to check
        file_list_path = os.path.join(self.test_driver.repo_dir, "files_to_check.txt")
        with open(file_list_path, "w") as f:
            for i in range(1, 6):
                f.write(f"{self.test_driver.repo_dir}/typing_error_multi{i}.php\n")

        # Test with --multi option
        self.test_driver.check_cmd(
            [
                "ERROR: {root}typing_error_multi1.php:2:40,42: Invalid return type (Typing[4110])",
                "  {root}typing_error_multi1.php:2:27,29: Expected `int`",
                "  {root}foo_3.php:3:23,28: But got `string`",
                "ERROR: {root}typing_error_multi2.php:2:40,42: Invalid return type (Typing[4110])",
                "  {root}typing_error_multi2.php:2:27,29: Expected `int`",
                "  {root}foo_3.php:3:23,28: But got `string`",
                "ERROR: {root}typing_error_multi3.php:2:40,42: Invalid return type (Typing[4110])",
                "  {root}typing_error_multi3.php:2:27,29: Expected `int`",
                "  {root}foo_3.php:3:23,28: But got `string`",
                "ERROR: {root}typing_error_multi4.php:2:40,42: Invalid return type (Typing[4110])",
                "  {root}typing_error_multi4.php:2:27,29: Expected `int`",
                "  {root}foo_3.php:3:23,28: But got `string`",
                "ERROR: {root}typing_error_multi5.php:2:40,42: Invalid return type (Typing[4110])",
                "  {root}typing_error_multi5.php:2:27,29: Expected `int`",
                "  {root}foo_3.php:3:23,28: But got `string`",
            ],
            options=["--multi", file_list_path],
            stdin="",
        )

        # Test with a large number of files (15) to trigger parallel processing
        large_file_list_path = os.path.join(
            self.test_driver.repo_dir, "large_files_to_check.txt"
        )
        with open(large_file_list_path, "w") as f:
            for i in range(1, 16):
                # Use the typing_error_multi files from the first test
                f.write(f"{self.test_driver.repo_dir}/typing_error_multi{i}.php\n")

        # We're not checking the exact error messages here because the order might vary due to
        # parallel processing. Instead, we'll check that we get the expected number of errors.
        output, _ = self.test_driver.check_cmd(
            None, options=["--multi", large_file_list_path], stdin=""
        )
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
