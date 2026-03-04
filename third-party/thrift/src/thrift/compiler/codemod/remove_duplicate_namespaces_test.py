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
import subprocess
import tempfile
import textwrap
import unittest

import pkg_resources
from xplat.thrift.compiler.codemod.test_utils import read_file, run_binary, write_file


class RemoveDuplicateNamespacesTest(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, tmp, True)
        self.tmp = tmp
        self.addCleanup(os.chdir, os.getcwd())
        os.chdir(self.tmp)
        self.maxDiff = None

    def test_no_duplicates(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                namespace cpp2 foo
                namespace python bar

                struct S {
                  1: i32 x;
                }
                """
            ),
        )

        binary = pkg_resources.resource_filename(__name__, "codemod")
        run_binary(binary, "foo.thrift")

        self.assertEqual(
            read_file("foo.thrift"),
            textwrap.dedent(
                """\
                namespace cpp2 foo
                namespace python bar

                struct S {
                  1: i32 x;
                }
                """
            ),
        )

    def test_basic_duplicate_removal(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                namespace cpp2 first
                namespace python bar
                namespace cpp2 second

                struct S {
                  1: i32 x;
                }
                """
            ),
        )

        binary = pkg_resources.resource_filename(__name__, "codemod")
        run_binary(binary, "foo.thrift")

        self.assertEqual(
            read_file("foo.thrift"),
            textwrap.dedent(
                """\
                namespace cpp2 first
                namespace python bar

                struct S {
                  1: i32 x;
                }
                """
            ),
        )

    def test_multiple_languages_with_duplicates(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                namespace cpp2 a
                namespace python b
                namespace cpp2 c
                namespace python d

                struct S {
                  1: i32 x;
                }
                """
            ),
        )

        binary = pkg_resources.resource_filename(__name__, "codemod")
        run_binary(binary, "foo.thrift")

        self.assertEqual(
            read_file("foo.thrift"),
            textwrap.dedent(
                """\
                namespace cpp2 a
                namespace python b

                struct S {
                  1: i32 x;
                }
                """
            ),
        )

    def test_trailing_content_on_duplicate_skips_file(self):
        original = textwrap.dedent(
            """\
            namespace cpp2 first
            namespace cpp2 second // keep this

            struct S {
              1: i32 x;
            }
            """
        )
        write_file("foo.thrift", original)

        binary = pkg_resources.resource_filename(__name__, "codemod")
        result = subprocess.run(
            [binary, "foo.thrift"],
            capture_output=True,
            text=True,
        )

        # File should be unchanged.
        self.assertEqual(read_file("foo.thrift"), original)
        # Warning should be emitted on stderr.
        self.assertIn("warning", result.stderr)
        self.assertIn("trailing content", result.stderr)

    def test_duplicate_with_trailing_semicolon(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                namespace cpp2 first;
                namespace python bar
                namespace cpp2 second;

                struct S {
                  1: i32 x;
                }
                """
            ),
        )

        binary = pkg_resources.resource_filename(__name__, "codemod")
        run_binary(binary, "foo.thrift")

        self.assertEqual(
            read_file("foo.thrift"),
            textwrap.dedent(
                """\
                namespace cpp2 first;
                namespace python bar

                struct S {
                  1: i32 x;
                }
                """
            ),
        )
