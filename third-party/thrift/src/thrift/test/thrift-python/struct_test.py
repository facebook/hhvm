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

import dataclasses
import importlib
import types
import unittest

from thrift.python.mutable_types import MutableStruct, MutableStructOrUnion
from thrift.python.types import StructMeta

from thrift.test.thrift_python.struct_test.thrift_mutable_types import (  # @manual=//thrift/test/thrift-python:struct_test_thrift-python-types
    TestStruct as TestStructMutable,
    TestStructWithDefaultValues as TestStructWithDefaultValuesMutable,
)

from thrift.test.thrift_python.struct_test.thrift_types import (
    TestStruct as TestStructImmutable,
    TestStructWithDefaultValues as TestStructWithDefaultValuesImmutable,
)


class ThriftPython_ImmutableStruct_Test(unittest.TestCase):
    def setUp(self) -> None:
        # Disable maximum printed diff length.
        self.maxDiff = None

    def test_creation(self) -> None:
        # Field initialization at instantiation time
        w_new = TestStructImmutable(unqualified_string="hello, world!")
        self.assertEqual(w_new.unqualified_string, "hello, world!")

    def test_default_values(self) -> None:
        # Custom default values:
        # Newly created instance has custom default values for non-optional
        # fields, but custom default values for optional fields are ignored.
        self.assertEquals(
            TestStructWithDefaultValuesImmutable(),
            TestStructWithDefaultValuesImmutable(
                unqualified_integer=42,
                optional_integer=None,
                unqualified_struct=TestStructImmutable(unqualified_string="hello"),
                optional_struct=None,
            ),
        )

        # Intrinsic default values:
        # optional struct field is None
        self.assertIsNone(
            TestStructWithDefaultValuesImmutable().optional_struct_intrinsic_default
        )

        # unqualified struct field is default-initialized
        self.assertEquals(
            TestStructImmutable(),
            TestStructWithDefaultValuesImmutable().unqualified_struct_intrinsic_default,
        )

    def test_type_safety(self) -> None:
        # Field type is validated on instantiation
        with self.assertRaisesRegex(
            TypeError,
            (
                "Cannot create internal string data representation. Expected "
                "type <class 'str'>, got: <class 'int'>."
            ),
        ):
            TestStructImmutable(unqualified_string=42)

    def test_equality_and_hashability(self) -> None:
        # Equality
        w_new = TestStructImmutable(unqualified_string="hello, world!")
        self.assertEqual(w_new, w_new)
        w_new2 = TestStructImmutable(unqualified_string="hello, world!")
        self.assertEqual(w_new, w_new2)
        self.assertIsNot(w_new, w_new2)

        # Immutable types are hashable, with proper semantics.
        self.assertEqual(hash(w_new), hash(w_new2))
        self.assertIn(w_new, {w_new2})

        mapping = {w_new: 123}
        self.assertIn(w_new, mapping)
        self.assertIn(w_new2, mapping)
        self.assertEqual(mapping[w_new], 123)
        self.assertEqual(mapping[w_new2], 123)

        mapping[w_new2] = 456
        self.assertEqual(mapping[w_new], 456)
        self.assertEqual(mapping[w_new2], 456)

    def test_type(self) -> None:
        self.assertEqual(type(TestStructImmutable), StructMeta)
        self.assertEqual(type(TestStructImmutable()), TestStructImmutable)

    def test_iteration(self) -> None:
        # Iterating over the class yields tuples of (field_name, None).
        self.assertSetEqual(
            set(TestStructImmutable),
            {("unqualified_string", None), ("optional_string", None)},
        )

        # Iterating over an instance yields (field_name, field_value) tuples.
        self.assertSetEqual(
            set(TestStructImmutable(unqualified_string="hello")),
            {("unqualified_string", "hello"), ("optional_string", None)},
        )


