#!/usr/bin/env python3
# pyre-strict

import os.path
import tempfile
from typing import cast, List, Tuple

from hh_fanout_test_driver import (
    Env,
    generate_saved_state,
    Path,
    run_hh_fanout,
    SavedStateInfo,
)
from libfb.py.testutil import BaseFacebookTestCase


class TestHhFanout(BaseFacebookTestCase):
    @property
    def hh_fanout_path(self) -> str:
        return os.environ["HH_FANOUT"]

    @property
    def hh_server_path(self) -> str:
        return os.environ["HH_SERVER"]

    def set_up_work_dir(self, root_dir: Path) -> Tuple[Env, SavedStateInfo]:
        with open(os.path.join(root_dir, ".hhconfig"), "w") as f:
            f.write("")

        env = Env(
            root_dir=root_dir,
            hh_fanout_path=self.hh_fanout_path,
            hh_server_path=self.hh_server_path,
        )
        saved_state_info = generate_saved_state(env, target_dir=root_dir)
        return (env, saved_state_info)

    def write(self, path: Path, contents: str) -> None:
        with open(path, "w") as f:
            f.write(contents)

    def delete(self, path: Path) -> None:
        os.unlink(path)

    def test_deleted_file(self) -> None:
        work_dir: str
        with tempfile.TemporaryDirectory() as work_dir:

            def file(path: Path) -> Path:
                return os.path.join(work_dir, path)

            self.write(
                file("foo.php"),
                """<?hh
function foo(): void {}
""",
            )
            self.write(
                file("deleted.php"),
                """<?hh
function deleted(): void {
    foo();
}
""",
            )
            (env, saved_state_info) = self.set_up_work_dir(work_dir)
            self.delete(file("deleted.php"))
            result = run_hh_fanout(
                env=env,
                saved_state_info=saved_state_info,
                changed_files=[file("deleted.php")],
                args=[file("foo.php")],
                cursor=None,
            )
            result_files = cast(List[str], result["files"])
            self.assertSetEqual(set(result_files), {file("foo.php")})

    def test_filter_hack_files(self) -> None:
        work_dir: str
        with tempfile.TemporaryDirectory() as work_dir:

            def file(path: Path) -> Path:
                return os.path.join(work_dir, path)

            (env, saved_state_info) = self.set_up_work_dir(work_dir)
            self.write(
                file("foo.json"),
                """
{"changes":["/foo/bar.php",
"saved-state.json"],
"corresponding_base_revision":"-1",
"state":"hh_mini_saved_state",
"deptable":"hh_mini_saved_state.sql"}
""",
            )
            self.write(
                file("foo.php"),
                """<?hh
function foo(): void {}
""",
            )
            result = run_hh_fanout(
                env=env,
                saved_state_info=saved_state_info,
                changed_files=[file("foo.json"), file("foo.php")],
                args=[file("foo.json"), file("foo.php")],
                cursor=None,
            )
            result_files = cast(List[str], result["files"])
            self.assertEqual(set(result_files), {file("foo.php")})
