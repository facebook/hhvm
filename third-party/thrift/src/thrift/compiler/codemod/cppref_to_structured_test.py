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

import os
import shutil
import tempfile
import textwrap
import unittest

from thrift.compiler.codemod.test_utils import read_file, run_binary, write_file


# TODO(urielrivas): We can use clangr's unit-test formatting in the future.
class CppRefToUnstructured(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, tmp, True)
        self.tmp = tmp
        self.addCleanup(os.chdir, os.getcwd())
        os.chdir(self.tmp)
        self.maxDiff = None

    def test_basic_replace(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                struct Faa {
                    1: i32 faa1;
                    2: optional Faa faa2 (cpp.ref);
                    3: i32 faa3;
                }
                """
            ),
        )

        run_binary("cppref_to_structured", "foo.thrift")

        # NOTE: For current tests, user should rely on automated formatting.
        self.assertEqual(
            read_file("foo.thrift"),
            textwrap.dedent(
                """\
                include "thrift/annotation/cpp.thrift"
                struct Faa {
                    1: i32 faa1;
                    @cpp.Ref{type = cpp.RefType.Unique}
                2: optional Faa faa2 ;
                    3: i32 faa3;
                }
                """
            ),
        )

    def test_existing_includes(self):
        write_file("a.thrift", "")
        write_file("b.thrift", "")
        write_file("c.thrift", "")

        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                include "a.thrift"
                include "b.thrift"
                include "c.thrift"

                struct Faa {
                    1: i32 faa1;
                    2: optional Faa faa2 (cpp.ref);
                    3: i32 faa3;
                }
                """
            ),
        )

        run_binary("cppref_to_structured", "foo.thrift")

        self.assertEqual(
            read_file("foo.thrift"),
            textwrap.dedent(
                """\
                include "a.thrift"
                include "b.thrift"
                include "c.thrift"
                include "thrift/annotation/cpp.thrift"

                struct Faa {
                    1: i32 faa1;
                    @cpp.Ref{type = cpp.RefType.Unique}
                2: optional Faa faa2 ;
                    3: i32 faa3;
                }
                """
            ),
        )

    def test_namespaces(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                namespace cpp2 apache.thrift
                namespace py3 thrift.lib.thrift
                struct Faa {
                    1: i32 faa1;
                    2: optional Faa faa2 (cpp.ref);
                    3: i32 faa3;
                }
                """
            ),
        )

        run_binary("cppref_to_structured", "foo.thrift")

        self.assertEqual(
            read_file("foo.thrift"),
            textwrap.dedent(
                """\
                namespace cpp2 apache.thrift
                namespace py3 thrift.lib.thrift
                struct Faa {
                    1: i32 faa1;
                    2: optional Faa faa2 (cpp.ref);
                    3: i32 faa3;
                }
                """
            ),
        )

    def test_cpp_and_cpp2(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                struct Faa {
                    1: i32 faa1;
                    2: optional Faa faa2 (cpp.ref = "true", cpp2.ref = "true");
                    3: i32 faa3;
                }
                """
            ),
        )

        run_binary("cppref_to_structured", "foo.thrift")

        self.assertEqual(
            read_file("foo.thrift"),
            textwrap.dedent(
                """\
                include "thrift/annotation/cpp.thrift"
                struct Faa {
                    1: i32 faa1;
                    @cpp.Ref{type = cpp.RefType.Unique}
                2: optional Faa faa2 ;
                    3: i32 faa3;
                }
                """
            ),
        )
