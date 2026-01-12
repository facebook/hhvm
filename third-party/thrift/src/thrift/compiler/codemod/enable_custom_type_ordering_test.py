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
import subprocess
import tempfile
import unittest

import pkg_resources
from xplat.thrift.compiler.codemod.test_utils import read_file, run_binary, write_file

HEADER_NO_URI = """
include "thrift/annotation/cpp.thrift"
include "thrift/annotation/thrift.thrift"
"""

HEADER_WITH_URI = HEADER_NO_URI + 'package "apache.org/thrift/test"\n'

ORDERABILITY_TEST_PROGRAM = """
// There are 3 categories:
// * Foo*: With custom set/map
// * Bar*: Without custom set/map
// * Baz*: Special cases

@cpp.Type{template = "std::unordered_set"}
typedef set<i32> CustomSet1;
@cpp.Type{name = "std::unordered_set<int32_t>"}
typedef set<i32> CustomSet2;
@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
typedef set<i32> CustomSet3;
typedef set<i32> CustomSet4 (cpp.template = "std::unordered_set");
typedef set<i32> CustomSet5 (cpp.type = "std::unordered_set<int32_t>");

struct Foo1 { 1: CustomSet1 foo; }
struct Foo2 { 1: CustomSet2 foo; }
struct Foo3 { 1: CustomSet3 foo; }
// Random comment to illustrate codemod effect on comments.
struct Foo4 { 1: CustomSet4 foo; }
struct Foo5 { 1: CustomSet5 foo; }
struct Foo6 { 1: list<CustomSet1> foo; }
struct Foo7 {
  @cpp.Type{template = "std::unordered_set"}
  1: set<i32> foo;
}
struct Foo8 {
  @cpp.Type{template = "std::unordered_map"}
  1: map<i32, i32> foo;
}
struct Foo9 {
  1: i32 field1;
  2: CustomSet1 field2;
}
struct Foo10 {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: optional CustomSet1 foo;
}
struct Foo11 {
  @thrift.Box
  1: optional CustomSet1 foo;
}

@cpp.Type{template = "std::deque"}
typedef list<i32> CustomList1

struct Bar1 { 1: set<i32> foo; }
struct Bar2 { 1: CustomList1 foo; }
struct Bar3 { 1: Foo1 foo; }
struct Bar4 { 1: map<i32, Foo4> foo; }

// Weirdly field adapters don't count as custom type, probably a bug.
struct Baz1 {
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  1: set<i32> baz;
}

// Always unorderable due to the lack of URI
@thrift.Uri{value = ""}
struct Baz2 {
  @cpp.Type{template = "std::unordered_set"}
  1: set<i32> foo;
}

// Has URI, but still unorderable since it contains unorderable field.
@thrift.Uri{value = "apache.org/thrift/Baz3"}
struct Baz3 {
  1: Baz2 foo;
}

// Already enabled custom type ordering
@cpp.EnableCustomTypeOrdering
struct Baz4 {
  @cpp.Type{template = "std::unordered_set"}
  1: set<i32> foo;
}
"""

ORDERABILITY_EXPECTED_RESULT = (
    HEADER_WITH_URI
    + """
// There are 3 categories:
// * Foo*: With custom set/map
// * Bar*: Without custom set/map
// * Baz*: Special cases

@cpp.Type{template = "std::unordered_set"}
typedef set<i32> CustomSet1;
@cpp.Type{name = "std::unordered_set<int32_t>"}
typedef set<i32> CustomSet2;
@cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
typedef set<i32> CustomSet3;
typedef set<i32> CustomSet4 (cpp.template = "std::unordered_set");
typedef set<i32> CustomSet5 (cpp.type = "std::unordered_set<int32_t>");

@cpp.EnableCustomTypeOrdering
struct Foo1 { 1: CustomSet1 foo; }
@cpp.EnableCustomTypeOrdering
struct Foo2 { 1: CustomSet2 foo; }
@cpp.EnableCustomTypeOrdering
struct Foo3 { 1: CustomSet3 foo; }
// Random comment to illustrate codemod effect on comments.
@cpp.EnableCustomTypeOrdering
struct Foo4 { 1: CustomSet4 foo; }
@cpp.EnableCustomTypeOrdering
struct Foo5 { 1: CustomSet5 foo; }
@cpp.EnableCustomTypeOrdering
struct Foo6 { 1: list<CustomSet1> foo; }
@cpp.EnableCustomTypeOrdering
struct Foo7 {
  @cpp.Type{template = "std::unordered_set"}
  1: set<i32> foo;
}
@cpp.EnableCustomTypeOrdering
struct Foo8 {
  @cpp.Type{template = "std::unordered_map"}
  1: map<i32, i32> foo;
}
@cpp.EnableCustomTypeOrdering
struct Foo9 {
  1: i32 field1;
  2: CustomSet1 field2;
}
@cpp.EnableCustomTypeOrdering
struct Foo10 {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: optional CustomSet1 foo;
}
@cpp.EnableCustomTypeOrdering
struct Foo11 {
  @thrift.Box
  1: optional CustomSet1 foo;
}

@cpp.Type{template = "std::deque"}
typedef list<i32> CustomList1

struct Bar1 { 1: set<i32> foo; }
struct Bar2 { 1: CustomList1 foo; }
struct Bar3 { 1: Foo1 foo; }
struct Bar4 { 1: map<i32, Foo4> foo; }

// Weirdly field adapters don't count as custom type, probably a bug.
struct Baz1 {
  @cpp.Adapter{name = "::apache::thrift::test::TemplatedTestAdapter"}
  1: set<i32> baz;
}

// Always unorderable due to the lack of URI
@thrift.Uri{value = ""}
struct Baz2 {
  @cpp.Type{template = "std::unordered_set"}
  1: set<i32> foo;
}

// Has URI, but still unorderable since it contains unorderable field.
@thrift.Uri{value = "apache.org/thrift/Baz3"}
struct Baz3 {
  1: Baz2 foo;
}

// Already enabled custom type ordering
@cpp.EnableCustomTypeOrdering
struct Baz4 {
  @cpp.Type{template = "std::unordered_set"}
  1: set<i32> foo;
}
"""
)


class EnableCustomTypeOrderingTest(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, tmp, True)
        self.tmp = tmp
        self.addCleanup(os.chdir, os.getcwd())
        os.chdir(self.tmp)
        self.maxDiff = None
        self._codemod_binary = pkg_resources.resource_filename(__name__, "codemod")

    def _codemod_and_return_result(self, input_program: str) -> str:
        write_file("foo.thrift", input_program)
        run_binary(self._codemod_binary, "foo.thrift")
        return read_file("foo.thrift")

    def test_basic_with_uri(self):
        self.assertEqual(
            self._codemod_and_return_result(
                HEADER_WITH_URI + ORDERABILITY_TEST_PROGRAM
            ),
            ORDERABILITY_EXPECTED_RESULT,
        )

    def test_basic_without_uri(self):
        PROGRAM = HEADER_NO_URI + ORDERABILITY_TEST_PROGRAM
        self.assertEqual(self._codemod_and_return_result(PROGRAM), PROGRAM)

    def test_unnecessary_annotation(self):
        with self.assertRaises(subprocess.CalledProcessError):
            self._codemod_and_return_result(
                HEADER_NO_URI
                + """

@cpp.EnableCustomTypeOrdering
struct TestUnnecessaryAnnotation {
  1: i32 a;
}
"""
            )
