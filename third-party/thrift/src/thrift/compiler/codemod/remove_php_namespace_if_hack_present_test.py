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

# pyre-unsafe

import os
import shutil
import tempfile
import textwrap
import unittest

import pkg_resources
from xplat.thrift.compiler.codemod.test_utils import read_file, run_binary, write_file


class RemovePhpNamespaceIfHackPresentTest(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, tmp, True)
        self.tmp = tmp
        self.addCleanup(os.chdir, os.getcwd())
        os.chdir(self.tmp)
        self.maxDiff = None
        self.binary = pkg_resources.resource_filename(__name__, "codemod")

    def test_both_present_removes_php(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                namespace php Foo.Bar
                namespace hack Foo.Bar

                struct S {
                  1: i32 x;
                }
                """
            ),
        )

        run_binary(self.binary, "foo.thrift")

        self.assertEqual(
            read_file("foo.thrift"),
            textwrap.dedent(
                """\
                namespace hack Foo.Bar

                struct S {
                  1: i32 x;
                }
                """
            ),
        )

    def test_only_php_no_change(self):
        original = textwrap.dedent(
            """\
            namespace php Foo.Bar

            struct S {
              1: i32 x;
            }
            """
        )
        write_file("foo.thrift", original)

        run_binary(self.binary, "foo.thrift")

        self.assertEqual(read_file("foo.thrift"), original)

    def test_only_hack_no_change(self):
        original = textwrap.dedent(
            """\
            namespace hack Foo.Bar

            struct S {
              1: i32 x;
            }
            """
        )
        write_file("foo.thrift", original)

        run_binary(self.binary, "foo.thrift")

        self.assertEqual(read_file("foo.thrift"), original)

    def test_neither_namespace_no_change(self):
        original = textwrap.dedent(
            """\
            namespace cpp2 foo

            struct S {
              1: i32 x;
            }
            """
        )
        write_file("foo.thrift", original)

        run_binary(self.binary, "foo.thrift")

        self.assertEqual(read_file("foo.thrift"), original)

    def test_php_before_other_namespaces(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                namespace php MyService
                namespace cpp2 my.service
                namespace hack my.service
                namespace py3 my.service

                struct S {
                  1: i32 x;
                }
                """
            ),
        )

        run_binary(self.binary, "foo.thrift")

        self.assertEqual(
            read_file("foo.thrift"),
            textwrap.dedent(
                """\
                namespace cpp2 my.service
                namespace hack my.service
                namespace py3 my.service

                struct S {
                  1: i32 x;
                }
                """
            ),
        )

    def test_php_after_hack(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                namespace hack my.service
                namespace php MyService

                struct S {
                  1: i32 x;
                }
                """
            ),
        )

        run_binary(self.binary, "foo.thrift")

        self.assertEqual(
            read_file("foo.thrift"),
            textwrap.dedent(
                """\
                namespace hack my.service

                struct S {
                  1: i32 x;
                }
                """
            ),
        )
