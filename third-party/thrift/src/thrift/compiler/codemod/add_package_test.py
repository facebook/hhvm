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


class ThriftPackage(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, tmp, True)
        self.tmp = tmp
        self.addCleanup(os.chdir, os.getcwd())
        os.chdir(self.tmp)
        self.maxDiff = None

    def trim(self, s):
        return "\n".join([line.strip() for line in s.splitlines()])

    def write_and_test(self, file, content, modified_content):
        write_file(file, textwrap.dedent(content))

        binary = pkg_resources.resource_filename(__name__, "codemod")
        run_binary(binary, file)

        self.assertEqual(
            self.trim(read_file(file)),
            self.trim(modified_content),
        )

    def test_existing_package(self):
        self.write_and_test(
            "foo.thrift",
            """\
                package "meta.com/thrift/annotation"

                struct Bar {}

                """,
            """\
                package "meta.com/thrift/annotation"

                struct Bar {}
                """,
        )

    def test_package_from_file_path(self):
        self.write_and_test(
            "fbcode/thrift/test/foo.thrift",
            """\
                /*
                 *  **License docblock**
                 */

                struct S {
                }

                """,
            """\
                /*
                 *  **License docblock**
                 */

                package "meta.com/thrift/test/foo"

                namespace cpp2 "cpp2"
                namespace hack ""
                namespace py3 ""

                struct S {
                }
                """,
        )

    def test_package_from_namespace(self):
        # When no domain is present, default is used
        self.write_and_test(
            "foo.thrift",
            """\
                namespace cpp2 "thrift.annotation"

                include "bar.thrift"
                struct foo {}

                """,
            """\
                package "meta.com/thrift/annotation"

                namespace hack ""
                namespace py3 ""
                namespace cpp2 "thrift.annotation"

                include "bar.thrift"
                struct foo {}
                """,
        )

        self.write_and_test(
            "foo.thrift",
            """\
                namespace cpp2 "thrift.annotation"
                namespace php "thrift.annotation"
                namespace py3 "thrift.annotation.foo"

                include "bar.thrift"
                struct foo {}

                """,
            """\
                package "meta.com/thrift/annotation"

                namespace cpp2 "thrift.annotation"
                namespace php "thrift.annotation"
                namespace py3 "thrift.annotation.foo"

                include "bar.thrift"
                struct foo {}
                """,
        )

    def test_with_common_namespace(self):

        # When domain is present but not in the common package, correctly uses the domain present in file
        self.write_and_test(
            "foo.thrift",
            """\
                namespace java "org.test.thrift_different.annotation"
                namespace cpp2 "org.apache.thrift_common_ns.annotation"
                namespace hack "thrift_common_ns.annotation"

                """,
            """\
                package "apache.org/thrift_common_ns/annotation"

                namespace java "org.test.thrift_different.annotation"
                namespace cpp2 "org.apache.thrift_common_ns.annotation"
                namespace hack "thrift_common_ns.annotation"
                """,
        )

    def test_with_common_namespace_after_modification(self):
        self.write_and_test(
            "foo.thrift",
            """\
                namespace cpp2 "apache.thrift_cpp2.annotation"
                namespace hack "apache.thrift_hack.annotation"
                namespace java "org.apache.thrift"

                """,
            """\
                package "apache.org/thrift/annotation"

                namespace cpp2 "apache.thrift_cpp2.annotation"
                namespace hack "apache.thrift_hack.annotation"
                namespace java "org.apache.thrift"
                """,
        )

        self.write_and_test(
            "foo.thrift",
            """\
                namespace cpp2 "test_cpp_cpp"
                namespace hack "test_hack_cpp"
                namespace java "test_java_cpp"

                """,
            """\
                package "meta.com/test_cpp"

                namespace cpp2 "test_cpp_cpp"
                namespace hack "test_hack_cpp"
                namespace java "test_java_cpp"
                """,
        )

    def test_with_common_identifiers(self):
        self.write_and_test(
            "foo.thrift",
            """\
                namespace cpp2 "meta.thrift_test.cpp2.annotation"
                namespace hack "meta.thrift.cpp2.annotation"
                namespace java "org.apache.cpp2.thrift.annotation"

                """,
            """\
                package "meta.com/cpp2/annotation"

                namespace cpp2 "meta.thrift_test.cpp2.annotation"
                namespace hack "meta.thrift.cpp2.annotation"
                namespace java "org.apache.cpp2.thrift.annotation"
                """,
        )

    def test_with_longest_pkg(self):

        # When minimum length is not met, use the longest path.
        self.write_and_test(
            "foo.thrift",
            """\
                namespace hack "meta.annotation"
                namespace java "org.apache.thrift.annotation"
                namespace cpp2 "meta.thrift_test.cpp2.annotation"

                """,
            """\
                package "meta.com/thrift_test/cpp2/annotation"

                namespace hack "meta.annotation"
                namespace java "org.apache.thrift.annotation"
                namespace cpp2 "meta.thrift_test.cpp2.annotation"
                """,
        )

    def test_cpp2_namespace(self):
        self.write_and_test(
            "foo.thrift",
            """\
                namespace cpp "thrift.annotation"
                namespace py3  "thrift.annotation"

                struct foo {}

                """,
            """\
                package "meta.com/thrift/annotation"

                namespace cpp2 "thrift.annotation.cpp2"
                namespace hack ""
                namespace py3  "thrift.annotation"

                struct foo {}
                """,
        )

        self.write_and_test(
            "foo.thrift",
            """\
                namespace hack "thrift.annotation"
                namespace cpp "thrift.annotation"
                namespace py3  "thrift.annotation"

                struct foo {}

                """,
            """\
                package "meta.com/thrift/annotation"

                namespace cpp2 "thrift.annotation.cpp2"
                namespace hack "thrift.annotation"
                namespace py3  "thrift.annotation"

                struct foo {}
                """,
        )

    def test_empty_namespace(self):

        self.write_and_test(
            "foo.thrift",
            """\
                namespace hack ""
                namespace py3  ""

                struct foo {}

                """,
            """\
                package "meta.com/foo"

                namespace cpp2 "cpp2"
                namespace hack ""
                namespace py3  ""

                struct foo {}
                """,
        )
