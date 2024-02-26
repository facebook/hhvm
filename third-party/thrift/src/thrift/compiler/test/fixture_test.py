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
import unittest

import pkg_resources
from thrift.compiler.test import fixture_utils

FIXTURE_ROOT = "."


thrift = fixture_utils.get_thrift_binary_path(thrift_bin_arg=None)
assert thrift
fixtures_root_dir = os.path.join(FIXTURE_ROOT, "thrift/compiler/test/fixtures")


def read_file(path):
    with open(path, "r") as f:
        return f.read()


def read_directory_filenames(path):
    files = []
    for filename in os.listdir(path):
        files.append(filename)
    return files


def gen_find_recursive_files(path):
    for root, _, files in os.walk(path):
        for f in files:
            yield os.path.relpath(os.path.join(root, f), path)


def cp_dir(source_dir, dest_dir):
    for src in gen_find_recursive_files(source_dir):
        source_full_path = os.path.join(source_dir, src)
        dest_full_path = os.path.join(dest_dir, src)

        dest_full_dir = os.path.dirname(dest_full_path)
        if not os.path.isdir(dest_full_dir):
            os.makedirs(dest_full_dir, 0o700)

        shutil.copy2(source_full_path, dest_full_path)


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

    def compare_code(self, path1, path2, cmd):
        gens = list(gen_find_recursive_files(path1))
        fixt = list(gen_find_recursive_files(path2))
        try:
            # Compare that the generated files are the same
            self.assertEqual(sorted(gens), sorted(fixt))
            for gen in gens:
                geng_path = os.path.join(path1, gen)
                genf_path = os.path.join(path2, gen)
                geng = read_file(geng_path)
                genf = read_file(genf_path)
                if geng == genf:
                    continue

                msg = ["Difference found in " + gen + ":"]
                for line in difflib.unified_diff(
                    genf.splitlines(),
                    geng.splitlines(),
                    genf_path,
                    geng_path,
                    lineterm="",
                ):
                    msg.append(line)
                msg.append("Command: {}".format(cmd))
                self.fail("\n".join(msg))
        except Exception:
            print(self.MSG, file=sys.stderr)
            traceback.print_exc(file=sys.stderr)
            raise

    def setUp(self):
        tmp = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, tmp, True)
        self.tmp = tmp
        self.maxDiff = None

    def runTest(self, name):
        fixture_dir = os.path.join(fixtures_root_dir, name)

        # Copy source *.thrift files to temporary folder for relative code gen
        cp_dir(
            os.path.join(fixture_dir, "src"), os.path.join(self.tmp, fixture_dir, "src")
        )
        # Copy thrift/annotation/ folder to temporary folder
        cp_dir(
            os.path.join(FIXTURE_ROOT, "thrift/annotation"),
            os.path.join(self.tmp, "thrift/annotation"),
        )
        # Copy thrift/lib/thrift/ folder to temporary folder
        cp_dir(
            os.path.join(FIXTURE_ROOT, "thrift/lib/thrift/"),
            os.path.join(self.tmp, "thrift/lib/thrift/"),
        )
        languages = set()
        for cmd in fixture_utils.read_lines(os.path.join(fixture_dir, "cmd")):
            # Skip commented out commands
            if cmd[0] == "#":
                continue

            args = shlex.split(cmd.strip())
            args[1] = os.path.relpath(os.path.join(fixture_dir, args[1]), FIXTURE_ROOT)
            # Get cmd language
            lang = args[0].rsplit(":", 1)[0] if ":" in args[0] else args[0]

            # Add to list of generated languages
            languages.add(lang)

            # Fix cpp args
            if "cpp" in lang:
                # Don't use os.path.join to avoid system-specific path separators.
                path = "thrift/compiler/test/fixtures/" + name
                extra = "include_prefix=" + path
                join = "," if ":" in args[0] else ":"
                args[0] = args[0] + join + extra

            # Generate arguments to run binary
            args = [
                thrift,
                "-r",
                "-I",
                self.tmp,
                "-o",
                os.path.join(self.tmp, fixture_dir),
                "--gen",
                args[0],
                *args[1:],
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
                args.remove("-r")

            # Run thrift compiler and generate files
            subprocess.check_call(
                args, cwd=os.path.join(self.tmp, FIXTURE_ROOT), close_fds=True
            )

        # Compare generated code to fixture code
        for lang in languages:
            # Edit lang to find correct directory
            lang = lang.rsplit("_", 1)[0] if "android_lite" in lang else lang
            lang = lang.rsplit("_", 1)[1] if "mstch_" in lang else lang
            lang = "py" if lang == "pyi" else lang

            gen_code = os.path.join(self.tmp, fixture_dir, "gen-" + lang)
            fixture_code = os.path.join(fixture_dir, "gen-" + lang)
            self.compare_code(gen_code, fixture_code, args)


def add_fixture(klazz, name):
    def test_method(self):
        self.runTest(name)

    test_method.__name__ = str("test_" + re.sub("[^0-9a-zA-Z]", "_", name))
    setattr(klazz, test_method.__name__, test_method)


fixtureNames = read_directory_filenames(fixtures_root_dir)
for name in fixtureNames:
    add_fixture(FixtureTest, name)

if __name__ == "__main__":
    unittest.main()
