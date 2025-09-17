# pyre-strict

import json
import os

from typing import List

import hphp.hack.test.integration.common_tests as common_tests
import hphp.hack.test.integration.test_case as test_case


class TestFindMyTests(test_case.TestCase[common_tests.CommonTestDriver]):
    @classmethod
    def get_template_repo(cls) -> str:
        return "hphp/hack/test/integration/data/find_my_tests_repo/"

    @classmethod
    def get_test_driver(cls) -> common_tests.CommonTestDriver:
        return common_tests.CommonTestDriver()

    def check_has_tests(
        self, symbols: List[str], expected_test_files: List[str]
    ) -> None:
        self.test_driver.start_hh_server()

        (output, err, retcode) = self.test_driver.run_check()
        if retcode != 0:
            print("stdout: " + output)
            print("stderr: " + err)
            self.fail("hh check failed")

        (output, err, retcode) = self.test_driver.run_check(
            options=["--json", "--find-my-tests"] + symbols
        )
        if retcode != 0:
            print("stdout: " + output)
            print("stderr: " + err)
            self.fail("hh --find-my-tests failed")

        print(f"hh response: {output}")

        result = json.loads(output)

        actual_test_files = {os.path.basename(test["file_path"]) for test in result}

        self.assertSetEqual(actual_test_files, set(expected_test_files))

    def test_filter_non_test_files(self) -> None:
        """Tests that we don't return files from non __tests__ folders

        Uses files from the a/ subdirectory (all of which have prefix A_)
        """

        # Crucially, this does not include A_NotATest.php
        self.check_has_tests(
            symbols=["A_Sub::target"], expected_test_files=["A_SubTest.php"]
        )

    def test_overrides(self) -> None:
        """Tests that we return calls to overrides of the requested symbol

        Uses files from the a/ subdirectory (all of which have prefix A_)
        """

        self.check_has_tests(
            symbols=["A_Middle::target"],
            expected_test_files=[
                "A_SubTest.php",
                "A_SubNoOverrideTest.php",
                "A_MiddleTest.php",
            ],
        )
        self.check_has_tests(
            symbols=["A_Super::target"],
            expected_test_files=[
                "A_SubTest.php",
                "A_SubNoOverrideTest.php",
                "A_MiddleTest.php",
                "A_SuperTest.php",
                "A_SiblingTest.php",
            ],
        )
