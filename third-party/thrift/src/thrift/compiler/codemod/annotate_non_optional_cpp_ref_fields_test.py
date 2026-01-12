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


class AnnotateNonOptionalCppRefFieldsTest(unittest.TestCase):
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
                struct NoRefField {
                    // No-op
                    1: i32 a;
                }

                // Should annotate
                struct A {
                    1: string field1 (cpp.ref = "true");

                    2: string field2 (cpp2.ref = "true");

                    3: string field3 (cpp.ref_type = "shared");

                    4: string field4 (cpp2.ref_type = "shared");

                    5: optional string optional_field1 (cpp.ref = "true");

                    6: optional string optional_field2 (cpp2.ref = "true");

                    7: optional string optional_field3 (cpp.ref_type = "shared");

                    8: optional string optional_field4 (cpp2.ref_type = "shared");
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

                struct NoRefField {
                    // No-op
                    1: i32 a;
                }

                // Should annotate
                struct A {
                    @cpp.AllowLegacyNonOptionalRef
                    1: string field1 (cpp.ref = "true");

                    @cpp.AllowLegacyNonOptionalRef
                    2: string field2 (cpp2.ref = "true");

                    @cpp.AllowLegacyNonOptionalRef
                    3: string field3 (cpp.ref_type = "shared");

                    @cpp.AllowLegacyNonOptionalRef
                    4: string field4 (cpp2.ref_type = "shared");

                    5: optional string optional_field1 (cpp.ref = "true");

                    6: optional string optional_field2 (cpp2.ref = "true");

                    7: optional string optional_field3 (cpp.ref_type = "shared");

                    8: optional string optional_field4 (cpp2.ref_type = "shared");
                }
                """
            ),
        )
