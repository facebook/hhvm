import os

import common_tests


class TestSafeRenameSoundDynamic(common_tests.CommonTestDriver):
    @classmethod
    def setUpClass(cls):
        super().setUpClass(
            template_repo="hphp/hack/test/integration/data/lsp_exchanges/"
        )

    def filename(self):
        return os.path.join(self.repo_dir, "a.php")

    def write_hh_config(self):
        with open(os.path.join(self.repo_dir, ".hhconfig"), "w") as f:
            f.write(
                """
like_type_hints = true
enable_sound_dynamic_type = true
union_intersection_type_hints = true
everything_sdt = true
"""
            )

    def run_hh(self, file_input) -> None:
        with open(self.filename(), "w") as f:
            f.write(file_input)
        self.write_hh_config()
        self.start_hh_server(changed_files=["a.php"], args=["--no-load"])

    def expect_contents(self, expected):
        with open(self.filename(), "r") as f:
            actual = f.read()
            self.assertEqual(actual, expected)

    def rename(self, line, col, new_name):
        self.check_cmd(
            expected_output=None,
            options=["--ide-refactor", f"{self.filename()}:{line}:{col}:{new_name}"],
        )

    def test_no_auto_dynamic(self) -> None:
        self.run_hh(
            """<?hh
<<__NoAutoDynamic>>
class Nad {
    public static function smeth(): void {}

    public function meth1(): void {}

    <<__NoAutoDynamic, __SupportDynamicType>>
    public function meth2(): void {}

    <<__SupportDynamicType>>
    public function meth3(): void {}
}
"""
        )
        # rename later lines first to avoid line numbers interfering. `<<__Deprecated>>`s can add lines
        self.rename(12, 23, "meth3b")
        self.rename(9, 23, "meth2b")
        self.rename(6, 23, "meth1b")
        self.rename(4, 28, "smethb")
        self.expect_contents(
            """<?hh
<<__NoAutoDynamic>>
class Nad {
    public static function smethb(): void {}

    public function meth1b(): void {}

    <<__Deprecated("Use `meth2b` instead")>>
    public function meth2(): void {
      $this->meth2b();
    }

    <<__NoAutoDynamic, __SupportDynamicType>>
    public function meth2b(): void {}

    <<__Deprecated("Use `meth3b` instead")>>
    public function meth3(): void {
      $this->meth3b();
    }

    <<__SupportDynamicType>>
    public function meth3b(): void {}
}
"""
        )

    def test_implicit_pess(self) -> None:
        self.run_hh(
            """<?hh

class Sd {
    public static function smeth(): void {}

    public function meth1(): void {}

    <<__NoAutoDynamic, __SupportDynamicType>>
    public function meth2(): void {}

    <<__SupportDynamicType>>
    public function meth3(): void {}

    <<__NoAutoDynamic>>
    public function meth4(): void {}
}
"""
        )
        # rename later lines first to avoid line numbers interfering. `<<__Deprecated>>`s can add lines
        self.rename(15, 23, "meth4b")
        self.rename(12, 23, "meth3b")
        self.rename(9, 23, "meth2b")
        self.rename(6, 23, "meth1b")
        self.rename(4, 28, "smethb")

        self.expect_contents(
            """<?hh

class Sd {

    <<__Deprecated("Use `smethb` instead")>>
    public static function smeth(): void {
      self::smethb();
    }
    public static function smethb(): void {}

    <<__Deprecated("Use `meth1b` instead")>>
    public function meth1(): void {
      $this->meth1b();
    }

    public function meth1b(): void {}

    <<__Deprecated("Use `meth2b` instead")>>
    public function meth2(): void {
      $this->meth2b();
    }

    <<__NoAutoDynamic, __SupportDynamicType>>
    public function meth2b(): void {}

    <<__Deprecated("Use `meth3b` instead")>>
    public function meth3(): void {
      $this->meth3b();
    }

    <<__SupportDynamicType>>
    public function meth3b(): void {}

    <<__Deprecated("Use `meth4b` instead")>>
    public function meth4(): void {
      $this->meth4b();
    }

    <<__NoAutoDynamic>>
    public function meth4b(): void {}
}
"""
        )