class ThriftPython_MutableStruct_Test(unittest.TestCase):
    def setUp(self) -> None:
        # Disable maximum printed diff length.
        self.maxDiff = None

    def test_creation_and_assignment(self) -> None:
        w_mutable = TestStructMutable()
        self.assertIsInstance(w_mutable, MutableStruct)
        self.assertIsInstance(w_mutable, MutableStructOrUnion)

        self.assertEqual(w_mutable.unqualified_string, None)
        w_mutable.unqualified_string = "hello, world!"
        self.assertEqual(w_mutable.unqualified_string, "hello, world!")

    def test_default_values(self) -> None:
        # Custom default values:
        # DO_BEFORE(aristidis,20240505): Add support for custom default values
        # to mutable thrift-python types (similar to immutable types).
        self.assertEquals(
            TestStructWithDefaultValuesMutable(),
            TestStructWithDefaultValuesMutable(
                unqualified_integer=None,
                optional_integer=None,
                unqualified_struct=None,
                optional_struct=None,
            ),
        )

        # Intrinsic default values:
        # optional struct field is None
        self.assertIsNone(
            TestStructWithDefaultValuesMutable().optional_struct_intrinsic_default
        )

        # DO_BEFORE(aristidis,20240506): unqualified struct field should be
        # default-initialized.
        self.assertIsNone(
            TestStructWithDefaultValuesMutable().unqualified_struct_intrinsic_default,
        )

    def test_equality_and_hashability(self) -> None:
        # Equality
        w_mutable = TestStructMutable(unqualified_string="hello, world!")
        self.assertEqual(w_mutable, w_mutable)

        w_mutable2 = TestStructMutable(unqualified_string="hello, world!")
        self.assertEqual(w_mutable.unqualified_string, w_mutable2.unqualified_string)
        self.assertEqual(w_mutable, w_mutable2)

        # Instances are not hashable
        with self.assertRaisesRegex(TypeError, "unhashable type: 'TestStruct'"):
            hash(w_mutable)

        with self.assertRaisesRegex(TypeError, "unhashable type: 'TestStruct'"):
            {w_mutable}

        # List and Tuple membership tests use equality (not hashing).
        self.assertIn(w_mutable, [w_mutable2])
        self.assertIn(w_mutable, (w_mutable2,))

        w_mutable2.unqualified_string = "changed value"
        self.assertNotIn(w_mutable, [w_mutable2])
        self.assertNotIn(w_mutable, (w_mutable2,))

    def test_subclass(self) -> None:
        with self.assertRaisesRegex(
            TypeError, "Inheriting from thrift-python data types is forbidden:"
        ):
            types.new_class("TestSubclass2", bases=(TestStructMutable,))

    def test_to_immutable(self) -> None:
        w_mutable = TestStructMutable(unqualified_string="hello")
        w_immutable = w_mutable._to_immutable()
        self.assertIsNot(w_mutable, w_immutable)

        # Even though their contents are the same, the mutable and immutable
        # instance are not "equal":
        self.assertEqual(w_mutable.unqualified_string, w_immutable.unqualified_string)
        self.assertNotEqual(w_mutable, w_immutable)

        # The newly obtained immutable object however is equal to a new
        # TestStructImmutable instance (with the same contents)
        self.assertEqual(w_immutable, TestStructImmutable(unqualified_string="hello"))

        w_mutable.unqualified_string = "hello, world!"
        self.assertNotEqual(
            w_mutable.unqualified_string, w_immutable.unqualified_string
        )

        # Check that converting to immutable validates field types
        w_mutable.unqualified_string = 42
        with self.assertRaisesRegex(
            TypeError,
            (
                "TypeError: Cannot create internal string data representation. "
                "Expected type <class 'str'>, got: <class 'int'>."
            ),
        ):
            w_mutable._to_immutable()

    def test_type(self) -> None:
        # Contrary to immutable struct, the type of the generated clas
        # is not MutableStructMeta, but 'type' (because it is a dataclass type)
        self.assertEqual(type(TestStructMutable), type)
        self.assertEqual(type(TestStructMutable()), TestStructMutable)

    def test_iteration(self) -> None:
        # Contrary to immutable types, the dataclass-based mutable type is not
        # iterable (mostly because we do not control the metaclass of
        # dataclasses).
        with self.assertRaisesRegex(TypeError, "'type' object is not iterable"):
            iter(TestStructMutable)

        self.assertSetEqual(
            {field.name for field in dataclasses.fields(TestStructMutable)},
            {"unqualified_string", "optional_string"},
        )

        # Iterating over an instance yields (field_name, field_value) tuples.
        self.assertSetEqual(
            set(TestStructMutable(unqualified_string="hello")),
            {("unqualified_string", "hello"), ("optional_string", None)},
        )
