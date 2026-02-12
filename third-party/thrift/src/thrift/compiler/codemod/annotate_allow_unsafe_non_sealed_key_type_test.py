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


class AnnotateAllowUnsafeNonSealedKeyTypeTest(unittest.TestCase):
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

    def test_field_with_non_sealed_map_key(self):
        self._check(
            before="""\
                include "thrift/annotation/thrift.thrift"

                struct NonSealedStruct {}

                struct S {
                  1: map<NonSealedStruct, i32> field1;
                }
                """,
            after="""\
                include "thrift/annotation/thrift.thrift"

                struct NonSealedStruct {}

                struct S {
                  @thrift.AllowUnsafeNonSealedKeyType
                  1: map<NonSealedStruct, i32> field1;
                }
                """,
        )

    def test_field_with_non_sealed_set_element(self):
        self._check(
            before="""\
                include "thrift/annotation/thrift.thrift"

                struct NonSealedStruct {}

                struct S {
                  1: set<NonSealedStruct> field1;
                }
                """,
            after="""\
                include "thrift/annotation/thrift.thrift"

                struct NonSealedStruct {}

                struct S {
                  @thrift.AllowUnsafeNonSealedKeyType
                  1: set<NonSealedStruct> field1;
                }
                """,
        )

    def test_typedef_to_non_sealed_set(self):
        self._check(
            before="""\
                include "thrift/annotation/thrift.thrift"

                struct NonSealedStruct {}

                typedef set<NonSealedStruct> MySet
                """,
            after="""\
                include "thrift/annotation/thrift.thrift"

                struct NonSealedStruct {}

                @thrift.AllowUnsafeNonSealedKeyType
                typedef set<NonSealedStruct> MySet
                """,
        )

    def test_typedef_to_non_sealed_map(self):
        self._check(
            before="""\
                include "thrift/annotation/thrift.thrift"

                struct NonSealedStruct {}

                typedef map<NonSealedStruct, string> MyMap
                """,
            after="""\
                include "thrift/annotation/thrift.thrift"

                struct NonSealedStruct {}

                @thrift.AllowUnsafeNonSealedKeyType
                typedef map<NonSealedStruct, string> MyMap
                """,
        )

    def test_function_param_with_non_sealed_map_key(self):
        self._check(
            before="""\
                include "thrift/annotation/thrift.thrift"

                struct NonSealedStruct {}

                service MyService {
                  void myFunc(
                    1: map<NonSealedStruct, i32> param1,
                  );
                }
                """,
            after="""\
                include "thrift/annotation/thrift.thrift"

                struct NonSealedStruct {}

                service MyService {
                  void myFunc(
                    @thrift.AllowUnsafeNonSealedKeyType
                    1: map<NonSealedStruct, i32> param1,
                  );
                }
                """,
        )

    def test_multiple_fields_some_needing_annotation(self):
        self._check(
            before="""\
                include "thrift/annotation/thrift.thrift"

                struct NonSealedStruct {}

                struct S {
                  1: map<NonSealedStruct, i32> field1;
                  2: map<i32, string> field2;
                  3: set<NonSealedStruct> field3;
                  4: set<string> field4;
                }
                """,
            after="""\
                include "thrift/annotation/thrift.thrift"

                struct NonSealedStruct {}

                struct S {
                  @thrift.AllowUnsafeNonSealedKeyType
                  1: map<NonSealedStruct, i32> field1;
                  2: map<i32, string> field2;
                  @thrift.AllowUnsafeNonSealedKeyType
                  3: set<NonSealedStruct> field3;
                  4: set<string> field4;
                }
                """,
        )

    def test_no_change_sealed_map_key(self):
        self._check(
            before="""\
                struct S {
                  1: map<i32, string> field1;
                  2: set<string> field2;
                }
                """,
            after="""\
                struct S {
                  1: map<i32, string> field1;
                  2: set<string> field2;
                }
                """,
        )

    def test_no_change_annotation_already_present(self):
        self._check(
            before="""\
                include "thrift/annotation/thrift.thrift"

                struct NonSealedStruct {}

                struct S {
                  @thrift.AllowUnsafeNonSealedKeyType
                  1: map<NonSealedStruct, i32> field1;
                }
                """,
            after="""\
                include "thrift/annotation/thrift.thrift"

                struct NonSealedStruct {}

                struct S {
                  @thrift.AllowUnsafeNonSealedKeyType
                  1: map<NonSealedStruct, i32> field1;
                }
                """,
        )

    def test_no_change_type_is_not_map_or_set(self):
        self._check(
            before="""\
                struct NonSealedStruct {}

                struct S {
                  1: list<NonSealedStruct> field1;
                  2: NonSealedStruct field2;
                  3: i32 field3;
                }
                """,
            after="""\
                struct NonSealedStruct {}

                struct S {
                  1: list<NonSealedStruct> field1;
                  2: NonSealedStruct field2;
                  3: i32 field3;
                }
                """,
        )

    def test_include_added_when_missing(self):
        self._check(
            before="""\
                struct NonSealedStruct {}

                struct S {
                  1: map<NonSealedStruct, i32> field1;
                }
                """,
            after="""\
                include "thrift/annotation/thrift.thrift"

                struct NonSealedStruct {}

                struct S {
                  @thrift.AllowUnsafeNonSealedKeyType
                  1: map<NonSealedStruct, i32> field1;
                }
                """,
        )

    def test_include_not_duplicated(self):
        self._check(
            before="""\
                include "thrift/annotation/thrift.thrift"

                struct NonSealedStruct {}

                struct S {
                  1: map<NonSealedStruct, i32> field1;
                }
                """,
            after="""\
                include "thrift/annotation/thrift.thrift"

                struct NonSealedStruct {}

                struct S {
                  @thrift.AllowUnsafeNonSealedKeyType
                  1: map<NonSealedStruct, i32> field1;
                }
                """,
        )

    def test_typedef_indirection_not_annotated(self):
        """A field using a typedef-to-non-sealed-set is NOT annotated.

        The error is on the typedef, not the field. The field's direct type is
        the typedef, which is not itself a map or set.
        """
        self._check(
            before="""\
                include "thrift/annotation/thrift.thrift"

                struct NonSealedStruct {}

                typedef set<NonSealedStruct> KeySet

                struct S {
                  1: KeySet field1;
                }
                """,
            after="""\
                include "thrift/annotation/thrift.thrift"

                struct NonSealedStruct {}

                @thrift.AllowUnsafeNonSealedKeyType
                typedef set<NonSealedStruct> KeySet

                struct S {
                  1: KeySet field1;
                }
                """,
        )
