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

import unittest

from thrift.python.mutable_types import to_thrift_list
from thrift.test.thrift_python.struct_test.thrift_mutable_types import (
    TestExceptionAsListElement,
    TestExceptionWithContainer,
)


class ThriftPython_MutableException_Test(unittest.TestCase):
    def setUp(self) -> None:
        # Disable maximum printed diff length.
        self.maxDiff = None

    def test_exception_attribute_cache(self) -> None:
        """
        exception TestExceptionAsListElement {
          1: string string_field;
          2: list<i32> list_int;
        }

        exception TestExceptionWithContainer {
          1: list<TestExceptionAsListElement> list_exception;
        }
        """
        e_elem = TestExceptionAsListElement(
            string_field="elem", list_int=to_thrift_list([1])
        )

        e = TestExceptionWithContainer(list_exception=to_thrift_list([e_elem, e_elem]))

        # `to_thrift_list()` always copies the containers inside it. However,
        # if the container element is a Thrift struct, union or exception,
        # assignment uses reference semantics for them.
        self.assertEqual(e_elem, e.list_exception[0])
        self.assertEqual(e_elem, e.list_exception[1])

        # `e_elem`, `e.list_exception[0]` and  `e.list_exception[1]` are the
        # "same" exceptions

        # Demonstrate that they are the "same" exceptions by updating the
        # `list_int`.
        self.assertEqual([1], e_elem.list_int)
        self.assertEqual([1], e.list_exception[0].list_int)
        self.assertEqual([1], e.list_exception[1].list_int)
        e.list_exception[0].list_int.append(2)
        self.assertEqual([1, 2], e_elem.list_int)
        self.assertEqual([1, 2], e.list_exception[0].list_int)
        self.assertEqual([1, 2], e.list_exception[1].list_int)

        # Demonstrate that they are the "same" exceptions by updating the `string_field`.
        # String fields are unique because when they are first accessed, they are read
        # from `exception._fbthrift_data`, converted to a Python type, and cached.
        # On subsequent accesses, the underlying `exception._fbthrift_data` is not
        # accessed again; the cached value is used.
        self.assertEqual("elem", e_elem.string_field)
        self.assertEqual("elem", e.list_exception[0].string_field)
        self.assertEqual("elem", e.list_exception[1].string_field)
        e.list_exception[1].string_field = "updated"
        self.assertEqual("updated", e_elem.string_field)
        self.assertEqual("updated", e.list_exception[0].string_field)
        self.assertEqual("updated", e.list_exception[1].string_field)
