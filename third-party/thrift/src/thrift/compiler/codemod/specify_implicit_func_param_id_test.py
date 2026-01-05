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


class SpecifyImplicitFuncParamIdTest(unittest.TestCase):
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
                    string foo;
                }

                service B1 {
                    i32 bar(i32 a, i32 b);
                }

                service B2 {
                    i32 bar(1: i32 a, i32 b);
                }

                service B3 {
                    i32 bar(1: i32 a, i32 b, 2: i32 c, i32 d, 3: i32 e);
                }

                interaction C1 {
                    void baz(i32 x);
                }

                interaction C2 {
                    void baz(
                        1: i32 x,
                        2: i32 y,
                        i32 z
                    );
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
                    string foo;
                }

                service B1 {
                    i32 bar(-1: i32 a, -2: i32 b);
                }

                service B2 {
                    i32 bar(1: i32 a, -1: i32 b);
                }

                service B3 {
                    i32 bar(1: i32 a, -1: i32 b, 2: i32 c, -2: i32 d, 3: i32 e);
                }

                interaction C1 {
                    void baz(-1: i32 x);
                }

                interaction C2 {
                    void baz(
                        1: i32 x,
                        2: i32 y,
                        -1: i32 z
                    );
                }
                """
            ),
        )
