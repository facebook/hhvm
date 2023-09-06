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
from apache.thrift.type.schema.thrift_types import ReturnType
from apache.thrift.type.standard.thrift_types import TypeName, TypeUri

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
        argsx = [
            thrift2ast,
            "--gen",
            "ast:protocol=compact",
            "-o",
            self.tmp,
            "-I",
            resources.path(__package__, "implicit_includes"),
            file,
        ]
        p = subprocess.run(argsx, capture_output=True)
        print("exit status:", p.returncode)
        print("stdout:", p.stdout.decode())
        print("stderr:", p.stderr.decode())
        self.assertEqual(p.returncode, 0)

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
                """
            ),
        )

        ast = self.run_thrift("foo.thrift")
        self.assertEqual(ast.definitions[0].structDef.attrs.name, "Foo")
        self.assertEqual(ast.definitions[0].structDef.fields[0].id, 1)
        srcRange = ast.definitions[0].structDef.attrs.sourceRange
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
                interaction I {}
                service Parent {}
                service Foo extends Parent {
                    void foo(1: string in) throws (1: Ex e);
                    i32 bar();
                    stream<string> baz();
                    i16, sink<i32 throws (1: Ex e), i64 throws (1: Ex e)> qux();
                    I startI();
                    performs I;
                }
                exception Ex {}
                """
            ),
        )

        ast = self.run_thrift("foo.thrift")
        serviceDef = ast.definitions[1].serviceDef
        self.assertEqual(serviceDef.attrs.name, "Foo")
        self.assertEqual(serviceDef.baseService.uri.scopedName, "foo.Parent")
        self.assertEqual(len(serviceDef.functions), 6)

        func = serviceDef.functions[0]
        self.assertEqual(func.attrs.name, "foo")
        self.assertEqual(func.returnTypes[0].thriftType.name.type, TypeName.Type.EMPTY)
        self.assertEqual(func.returnType.name.type, TypeName.Type.EMPTY)
        self.assertEqual(func.paramlist.fields[0].id, 1)
        self.assertEqual(func.paramlist.fields[0].attrs.name, "in")
        self.assertEqual(
            func.paramlist.fields[0].type.name.type, TypeName.Type.stringType
        )
        self.assertEqual(
            func.exceptions[0].type.name.exceptionType.scopedName, "foo.Ex"
        )
        self.assertEqual(func.streamOrSink, ReturnType())
        self.assertEqual(func.interactionType.uri, TypeUri())

        func = serviceDef.functions[1]
        self.assertEqual(func.attrs.name, "bar")
        self.assertEqual(
            func.returnTypes[0].thriftType.name.type, TypeName.Type.i32Type
        )
        self.assertEqual(func.returnType.name.type, TypeName.Type.i32Type)

        func = serviceDef.functions[2]
        print(func)
        self.assertEqual(func.attrs.name, "baz")
        self.assertEqual(len(func.returnTypes), 0)  # streams excluded
        self.assertEqual(func.returnType.name.type, TypeName.Type.EMPTY)
        self.assertEqual(
            func.streamOrSink.streamType.payload.name.type, TypeName.Type.stringType
        )

        func = serviceDef.functions[3]
        self.assertEqual(func.attrs.name, "qux")
        self.assertEqual(func.returnType.name.type, TypeName.Type.i16Type)
        self.assertEqual(
            func.streamOrSink.sinkType.payload.name.type, TypeName.Type.i32Type
        )
        self.assertEqual(len(func.streamOrSink.sinkType.clientExceptions), 1)
        self.assertEqual(
            func.streamOrSink.sinkType.finalResponse.name.type, TypeName.Type.i64Type
        )
        self.assertEqual(len(func.streamOrSink.sinkType.serverExceptions), 1)

        func = serviceDef.functions[4]
        self.assertEqual(func.attrs.name, "startI")
        self.assertEqual(func.returnType.name.type, TypeName.Type.EMPTY)
        self.assertEqual(func.interactionType.uri.scopedName, "foo.I")

        func = serviceDef.functions[5]
        self.assertEqual(func.attrs.name, "createI")
        self.assertEqual(func.returnType.name.type, TypeName.Type.EMPTY)
        self.assertEqual(func.interactionType.uri.scopedName, "foo.I")

    def test_docs(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """
                /** This is a struct and its name is Foo. */
                struct Foo {
                    1: i64 Bar;  ///< This is a field and its name is Bar.
                }
                """
            ),
        )

        ast = self.run_thrift("foo.thrift")
        print(ast.definitions)
        self.assertEqual(ast.definitions[0].structDef.attrs.name, "Foo")
        srcRange = ast.definitions[0].structDef.attrs.sourceRange
        self.assertEqual(srcRange.programId, 1)
        self.assertEqual(srcRange.beginLine, 3)
        docs = ast.definitions[0].structDef.attrs.docs
        self.assertEqual(
            docs.contents.rstrip(), "This is a struct and its name is Foo."
        )
        self.assertEqual(docs.sourceRange.programId, 1)
        self.assertEqual(docs.sourceRange.beginLine, 2)
        self.assertEqual(docs.sourceRange.beginColumn, 1)
        self.assertEqual(docs.sourceRange.endLine, 2)
        self.assertEqual(docs.sourceRange.endColumn, 45)

        docs = ast.definitions[0].structDef.fields[0].attrs.docs
        self.assertEqual(docs.contents.rstrip(), "This is a field and its name is Bar.")
        if os.name == "nt":
            return  # line separators differ on windows and mess up source ranges
        self.assertEqual(docs.sourceRange.programId, 1)
        self.assertEqual(docs.sourceRange.beginLine, 4)
        self.assertEqual(docs.sourceRange.beginColumn, 18)
        self.assertEqual(docs.sourceRange.endLine, 5)
        self.assertEqual(docs.sourceRange.endColumn, 1)

    def test_program(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """
                package "test.dev/foo"
                namespace cpp2 facebook.foo
                """
            ),
        )

        ast = self.run_thrift("foo.thrift")
        program = ast.programs[0]
        self.assertEqual(program.attrs.name, "foo")
        self.assertEqual(program.attrs.uri, "test.dev/foo")
        self.assertEqual(ast.sources[1].fileName, "foo.thrift")
        self.assertEqual(
            ast.values[ast.sources[1].namespaces["cpp2"] - 1].stringValue,
            b"facebook.foo",
        )

    def test_annotation(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """
                struct Annot {}
                @Annot
                struct S {}
                """
            ),
        )

        ast = self.run_thrift("foo.thrift")
        annot_id = next(iter(ast.definitions[1].structDef.attrs.annotations))
        annot = ast.values[annot_id - 1]
        # If standard library is not loaded this will have mapValue instead of objectValue
        self.assertEqual(
            annot.objectValue.members[1].objectValue.members[3].stringValue, "foo.Annot"
        )
