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
import tempfile
import textwrap
import unittest

import pkg_resources

from thrift.compiler.codemod.test_utils import read_file, run_binary, write_file


class HoistAnnotatedTypes(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, tmp, True)
        self.tmp = tmp
        self.addCleanup(os.chdir, os.getcwd())
        os.chdir(self.tmp)
        self.maxDiff = None

    def trim(self, s):
        return "\n".join([line.strip() for line in s.splitlines()])

    def test_basic_replace(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                typedef i32 foo (cpp.type = "foo")
                typedef i32 (cpp.type = "bar") bar

                struct S {
                    1: i32 (cpp.type = "baz") baz;
                    2: i32 qux (cpp.type = "oops!");
                    3: foo noAdd;
                    4: i32 (cpp.template = "oops!") notContainer;
                }

                """
            ),
        )

        binary = pkg_resources.resource_filename(__name__, "codemod")
        run_binary(binary, "foo.thrift")

        self.assertEqual(
            self.trim(read_file("foo.thrift")),
            self.trim(
                """\
                include "thrift/annotation/cpp.thrift"

                @cpp.Type{name = "foo"}
                typedef i32 foo
                @cpp.Type{name = "bar"}
                typedef i32  bar

                struct S {
                    @cpp.Type{name = "baz"}
                    1: i32  baz;
                    2: i32 qux ;
                    3: foo noAdd;
                    4: i32  notContainer;
                }
                """
            ),
        )

    def test_existing_include(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                include "thrift/annotation/cpp.thrift"

                typedef i32 foo (cpp.type = "foo")

                """
            ),
        )

        binary = pkg_resources.resource_filename(__name__, "codemod")
        run_binary(binary, "foo.thrift")

        self.assertEqual(
            self.trim(read_file("foo.thrift")),
            self.trim(
                """\
                include "thrift/annotation/cpp.thrift"

                @cpp.Type{name = "foo"}
                typedef i32 foo
                """
            ),
        )

    def test_ref(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                struct S {
                    1: i32 plain;
                    2: i32 ref (cpp.ref);
                    3: i32 ref2 (cpp2.ref);
                    4: i32 ref3 (cpp2.ref = "true");
                    5: i32 unique (cpp.ref_type = "unique");
                    6: i32 shared (cpp.ref_type = "shared");
                    7: i32 shared_const (cpp.ref_type = "shared_const");
                    8: i32 shared_mutable (cpp.ref_type = "shared_mutable");
                    9: i32 box (cpp.box);
                    10: i32 box2 (thrift.box);
                    11: i32 overwrite (cpp.ref, cpp.ref_type = "shared_const");
                }

                """
            ),
        )

        binary = pkg_resources.resource_filename(__name__, "codemod")
        run_binary(binary, "foo.thrift")

        self.assertEqual(
            self.trim(read_file("foo.thrift")),
            self.trim(
                """\
                include "thrift/annotation/cpp.thrift"

                include "thrift/annotation/thrift.thrift"

                struct S {
                    1: i32 plain;
                    @cpp.Ref{type = cpp.RefType.Unique}
                    2: i32 ref ;
                    @cpp.Ref{type = cpp.RefType.Unique}
                    3: i32 ref2 ;
                    @cpp.Ref{type = cpp.RefType.Unique}
                    4: i32 ref3 ;
                    @cpp.Ref{type = cpp.RefType.Unique}
                    5: i32 unique ;
                    @cpp.Ref{type = cpp.RefType.SharedMutable}
                    6: i32 shared ;
                    @cpp.Ref{type = cpp.RefType.Shared}
                    7: i32 shared_const ;
                    @cpp.Ref{type = cpp.RefType.SharedMutable}
                    8: i32 shared_mutable ;
                    @thrift.Box
                    9: i32 box ;
                    @thrift.Box
                    10: i32 box2 ;
                    @cpp.Ref{type = cpp.RefType.Shared}
                    11: i32 overwrite ;
                }
                """
            ),
        )

    def test_hack(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                enum E {} (hack.attributes="JSEnum, GraphQLEnum('SRTJobTypeEnum', 'Auto-generated from PHP enum SRTJobType.'), GraphQLLegacyNamingScheme, Oncalls('srt_core'), WarehouseEnum(shape('hive_enum_map' => true))")
                struct F {} (
  hack.attributes = '\\GraphQLEnum("InstagramRingType", "Identifier for the type of ring overlaid on a user\\x27s profile icon"), \\Oncalls("ig_rc_de")',
)
struct G {} (
  hack.attributes = "
  JSEnum,
  GraphQLLegacyNamingScheme, GraphQLEnum(
    'ServiceTagCategory',
    'The possible category types that a Service Tag can belong to',
  )
",
)

                """
            ),
        )

        binary = pkg_resources.resource_filename(__name__, "codemod")
        run_binary(binary, "foo.thrift")

        self.assertEqual(
            self.trim(read_file("foo.thrift")),
            self.trim(
                """\
                include "thrift/annotation/hack.thrift"

                @hack.Attributes{attributes = ["JSEnum", "GraphQLEnum('SRTJobTypeEnum', 'Auto-generated from PHP enum SRTJobType.')", "GraphQLLegacyNamingScheme", "Oncalls('srt_core')", "WarehouseEnum(shape('hive_enum_map' => true))"]}
                enum E {}
                @hack.Attributes{attributes = ['\\GraphQLEnum("InstagramRingType", "Identifier for the type of ring overlaid on a user\\x27s profile icon")', '\\Oncalls("ig_rc_de")']}
                struct F {}
                @hack.Attributes{attributes = ["JSEnum", "GraphQLLegacyNamingScheme", "GraphQLEnum(
                'ServiceTagCategory',
                'The possible category types that a Service Tag can belong to',
                )"]}
                struct G {}
                """
            ),
        )

    def test_python(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                enum E {} (py3.flags)
                typedef binary T (py3.hidden, py3.name = "U")

                """
            ),
        )

        binary = pkg_resources.resource_filename(__name__, "codemod")
        run_binary(binary, "foo.thrift")

        self.assertEqual(
            self.trim(read_file("foo.thrift")),
            self.trim(
                """\
                include "thrift/annotation/python.thrift"

                @python.Flags
                enum E {}
                @python.Name{name = "U"}
                @python.Py3Hidden
                typedef binary T
                """
            ),
        )

    def test_java(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                struct S {}
                (
                    java.swift.annotations = "@com.facebook.Foo
                        @com.facebook.Bar",
                    java.swift.mutable = "true",
                )
                """
            ),
        )

        binary = pkg_resources.resource_filename(__name__, "codemod")
        run_binary(binary, "foo.thrift")

        self.assertEqual(
            self.trim(read_file("foo.thrift")),
            self.trim(
                """\
                include "thrift/annotation/java.thrift"

                @java.Annotation{java_annotation = "@com.facebook.Foo
                    @com.facebook.Bar"}
                @java.Mutable
                struct S {}
                """
            ),
        )

    def test_go(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                struct S {
                    1: i32 field1 (go.name = "field4");
                    2: i32 field2 (go.tag = 'json:"clientID" yaml:"clientID"');
                }

                """
            ),
        )

        binary = pkg_resources.resource_filename(__name__, "codemod")
        run_binary(binary, "foo.thrift")

        self.assertEqual(
            self.trim(read_file("foo.thrift")),
            self.trim(
                """\
                include "thrift/annotation/go.thrift"

                struct S {
                    @go.Name{name = "field4"}
                    1: i32 field1 ;
                    @go.Tag{tag = 'json:"clientID" yaml:"clientID"'}
                    2: i32 field2 ;
                }
                """
            ),
        )
