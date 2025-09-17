# pyre-strict

import json
import os

from typing import List, Optional, Tuple

import hphp.hack.test.integration.common_tests as common_tests
import hphp.hack.test.integration.test_case as test_case


class TestFindMyTests(test_case.TestCase[common_tests.CommonTestDriver]):
    @classmethod
    def get_template_repo(cls) -> str:
        return "hphp/hack/test/integration/data/find_my_tests_repo/"

    @classmethod
    def get_test_driver(cls) -> common_tests.CommonTestDriver:
        return common_tests.CommonTestDriver()

    def check_has_tests_with_distances(
        self,
        expected_test_files: List[Tuple[str, Optional[int]]],
        symbols: List[str],
        max_distance: int = 1,
    ) -> None:
        self.test_driver.start_hh_server()

        (output, err, retcode) = self.test_driver.run_check()
        if retcode != 0:
            print("stdout: " + output)
            print("stderr: " + err)
            self.fail("hh check failed")

        (output, err, retcode) = self.test_driver.run_check(
            options=[
                "--json",
                "--find-my-tests-max-distance",
                str(max_distance),
                "--find-my-tests",
            ]
            + symbols
        )
        if retcode != 0:
            print("stdout: " + output)
            print("stderr: " + err)
            self.fail("hh --find-my-tests failed")

        print(f"hh response: {output}")

        result = json.loads(output)

        actual_tests_dict = {
            os.path.basename(test["file_path"]): test["distance"] for test in result
        }

        expected_tests_dict = dict(expected_test_files)

        self.assertSetEqual(
            set(actual_tests_dict.keys()), set(expected_tests_dict.keys())
        )

        for expected_test_file, expected_dist in expected_tests_dict.items():
            if expected_dist is not None:
                self.assertEqual(expected_dist, actual_tests_dict[expected_test_file])

    def check_has_tests(
        self,
        expected_test_files: List[str],
        symbols: List[str],
        max_distance: int = 1,
    ) -> None:
        expected_test_files_no_distances: List[Tuple[str, Optional[int]]] = [
            (file, None) for file in expected_test_files
        ]
        self.check_has_tests_with_distances(
            symbols=symbols,
            expected_test_files=expected_test_files_no_distances,
            max_distance=max_distance,
        )

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

    def test_indirection(self) -> None:
        """Tests that if we set --find-my-tests-max-distance > 1, we get tests that don't call the symbol directly

        Uses files from b/ subdirectory
        """

        for max_distance in range(1, 5):
            # Our test files are set up such that B_UsesDistNTest.php
            # has a call with distance N to B_Def::foo
            expected_tests: List[Tuple[str, Optional[int]]] = [
                (f"B_UsesDist{distance}Test.php", distance)
                for distance in range(1, max_distance + 1)
            ]

            print(f"Expected test files {expected_tests}")

            self.check_has_tests_with_distances(
                symbols=["B_Def::foo"],
                expected_test_files=expected_tests,
                max_distance=max_distance,
            )
