import os
import tempfile
from typing import List

import common_tests


class TestCodemodSdt(common_tests.CommonTestDriver):
    @classmethod
    def setUpClass(cls):
        super().setUpClass(template_repo="hphp/hack/test/integration/data/codemod_sdt/")

    def filename(self):
        return os.path.join(self.repo_dir, "integration_test_codemod_sdt1.php")

    def codemod_sdt(self, codemod_jsonl: str, codemod_sdt_args: List[str]) -> str:
        with tempfile.NamedTemporaryFile(mode="w", encoding="utf-8") as f:
            f.write(codemod_jsonl)
            f.flush()
            codemod_file = f.name

            self.start_hh_server()
            log_remotely_arg = "false"
            stdout, stderr, ret_code = self.run_check(
                options=[
                    "--codemod-sdt",
                    codemod_file,
                    log_remotely_arg,
                    *codemod_sdt_args,
                ],
            )
            if ret_code != 0:
                self.fail(f"codemod failed: {ret_code=} {stderr=}")
            return stdout

    def expect_contents(self, expected):
        with open(self.filename(), "r") as f:
            actual = f.read()
            self.assertEqual(actual, expected)

    def test_codemod(self) -> None:
        # jsonl produced by sdt_analysis_exe
        codemod_jsonl = """
{"entry_kind":"stats","id_cnt":3,"nadable_cnt":3,"syntactically_nadable_cnt":3}
{"entry_kind":"add_no_auto_dynamic_attr","items":[{"kind":"((Cclass Concrete))","path":"integration_test_codemod_sdt1.php","sid":"\\C"},{"kind":"(Cinterface)","path":"integration_test_codemod_sdt1.php","sid":"\\I"}]}
{"entry_kind":"add_no_auto_dynamic_attr","items":[{"kind":"((Cclass Concrete))","path":"integration_test_codemod_sdt1.php","sid":"\\D"},{"kind":"(Cinterface)","path":"integration_test_codemod_sdt1.php","sid":"\\I"}]}
        """.strip()
        stdout = self.codemod_sdt(codemod_jsonl, ["the-tag", "cumulative-groups"])
        self.assertIn('"patches_json"', stdout, msg="logging probably works")
        self.assertIn('"the-tag"', stdout, msg="logging probably works")
        self.expect_contents(
            """<?hh

<<__NoAutoDynamic>>
interface I {
  public function foo(vec<int> $_): void;
}

<<__NoAutoDynamic>>
final class C implements I {
  public function foo(vec<int> $_): void {}
}

<<__NoAutoDynamic>>
final class D implements I {
  public function foo(vec<int> $_): void {}
}
"""
        )
