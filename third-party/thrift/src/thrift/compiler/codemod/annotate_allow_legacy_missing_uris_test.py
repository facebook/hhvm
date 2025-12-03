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

    def test_adds_package_annotation_for_missing_uris(self):
        """Test that package-level annotation is added when types are missing URIs."""
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                struct TestStruct {
                  1: i32 a;
                  2: i32 b;
                }

                enum TestEnum {
                  VALUE1 = 1,
                  VALUE2 = 2,
                }

                union TestUnion {
                  1: i32 x;
                  2: string y;
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
                include "thrift/annotation/thrift.thrift"

                @thrift.AllowLegacyMissingUris
                package;

                struct TestStruct {
                  1: i32 a;
                  2: i32 b;
                }

                enum TestEnum {
                  VALUE1 = 1,
                  VALUE2 = 2,
                }

                union TestUnion {
                  1: i32 x;
                  2: string y;
                }
                """
            ),
        )

    def test_adds_package_annotation_for_missing_uris_with_namespace(self):
        """
        Test that package-level annotation is added when types are missing URIs,
        above any existing namespace.
        """
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                namespace py3 foo.bar

                struct TestStruct {
                  1: i32 a;
                  2: i32 b;
                }

                enum TestEnum {
                  VALUE1 = 1,
                  VALUE2 = 2,
                }

                union TestUnion {
                  1: i32 x;
                  2: string y;
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
                include "thrift/annotation/thrift.thrift"

                @thrift.AllowLegacyMissingUris
                package;

                namespace py3 foo.bar

                struct TestStruct {
                  1: i32 a;
                  2: i32 b;
                }

                enum TestEnum {
                  VALUE1 = 1,
                  VALUE2 = 2,
                }

                union TestUnion {
                  1: i32 x;
                  2: string y;
                }
                """
            ),
        )

    def test_no_change_when_package_has_uri(self):
        """Test that no annotation is added when package already provides URIs."""
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                package "facebook.com/thrift/test"

                struct TestStruct {
                  1: i32 a;
                }
                """
            ),
        )

        binary = pkg_resources.resource_filename(__name__, "codemod")
        run_binary(binary, "foo.thrift")

        # Should remain unchanged
        self.assertEqual(
            read_file("foo.thrift"),
            textwrap.dedent(
                """\
                package "facebook.com/thrift/test"

                struct TestStruct {
                  1: i32 a;
                }
                """
            ),
        )

    def test_no_change_when_annotation_already_present(self):
        """Test that no duplicate annotation is added if already present."""
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                include "thrift/annotation/thrift.thrift"

                @thrift.AllowLegacyMissingUris
                package ""

                struct TestStruct {
                  1: i32 a;
                }
                """
            ),
        )

        binary = pkg_resources.resource_filename(__name__, "codemod")
        run_binary(binary, "foo.thrift")

        # Should remain unchanged
        self.assertEqual(
            read_file("foo.thrift"),
            textwrap.dedent(
                """\
                include "thrift/annotation/thrift.thrift"

                @thrift.AllowLegacyMissingUris
                package ""

                struct TestStruct {
                  1: i32 a;
                }
                """
            ),
        )

    def test_no_change_when_types_have_explicit_uris(self):
        """Test that no annotation is added when all types have explicit URIs."""
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                include "thrift/annotation/thrift.thrift"

                @thrift.Uri{value = "facebook.com/thrift/test/TestStruct"}
                struct TestStruct {
                  1: i32 a;
                }
                """
            ),
        )

        binary = pkg_resources.resource_filename(__name__, "codemod")
        run_binary(binary, "foo.thrift")

        # Should remain unchanged (no package annotation needed)
        self.assertEqual(
            read_file("foo.thrift"),
            textwrap.dedent(
                """\
                include "thrift/annotation/thrift.thrift"

                @thrift.Uri{value = "facebook.com/thrift/test/TestStruct"}
                struct TestStruct {
                  1: i32 a;
                }
                """
            ),
        )

    def test_no_annotation_for_typedef(self):
        """Test that typedefs don't trigger package annotation (they don't need URIs)."""
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                typedef i32 MyInt
                typedef string MyString
                """
            ),
        )

        binary = pkg_resources.resource_filename(__name__, "codemod")
        run_binary(binary, "foo.thrift")

        # Should remain unchanged (typedefs don't need URIs)
        self.assertEqual(
            read_file("foo.thrift"),
            textwrap.dedent(
                """\
                typedef i32 MyInt
                typedef string MyString
                """
            ),
        )

    def test_adds_annotation_before_existing_empty_package(self):
        """Test that annotation is added before existing empty package declaration."""
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """\
                package;

                struct TestStruct {
                  1: i32 a;
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
                include "thrift/annotation/thrift.thrift"

                @thrift.AllowLegacyMissingUris
                package;

                struct TestStruct {
                  1: i32 a;
                }
                """
            ),
        )
