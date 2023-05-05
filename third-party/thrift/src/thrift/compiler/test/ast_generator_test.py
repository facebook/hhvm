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

import os
import shutil
import subprocess
import tempfile
import textwrap
import unittest
from contextlib import ExitStack
from importlib import resources

from apache.thrift.ast.thrift_types import Ast

# @manual=//thrift/compiler/test:compiler_failure_test-library
from thrift.compiler.test.compiler_failure_test import write_file
from thrift.python.serializer import deserialize

file_manager = ExitStack()
thrift2ast = str(
    file_manager.enter_context(resources.path(__package__, "thrift2ast")).absolute()
)


class AstGeneratorTest(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, tmp, True)
        self.tmp = tmp
        self.addCleanup(os.chdir, os.getcwd())
        os.chdir(self.tmp)
        self.maxDiff = None

    def run_thrift(self, file):
        argsx = [thrift2ast, "--gen", "ast:protocol=compact", "-o", self.tmp, file]
        subprocess.run(argsx, check=True)

        with open(self.tmp + "/gen-ast/" + file[:-7] + ".ast", "rb") as f:
            encoded = f.read()
            return deserialize(Ast, encoded)

    def test_struct(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """
                struct Foo {
                    1: i64 int;
                }

                const i16 answer = 42;
                """
            ),
        )

        ast = self.run_thrift("foo.thrift")
        self.assertEqual(ast.definitions[0].structDef.attrs.name, "Foo")
        self.assertEqual(ast.definitions[0].structDef.fields[0].id, 1)
        srcRange = ast.sourceRanges[1]
        self.assertEqual(srcRange.programId, 1)
        self.assertEqual(srcRange.beginLine, 2)
        self.assertEqual(srcRange.beginColumn, 1)
        self.assertEqual(srcRange.endLine, 4)
        self.assertEqual(srcRange.endColumn, 2)

        self.assertEqual(ast.definitions[1].constDef.attrs.name, "answer")
        self.assertEqual(ast.values[ast.definitions[1].constDef.value - 1].i16Value, 42)
