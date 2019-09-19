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
        func = func.replace("::", "++")
        return func.replace("\\", "__")

    def assert_output_matches(self, output: str, fname_expected: str) -> None:
        expected_file = os.path.join(
            self.repo_dir, "expected/{}.php.exp".format(fname_expected)
        )
        with open(expected_file) as expected:
            expected = expected.read().strip()
            output = output.strip()
            self.assertMultiLineEqual(output, expected)

    def check_extract_standalone(
        self, function_to_extract: str, typecheck=True
    ) -> None:
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
        if typecheck:
            with open(extracted_file, "w") as f:
                print(generated_code, file=f, flush=True)
            assert self.run_single_typecheck(extracted_file) == 0
        # Check if the generated code is the same as expected
        expected_fname = self.function_to_filename(function_to_extract)
        self.assert_output_matches(generated_code, expected_fname)

    def check_failing(self, function_to_extract: str) -> None:
        self.check_extract_standalone(function_to_extract, typecheck=False)


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

    def test_extract(self) -> None:
        paths = [
            "\\shallow_toplevel",
            "\\with_typedefs",
            "\\with_generics",
            "\\Ns\\same_name_different_namespaces",
            "\\with_interface",
            "\\with_overriding",
            "\\with_enum_and_constant",
            "\\with_traits",
            "\\with_requiring_trait",
            "\\with_nontrivial_fun_decls",
            "\\call_defaulted",
            "\\use_properties",
            "\\call_constructors",
            "\\with_constants",
            "\\SimpleClass::simple_method",
            "\\with_type_constants",
            "\\WithAbstractConst::with_abstract_type_constants",
            "\\WithConst::with_type_constants",
            "\\with_generic_type",
            "\\with_generic_method",
            "\\builtin_argument_types",
        ]

        for path in paths:
            with self.subTest(path=path):
                self.test_driver.check_extract_standalone(path)

    def test_failing(self) -> None:
        self.test_driver.check_failing("\\nonexistent_function")
        self.test_driver.check_failing("\\nonexistent_dependency")
