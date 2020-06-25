#!/usr/bin/env python3
# pyre-strict

import os.path
import tempfile
from typing import List, Mapping, Tuple, cast

from hh_fanout_test_driver import (
    Env,
    Path,
    SavedStateInfo,
    generate_saved_state,
    run_hh_fanout,
    run_hh_fanout_calculate_errors,
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
            # `uses_foo.php` should not be in the fanout of `foo.php` yet,
            # since we haven't done a typecheck of it, so we don't know about
            # the new dependency edge from `foo.php` to `uses_foo.php`.
            self.assertSetEqual(set(result_files), {file("foo.php")})

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
            try:
                result = run_hh_fanout(
                    env=env,
                    saved_state_info=saved_state_info,
                    changed_files=["foo.php"],
                    args=["foo.php"],
                    cursor="nonexistent",
                )
                print(result)
                self.fail("Should have failed to find the cursor ID")
            except RuntimeError as e:
                self.assertIn("Cursor with ID nonexistent not found", str(e))

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

    def test_calculate_errors(self) -> None:
        work_dir: str
        with tempfile.TemporaryDirectory() as work_dir:

            def file(path: Path) -> Path:
                return os.path.join(work_dir, path)

            (env, saved_state_info) = self.set_up_work_dir(work_dir)
            self.write(
                file("foo.php"),
                """<?hh
function foo(): void {
    return 1;
}
""",
            )

            result = run_hh_fanout(
                env=env,
                saved_state_info=saved_state_info,
                changed_files=[file("foo.php")],
                args=[file("foo.php")],
                cursor=None,
            )
            cursor1 = cast(str, result["cursor"])

            result = run_hh_fanout_calculate_errors(
                env=env, saved_state_info=saved_state_info, cursor=cursor1
            )
            cursor2 = cast(str, result["cursor"])
            errors = cast(
                Mapping[int, Mapping[str, Mapping[int, Mapping[str, str]]]],
                result["errors"],
            )
            self.assertGreater(cursor2, cursor1)
            self.assertNotEmpty(errors)
            self.assertIn("You cannot return a value", errors[0]["message"][0]["descr"])

            result = run_hh_fanout_calculate_errors(
                env=env, saved_state_info=saved_state_info, cursor=cursor2
            )
            cursor3 = result["cursor"]
            self.assertEqual(cursor3, cursor2)
