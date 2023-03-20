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

    def codemod_sdt(self, codemod_jsonl: str, codemod_sdt_args: List[str]) -> None:
        with tempfile.NamedTemporaryFile(mode="w", encoding="utf-8") as f:
            f.write(codemod_jsonl)
            f.flush()
            codemod_file = f.name

            self.start_hh_server()
            self.check_cmd(
                expected_output=None,
                options=["--codemod-sdt", codemod_file, *codemod_sdt_args],
            )

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
        self.codemod_sdt(codemod_jsonl, ["cumulative-groups"])
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
