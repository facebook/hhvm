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

    def test_basic_replace(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                typedef map<i32 (cpp.type = "uint32_t"), S> MyMap;

                struct S {
                    1: list<S (annotated)> (annotated) foo;
                    2: map<string, map<i32, i32> (annotated)> bar;
                    3: set<set<i16 (annotated)> (annotated)> nested;
                }

                service X {
                    i32 (cpp.type = "uint32_t") foo(1: S (annotated) s);
                }

                const S (moar_annotated) c = {};
                """
            ),
        )

        binary = pkg_resources.resource_filename(__name__, "codemod")
        run_binary(binary, "foo.thrift")

        self.assertEqual(
            read_file("foo.thrift"),
            textwrap.dedent(
                """\
                typedef map<i32_7683, S> MyMap;

                struct S {
                    1: list<S_866> (annotated) foo;
                    2: map<string, map_i32_i32_7359> bar;
                    3: set<set_i16_5922> nested;
                }

                service X {
                    i32_7683 foo(1:S_866 s);
                }

                const S_8730 c = {};

                // The following were automatically generated and may benefit from renaming.
                typedef S (annotated = "1") S_866
                typedef S (moar_annotated = "1") S_8730
                typedef i16 (annotated = "1") i16_2065
                typedef i32 (cpp.type = "uint32_t") i32_7683
                typedef map<i32, i32> (annotated = "1") map_i32_i32_7359
                typedef set<i16_2065> (annotated = "1") set_i16_5922
                """
            ),
        )
