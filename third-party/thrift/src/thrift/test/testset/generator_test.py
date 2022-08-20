#!/usr/bin/env python3
# Copyright (c) Facebook, Inc. and its affiliates.
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
import shutil
import sys
import tempfile
import traceback
import unittest

from thrift.test.testset import generator


golden_root_dir = os.getenv("THRIFT_GOLDEN_DIR")


def read_file(path):
    with open(path, "r") as f:
        return f.read()


def gen_find_recursive_files(path):
    for root, _, files in os.walk(path):
        for f in files:
            yield os.path.relpath(os.path.join(root, f), path)


class GoldenTest(unittest.TestCase):

    MSG = """One or more testset outputs are out of sync with the generator.
To sync them, run:
  thrift/test/testset/generator.py --install_dir ./thrift/test/testset/golden
"""

    def compare_code(self, path1, path2):
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
                    geng.splitlines(),
                    genf.splitlines(),
                    geng_path,
                    genf_path,
                    lineterm="",
                ):
                    msg.append(line)
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

    def testGolden(self):
        generator.generate(self.tmp)
        # Compare generated to golden.
        self.compare_code(golden_root_dir, self.tmp)


if __name__ == "__main__":
    unittest.main()
