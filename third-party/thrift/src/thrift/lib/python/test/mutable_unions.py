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

# pyre-strict

import copy
import unittest

from testing.thrift_mutable_types import Digits, Integers

from thrift.python.mutable_types import to_thrift_list, to_thrift_map, to_thrift_set
from thrift.test.thrift_python.union_test.thrift_mutable_types import (
    TestStruct,
    TestUnion,
    TestUnionContainerTypes,
)


class ThriftPython_MutableUnion_Test(unittest.TestCase):
    def setUp(self) -> None:
        # Disable maximum printed diff length.
        self.maxDiff = None

    def test_union_instances(self) -> None:
        """
        struct Digits {
          1: optional list<Integers> data;
        }
        union Integers {
          1: byte tiny;
          2: i16 small;
          3: i32 medium;
          ...
        }
        """
        digit = Integers(tiny=1)
        self.assertEqual(
            digit.FbThriftUnionFieldEnum.tiny, digit.fbthrift_current_field
        )
        self.assertEqual(1, digit.fbthrift_current_value)

        # Initialize the list with the same instance of `digit`.
        # `to_thrift_list()` uses reference semantics if it contains union
        # instances.
        digits = Digits(data=to_thrift_list([digit, digit]))

        # `digit`, `digits.data[0]`, and `digits.data[1]` are the same union
        # instances
        self.assertEqual(1, digit.fbthrift_current_value)
        # pyre-ignore[16]: Optional type has no attribute `__getitem__`
        self.assertEqual(1, digits.data[0].fbthrift_current_value)
        self.assertEqual(1, digits.data[1].fbthrift_current_value)

        # The variables `digit`, `digits.data[0]`  and `digits.data[1]` are
        # bound to the same instance, so updating `digit` will affect the others.
        digit.small = 5
        self.assertEqual(5, digit.fbthrift_current_value)
        self.assertEqual(5, digits.data[0].fbthrift_current_value)
        self.assertEqual(5, digits.data[1].fbthrift_current_value)

        # The variables `digit`, `digits.data[0]`  and `digits.data[1]` are
        # bound to the same instance, so updating `digits.data[0]` will affect
        # the others.
        digits.data[0].medium = 10
        self.assertEqual(10, digit.fbthrift_current_value)
        self.assertEqual(10, digits.data[0].fbthrift_current_value)
        self.assertEqual(10, digits.data[1].fbthrift_current_value)

    def test_copy(self) -> None:
        """
        struct TestStruct {
          1: string unqualified_string;
          2: optional string optional_string;
        }

        union TestUnion {
          1: string string_field;
          2: i32 int_field;
          3: TestStruct struct_field;
        }
        """
        u = TestUnion(string_field="Hello")

        u_copy = copy.copy(u)

        self.assertEqual(
            TestUnion.FbThriftUnionFieldEnum.string_field, u_copy.fbthrift_current_field
        )
        self.assertEqual("Hello", u_copy.fbthrift_current_value)

        # `u_copy` is shallow copy of `u`
        # Since strings are immutable in Python, the shallow copy `u_copy` holds
        # a separate reference to the same string object. When we modify the
        # `string_field` of `u`, it does not affect `u_copy`, as the string
        # itself is immutable and a new string object is created for `u`.
        u.string_field += " World!"
        self.assertEqual(
            TestUnion.FbThriftUnionFieldEnum.string_field, u.fbthrift_current_field
        )
        self.assertEqual("Hello World!", u.fbthrift_current_value)

        self.assertEqual(
            TestUnion.FbThriftUnionFieldEnum.string_field, u_copy.fbthrift_current_field
        )
        self.assertEqual("Hello", u_copy.fbthrift_current_value)

        # In the second part of the test, we create another `TestUnion` instance
        # `u2` with a `struct_field` that contains a `TestStruct`. We then create
        # a shallow copy of `u2` called `u2_copy`. Unlike strings, structs are
        # mutable types. The shallow copy `u2_copy` holds a reference to the same
        # `TestStruct` object as `u2`. Therefore, when we modify the
        # `unqualified_string` field of the `struct_field` in `u2`, the change
        # is reflected in `u2_copy` as well, since both `u2` and `u2_copy`
        # reference the same `TestStruct` object.
        u2 = TestUnion(struct_field=TestStruct(unqualified_string="Hello"))

        u2_copy = copy.copy(u2)

        self.assertEqual(
            TestUnion.FbThriftUnionFieldEnum.struct_field,
            u2_copy.fbthrift_current_field,
        )
        self.assertEqual(
            TestStruct(unqualified_string="Hello"), u2_copy.fbthrift_current_value
        )

        # `u2_copy` is shallow copy of `u2`
        u2.struct_field.unqualified_string = "Hello World!"
        self.assertEqual(
            TestUnion.FbThriftUnionFieldEnum.struct_field, u2.fbthrift_current_field
        )
        self.assertEqual(
            TestStruct(unqualified_string="Hello World!"), u2.fbthrift_current_value
        )

        self.assertEqual(
            TestUnion.FbThriftUnionFieldEnum.struct_field,
            u2_copy.fbthrift_current_field,
        )
        self.assertEqual(
            TestStruct(unqualified_string="Hello World!"),
            u2_copy.fbthrift_current_value,
        )

    def test_deepcopy(self) -> None:
        """
        struct TestStruct {
          1: string unqualified_string;
          2: optional string optional_string;
        }

        union TestUnion {
          1: string string_field;
          2: i32 int_field;
          3: TestStruct struct_field;
        }
        """
        u = TestUnion(string_field="Hello")

        u_deepcopy = copy.deepcopy(u)
        self.assertEqual(
            TestUnion.FbThriftUnionFieldEnum.string_field,
            u_deepcopy.fbthrift_current_field,
        )
        self.assertEqual("Hello", u_deepcopy.fbthrift_current_value)

        # `u` and `u_deepcopy` are independent of each other; any update to
        # `u` should have no effect on `u_deepcopy`.
        u.string_field += " World!"
        self.assertEqual(
            TestUnion.FbThriftUnionFieldEnum.string_field, u.fbthrift_current_field
        )
        self.assertEqual("Hello World!", u.fbthrift_current_value)

        self.assertEqual(
            TestUnion.FbThriftUnionFieldEnum.string_field,
            u_deepcopy.fbthrift_current_field,
        )
        self.assertEqual("Hello", u_deepcopy.fbthrift_current_value)

        # Since a string is immutable, a new string object is created during an
        # update. We need to repeat the test above with a struct field that can
        # be mutated to show that `Struct` instances are separate.
        u2 = TestUnion(struct_field=TestStruct(unqualified_string="Hello"))

        u2_deepcopy = copy.deepcopy(u2)

        self.assertEqual(
            TestUnion.FbThriftUnionFieldEnum.struct_field,
            u2_deepcopy.fbthrift_current_field,
        )
        self.assertEqual(
            TestStruct(unqualified_string="Hello"), u2_deepcopy.fbthrift_current_value
        )

        # `u` and `u_deepcopy` are independent of each other; any update to
        # `u` should have no effect on `u_deepcopy`.
        u2.struct_field.unqualified_string = "Hello World!"
        self.assertEqual(
            TestUnion.FbThriftUnionFieldEnum.struct_field, u2.fbthrift_current_field
        )
        self.assertEqual(
            TestStruct(unqualified_string="Hello World!"), u2.fbthrift_current_value
        )

        self.assertEqual(
            TestUnion.FbThriftUnionFieldEnum.struct_field,
            u2_deepcopy.fbthrift_current_field,
        )
        self.assertEqual(
            TestStruct(unqualified_string="Hello"),
            u2_deepcopy.fbthrift_current_value,
        )

    def test_union_repr(self) -> None:
        u = TestUnionContainerTypes()
        self.assertEqual(u, eval(repr(u)))
        self.assertEqual(repr(u), "TestUnionContainerTypes(EMPTY=None)")

        u = TestUnionContainerTypes(int_field=3)
        self.assertEqual(u, eval(repr(u)))
        self.assertEqual(repr(u), "TestUnionContainerTypes(int_field=3)")

        u = TestUnionContainerTypes(list_i32=to_thrift_list([1, 2, 3]))
        self.assertEqual(u, eval(repr(u)))
        self.assertEqual(
            repr(u), "TestUnionContainerTypes(list_i32=to_thrift_list([1, 2, 3]))"
        )

        u = TestUnionContainerTypes(set_i32=to_thrift_set({1, 2, 3}))
        self.assertEqual(u, eval(repr(u)))
        self.assertEqual(
            repr(u), "TestUnionContainerTypes(set_i32=to_thrift_set({1, 2, 3}))"
        )

        u = TestUnionContainerTypes(map_i32_str=to_thrift_map({1: "abc"}))
        self.assertEqual(u, eval(repr(u)))
        self.assertEqual(
            repr(u), "TestUnionContainerTypes(map_i32_str=to_thrift_map({1: 'abc'}))"
        )
