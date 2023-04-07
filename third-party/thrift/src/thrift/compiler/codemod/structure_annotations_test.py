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
                    1: i32 (cpp.template = "baz") baz;
                    2: i32 qux (cpp.template = "oops!");
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
                    @cpp.Type{template = "baz"}
                    1: i32  baz;
                    2: i32 qux ;
                }
                """
            ),
        )
