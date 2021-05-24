# pyre-strict
from __future__ import absolute_import, unicode_literals

import json
import os

from common_tests import CommonTestDriver
from test_case import TestCase


class TastHolesDriver(CommonTestDriver):

    error_file_ext = ".holes"
    auto_namespace_map = '{"PHP": "HH\\\\Lib\\\\PHP"}'
    repo_dir = "hphp/hack/test/integration/data/holes"

    def write_load_config(
        self, use_serverless_ide: bool = False, use_saved_state: bool = False
    ) -> None:
        with open(os.path.join(self.repo_dir, ".hhconfig"), "w") as f:
            f.write(
                """
auto_namespace_map = {}
allowed_fixme_codes_strict = 4101,4323,4110
allowed_decl_fixme_codes = 4101,4323,4110
disable_xhp_element_mangling = false
""".format(
                    self.auto_namespace_map
                )
            )

    def expected_file_name(self, file_name: str) -> str:
        return "{}.holes.exp".format(file_name)

    def expected_file_path(self, file_name: str) -> str:
        return os.path.join(self.repo_dir, self.expected_file_name(file_name))

    def expected_type_error(self, file_name: str) -> str:
        with open(self.expected_file_path(file_name)) as expected_file:
            return expected_file.read().strip()

    def extract_type_error(self, file_name: str) -> str:
        arg = os.path.join(self.repo_dir, file_name)
        extracted_error, err, retcode = self.run_check(
            options=["--tast-holes", arg, "--json"]
        )
        self.assertEqual(
            0,
            retcode,
            "hh --tast-holes {} returned non-zero code: {}".format(file_name, err),
        )
        return json.dumps(
            json.loads(extracted_error.replace(self.repo_dir, "")), indent=2
        )

    def assert_expected_error_matches_extracted_error(self, file_name: str) -> None:
        self.assertMultiLineEqual(
            self.expected_type_error(file_name),
            self.extract_type_error(file_name),
            f"The expected result of extracting tast Holes from {file_name} doesn't match the extracted ",
        )


class TestTastHoles(TestCase[TastHolesDriver]):
    @classmethod
    def setUpClass(cls) -> None:
        super().setUpClass()

    @classmethod
    def get_template_repo(cls) -> str:
        return "hphp/hack/test/integration/data/holes"

    @classmethod
    def get_test_driver(cls) -> TastHolesDriver:
        return TastHolesDriver()

    def test_type_error(self) -> None:
        self.test_driver.write_load_config()

        cases = [
            "call_single.php",
            "call_multiple.php",
            "call_unpack.php",
            "return_expr_only.php",
            "return_and_fn_arg.php",
            "member_call_multiple.php",
            "coincident_holes.php",
        ]

        for file_name in cases:
            with self.subTest(msg=f"{file_name}"):
                self.test_driver.assert_expected_error_matches_extracted_error(
                    file_name
                )
