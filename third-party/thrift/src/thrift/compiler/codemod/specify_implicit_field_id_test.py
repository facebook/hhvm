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


class HoistAnnotatedTypesTest(unittest.TestCase):
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
                struct Annotation {}

                struct A {
                    1: string foo;
                }
                struct B {
                    string foo;
                }
                struct C {
                    string foo; // comment
                    string bar; /// docblock
                }
                struct D {
                    string foo;
                    1: string bar;
                }
                struct E {
                    // comment
                    string foo;
                    /// docblock
                    string bar;
                }
                struct F {
                    // @lint-ignore thrift-compiler-warning Negative field id is deprecated, don't add new ones.
                    -1: string foo;
                    1: string bar;
                    string added_after_codemod;
                }
                struct G {
                    @Annotation
                    string foo;
                }
                struct H {
                    optional string foo;
                    required string bar;
                    string baz;
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
                struct Annotation {}

                struct A {
                    1: string foo;
                }
                struct B {
                    // @lint-ignore thrift-compiler-warning Negative field id is deprecated, don't add new ones.
                    -1: string foo;
                }
                struct C {
                    // @lint-ignore thrift-compiler-warning Negative field id is deprecated, don't add new ones.
                    -1: string foo; // comment
                    // @lint-ignore thrift-compiler-warning Negative field id is deprecated, don't add new ones.
                    -2: string bar; /// docblock
                }
                struct D {
                    // @lint-ignore thrift-compiler-warning Negative field id is deprecated, don't add new ones.
                    -1: string foo;
                    1: string bar;
                }
                struct E {
                    // comment
                    // @lint-ignore thrift-compiler-warning Negative field id is deprecated, don't add new ones.
                    -1: string foo;
                    /// docblock
                    // @lint-ignore thrift-compiler-warning Negative field id is deprecated, don't add new ones.
                    -2: string bar;
                }
                struct F {
                    // @lint-ignore thrift-compiler-warning Negative field id is deprecated, don't add new ones.
                    -1: string foo;
                    1: string bar;
                    // @lint-ignore thrift-compiler-warning Negative field id is deprecated, don't add new ones.
                    -2: string added_after_codemod;
                }
                struct G {
                    @Annotation
                    // @lint-ignore thrift-compiler-warning Negative field id is deprecated, don't add new ones.
                    -1: string foo;
                }
                struct H {
                    // @lint-ignore thrift-compiler-warning Negative field id is deprecated, don't add new ones.
                    -1: optional string foo;
                    // @lint-ignore thrift-compiler-warning Negative field id is deprecated, don't add new ones.
                    -2: required string bar;
                    // @lint-ignore thrift-compiler-warning Negative field id is deprecated, don't add new ones.
                    -3: string baz;
                }
                """
            ),
        )
