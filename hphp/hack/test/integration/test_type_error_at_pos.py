from __future__ import absolute_import, unicode_literals

import json
import os

from common_tests import CommonTestDriver
from test_case import TestCase


class TypeErrorAtPosDriver(CommonTestDriver):

    error_file_ext = ".err"
    auto_namespace_map = '{"PHP": "HH\\\\Lib\\\\PHP"}'
    repo_dir = "hphp/hack/test/integration/data/holes"

    def write_load_config(
        self, use_serverless_ide: bool = False, use_saved_state: bool = False
    ) -> None:
        with open(os.path.join(self.repo_dir, ".hhconfig"), "w") as f:
            f.write(
                """
auto_namespace_map = {}
allowed_fixme_codes_strict = 4101,4323
allowed_decl_fixme_codes = 4101,4323
disable_xhp_element_mangling = false
enable_sound_dynamic_type = true
everything_sdt = true
like_type_hints = true
""".format(
                    self.auto_namespace_map
                )
            )

    def expected_file_name(self, file_name: str, row_num: int, col_num: int) -> str:
        return "{}+{}+{}.err.exp".format(file_name, row_num, col_num)

    def expected_file_path(self, file_name: str, row_num: int, col_num: int) -> str:
        return os.path.join(
            self.repo_dir, self.expected_file_name(file_name, row_num, col_num)
        )

    def expected_type_error(self, file_name: str, row_num: int, col_num: int) -> str:
        with open(
            self.expected_file_path(file_name, row_num, col_num)
        ) as expected_file:
            return expected_file.read().strip()

    def extract_type_error(self, file_name: str, row_num: int, col_num: int) -> str:
        arg = os.path.join(
            self.repo_dir, "{}:{}:{}".format(file_name, row_num, col_num)
        )
        extracted_error, _, retcode = self.run_check(
            options=["--type-error-at-pos", arg, "--json"]
        )
        self.assertEqual(
            0,
            retcode,
            "hh --type-error-at-pos {} returned non-zero code".format(arg),
        )
        return json.dumps(
            json.loads(extracted_error.replace(self.repo_dir, "")), indent=2
        )

    def assert_expected_error_matches_extracted_error(
        self, file_name: str, row_num: int, col_num: int
    ) -> None:
        self.assertMultiLineEqual(
            self.expected_type_error(file_name, row_num, col_num),
            self.extract_type_error(file_name, row_num, col_num),
            f"The expected result of extracting type error in {file_name}, row {row_num}, column {col_num}, doesn't match the extracted ",
        )


class TestTypeErrorAtPos(TestCase[TypeErrorAtPosDriver]):
    @classmethod
    def setUpClass(cls) -> None:
        super().setUpClass()

    @classmethod
    def get_template_repo(cls) -> str:
        return "hphp/hack/test/integration/data/holes"

    @classmethod
    def get_test_driver(cls) -> TypeErrorAtPosDriver:
        return TypeErrorAtPosDriver()

    def test_type_error(self) -> None:
        self.test_driver.write_load_config()

        cases = [
            ("call_single.php", [(7, 10), (10, 10)]),
            ("call_multiple.php", [(7, 5), (7, 8)]),
            ("call_unpack.php", [(7, 8)]),
            ("return_expr_only.php", [(5, 10), (5, 21)]),
            ("return_and_fn_arg.php", [(5, 10), (5, 21)]),
        ]

        for file_name, positions in cases:
            for row_num, col_num in positions:
                with self.subTest(msg=f"{file_name}:{row_num}:{col_num}"):
                    self.test_driver.assert_expected_error_matches_extracted_error(
                        file_name, row_num, col_num
                    )
