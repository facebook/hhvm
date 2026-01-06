# pyre-strict

import json
import os
import sys

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
        prefix: str,
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
                "--autostart-server=false",
                "--json",
                "--find-my-tests-max-distance",
                str(max_distance),
                "--find-my-tests",
            ]
            + symbols
        )

        print(
            self.test_driver.get_all_logs(self.test_driver.repo_dir).current_server_log
        )

        if retcode != 0:
            print("stdout: " + output)
            print("stderr: " + err)
            self.fail("hh --find-my-tests failed")

        print(f"hh response: {output}")

        result = json.loads(output)

        actual_tests_dict = {test["file_path"]: test["distance"] for test in result}

        absolute_expected_test_files = [
            (os.path.join(prefix, file), distance)
            for (file, distance) in expected_test_files
        ]

        expected_tests_dict = dict(absolute_expected_test_files)

        self.assertSetEqual(
            set(actual_tests_dict.keys()), set(expected_tests_dict.keys())
        )

        for expected_test_file, expected_dist in expected_tests_dict.items():
            if expected_dist is not None:
                self.assertEqual(expected_dist, actual_tests_dict[expected_test_file])

    def check_has_tests(
        self,
        prefix: str,
        expected_test_files: List[str],
        symbols: List[str],
        max_distance: int = 1,
    ) -> None:
        expected_test_files_no_distances: List[Tuple[str, Optional[int]]] = [
            (file, None) for file in expected_test_files
        ]
        self.check_has_tests_with_distances(
            prefix=prefix,
            symbols=symbols,
            expected_test_files=expected_test_files_no_distances,
            max_distance=max_distance,
        )

    def test_filter_non_test_files(self) -> None:
        """Tests that we don't return files from non __tests__ folders and only files that include a class extending a test base class (e.g., WWWTest)

        Uses files from the a/ subdirectory (all of which have prefix A_)
        """

        prefix = os.path.join(self.test_driver.repo_dir, "a", "__tests__")

        # Crucially, this does not include A_NotATest.php or A_AlsoNotATest.php
        self.check_has_tests(
            prefix=prefix,
            symbols=["A_Sub::target"],
            expected_test_files=["A_SubTest.php"],
        )

    def test_overrides(self) -> None:
        """Tests that we return calls to overrides of the requested symbol

        Uses files from the a/ subdirectory (all of which have prefix A_)
        """

        prefix = os.path.join(self.test_driver.repo_dir, "a", "__tests__")

        self.check_has_tests(
            prefix=prefix,
            symbols=["A_Middle::target"],
            expected_test_files=[
                "A_SubTest.php",
                "A_SubNoOverrideTest.php",
                "A_MiddleTest.php",
            ],
        )
        self.check_has_tests(
            prefix=prefix,
            symbols=["A_Super::target"],
            expected_test_files=[
                "A_SubTest.php",
                "A_SubNoOverrideTest.php",
                "A_MiddleTest.php",
                "A_SuperTest.php",
                "A_SiblingTest.php",
            ],
        )

    def test_distance_zero(self) -> None:
        """Tests that if we ask about a test file itself, it is returned with distance 0

        Uses files from the a/ subdirectory (all of which have prefix A_)
        """

        prefix = os.path.join(self.test_driver.repo_dir, "a", "__tests__")

        self.check_has_tests_with_distances(
            prefix=prefix,
            symbols=["A_SubTest::target"],
            expected_test_files=[("A_SubTest.php", 0)],
        )

    def test_indirection(self) -> None:
        """Tests that if we set --find-my-tests-max-distance > 1, we get tests that don't call the symbol directly

        Uses files from b/ subdirectory
        """

        prefix = os.path.join(self.test_driver.repo_dir, "b", "__tests__")

        for max_distance in range(1, 5):
            # Our test files are set up such that B_UsesDistNTest.php
            # has a call with distance N to B_Def::foo
            expected_tests: List[Tuple[str, Optional[int]]] = [
                (f"B_UsesDist{distance}Test.php", distance)
                for distance in range(1, max_distance + 1)
            ]

            print(f"Expected test files {expected_tests}")

            self.check_has_tests_with_distances(
                prefix=prefix,
                symbols=["B_Def::foo"],
                expected_test_files=expected_tests,
                max_distance=max_distance,
            )

    def test_construction1(self) -> None:
        """Tests that we detect object creation (without explicit constructors)

        Uses files from c/ subdirectory

        """

        prefix = os.path.join(self.test_driver.repo_dir, "c", "__tests__")

        self.check_has_tests_with_distances(
            prefix=prefix,
            symbols=["C1_Mid::origin"],
            expected_test_files=[("C1_MidTest.php", 3), ("C1_DirectNewTest.php", 2)],
            max_distance=10,
        )

    def test_construction2(self) -> None:
        """Tests that we detect object creation (with explicit constructors)

        Uses files from c/ subdirectory
        """

        prefix = os.path.join(self.test_driver.repo_dir, "c", "__tests__")

        self.check_has_tests_with_distances(
            prefix=prefix,
            symbols=["C2_Mid::origin"],
            expected_test_files=[("C2_MidTest.php", 3), ("C2_DirectNewTest.php", 2)],
            max_distance=10,
        )

    def test_bypassvisibility(self) -> None:
        """Tests involving some common usages of BypassVisibility (and a bit of DataProvider).

        Note that FindMyTests currently has no dedicated handling for either of those constructs.
        However, we should ensure that these tests keep on being detected, even if this is
        currently achieved through other mechanisms.

        Uses files from d/ subdirectory
        """

        prefix = os.path.join(self.test_driver.repo_dir, "d", "__tests__")

        self.check_has_tests_with_distances(
            prefix=prefix,
            symbols=["Method|D1::origin"],
            expected_test_files=[
                ("D1_Test1.php", 2),
                ("D1_Test2.php", 3),
                ("D1_Test3.php", 2),
                ("D1_Test4.php", 2),
                ("D1_Test5.php", 2),
                ("D1_Test6.php", 3),
            ],
            max_distance=10,
        )

    def test_enums(self) -> None:
        prefix = os.path.join(self.test_driver.repo_dir, "e", "__tests__")

        self.check_has_tests_with_distances(
            prefix=prefix,
            symbols=["Class|E1"],
            expected_test_files=[
                ("E1_Test1.php", 1),
                ("E1_Test2.php", 2),
            ],
            max_distance=10,
        )

    def test_enum_classes1(self) -> None:
        prefix = os.path.join(self.test_driver.repo_dir, "e", "__tests__")

        self.check_has_tests_with_distances(
            prefix=prefix,
            symbols=["Class|E2"],
            expected_test_files=[
                ("E2_Test1.php", 1),
                ("E2_Test2.php", 2),
            ],
            max_distance=10,
        )

    def test_enum_classes2(self) -> None:
        prefix = os.path.join(self.test_driver.repo_dir, "e", "__tests__")

        # TODO(T246671457) We would like E3_Test1.php to be found here, but it's currently not
        self.check_has_tests_with_distances(
            prefix=prefix,
            symbols=["Class|E3_Super"],
            expected_test_files=[],
            max_distance=10,
        )

    def test_typedefs_and_typeconsts1(self) -> None:
        """Tests that we detect references between typedefs, typeconsts, and usages of them.

        Uses files from f/ subdirectory
        """

        prefix = os.path.join(self.test_driver.repo_dir, "f", "__tests__")

        self.check_has_tests_with_distances(
            prefix=prefix,
            symbols=["Typedef|F1_TypeAlias1"],
            expected_test_files=[
                # F1_Test2, F1_Test6, F1_Test8 should not be selected.
                ("F1_Test1.php", 3),
                ("F1_Test3.php", 1),
                ("F1_Test4.php", 2),
                ("F1_Test5.php", 2),
                ("F1_Test7.php", 4),
                # ("F1_Test9.php", 3), should be selected once T247845469 is fixed
            ],
            max_distance=10,
        )

    def test_typedefs_and_typeconsts2(self) -> None:
        """Tests that we detect references between typedefs, typeconsts, and usages of them.

        Uses files from f/ subdirectory
        """

        prefix = os.path.join(self.test_driver.repo_dir, "f", "__tests__")

        # Similar to previous test, but now starting at the second hop (F1_ClassWithTypeConst1::TBar)
        # Everything should still be selected (with decremented distance), except F1_Test3 and F1_Test9 which are only selected if F1_TypeAlias1 is traversed.

        self.check_has_tests_with_distances(
            prefix=prefix,
            symbols=["Typeconst|F1_ClassWithTypeConst1::TBar"],
            expected_test_files=[
                # F1_Test2, F1_Test3, F1_Test6, F1_Test8 should not be selected.
                ("F1_Test1.php", 2),
                ("F1_Test4.php", 1),
                ("F1_Test5.php", 1),
                ("F1_Test7.php", 3),
            ],
            max_distance=10,
        )
