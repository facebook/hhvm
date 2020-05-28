#!/usr/bin/env python3
# pyre-strict

import os.path
import subprocess
import tempfile
from typing import List, Tuple, cast

from hh_fanout_test_driver import (
    Env,
    Path,
    SavedStateInfo,
    generate_saved_state,
    run_hh_fanout,
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
            self.assertSetEqual(
                set(result_files),
                {
                    file("foo.php"),
                    # Currently, `deleted.php` is included in the fanout. This
                    # is probably a bug. It no longer exists on disk, so it
                    # shouldn't be included in the fanout. It may be resolved
                    # once we unify naming and dependency hashes (T64327364).
                    file("deleted.php"),
                },
            )

    def test_cursor_advance(self) -> None:
        work_dir: str
        with tempfile.TemporaryDirectory() as work_dir:

            def file(path: Path) -> Path:
                return os.path.join(work_dir, path)

            (env, saved_state_info) = self.set_up_work_dir(work_dir)
            self.write(
                file("foo.php"),
                """<?hh
function foo(): void {}
""",
            )
            self.write(
                file("uses_foo.php"),
                """<?hh
function uses_foo(): void {
    // Doesn't yet use foo
}
""",
            )

            result = run_hh_fanout(
                env=env,
                saved_state_info=saved_state_info,
                changed_files=[file("foo.php"), file("uses_foo.php")],
                args=[file("foo.php")],
                cursor=None,
            )
            result_files = cast(List[str], result["files"])
            self.assertSetEqual(set(result_files), {file("foo.php")})
            cursor1 = result.get("cursor")
            self.assertIsNotNone(cursor1)
            cursor1 = cast(str, cursor1)

            self.write(
                file("uses_foo.php"),
                """<?hh
function uses_foo(): void {
    foo();
}
""",
            )
            result = run_hh_fanout(
                env=env,
                saved_state_info=saved_state_info,
                changed_files=[file("uses_foo.php")],
                args=[file("foo.php")],
                cursor=cursor1,
            )
            result_files = cast(List[str], result["files"])
            self.assertSetEqual(
                set(result_files), {file("foo.php"), file("uses_foo.php")}
            )
            cursor2 = result.get("cursor")
            self.assertIsNotNone(cursor2)
            cursor2 = cast(str, cursor2)
            self.assertGreater(cursor2, cursor1)

    def test_invalid_cursor(self) -> None:
        work_dir: str
        with tempfile.TemporaryDirectory() as work_dir:

            def file(path: Path) -> Path:
                return os.path.join(work_dir, path)

            (env, saved_state_info) = self.set_up_work_dir(work_dir)
            self.write(
                file("foo.php"),
                """<?hh
""",
            )
            with self.assertRaises(subprocess.CalledProcessError):
                result = run_hh_fanout(
                    env=env,
                    saved_state_info=saved_state_info,
                    changed_files=["foo.php"],
                    args=["foo.php"],
                    cursor="nonexistent",
                )
                print(result)
