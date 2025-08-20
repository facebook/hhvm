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


class TestAddNamespacePy3(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, tmp, True)
        self.tmp = tmp
        self.addCleanup(os.chdir, os.getcwd())
        os.chdir(self.tmp)
        self.maxDiff = None

    def trim(self, s):
        return "\n".join([line.strip() for line in s.splitlines()])

    def write_and_test(self, thrift_file, namespace_py3, content, modified_content):
        write_file(thrift_file, textwrap.dedent(content))

        binary = pkg_resources.resource_filename(__name__, "codemod")
        run_binary(
            binary,
            "-I",
            ".",
            "-I",
            "include1",
            "-I",
            "include2",
            thrift_file,
            namespace_py3,
        )

        self.assertEqual(
            self.trim(read_file(thrift_file)),
            self.trim(modified_content),
        )

    def test_add_namespace_when_no_others_exist(self):
        self.write_and_test(
            "path/to/foo.thrift",
            "path.to",
            """\
                struct Bar {}

                """,
            """\
                namespace py3 path.to

                struct Bar {}
                """,
        )

    def test_add_namespace_when_others_exist(self):
        self.write_and_test(
            "path/to/foo.thrift",
            "different.on.purpose",
            """\
                namespace cpp path.to

                struct Bar {}

                """,
            """\
                namespace py3 different.on.purpose
                namespace cpp path.to

                struct Bar {}
                """,
        )

    def test_do_not_add_namespace_when_it_already_exists_and_is_non_empty(self):
        self.write_and_test(
            "path/to/foo.thrift",
            "path.to",
            """\
                namespace py3 path.to

                struct Bar {}

                """,
            """\
                namespace py3 path.to

                struct Bar {}
                """,
        )

    def test_overwrite_namespace_when_it_is_empty(self):
        self.write_and_test(
            "path/to/foo.thrift",
            "different.on.purpose",
            """\
                namespace py3 ""

                struct Bar {}

                """,
            """\
                namespace py3 different.on.purpose

                struct Bar {}
                """,
        )
