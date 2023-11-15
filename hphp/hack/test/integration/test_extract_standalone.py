from __future__ import absolute_import, unicode_literals

import os
from sys import stderr
from typing import List

from common_tests import CommonTestDriver
from test_case import TestCase

hh_single_type_check = os.path.abspath(os.getenv("HH_SINGLE_TYPE_CHECK", "<undefined>"))
# we expect env var to be repo-relative, so abspath will find it relative to CWD which is repo-root
if not os.path.exists(hh_single_type_check):
    print("Not found HH_SINGLE_TYPE_CHECK=" + hh_single_type_check, file=stderr)
    exit(1)


class ExtractStandaloneDriver(CommonTestDriver):

    UPDATE_TESTS = False

    error_file_ext = ".err"
    auto_namespace_map = '{"PHP": "HH\\\\Lib\\\\PHP"}'

    def write_load_config(
        self, use_serverless_ide: bool = False, use_saved_state: bool = False
    ) -> None:
        with open(os.path.join(self.repo_dir, ".hhconfig"), "w") as f:
            f.write(
                """
auto_namespace_map = {}
allowed_fixme_codes_strict = 4101
allowed_decl_fixme_codes = 4101
disable_xhp_element_mangling = false
""".format(
                    self.auto_namespace_map
                )
            )

    def expected_file_name(self, function_name: str) -> str:
        return "expected/{}.php.exp".format(
            function_name.replace(":", "+").replace("\\", "__")
        )

    def expected_file_path(self, function_name: str) -> str:
        return os.path.join(self.repo_dir, self.expected_file_name(function_name))

    def expected_code(self, function_name: str) -> str:
        with open(self.expected_file_path(function_name)) as expected_file:
            return expected_file.read().strip()

    def expected_code_type_errors(self, function_name: str) -> str:
        with open(
            self.expected_file_path(function_name) + self.error_file_ext
        ) as error_file:
            return error_file.read().strip()

    def extract_code(self, function_name: str) -> str:
        extracted_code, _, retcode = self.run_check(
            options=["--extract-standalone", function_name]
        )
        self.assertEqual(
            0,
            retcode,
            "hh --extract-standalone {} returned non-zero code".format(function_name),
        )
        result = extracted_code.strip()
        if self.UPDATE_TESTS:
            expected_file_path = os.path.join(
                os.getcwd(),
                TestExtractStandalone.get_template_repo(),
                self.expected_file_name(function_name),
            )
            with open(expected_file_path, "w") as expected_file:
                expected_file.write(result)
                expected_file.write("\n")
        return result

    def assert_expected_code_matches_extracted_code(self, function_name: str) -> None:
        self.assertMultiLineEqual(
            self.expected_code(function_name),
            self.extract_code(function_name),
            f"The expected result of extracting {function_name} doesn't match the extracted code",
        )

    def type_check_expected_files(self, function_names: List[str]) -> None:
        files = [
            self.expected_file_path(function_name) for function_name in function_names
        ]
        self.proc_call(
            [
                hh_single_type_check,
                "--allowed-fixme-codes-strict",
                "4101",
                "--allowed-decl-fixme-codes",
                "4101",
                "--auto-namespace-map",
                self.auto_namespace_map,
                "--batch-files",
                "--out-extension",
                self.error_file_ext,
            ]
            + files
        )

    def assert_expected_code_is_well_typed(self, function_name: str) -> None:
        self.assertMultiLineEqual(
            "No errors",
            self.expected_code_type_errors(function_name),
            f"The expected result of extracting {function_name} has type errors",
        )


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

        function_names = [
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
            "\\WithReactiveMethods::call_reactive",
            "\\frob_query",
            "\\corge",
            "\\with_implementations",
            "\\with_constructor_dependency",
            "\\kwery",
            "\\with_newtype_with_bound",
            "\\with_newtype_with_newtype_bound",
            "\\with_method_defined_in_trait",
            "\\with_method_defined_in_trait2",
            "\\ImplementingBase::must_implement",
            "\\CCC::with_nested_type_access",
            "\\TFlob::g",
            "\\Derived::overridden",
            "\\with_xhp",
            "\\WithOptionalConstructorArguments::get",
            "\\TExtendsWithConsistentConstruct::get",
            "\\with_IEWGPCOUP",
            "\\with_contra_tparam",
            "\\WithLateInit::getCount",
            "\\TFlobby::g",
            "\\:foo::render",
            "\\with_unsafe_type_hh_fixme",
            "\\with_reified_generics",
            "\\SealedInterface::method",
            "\\WithTypeAliasHint::getX",
            "\\respects_newtype_abstraction",
            "\\function_in_typedef",
            "\\contexts_in_typedef",
            "\\with_argument_dependent_context",
            "\\Contextual::with_argument_dependent_context",
            "\\WithContextConstant::has_io",
            "\\with_optional_argument_dependent_context",
            "\\with_expr_in_user_attrs",
            "\\with_arg_with_sealed_whitelist",
            "\\with_user_attr",
            "\\with_param_with_user_attr",
            "\\with_tparam_with_user_attr",
            "\\WithPropWithUserAttr::foo",
            "\\WithStaticPropWithUserAttr::foo",
            "\\WithTypeConstantWithUserAttr::foo",
            "\\WithMethodWithUserAttr::foo",
            "\\WithUserAttr::foo",
            "\\enum_with_user_attr",
            "\\opaque_with_user_attr",
            "\\transparent_with_user_attr",
            "\\with_constr_prop_with_user_attr",
            "\\with_where_constraint",
            "\\with_open_shape",
            "\\TestExtractConstruct::__construct",
            "\\with_escaped_char_in_attr",
            "\\with_class_name_in_attr",
            "\\with_tparam_constraint",
            "\\with_prop_in_construct",
            "\\WithTypeConstantParamConstraint::foo",
        ]

        for function_name in function_names:
            with self.subTest(msg=function_name):
                self.test_driver.assert_expected_code_matches_extracted_code(
                    function_name
                )

        self.test_driver.type_check_expected_files(function_names)

        for function_name in function_names:
            with self.subTest(msg=function_name):
                self.test_driver.assert_expected_code_is_well_typed(function_name)

    def test_failing(self) -> None:
        self.test_driver.write_load_config()

        self.test_driver.assert_expected_code_matches_extracted_code(
            "\\nonexistent_function"
        )
        self.test_driver.assert_expected_code_matches_extracted_code(
            "\\nonexistent_dependency"
        )
