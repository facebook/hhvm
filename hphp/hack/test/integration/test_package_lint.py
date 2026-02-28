# pyre-strict
from __future__ import absolute_import, unicode_literals

import json
import os

from hphp.hack.test.integration.common_tests import CommonTestDriver
from hphp.hack.test.integration.test_case import TestCase


class PackageLintDriver(CommonTestDriver):
    repo_dir = "hphp/hack/test/integration/data/packages"

    def write_load_config(
        self, use_serverless_ide: bool = False, use_saved_state: bool = False
    ) -> None:
        with open(os.path.join(self.repo_dir, ".hhconfig"), "w") as f:
            f.write(
                """
packages_config_path = PACKAGES.toml
"""
            )

    def expected_file_name(self, file_name: str) -> str:
        return "{}.json.exp".format(file_name)

    def expected_file_path(self, file_name: str) -> str:
        return os.path.join(self.repo_dir, self.expected_file_name(file_name))

    def expected_json(self, file_name: str) -> str:
        with open(self.expected_file_path(file_name)) as expected_file:
            return expected_file.read().strip()

    def get_package_lint_info(self, file_name: str) -> str:
        arg = os.path.join(self.repo_dir, file_name)
        output, err, retcode = self.run_check(
            options=[
                "--package-lint",
                arg,
            ]
        )
        self.assertEqual(
            0,
            retcode,
            "hh --package-lint {} returned non-zero code: {}".format(file_name, err),
        )
        return json.dumps(json.loads(output.replace(self.repo_dir, "")), indent=2)

    def assert_json_matches(self, file_name: str) -> None:
        self.assertMultiLineEqual(
            self.expected_json(file_name),
            self.get_package_lint_info(file_name),
            f"JSON does not match for {file_name}",
        )

    def assert_no_errors(self) -> None:
        output, err, retcode = self.run_check(
            options=[
                "--json",
            ]
        )
        print(output)
        res = json.loads(output)
        print(res)
        self.assertEqual(
            len(res["errors"]),
            0,
            f"Found typing errors in the repo:\n{json.dumps(res['errors'], indent=2)}",
        )


class TestPackageLint(TestCase[PackageLintDriver]):
    @classmethod
    def setUpClass(cls) -> None:
        super().setUpClass()

    @classmethod
    def get_template_repo(cls) -> str:
        return "hphp/hack/test/integration/data/packages"

    @classmethod
    def get_test_driver(cls) -> PackageLintDriver:
        return PackageLintDriver()

    def test_repo(self) -> None:
        self.test_driver.write_load_config()
        self.test_driver.assert_no_errors()

    def test_json(self) -> None:
        self.test_driver.write_load_config()

        cases = [
            "foo/targets.php",
            "bar/targets.php",
            "bar/targets_override.php",
            "bar/targets_override_used.php",
        ]

        for file_name in cases:
            with self.subTest(msg=f"{file_name}"):
                self.test_driver.assert_json_matches(file_name)
