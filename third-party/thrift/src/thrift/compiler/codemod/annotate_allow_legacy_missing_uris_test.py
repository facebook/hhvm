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


class AnnotateAllowLegacyMissingUrisTest(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, tmp, True)
        self.tmp = tmp
        self.addCleanup(os.chdir, os.getcwd())
        os.chdir(self.tmp)
        self.maxDiff = None

    def _check(self, *, before: str, after: str) -> None:
        write_file(
            "foo.thrift",
            textwrap.dedent(before),
        )

        binary = pkg_resources.resource_filename(__name__, "codemod")
        run_binary(binary, "foo.thrift")

        self.assertEqual(
            read_file("foo.thrift"),
            textwrap.dedent(after),
        )

    def test_adds_annotation_package_and_include(self):
        self._check(
            before="""\
                struct S { }
                """,
            after="""\
                include "thrift/annotation/thrift.thrift"


                @thrift.AllowLegacyMissingUris
                package;

                struct S { }
                """,
        )

    def test_adds_annotation_existing_package_no_include(self):
        self._check(
            before="""\
                package;

                struct S { }
                """,
            after="""\
                include "thrift/annotation/thrift.thrift"

                @thrift.AllowLegacyMissingUris
                package;

                struct S { }
                """,
        )

    def test_adds_annotation_existing_package_and_other_includes(self):
        self._check(
            before="""\
                include "thrift/annotation/cpp.thrift"

                package;

                struct S { }
                """,
            after="""\
                include "thrift/annotation/cpp.thrift"
                include "thrift/annotation/thrift.thrift"

                @thrift.AllowLegacyMissingUris
                package;

                struct S { }
                """,
        )

    def test_adds_annotation_existing_package_before_include(self):
        self._check(
            before="""\
                package;

                include "thrift/annotation/cpp.thrift"

                struct S { }
                """,
            after="""\
                @thrift.AllowLegacyMissingUris
                package;

                include "thrift/annotation/cpp.thrift"
                include "thrift/annotation/thrift.thrift"

                struct S { }
                """,
        )

    def test_adds_annotation_with_namespaces(self):
        self._check(
            before="""\
                namespace cpp2 foo

                struct S { }
                """,
            after="""\
                include "thrift/annotation/thrift.thrift"


                @thrift.AllowLegacyMissingUris
                package;

                namespace cpp2 foo

                struct S { }
                """,
        )

    def test_adds_annotation_with_namespaces_and_other_include(self):
        self._check(
            before="""\
                namespace cpp2 foo

                include "thrift/annotation/cpp.thrift"

                struct S { }
                """,
            after="""\
                namespace cpp2 foo

                include "thrift/annotation/cpp.thrift"
                include "thrift/annotation/thrift.thrift"

                @thrift.AllowLegacyMissingUris
                package;


                struct S { }
                """,
        )

    def test_adds_annotation_with_namespaces_before_include(self):
        self._check(
            before="""\
                namespace cpp2 foo

                include "thrift/annotation/thrift.thrift"
                include "thrift/annotation/cpp.thrift"

                struct S { }
                """,
            after="""\
                namespace cpp2 foo

                include "thrift/annotation/thrift.thrift"
                include "thrift/annotation/cpp.thrift"

                @thrift.AllowLegacyMissingUris
                package;


                struct S { }
                """,
        )

    def test_no_change_when_package_has_uri(self):
        """Test that no annotation is added when package already provides URIs."""
        self._check(
            before="""\
                package "facebook.com/thrift/test"

                struct TestStruct {
                  1: i32 a;
                }
                """,
            after="""\
                package "facebook.com/thrift/test"

                struct TestStruct {
                  1: i32 a;
                }
                """,
        )

    def test_no_change_when_annotation_already_present(self):
        """Test that no duplicate annotation is added if already present."""
        self._check(
            before="""\
                include "thrift/annotation/thrift.thrift"

                @thrift.AllowLegacyMissingUris
                package ""

                struct TestStruct {
                  1: i32 a;
                }
                """,
            after="""\
                include "thrift/annotation/thrift.thrift"

                @thrift.AllowLegacyMissingUris
                package ""

                struct TestStruct {
                  1: i32 a;
                }
                """,
        )

    def test_no_change_when_types_have_explicit_uris(self):
        """Test that no annotation is added when all types have explicit URIs."""
        self._check(
            before="""\
                include "thrift/annotation/thrift.thrift"

                @thrift.Uri{value = "facebook.com/thrift/test/TestStruct"}
                struct TestStruct {
                  1: i32 a;
                }
                """,
            after="""\
                include "thrift/annotation/thrift.thrift"

                @thrift.Uri{value = "facebook.com/thrift/test/TestStruct"}
                struct TestStruct {
                  1: i32 a;
                }
                """,
        )

    def test_no_annotation_for_typedef(self):
        """Test that typedefs don't trigger package annotation (they don't need URIs)."""
        self._check(
            before="""\
                typedef i32 MyInt
                typedef string MyString
                """,
            after="""\
                typedef i32 MyInt
                typedef string MyString
                """,
        )
