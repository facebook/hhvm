from __future__ import absolute_import, unicode_literals

import os
from sys import stderr

import hh_paths
from common_tests import CommonTestDriver
from test_case import TestCase


auto_namespace_map = '{"PHP": "HH\\\\Lib\\\\PHP"}'


class ExtractStandaloneDriver(CommonTestDriver):
    def write_load_config(self, use_saved_state: bool = False) -> None:
        with open(os.path.join(self.repo_dir, ".hhconfig"), "w") as f:
            f.write(
                """
auto_namespace_map = {}
""".format(
                    auto_namespace_map
                )
            )

    def run_single_typecheck(self, filename: str) -> int:
        _, _, retcode = self.proc_call(
            [
                hh_paths.hh_single_type_check,
                "--auto-namespace-map",
                auto_namespace_map,
                filename,
            ]
        )
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
        expected_file = "expected/{}.php.exp".format(fname_expected)
        expected_file_absolute = os.path.join(self.repo_dir, expected_file)
        with open(expected_file_absolute) as expected:
            expected = expected.read().strip()
            output = output.strip()
            self.assertMultiLineEqual(
                output, expected, f"Mismatch in file {expected_file}"
            )

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
            raise AssertionError()

        extracted_file = os.path.join(self.repo_dir, "extracted.php.out")
        # Check if the generated code is the same as expected
        expected_fname = self.function_to_filename(function_to_extract)
        self.assert_output_matches(generated_code, expected_fname)
        # Check if the generated code typechecks
        if typecheck:
            with open(extracted_file, "w") as f:
                print(generated_code, file=f, flush=True)
            assert self.run_single_typecheck(extracted_file) == 0

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
        self.test_driver.write_load_config()

        paths = [
            "\\shallow_toplevel",
            "\\with_typedefs",
            "\\with_generics",
            "\\with_generics_with_bounds",
            "\\Ns\\same_name_different_namespaces",
            "\\with_interface",
            "\\with_overriding",
            "\\with_enum_and_constant",
            "\\with_traits",
            "\\with_requiring_trait",
            "\\with_nontrivial_fun_decls",
            "\\call_defaulted",
            "\\call_with_default_and_variadic",
            "\\call_with_default_and_anonymous_variadic",
            "\\use_properties",
            "\\call_constructors",
            "\\with_constants",
            "\\SimpleClass::simple_method",
            "\\with_type_constants",
            "\\WithAbstractConst::with_abstract_type_constants",
            "\\WithConst::with_type_constants",
            "\\with_bounded_generic_class_tparam",
            "\\with_generic_type",
            "\\with_generic_method",
            "\\builtin_argument_types",
            "\\with_static_property",
            "\\SimpleDerived::call_parent_method",
            "\\recursive_function",
            "\\WithRecursiveMethods::recursive_static",
            "\\WithRecursiveMethods::recursive_instance",
            "\\does_not_use_class_methods",
            "\\with_requiring_interface",
            "\\with_generic_interface",
            "\\with_non_generic_type",
            "\\with_mapped_namespace",
            "\\WithNameMatchingClassName",
            "\\with_generic_method_with_wildcard_tparam",
            "\\with_is_refinement",
            "\\with_switch",
            "\\with_classname",
            "\\with_parent_constructor_call",
            "\\with_type_const_from_required_interface",
            "\\with_built_in_constant",
            "\\with_shape_type_alias",
            "\\with_enum_type_alias",
            "\\with_enum_class_name",
            "\\with_type_const_from_implemented_interface",
            "\\with_nested_type_const",
            "\\with_indirect_require_extends",
            "\\call_reactive",
            "\\call_shallow_reactive",
            "\\WithReactiveMethods::call_reactive",
            "\\WithReactiveMethods::call_shallow_reactive",
            "\\frob_query",
            "\\corge",
        ]

        for path in paths:
            with self.subTest(path=path):
                self.test_driver.check_extract_standalone(path)

    def test_failing(self) -> None:
        self.test_driver.write_load_config()
        self.test_driver.check_failing("\\nonexistent_function")
        self.test_driver.check_failing("\\nonexistent_dependency")
