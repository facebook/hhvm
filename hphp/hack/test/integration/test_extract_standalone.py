from __future__ import absolute_import, unicode_literals

import os
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

    def check_extract_standalone(self, function_to_extract: str) -> int:
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
            return retcode

        # Write the generated code to stderr to debug
        print(generated_code, file=stderr)
        extracted_file = os.path.join(self.repo_dir, "extracted.php")
        # TODO: we shouldn't write the generated code to a file because it adds
        # repeated declarations. A better approach is reading from stdin in
        # hh_single_type_check.
        with open(extracted_file, "w") as f:
            print(generated_code, file=f)
        return self.run_single_typecheck(extracted_file)


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

    def test_extract_toplevel(self) -> None:
        assert self.test_driver.check_extract_standalone("\\f") == 0

    def test_extract_with_typedef(self) -> None:
        assert self.test_driver.check_extract_standalone("\\with_types") == 0

    def test_extract_with_generic_deps(self) -> None:
        assert self.test_driver.check_extract_standalone("\\h") == 0
