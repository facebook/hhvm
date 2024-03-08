#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import difflib
import os
import re
import shlex
import shutil
import subprocess
import sys
import tempfile
import traceback
import typing
import unittest
from pathlib import Path

import pkg_resources
from thrift.compiler.test import fixture_utils


_THRIFT_BIN_PATH = fixture_utils.get_thrift_binary_path(thrift_bin_arg=None)
assert _THRIFT_BIN_PATH

_FIXTURES_ROOT_DIR_RELPATH = Path("thrift/compiler/test/fixtures")


def _gen_find_recursive_files(top: Path) -> typing.Generator[Path, None, None]:
    """Yields a Path for every file under `top`, relative to it."""

    for root, _, filenames in os.walk(top):
        root_path = Path(root)
        for filename in filenames:
            yield (root_path / filename).relative_to(top)


def _cp_dir(source_root_dir_abspath: Path, dest_root_dir_abspath: Path) -> None:
    """
    Recursively copies the contents of the given directory.

    Args:
        source_root_dir_abspath: Absolute path to the directory to copy (which
          must exist)

        dest_root_dir_abspath: Absolute path to the destination directory. It
          will be created (along with any missing parent directories) if needed.
    """
    assert source_root_dir_abspath.is_absolute()
    assert source_root_dir_abspath.is_dir()
    assert dest_root_dir_abspath.is_absolute()

    for src_file_relpath in _gen_find_recursive_files(source_root_dir_abspath):
        source_file_abspath = source_root_dir_abspath / src_file_relpath
        dest_file_abspath = dest_root_dir_abspath / src_file_relpath

        dest_dir_abspath = dest_file_abspath.parent
        if not dest_dir_abspath.is_dir():
            dest_dir_abspath.mkdir(parents=True)

        shutil.copy2(source_file_abspath, dest_file_abspath)


class FixtureTest(unittest.TestCase):

    MSG = " ".join(
        [
            "One or more fixtures are out of sync with the thrift compiler.",
            "To sync them, build thrift and then run:",
            "`thrift/compiler/test/build_fixtures <build-dir>`, where",
            "<build-dir> is a path where the program `thrift/compiler/thrift`",
            "may be found.",
        ]
    )

    def _compare_code(
        self, gen_code_path: Path, fixture_code_path: Path, cmd: typing.List[str]
    ) -> None:
        """
        Checks that the contents of the files under the two given paths are
        identical, and fails this test if that is not the case.
        """
        gen_file_relpaths = list(_gen_find_recursive_files(gen_code_path))
        fixture_file_relpaths = list(_gen_find_recursive_files(fixture_code_path))

        try:
            # Compare that the generated files are the same
            self.assertEqual(sorted(gen_file_relpaths), sorted(fixture_file_relpaths))

            for gen_file_relpath in gen_file_relpaths:
                gen_file_path = gen_code_path / gen_file_relpath
                fixture_file_path = fixture_code_path / gen_file_relpath
                gen_file_contents = gen_file_path.read_text()
                fixture_file_contents = fixture_file_path.read_text()
                if gen_file_contents == fixture_file_contents:
                    continue

                msg = [f"Difference found in {gen_file_relpath}:"]
                for line in difflib.unified_diff(
                    fixture_file_contents.splitlines(),
                    gen_file_contents.splitlines(),
                    str(fixture_file_path),
                    str(gen_file_path),
                    lineterm="",
                ):
                    msg.append(line)
                msg.append("Command: {}".format(cmd))
                self.fail("\n".join(msg))
        except Exception:
            print(self.MSG, file=sys.stderr)
            traceback.print_exc(file=sys.stderr)
            raise

    def setUp(self) -> None:
        tmp = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, tmp, True)
        self.tmp_dir_abspath = Path(tmp).resolve(strict=True)
        self.maxDiff = None

    def runTest(self, fixture_name: str) -> None:
        fixture_dir_relpath = _FIXTURES_ROOT_DIR_RELPATH / fixture_name

        # Copy required directories to temporary folder.
        self._copy_dir_to_tmp(fixture_dir_relpath / "src")
        self._copy_dir_to_tmp(Path("thrift/annotation/"))
        self._copy_dir_to_tmp(Path("thrift/lib/thrift/"))

        languages = set()
        for cmd in fixture_utils.read_lines(Path(fixture_dir_relpath) / "cmd"):
            # Skip commented out commands
            if cmd[0] == "#":
                continue

            (unique_name, generator_spec, target_filename) = shlex.split(cmd.strip())
            assert re.match(r"^\w+:", unique_name)

            target_file_relpath = fixture_dir_relpath / target_filename

            # Get cmd language
            lang = (
                generator_spec.rsplit(":", 1)[0]
                if ":" in generator_spec
                else generator_spec
            )

            # Add to list of generated languages
            languages.add(lang)

            # Fix cpp args
            if "cpp" in lang or "py3" in lang or "python_capi" in lang:
                # Don't use os.path.join to avoid system-specific path separators.
                path = "thrift/compiler/test/fixtures/" + fixture_name
                extra = "include_prefix=" + path
                join = "," if ":" in generator_spec else ":"
                generator_spec = generator_spec + join + extra

            # Generate arguments to run binary
            build_command_args = [
                _THRIFT_BIN_PATH,
                "-r",
                "-I",
                self.tmp_dir_abspath,
                "-o",
                os.path.join(self.tmp_dir_abspath, fixture_dir_relpath),
                "--gen",
                generator_spec,
                target_file_relpath,
            ]

            # Do not recurse in py generators due to a bug in the py generator
            # Remove once migration to mustache is done
            if (
                ("cpp2" == lang)
                or ("schema" == lang)
                or ("mstch_cpp2" == lang)
                or ("mstch_java" == lang)
                or ("mstch_python" == lang)
            ):
                build_command_args.remove("-r")

            # Run thrift compiler and generate files
            subprocess.check_call(
                build_command_args,
                cwd=self.tmp_dir_abspath,
                close_fds=True,
            )

        # Compare generated code to fixture code
        for lang in languages:
            # Edit lang to find correct directory
            lang = lang.rsplit("_", 1)[0] if "android_lite" in lang else lang
            lang = lang.rsplit("_", 1)[1] if "mstch_" in lang else lang
            lang = "py" if lang == "pyi" else lang

            gen_code_abspath = (
                self.tmp_dir_abspath / fixture_dir_relpath / ("gen-" + lang)
            )

            fixture_code_relpath = fixture_dir_relpath / ("gen-" + lang)

            self._compare_code(
                gen_code_abspath, fixture_code_relpath, build_command_args
            )

    def _copy_dir_to_tmp(self, rel_dir: Path) -> None:
        """
        Recursively copies the given directory (relative to the current working
        directory) to the temporary directory for this test case.
        """
        _cp_dir(Path.cwd() / rel_dir, self.tmp_dir_abspath / rel_dir)


def _add_fixture(klazz, fixture_name: str) -> None:
    def test_method(self):
        self.runTest(fixture_name)

    test_method.__name__ = str("test_" + re.sub("[^0-9a-zA-Z]", "_", fixture_name))
    setattr(klazz, test_method.__name__, test_method)


for fixture_name in fixture_utils.get_all_fixture_names(_FIXTURES_ROOT_DIR_RELPATH):
    _add_fixture(FixtureTest, fixture_name)

if __name__ == "__main__":
    unittest.main()
