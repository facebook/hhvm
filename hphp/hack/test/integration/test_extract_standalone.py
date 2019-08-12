from __future__ import absolute_import, unicode_literals

import os
from difflib import ndiff
from sys import stderr

import hh_paths
from common_tests import CommonTestDriver
from test_case import TestCase


class ExtractStandaloneDriver(CommonTestDriver):
    def run_single_typecheck(self, filename: str) -> int:
        _, _, retcode = self.proc_call([hh_paths.hh_single_type_check, filename])
        if retcode != 0:
            print(
                "hh_single_type_check returned non-zero code: {}".format(retcode),
                file=stderr,
            )
        return retcode

    @staticmethod
    def function_to_filename(func) -> str:
        return str.replace(func, "\\", "__")

    def check_extract_standalone(self, function_to_extract: str) -> None:
        generated_code, _, retcode = self.run_check(
            options=["--extract-standalone", function_to_extract]
        )
        if retcode != 0:
            print(
                "hh --extract-standalone {} returned non-zero code: {}".format(
                    function_to_extract, retcode
                ),
                file=stderr,
            )
            assert False

        extracted_file = os.path.join(self.repo_dir, "extracted.php.out")
        # Check if the generated code typechecks
        with open(extracted_file, "w") as f:
            print(generated_code, file=f, flush=True)
        assert self.run_single_typecheck(extracted_file) == 0
        # Check if the generated code is the same as expected
        expected_fname = self.function_to_filename(function_to_extract)
        expected_file = os.path.join(
            self.repo_dir, "expected/{}.php.exp".format(expected_fname)
        )
        with open(expected_file) as expected:
            expected_code = expected.read()
            expected_code = expected_code.strip()
            generated_code = generated_code.strip()
            if expected_code != generated_code:
                print("Generated code is not equal to expected:", file=stderr)
                print(
                    "\n".join(
                        ndiff(expected_code.splitlines(), generated_code.splitlines())
                    ),
                    file=stderr,
                )
            assert expected_code == generated_code


class TestExtractStandalone(TestCase[ExtractStandaloneDriver]):
    @classmethod
    def setUpClass(cls) -> None:
        super().setUpClass()

    @classmethod
    def get_template_repo(cls) -> str:
        return "hphp/hack/test/integration/data/dependencies"

    @classmethod
    def get_test_driver(cls) -> ExtractStandaloneDriver:
        return ExtractStandaloneDriver()

    def test(self) -> None:
        list(
            map(
                self.test_driver.check_extract_standalone,
                [
                    "\\shallow_toplevel",
                    "\\with_typedefs",
                    "\\with_generics",
                    "\\Ns\\same_name_different_namespaces",
                    "\\with_interface",
                    "\\with_overriding",
                    "\\with_enum_and_constant",
                    "\\with_traits",
                    "\\with_requiring_trait",
                ],
            )
        )
