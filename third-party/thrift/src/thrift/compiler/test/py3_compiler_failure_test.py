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

from __future__ import absolute_import, division, print_function, unicode_literals

import os
import shutil
import subprocess
import sys
import tempfile
import textwrap
import unittest

# @manual=//thrift/compiler/test:compiler_failure_test-library
from thrift.compiler.test.compiler_failure_test import thrift, write_file


class CompilerFailureTest(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, tmp, True)
        self.tmp = tmp
        self.addCleanup(os.chdir, os.getcwd())
        os.chdir(self.tmp)
        self.maxDiff = None

    def run_thrift(self, *args):
        argsx = [thrift, "--gen", "mstch_py3"]
        argsx.extend(args)
        pipe = subprocess.PIPE
        p = subprocess.Popen(argsx, stdout=pipe, stderr=pipe)
        out, err = p.communicate()
        out = out.decode(sys.getdefaultencoding())
        err = err.decode(sys.getdefaultencoding())
        err = err.replace("{}/".format(self.tmp), "")
        return p.returncode, out, err

    # returns expected error message, `args` is a list of `(lineno, name)`
    def _expected_enum_value_name_err(self, *args):
        errors = [
            "[WARNING:] Could not load Thrift standard libraries: Could not find include file thrift/lib/thrift/schema.thrift\n"
            "[WARNING:foo.thrift:4] The annotation py3.name is deprecated. Please use @python.Name instead.\n"
            "[WARNING:foo.thrift:8] The annotation py3.name is deprecated. Please use @python.Name instead.\n"
        ] + [
            (
                f"[ERROR:foo.thrift:{lineno}] "
                f"'{name}' should not be used as an enum/union field name in thrift-py3. "
                'Use a different name or annotate the field with `(py3.name="<new_py_name>")`\n'
            )
            for (lineno, name) in args
        ]
        return "".join(errors)

    def test_enum_invalid_value_names(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """
                enum Foo {
                    name = 1,
                    value = 2 (py3.name = "value_"),
                }

                enum Bar {
                    name = 1 (py3.name = "name_"),
                    value = 2,
                }
                """
            ),
        )

        ret, out, err = self.run_thrift("foo.thrift")
        self.assertEqual(ret, 1)
        self.assertEqual(
            err, self._expected_enum_value_name_err((3, "name"), (9, "value"))
        )

    def test_union_invalid_field_names(self):
        write_file(
            "foo.thrift",
            textwrap.dedent(
                """
                union Foo {
                    1: string name;
                    2: i32 value (py3.name = "value_");
                }

                union Bar {
                    1: string name (py3.name = "name_");
                    2: i32 value;
                }
                """
            ),
        )

        ret, out, err = self.run_thrift("foo.thrift")
        self.assertEqual(ret, 1)
        self.assertEqual(
            err, self._expected_enum_value_name_err((3, "name"), (9, "value"))
        )
