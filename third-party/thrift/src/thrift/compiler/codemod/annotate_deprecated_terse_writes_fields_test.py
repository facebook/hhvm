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


class AnnotateDeprecatedTerseWritesFieldsTest(unittest.TestCase):
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
                struct TerseWrite {} (thrift.uri = "facebook.com/thrift/annotation/TerseWrite")

                struct MyStruct{}
                union MyUnion {}
                exception MyException {}

                struct NoOpFields {
                    1: optional i32 a;
                    2: required i32 b;
                    @TerseWrite
                    3: i32 c;
                    4: MyStruct d;
                    5: MyUnion e;
                    6: MyException f;
                    7: MyStruct g (cpp.ref_type = "shared");
                }

                union NoOpFieldsUnion {
                    1: i32 a;
                }

                struct ShouldAnnotateFields {
                    1: string a;
                    2: string b (cpp.ref = "true");
                    3: MyStruct c (cpp.ref = "true");
                    4: string d (cpp.ref_type = "shared");
                }

                exception ShouldAnnotateFieldsException {
                    1: string a;
                    2: string b (cpp.ref = "true");
                    3: MyStruct c (cpp.ref = "true");
                    4: string d (cpp.ref_type = "shared");
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
                include "thrift/annotation/cpp.thrift"

                struct TerseWrite {} (thrift.uri = "facebook.com/thrift/annotation/TerseWrite")

                struct MyStruct{}
                union MyUnion {}
                exception MyException {}

                struct NoOpFields {
                    1: optional i32 a;
                    2: required i32 b;
                    @TerseWrite
                    3: i32 c;
                    4: MyStruct d;
                    5: MyUnion e;
                    6: MyException f;
                    7: MyStruct g (cpp.ref_type = "shared");
                }

                union NoOpFieldsUnion {
                    1: i32 a;
                }

                struct ShouldAnnotateFields {
                    @cpp.DeprecatedTerseWrite
                    1: string a;
                    @cpp.DeprecatedTerseWrite
                    2: string b (cpp.ref = "true");
                    @cpp.DeprecatedTerseWrite
                    3: MyStruct c (cpp.ref = "true");
                    @cpp.DeprecatedTerseWrite
                    4: string d (cpp.ref_type = "shared");
                }

                exception ShouldAnnotateFieldsException {
                    @cpp.DeprecatedTerseWrite
                    1: string a;
                    @cpp.DeprecatedTerseWrite
                    2: string b (cpp.ref = "true");
                    @cpp.DeprecatedTerseWrite
                    3: MyStruct c (cpp.ref = "true");
                    @cpp.DeprecatedTerseWrite
                    4: string d (cpp.ref_type = "shared");
                }
                """
            ),
        )
