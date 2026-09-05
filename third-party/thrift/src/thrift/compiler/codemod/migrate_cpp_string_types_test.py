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


class MigrateCppStringTypesTest(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, tmp, True)
        self.addCleanup(os.chdir, os.getcwd())
        os.chdir(tmp)
        self.maxDiff = None

        write_file(
            "thrift/annotation/cpp.thrift",
            textwrap.dedent(
                """\
                package "facebook.com/thrift/annotation/cpp"

                struct Type {
                    1: string name;
                    2: string template;
                }
                struct Adapter {
                    1: string name;
                }
                """
            ),
        )

    def test_migrates_string_types_selected_by_input(self):
        before = textwrap.dedent(
            """\
            include "thrift/annotation/cpp.thrift"

            namespace cpp2 example

            @cpp.Type{name = "folly::fbstring"}
            typedef binary Bytes

            @cpp.Type{name = "::apache::thrift::ManagedStringViewWithConversions"}
            typedef string ManagedString

            struct Record {
              @cpp.Type{name = "::facebook::admarket::MicroString"}
              1: string label;

              @cpp.Type{name = "folly::IOBuf"}
              2: binary chained;

              @cpp.Type{name = "std::unique_ptr<folly::IOBuf>"}
              3: binary chained_ptr;

              @cpp.Type{name = "::folly::IOBuf"}
              7: binary qualified_chained;

              @cpp.Type{name = "::std::unique_ptr< ::folly::IOBuf >"}
              8: binary qualified_chained_ptr;

              @cpp.Type{name = "CustomString"}
              4: string custom;

              @cpp.Adapter{name = "ExistingAdapter"}
              @cpp.Type{name = "AlreadyAdaptedString"}
              5: string already_adapted;

              @cpp.Type{name = "QualifiedString", template = "OtherTemplate"}
              6: string type_with_extra_key;
            }
            """
        )
        expected = textwrap.dedent(
            """\
            include "thrift/annotation/cpp.thrift"

            namespace cpp2 example

            @cpp.Adapter{name = "::apache::thrift::StringTypeAdapter<folly::fbstring>"}
            typedef binary Bytes

            @cpp.Adapter{name = "::apache::thrift::StringTypeAdapter<::apache::thrift::ManagedStringViewWithConversions>"}
            typedef string ManagedString

            struct Record {
              @cpp.Adapter{name = "::apache::thrift::StringTypeAdapter<::facebook::admarket::MicroString>"}
              1: string label;

              @cpp.Type{name = "folly::IOBuf"}
              2: binary chained;

              @cpp.Type{name = "std::unique_ptr<folly::IOBuf>"}
              3: binary chained_ptr;

              @cpp.Type{name = "::folly::IOBuf"}
              7: binary qualified_chained;

              @cpp.Type{name = "::std::unique_ptr< ::folly::IOBuf >"}
              8: binary qualified_chained_ptr;

              @cpp.Adapter{name = "::apache::thrift::StringTypeAdapter<CustomString>"}
              4: string custom;

              @cpp.Adapter{name = "ExistingAdapter"}
              @cpp.Type{name = "AlreadyAdaptedString"}
              5: string already_adapted;

              @cpp.Type{name = "QualifiedString", template = "OtherTemplate"}
              6: string type_with_extra_key;
            }
            """
        )

        write_file("foo.thrift", before)
        binary = pkg_resources.resource_filename(__name__, "codemod")
        run_binary(binary, "foo.thrift")
        self.assertEqual(read_file("foo.thrift"), expected)

        run_binary(binary, "foo.thrift")
        self.assertEqual(read_file("foo.thrift"), expected)
