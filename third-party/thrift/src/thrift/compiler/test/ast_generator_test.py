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
import sys
import tempfile
import textwrap
import unittest
from contextlib import ExitStack
from importlib import resources

from apache.thrift.ast.thrift_types import Ast
from apache.thrift.type.standard.thrift_types import TypeName

# @manual=//thrift/compiler/test:compiler_failure_test-library
from thrift.compiler.test.compiler_failure_test import write_file
from thrift.python.serializer import deserialize

file_manager = ExitStack()
thrift2ast = str(
    file_manager.enter_context(resources.path(__package__, "thrift2ast")).absolute()
)


@unittest.skipIf(sys.platform == "win32", "test not supported on Windows")
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
        p = subprocess.run(argsx, check=True, capture_output=True)
        print("exit status:", p.returncode)
        print("stdout:", p.stdout.decode())
        print("stderr:", p.stderr.decode())

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

    def test_const_types(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """
                struct Foo {
                    1: i64 int;
                }

                const i16 answer = 42;
                const Foo structured = {"int": 1};
                """
            ),
        )

        ast = self.run_thrift("foo.thrift")
        self.assertEqual(ast.definitions[1].constDef.attrs.name, "answer")
        self.assertEqual(ast.values[ast.definitions[1].constDef.value - 1].i16Value, 42)

        self.assertEqual(ast.definitions[2].constDef.attrs.name, "structured")
        self.assertEqual(
            ast.values[ast.definitions[2].constDef.value - 1]
            .objectValue.members[1]
            .i64Value,
            1,
        )

    def test_service(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """
                service Foo {
                    void foo();
                    i32 bar();
                    stream<string> baz();
                }
                """
            ),
        )

        ast = self.run_thrift("foo.thrift")
        serviceDef = ast.definitions[0].serviceDef
        self.assertEqual(serviceDef.attrs.name, "Foo")
        self.assertEqual(len(serviceDef.functions), 3)
        self.assertEqual(serviceDef.functions[0].attrs.name, "foo")
        self.assertEqual(
            serviceDef.functions[0].returnTypes[0].thriftType.name.type,
            TypeName.Type.EMPTY,
        )
        self.assertEqual(serviceDef.functions[1].attrs.name, "bar")
        self.assertEqual(
            serviceDef.functions[1].returnTypes[0].thriftType.name.type,
            TypeName.Type.i32Type,
        )
        self.assertEqual(serviceDef.functions[2].attrs.name, "baz")
        self.assertEqual(
            len(serviceDef.functions[2].returnTypes), 0
        )  # streams not supported yet
