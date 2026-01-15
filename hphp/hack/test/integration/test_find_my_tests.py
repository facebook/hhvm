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
        prefix: str,
        expected_test_files: List[Tuple[str, Optional[int]]],
        symbols: List[str],
        max_distance: int = 1,
        max_test_files: Optional[int] = None,
        workers: Optional[int] = None,
    ) -> None:
        args = []
        if workers is not None:
            args = ["--max-procs", str(workers)]

        self.test_driver.start_hh_server(args=args)

        (output, err, retcode) = self.test_driver.run_check()
        if retcode != 0:
            print("stdout: " + output)
            print("stderr: " + err)
            self.fail("hh check failed")

        options = [
            "--autostart-server=false",
            "--json",
            "--find-my-tests-max-distance",
            str(max_distance),
        ]
        if max_test_files is not None:
            options.extend(["--find-my-tests-max-test-files", str(max_test_files)])
        options.append("--find-my-tests")

        (output, err, retcode) = self.test_driver.run_check(options=options + symbols)

        print(
            self.test_driver.get_all_logs(self.test_driver.repo_dir).current_server_log
        )

        if retcode != 0:
            print("stdout: " + output)
            print("stderr: " + err)
            self.fail("hh --find-my-tests failed")

        print(f"hh response: {output}")

        result = json.loads(output)

        result_sorted = sorted(
            result, key=lambda entry: (entry["distance"], entry["file_path"])
        )
        self.assertListEqual(
            result, result_sorted, "hh --find-my-tests result not sorted as expected"
        )

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

    def test_max_test_files(self) -> None:
        """Tests that --find-my-tests-max-test-files limits the number of test files returned.

        This is just reusing the definitions for test_indrection, but running with a limit.
        """

        prefix = os.path.join(self.test_driver.repo_dir, "b", "__tests__")

        expected_tests: List[Tuple[str, Optional[int]]] = [
            ("B_UsesDist1Test.php", 1),
            ("B_UsesDist2Test.php", 2),
        ]

        self.check_has_tests_with_distances(
            prefix=prefix,
            symbols=["B_Def::foo"],
            expected_test_files=expected_tests,
            max_distance=4,
            max_test_files=2,
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

    def test_duplicates(self) -> None:
        """Tests two kinds of duplication:
        - Passing the same root twice
        - Having a method call an existing method in the graph twice

        Uses files from g/ subdirectory
        """
        prefix = os.path.join(self.test_driver.repo_dir, "g", "__tests__")

        self.check_has_tests_with_distances(
            prefix=prefix,
            symbols=["G_Root::duplicateRoot", "G_Root::duplicateRoot"],
            expected_test_files=[("G_DuplicateTest.php", 3)],
            max_distance=10,
        )

    def test_cycle(self) -> None:
        """Tests that cycles in the call graph do not cause issues.

        Uses files from g/ subdirectory
        """
        prefix = os.path.join(self.test_driver.repo_dir, "g", "__tests__")

        self.check_has_tests_with_distances(
            prefix=prefix,
            symbols=["G_Root::cycleRoot"],
            expected_test_files=[("G_CycleTest.php", 3)],
            max_distance=10,
        )

    def test_multi_path(self) -> None:
        """Tests that when there are multiple paths to the same method,
        we use the shortest distance.

        Uses files from g/ subdirectory
        """
        prefix = os.path.join(self.test_driver.repo_dir, "g", "__tests__")

        self.check_has_tests_with_distances(
            prefix=prefix,
            symbols=["G_Root::multiPathRoot"],
            expected_test_files=[("G_MultiPathTest.php", 3)],
            max_distance=10,
        )

    def test_determinism(self) -> None:
        """Tests that graph is built in deterministic fashion

        Uses files from h/ subdirectory
        """

        prefix = os.path.join(self.test_driver.repo_dir, "h", "__tests__")

        os.makedirs(prefix, exist_ok=True)
        for i in range(0, 100):
            file_path = os.path.join(prefix, f"H_Test{i:02d}.php")
            with open(file_path, "w") as f:
                f.write(
                    f"""<?hh

class H_Test{i:02d} extends WWWTest {{

    public static function test(): void {{
        H_Root::root();
    }}

}}
"""
                )

        # We run with more workers than just the default of 2, since MultiWorker is a main
        # source of nondeterminism in hh.
        workers = 10

        # We cap the number returned files at half of what we would be seeing otherwise.
        # The whole point of this test is that the cut-off should be deterministic
        max_test_files = 50

        # Various folds on lists cause us to reverse lists during graph construction.
        # That means that in the current implementation, we end up returning
        # Test50.php to Test99.php, not Test00.php to Test49.php.
        # Nothing wrong with that, we don't care as long it's deterministic.
        expected_test_files: List[Tuple[str, Optional[int]]] = [
            (f"H_Test{i:02d}.php", 1) for i in range(50, 100)
        ]

        self.check_has_tests_with_distances(
            prefix=prefix,
            symbols=["H_Root::root"],
            expected_test_files=expected_test_files,
            max_distance=10,
            max_test_files=max_test_files,
            workers=workers,
        )


class TestFindMyTestsIllformed(test_case.TestCase[common_tests.CommonTestDriver]):
    """Tests that hh --find-my-tests handles ill-formed files gracefully.

    Uses a separate repo that contains files with syntax errors.
    """

    @classmethod
    def get_template_repo(cls) -> str:
        return "hphp/hack/test/integration/data/find_my_tests_illformed_repo/"

    @classmethod
    def get_test_driver(cls) -> common_tests.CommonTestDriver:
        return common_tests.CommonTestDriver()

    def test_illformed_files(self) -> None:
        """Tests that --find-my-tests succeeds even when the repo contains ill-formed files."""
        self.test_driver.start_hh_server()

        (output, err, retcode) = self.test_driver.run_check()
        print("hh check output: " + output)
        print("hh check stderr: " + err)
        self.assertEqual(
            retcode,
            2,
            "Expected hh check to return exit code 2 (type error) due to ill-formed file",
        )

        (output, err, retcode) = self.test_driver.run_check(
            options=[
                "--autostart-server=false",
                "--json",
                "--find-my-tests-max-distance",
                "2",
                "--find-my-tests",
            ]
            + ["Root::root"]
        )

        print(
            self.test_driver.get_all_logs(self.test_driver.repo_dir).current_server_log
        )

        if retcode != 0:
            print("stdout: " + output)
            print("stderr: " + err)
            self.fail("hh --find-my-tests failed when it should have succeeded")

        print(f"hh response: {output}")

        # Verify the response is valid JSON (even if empty)
        result = json.loads(output)
        self.assertIsInstance(result, list)

        self.assertEqual(len(result), 2, "Expected 2 test file to be returned")

        expected_test_files = {
            os.path.join(self.test_driver.repo_dir, "src", "__tests__", file)
            for file in ["MyTest1.php", "MyTest3.php"]
        }
        actual_files = {file_result["file_path"] for file_result in result}

        self.assertSetEqual(expected_test_files, actual_files)

        for r in result:
            self.assertEqual(r["distance"], 2)
