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


class RelativeIncludeTest(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, tmp, True)
        self.tmp = tmp
        self.addCleanup(os.chdir, os.getcwd())
        os.chdir(self.tmp)
        self.maxDiff = None

    def test_basic_replace(self):
        write_file(
            "path/other.thrift",
            textwrap.dedent(
                """\
                struct MyStruct {}
                """
            ),
        )
        write_file(
            "path/root.thrift",
            textwrap.dedent(
                """\
                include "thrift/annotation/thrift.thrift"
                include "other.thrift"

                struct MyStruct {
                  1: other.MyStruct field;
                }
                """
            ),
        )

        binary = pkg_resources.resource_filename(__name__, "codemod")
        run_binary(binary, "path/root.thrift")

        self.assertEqual(
            read_file("path/root.thrift"),
            textwrap.dedent(
                """\
                include "thrift/annotation/thrift.thrift"
                include "path/other.thrift"

                struct MyStruct {
                  1: other.MyStruct field;
                }
                """
            ),
        )

    def test_configerator_replace(self):
        write_file(
            "source/path/other.thrift",
            textwrap.dedent(
                """\
                struct MyStruct {}
                """
            ),
        )
        write_file(
            "source/path/relative.thrift",
            textwrap.dedent(
                """\
                struct MyStruct2 {}
                """
            ),
        )
        write_file(
            "source/path/a/other2.thrift",
            textwrap.dedent(
                """\
                struct MyStruct23 {}
                """
            ),
        )
        write_file(
            "source/path/root.thrift",
            textwrap.dedent(
                """\
                include "relative.thrift"
                include "path/other.thrift"
                include "path/a/other2.thrift"

                struct MyStruct {
                  1: other.MyStruct field;
                  2: relative.MyStruct2 field2;
                  3: other2.MyStruct3 field3;
                }
                """
            ),
        )

        binary = pkg_resources.resource_filename(__name__, "codemod")
        run_binary(binary, "source/path/root.thrift")

        self.assertEqual(
            read_file("source/path/root.thrift"),
            textwrap.dedent(
                """\
                include "path/relative.thrift"
                include "path/other.thrift"
                include "path/a/other2.thrift"

                struct MyStruct {
                  1: other.MyStruct field;
                  2: relative.MyStruct2 field2;
                  3: other2.MyStruct3 field3;
                }
                """
            ),
        )
