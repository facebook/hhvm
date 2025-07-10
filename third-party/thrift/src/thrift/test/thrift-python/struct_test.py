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
import pickle
import types
import typing
import unittest
from datetime import datetime
from sys import platform

import thrift.python.mutable_serializer as mutable_serializer

import thrift.python.serializer as immutable_serializer
import thrift.python.types

from folly.iobuf import IOBuf

from parameterized import parameterized
from thrift.python.exceptions import Error
from thrift.python.mutable_containers import MutableList, MutableMap, MutableSet

from thrift.python.mutable_exceptions import MutableGeneratedError
from thrift.python.mutable_types import (
    MutableStruct,
    MutableStructMeta,
    MutableStructOrUnion,
    to_thrift_list,
    to_thrift_map,
    to_thrift_set,
)
from thrift.python.types import (
    isset as immutable_isset,
    Struct as ImmutableStruct,
    StructMeta as ImmutableStructMeta,
    StructOrUnion as ImmutableStructOrUnion,
)

from thrift.test.thrift_python.struct_test.thrift_abstract_types import TestEnum
from thrift.test.thrift_python.struct_test.thrift_mutable_types import (
    bool_constant,
    byte_constant,
    double_constant,
    float_constant,
    i16_constant,
    i32_constant,
    i64_constant,
    list_constant,
    map_constant,
    set_constant,
    string_constant,
    struct_constant,
    TestExceptionAllThriftPrimitiveTypes as TestExceptionAllThriftPrimitiveTypesMutable,
    TestExceptionCopy as TestExceptionCopyMutable,
    TestStruct as TestStructMutable,
    TestStructAdaptedTypes as TestStructAdaptedTypesMutable,
    TestStructAllThriftContainerTypes as TestStructAllThriftContainerTypesMutable,
    TestStructAllThriftPrimitiveTypes as TestStructAllThriftPrimitiveTypesMutable,
    TestStructAllThriftPrimitiveTypesWithDefaultValues as TestStructAllThriftPrimitiveTypesWithDefaultValuesMutable,
    TestStructAsListElement as TestStructAsListElementMutable,
    TestStructContainerAssignment as TestStructContainerAssignmentMutable,
    TestStructCopy as TestStructCopyMutable,
    TestStructEmpty as TestStructEmptyMutable,
    TestStructEmptyAlias as TestStructEmptyAliasMutable,
    TestStructNested_0 as TestStructNested_0_Mutable,
    TestStructNested_1 as TestStructNested_1_Mutable,
    TestStructNested_2 as TestStructNested_2_Mutable,
    TestStructWithDefaultValues as TestStructWithDefaultValuesMutable,
    TestStructWithTypedefField as TestStructWithTypedefFieldMutable,
)

from thrift.test.thrift_python.struct_test.thrift_types import (
    TestStruct as TestStructImmutable,
    TestStructAdaptedTypes as TestStructAdaptedTypesImmutable,
    TestStructAllThriftContainerTypes as TestStructAllThriftContainerTypesImmutable,
    TestStructAllThriftPrimitiveTypes as TestStructAllThriftPrimitiveTypesImmutable,
    TestStructAllThriftPrimitiveTypesWithDefaultValues as TestStructAllThriftPrimitiveTypesWithDefaultValuesImmutable,
    TestStructWithDefaultValues as TestStructWithDefaultValuesImmutable,
)

max_byte: int = 2**7 - 1
max_i16: int = 2**15 - 1
max_i32: int = 2**31 - 1
max_i64: int = 2**63 - 1


def _thrift_serialization_round_trip(
    test: unittest.TestCase,
    module: types.ModuleType,
    control: typing.Union[
        MutableStructOrUnion, ImmutableStructOrUnion, MutableGeneratedError
    ],
) -> None:
    for proto in module.Protocol:
        encoded = module.serialize(control, protocol=proto)
        test.assertIsInstance(encoded, bytes)

        decoded = module.deserialize(type(control), encoded, protocol=proto)
        test.assertIsInstance(decoded, type(control))
        test.assertEqual(control, decoded)


def _pickle_round_trip(
    test: unittest.TestCase,
    control: typing.Union[MutableStructOrUnion, ImmutableStructOrUnion],
) -> None:
    pickled = pickle.dumps(control, protocol=pickle.HIGHEST_PROTOCOL)
    unpickled = pickle.loads(pickled)
    test.assertIsInstance(unpickled, type(control))
    test.assertEqual(control, unpickled)


class ThriftPython_ImmutableStruct_Test(unittest.TestCase):
    def setUp(self) -> None:
        # Disable maximum printed diff length.
        self.maxDiff = None

    def test_creation(self) -> None:
        # Field initialization at instantiation time
        w_new = TestStructImmutable(unqualified_string="hello, world!")
        self.assertEqual(w_new.unqualified_string, "hello, world!")

    def test_creation_with_None(self) -> None:
        self.assertEqual(
            TestStructImmutable(),
            TestStructImmutable(unqualified_string=None, optional_string=None),
        )

        self.assertEqual(
            TestStructImmutable(unqualified_string="hello, world!"),
            TestStructImmutable(
                unqualified_string="hello, world!", optional_string=None
            ),
        )

    def test_call(self) -> None:
        w = TestStructImmutable(unqualified_string="hello, world!")

        # Call operator: create a copy, with new values.
        # Original instance is unchanged.
        w2 = w(unqualified_string="foobar")
        self.assertIsNot(w, w2)
        self.assertEqual(w2.unqualified_string, "foobar")
        self.assertEqual(w.unqualified_string, "hello, world!")
        self.assertNotEqual(w, w2)

        # Call operator with no values given: returns self
        # Note: this is equivalent to returning a copy of self in the immutable
        # domain (since their contents cannot change).
        w3 = w()
        self.assertIs(w, w3)

        # Call operator with invalid type
        with self.assertRaisesRegex(
            TypeError,
            (
                "error .* Thrift struct field 'unqualified_string': Cannot create "
                "internal string data representation. Expected type <class 'str'>, got: "
                "<class 'int'>."
            ),
        ):
            # pyre-ignore[6]: Intentional for test
            w(unqualified_string=42)

    def test_call_with_None(self) -> None:
        w = TestStructImmutable(unqualified_string="hello", optional_string="world")

        # TEST: Updating a value with "None" means resetting to its default value.
        self.assertEqual(
            w(unqualified_string=None),
            TestStructImmutable(unqualified_string="", optional_string="world"),
        )
        w1 = w(optional_string=None)
        self.assertIsNone(w1.optional_string)
        self.assertEqual(w1, TestStructImmutable(unqualified_string="hello"))
        self.assertEqual(
            w1, TestStructImmutable(unqualified_string="hello", optional_string=None)
        )

        w2 = w(unqualified_string=None, optional_string=None)
        self.assertEqual(w2, TestStructImmutable())
        self.assertIsNone(w2.optional_string)

    def test_intrinsic_default_values_for_unqualified_fields(self) -> None:
        # GIVEN
        # Even though this test is primarily about
        # the intrinsic default values for unqualified fields,
        # for sanity, set the optional fields to None.
        expected_primitive = TestStructAllThriftPrimitiveTypesImmutable(
            unqualified_string="",
            optional_string=None,
            unqualified_i32=0,
            optional_i32=None,
            unqualified_double=0.0,
            optional_double=None,
            unqualified_bool=False,
            optional_bool=None,
            unqualified_byte=0,
            optional_byte=None,
            unqualified_i16=0,
            optional_i16=None,
            unqualified_i64=0,
            optional_i64=None,
            unqualified_float=0,
            optional_float=None,
        )

        self.assertEqual(
            expected_primitive, TestStructAllThriftPrimitiveTypesImmutable()
        )

        expected_container = TestStructAllThriftContainerTypesImmutable(
            unqualified_list_i32=[],
            optional_list_i32=None,
            unqualified_set_string=set(),
            optional_set_string=None,
            unqualified_map_string_i32={},
            optional_map_string_i32=None,
        )

        self.assertEqual(
            expected_container, TestStructAllThriftContainerTypesImmutable()
        )

    def test_default_values(self) -> None:
        # Custom default values:
        # Newly created instance has custom default values for non-optional
        # fields, but custom default values for optional fields are ignored.
        self.assertEqual(
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
        self.assertEqual(
            TestStructImmutable(),
            TestStructWithDefaultValuesImmutable().unqualified_struct_intrinsic_default,
        )

    def test_type_safety(self) -> None:
        # Field type is validated on instantiation
        with self.assertRaisesRegex(
            TypeError,
            (
                "error initializing Thrift struct field 'unqualified_string': Cannot "
                "create internal string data representation. Expected type <class 'str'>, "
                "got: <class 'int'>"
            ),
        ):
            # pyre-ignore[6]: Intentional for test
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

    def test_ordering(self) -> None:
        self.assertLess(
            TestStructImmutable(unqualified_string="a"),
            TestStructImmutable(unqualified_string="b"),
        )
        self.assertLess(
            TestStructImmutable(unqualified_string="a", optional_string="z"),
            TestStructImmutable(unqualified_string="b", optional_string="a"),
        )
        self.assertGreater(
            TestStructImmutable(unqualified_string="b", optional_string="z"),
            TestStructImmutable(unqualified_string="b", optional_string="a"),
        )

    def test_subclass(self) -> None:
        err = (
            r"Inheritance from generated thrift struct \w+ is deprecated."
            r" Please use composition."
        )
        with self.assertRaisesRegex(TypeError, err):
            types.new_class(
                "TestImmutableSubclass",
                bases=(TestStructImmutable,),
                exec_body=lambda ns: ns.update(_fbthrift_SPEC=()),
            )

    def test_base_classes(self) -> None:
        self.assertIsInstance(TestStructImmutable(), ImmutableStruct)
        self.assertIsInstance(TestStructImmutable(), ImmutableStructOrUnion)

    def test_type(self) -> None:
        self.assertEqual(type(TestStructImmutable), ImmutableStructMeta)
        self.assertEqual(type(TestStructImmutable()), TestStructImmutable)

    def test_iteration(self) -> None:
        # Iterating over the class yields tuples of (field_name, None).
        self.assertSetEqual(
            # pyre-ignore[6]: Fixme: type error to be addressed later
            set(TestStructImmutable),
            {("unqualified_string", None), ("optional_string", None)},
        )

        # Iterating over an instance yields (field_name, field_value) tuples.
        self.assertSetEqual(
            set(TestStructImmutable(unqualified_string="hello")),
            {("unqualified_string", "hello"), ("optional_string", None)},
        )

    def test_del_attribute(self) -> None:
        w = TestStructImmutable(unqualified_string="hello, world!")

        # Attributes of immutable types cannot be deleted.
        with self.assertRaisesRegex(AttributeError, "unqualified_string"):
            del w.unqualified_string
        self.assertEqual(w.unqualified_string, "hello, world!")

        # deleting after setting is no longer a no-op;
        # it consistently raises AttributeError
        with self.assertRaisesRegex(AttributeError, "unqualified_string"):
            del w.unqualified_string

        self.assertEqual(w.unqualified_string, "hello, world!")

        # However, a new instance of the object can be created with a specific
        # attribute "deleted", by explicitly assigning it the "None" value:
        w_cleared = w(unqualified_string=None)
        self.assertEqual(w_cleared.unqualified_string, "")

        # "Deleting" a field (by creating a new instance) resets it to its
        # *standard default value*, i.e. the custom default value if provided
        # (see thrift/doc/idl/index.md#default-values).
        w_default_values = TestStructWithDefaultValuesImmutable(unqualified_integer=123)
        self.assertEqual(w_default_values.unqualified_integer, 123)
        w_default_values_cleared = w_default_values(unqualified_integer=None)
        self.assertEqual(w_default_values_cleared.unqualified_integer, 42)

    def test_type_hints(self) -> None:
        # thrift-python immutable structs do not include type hints directly
        # (although a separate .pyi interface is generated to allow tooling to
        # do static type checking)
        self.assertEqual(typing.get_type_hints(TestStructImmutable), {})

    def test_serialization_round_trip(self) -> None:
        s = TestStructAllThriftPrimitiveTypesImmutable()

        # Default initialized fields set their isset flags to `False`
        for field, _ in TestStructAllThriftPrimitiveTypesImmutable:
            self.assertFalse(immutable_isset(s)[field])

        s = TestStructAllThriftPrimitiveTypesImmutable(
            unqualified_string="Hello world!",
            optional_string="Hello optional!",
            unqualified_i32=19,
            optional_i32=23,
            unqualified_double=2.1,
            optional_double=1.3,
            unqualified_bool=True,
            optional_bool=False,
            unqualified_byte=1,
            optional_byte=1,
            unqualified_i16=211,
            optional_i16=234,
            unqualified_i64=211,
            optional_i64=234,
            unqualified_float=2.0,
            optional_float=1.0,
        )

        # All the fields are initialized above, so all isset flags should be
        # set to `True`.
        for field, _ in TestStructAllThriftPrimitiveTypesImmutable:
            self.assertTrue(immutable_isset(s)[field])

        _thrift_serialization_round_trip(self, immutable_serializer, s)
        _pickle_round_trip(self, s)

        s_default_value = TestStructAllThriftPrimitiveTypesWithDefaultValuesImmutable()
        _thrift_serialization_round_trip(self, immutable_serializer, s_default_value)
        _pickle_round_trip(self, s_default_value)

    def test_adapted_types(self) -> None:
        s = TestStructAdaptedTypesImmutable()
        # standard default value for i32 is 0, therefore `fromtimestamp(0)`
        self.assertEqual(
            s.unqualified_adapted_i32_to_datetime, datetime.fromtimestamp(0)
        )
        self.assertEqual(s.unqualified_adapted_string_to_i32, 123)

        new_date = datetime(2024, 5, 6, 12, 0, 0)
        s = s(unqualified_adapted_i32_to_datetime=new_date)
        self.assertEqual(s.unqualified_adapted_i32_to_datetime, new_date)

        with self.assertRaisesRegex(
            AttributeError,
            (
                "error .* Thrift struct field "
                "'unqualified_adapted_i32_to_datetime': 'int' object has no attribute "
                "'timestamp'"
            ),
        ):
            # Thrift simply passes the value to the adapter class. All type
            # checking is performed within the adapter class.
            # pyre-ignore[6]: Intentional for test
            s = s(unqualified_adapted_i32_to_datetime=123)

        s = s(unqualified_adapted_string_to_i32=999)
        self.assertEqual(s.unqualified_adapted_string_to_i32, 999)

    @parameterized.expand(
        [
            (
                TestStructAdaptedTypesImmutable(
                    optional_adapted_i32_to_datetime=datetime.fromtimestamp(86400)
                ),
            ),
            (
                TestStructAdaptedTypesImmutable(
                    unqualified_adapted_i32_to_datetime=datetime.fromtimestamp(0),
                    optional_adapted_i32_to_datetime=datetime.fromtimestamp(86400),
                ),
            ),
        ]
    )
    def test_adapter_serialization_round_trip(self, struct: ImmutableStruct) -> None:
        _thrift_serialization_round_trip(self, immutable_serializer, struct)
        _pickle_round_trip(self, struct)

    def test_to_immutable_python(self) -> None:
        w = TestStructImmutable(unqualified_string="hello, world!")
        self.assertIs(w, w._to_python())

    def test_to_mutable_python(self) -> None:
        w_immutable = TestStructImmutable(unqualified_string="hello, world!")
        w_mutable = w_immutable._to_mutable_python()
        self.assertIsNot(w_immutable, w_mutable)
        self.assertNotEqual(w_immutable, w_mutable)
        self.assertEqual(set(w_immutable), set(w_mutable))

        # Struct with container fields
        w_immutable_containers = TestStructAllThriftContainerTypesImmutable(
            unqualified_list_i32=[1, 2, 3],
            unqualified_set_string={"a", "b", "c"},
            unqualified_map_string_i32={"a": 1},
        )
        w_mutable_containers = w_immutable_containers._to_mutable_python()
        self.assertIsNot(w_immutable_containers, w_mutable_containers)
        self.assertNotEqual(w_immutable_containers, w_mutable_containers)
        self.assertEqual(list(w_immutable_containers), list(w_mutable_containers))

    def test_match(self) -> None:
        # Canonical case: match fields
        match TestStructImmutable(unqualified_string="Hello, world!"):
            case TestStructImmutable(unqualified_string=x, optional_string=None):
                self.assertEqual(x, "Hello, world!")
            case _:
                self.fail("Expected match, got none.")

        # Any instance will match a Class pattern with no argument:
        match TestStructImmutable(
            unqualified_string="Hello, ", optional_string="world!"
        ):
            case TestStructImmutable():
                pass  # Expected
            case _:
                self.fail("Expected match, got none.")

        # Capturing value of unset optional field (i.e., None):
        match TestStructImmutable(unqualified_string="Hello, world!"):
            case TestStructImmutable(optional_string=x):
                self.assertIsNone(x)
            case _:
                self.fail("Expected match, got none.")

        # Do not match if values differ
        match TestStructImmutable(unqualified_string="Hello"):
            case TestStructImmutable(unqualified_string="world!", optional_string=x):
                self.fail(f"Unexpected match: {x}")
            case _:
                pass  # Expected

        # Match default values
        match TestStructWithDefaultValuesImmutable():
            case TestStructWithDefaultValuesImmutable(
                unqualified_integer=x,
                unqualified_list_i32=y,
                optional_integer=None,
            ):
                self.assertEqual(x, 42)
                self.assertEqual(y, [1, 2, 3])
            case _:
                self.fail("Expected match, got none.")

        # Same, but using sequence capture pattern
        match TestStructWithDefaultValuesImmutable():
            case TestStructWithDefaultValuesImmutable(
                unqualified_integer=x,
                unqualified_list_i32=[a, b, c],
                optional_integer=None,
            ):
                self.assertEqual(x, 42)
                self.assertEqual(a, 1)
                self.assertEqual(b, 2)
                self.assertEqual(c, 3)

            case _:
                self.fail("Expected match, got none.")

        # Match nested struct
        match TestStructWithDefaultValuesImmutable():
            case TestStructWithDefaultValuesImmutable(unqualified_struct=x):
                self.assertEqual(x, TestStructImmutable(unqualified_string="hello"))
            case _:
                self.fail("Expected match, got none.")

        # Match nested struct, capture nested field
        match TestStructWithDefaultValuesImmutable():
            case TestStructWithDefaultValuesImmutable(
                unqualified_struct=TestStructImmutable(unqualified_string=x)
            ):
                self.assertEqual(x, "hello")
            case _:
                self.fail("Expected match, got none.")

        # Match adapted types
        match TestStructAdaptedTypesImmutable(
            unqualified_adapted_i32_to_datetime=datetime.fromtimestamp(1733556290)
        ):
            case TestStructAdaptedTypesImmutable(unqualified_adapted_i32_to_datetime=x):
                self.assertIsInstance(x, datetime)
                self.assertEqual(x, datetime.fromtimestamp(1733556290))
            case _:
                self.fail("Expected match, got none.")


class ThriftPython_MutableStruct_Test(unittest.TestCase):
    def setUp(self) -> None:
        # Disable maximum printed diff length.
        self.maxDiff = None

    def test_creation_and_assignment(self) -> None:
        w = TestStructMutable()
        self.assertEqual(w.unqualified_string, "")
        w.unqualified_string = "hello, world!"
        self.assertEqual(w.unqualified_string, "hello, world!")

        w2 = TestStructMutable(unqualified_string="hello")
        self.assertEqual(w2.unqualified_string, "hello")
        w2.unqualified_string += ", world!"
        self.assertEqual(w2.unqualified_string, "hello, world!")

    def test_creation_with_None(self) -> None:
        self.assertEqual(
            TestStructMutable(),
            TestStructMutable(unqualified_string=None, optional_string=None),
        )
        self.assertEqual(
            TestStructMutable(unqualified_string="hello, world!"),
            TestStructMutable(unqualified_string="hello, world!", optional_string=None),
        )

    def test_call(self) -> None:
        w = TestStructMutable(unqualified_string="hello, world!")

        # Call operator: create a (deep) copy, with new values.
        # Original instance is unchanged.
        w2 = w(unqualified_string="foobar")
        self.assertIsNot(w, w2)
        self.assertEqual(w2.unqualified_string, "foobar")
        self.assertEqual(w.unqualified_string, "hello, world!")
        self.assertNotEqual(w, w2)

        # Call operator with no values given: returns (deep) copy of self
        # Note the difference with immutable types (which return self).
        w3 = w()
        self.assertIsNot(w, w3)
        self.assertEqual(w, w3)

    def test_intrinsic_default_values_for_unqualified_fields(self) -> None:
        # GIVEN
        # Even though this test is primarily about
        # the intrinsic default values for unqualified fields,
        # for sanity, set the optional fields to None.
        expected_primitive = TestStructAllThriftPrimitiveTypesMutable(
            unqualified_string="",
            optional_string=None,
            unqualified_i32=0,
            optional_i32=None,
            unqualified_double=0.0,
            optional_double=None,
            unqualified_bool=False,
            optional_bool=None,
            unqualified_byte=0,
            optional_byte=None,
            unqualified_i16=0,
            optional_i16=None,
            unqualified_i64=0,
            optional_i64=None,
            unqualified_float=0,
            optional_float=None,
        )

        self.assertEqual(expected_primitive, TestStructAllThriftPrimitiveTypesMutable())

        expected_container = TestStructAllThriftContainerTypesMutable(
            unqualified_list_i32=to_thrift_list([]),
            optional_list_i32=None,
            unqualified_set_string=to_thrift_set(set()),
            optional_set_string=None,
            unqualified_map_string_i32=to_thrift_map({}),
            optional_map_string_i32=None,
        )

        self.assertEqual(expected_container, TestStructAllThriftContainerTypesMutable())

    def test_default_values(self) -> None:
        # Newly created instance has custom default values for non-optional
        # fields, but custom default values for optional fields are ignored.
        self.assertEqual(
            TestStructWithDefaultValuesMutable(),
            TestStructWithDefaultValuesMutable(
                unqualified_integer=42,
                optional_integer=None,
                unqualified_struct=TestStructMutable(unqualified_string="hello"),
                optional_struct=None,
            ),
        )

        # Intrinsic default values:
        # optional struct field is None
        self.assertIsNone(
            TestStructWithDefaultValuesMutable().optional_struct_intrinsic_default
        )

        self.assertEqual(
            TestStructMutable(),
            TestStructWithDefaultValuesMutable().unqualified_struct_intrinsic_default,
        )

    def test_reset(self) -> None:
        w = TestStructWithDefaultValuesMutable(optional_integer=123)
        self.assertEqual(w.optional_integer, 123)
        self.assertEqual(w.unqualified_integer, 42)
        w.fbthrift_reset()
        self.assertIsNone(w.optional_integer)
        self.assertEqual(w.unqualified_integer, 42)

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

    def test_ordering(self) -> None:
        self.assertLess(
            TestStructMutable(unqualified_string="a"),
            TestStructMutable(unqualified_string="b"),
        )
        self.assertLess(
            TestStructMutable(unqualified_string="a", optional_string="z"),
            TestStructMutable(unqualified_string="b", optional_string="a"),
        )
        self.assertGreater(
            TestStructMutable(unqualified_string="b", optional_string="z"),
            TestStructMutable(unqualified_string="b", optional_string="a"),
        )

    def test_subclass(self) -> None:
        with self.assertRaisesRegex(
            TypeError, "Inheriting from thrift-python data types is forbidden:"
        ):
            types.new_class("TestSubclass2", bases=(TestStructMutable,))

    def test_base_classes(self) -> None:
        self.assertIsInstance(TestStructMutable(), MutableStruct)
        self.assertIsInstance(TestStructMutable(), MutableStructOrUnion)

    def test_type(self) -> None:
        self.assertEqual(type(TestStructMutable), MutableStructMeta)
        self.assertEqual(type(TestStructMutable()), TestStructMutable)

    def test_iteration(self) -> None:
        # Iterating over the class yields tuples of (field_name, None).
        self.assertSetEqual(
            # pyre-ignore[6]: Fixme: type error to be addressed later
            set(TestStructMutable),
            {("unqualified_string", None), ("optional_string", None)},
        )

        # Iterating over an instance yields (field_name, field_value) tuples.
        self.assertSetEqual(
            set(TestStructMutable(unqualified_string="hello")),
            {("unqualified_string", "hello"), ("optional_string", None)},
        )

    def test_del_attribute(self) -> None:
        w = TestStructMutable(unqualified_string="hello", optional_string="world")

        # Deleting an attribute on a (mutable) thrift-python instance raises
        # `AttributeError`
        with self.assertRaises(AttributeError):
            del w.unqualified_string

        with self.assertRaises(AttributeError):
            del w.optional_string

        self.assertEqual(w.unqualified_string, "hello")
        self.assertEqual(w.optional_string, "world")

        with self.assertRaises(AttributeError):
            del w.unqualified_string

        with self.assertRaises(AttributeError):
            del w.optional_string

    def test_type_hints(self) -> None:
        # Similar to thrift-python immutable structs, mutable structs do not
        # include type hints directly
        self.assertEqual(typing.get_type_hints(TestStructMutable), {})

    def test_to_immutable_python(self) -> None:
        w_mutable = TestStructMutable(unqualified_string="hello")
        w_immutable = w_mutable._to_python()
        self.assertIsNot(w_mutable, w_immutable)

        # Even though their contents are the same, the mutable and immutable
        # instance are not "equal":
        self.assertEqual(w_mutable.unqualified_string, w_immutable.unqualified_string)
        # Remember: set(<struct>) returns a set of (field name, value) tuples - see
        # test_iteration() above.
        self.assertSetEqual(set(w_mutable), set(w_immutable))
        self.assertNotEqual(w_mutable, w_immutable)

        # The newly obtained immutable object however is equal to a new
        # TestStructImmutable instance (with the same contents)
        self.assertEqual(w_immutable, TestStructImmutable(unqualified_string="hello"))

        w_mutable.unqualified_string = "hello, world!"
        self.assertNotEqual(
            w_mutable.unqualified_string, w_immutable.unqualified_string
        )

    def test_to_mutable_python(self) -> None:
        w_mutable = TestStructMutable(unqualified_string="hello")
        self.assertIs(w_mutable, w_mutable._to_mutable_python())

    def test_create_and_assign_for_i32(self) -> None:
        s = TestStructAllThriftPrimitiveTypesMutable(unqualified_i32=11)

        # Check the value assigned during initialization
        self.assertEqual(11, s.unqualified_i32)

        # Set the value and read it back
        s.unqualified_i32 = 23
        self.assertEqual(23, s.unqualified_i32)

        # `del struct.field_name` raises a `AttributeError`
        with self.assertRaises(AttributeError):
            del s.unqualified_i32

        # Assigning `None` raises a `TypeError`
        with self.assertRaises(TypeError):
            # pyre-ignore[8]: Intentional for test
            s.unqualified_i32 = None

        # Assigning a value of the wrong type raises a `TypeError`
        with self.assertRaises(TypeError):
            # pyre-ignore[8]: Intentional for test
            s.unqualified_i32 = "This is not an integer"

        # Boundary check for integral types
        with self.assertRaises(OverflowError):
            s.unqualified_i32 = 2**31

        s = TestStructAllThriftPrimitiveTypesWithDefaultValuesMutable()
        # from IDL: i32 unqualified_i32 = 2048;

        # Check the value from IDL during initialization
        self.assertEqual(2048, s.unqualified_i32)

        # Set the value and read it back
        s.unqualified_i32 = 32
        self.assertEqual(32, s.unqualified_i32)

        # `del struct.field_name` raises a `AttributeError`
        with self.assertRaises(AttributeError):
            del s.unqualified_i32

        # Assigning `None` raises a `TypeError`
        with self.assertRaises(TypeError):
            # pyre-ignore[8]: Intentional for test
            s.unqualified_i32 = None

        # Assigning a value of the wrong type raises a `TypeError`
        with self.assertRaises(TypeError):
            # pyre-ignore[8]: Intentional for test
            s.unqualified_i32 = "This is not an integer"

        # Boundary check for integral types
        with self.assertRaises(OverflowError):
            s.unqualified_i32 = max_i32 + 1

    def test_create_and_assign_for_optional_i32_and_optional_string(self) -> None:
        s = TestStructAllThriftPrimitiveTypesMutable()

        # Check optional fields are `None`
        self.assertIsNone(s.optional_i32)
        self.assertIsNone(s.optional_string)

        # Set the values and read them back
        s.optional_i32 = 23
        s.optional_string = "thrift"

        self.assertEqual(23, s.optional_i32)
        self.assertEqual("thrift", s.optional_string)

        # `del struct.field_name` raises a `AttributeError`
        with self.assertRaises(AttributeError):
            del s.optional_i32

        with self.assertRaises(AttributeError):
            del s.optional_string

        # Assigning a value of the wrong type raises a `TypeError`
        with self.assertRaisesRegex(TypeError, "is not a <class 'int'>"):
            # pyre-ignore[8]: Intentional for test
            s.optional_i32 = "This is not an integer"

        with self.assertRaisesRegex(TypeError, "Expected type <class 'str'>"):
            # pyre-ignore[8]: Intentional for test
            s.optional_string = 42

        self.assertEqual(23, s.optional_i32)
        self.assertEqual("thrift", s.optional_string)

        # Assigning `None` is reseting the optional field to `None`
        s.optional_i32 = None
        s.optional_string = None

        self.assertIsNone(s.optional_i32)
        self.assertIsNone(s.optional_string)

        # Assigning a value of the wrong type raises a `TypeError` when fields
        # are `None`
        with self.assertRaisesRegex(TypeError, "is not a <class 'int'>"):
            # pyre-ignore[8]: Intentional for test
            s.optional_i32 = "This is not an integer"

        with self.assertRaisesRegex(TypeError, "Expected type <class 'str'>"):
            # pyre-ignore[8]: Intentional for test
            s.optional_string = 42

        # Boundary check for integral types
        with self.assertRaises(OverflowError):
            s.optional_i32 = -(2**31 + 1)

    def test_create_and_assign_for_optional_container(self) -> None:
        s = TestStructAllThriftContainerTypesMutable()

        # Check optional field is `None`
        self.assertIsNone(s.optional_list_i32)

        # Set the value and read it back
        s.optional_list_i32 = to_thrift_list([1, 2, 3])
        self.assertEqual([1, 2, 3], s.optional_list_i32)

        # `del struct.field_name` raises a `AttributeError`
        with self.assertRaises(AttributeError):
            del s.optional_list_i32

        # Assigning a value of the wrong type raises a `TypeError`
        with self.assertRaisesRegex(TypeError, "is not a <class 'int'>"):
            s.optional_list_i32 = to_thrift_list(["list", "with", "different", "type"])

        self.assertEqual([1, 2, 3], s.optional_list_i32)

        # Assigning `None` is reseting the optional field to `None`
        s.optional_list_i32 = None

        # Assigning a value of the wrong type raises a `TypeError` when fields
        # are `None`
        with self.assertRaisesRegex(TypeError, "is not a <class 'int'>"):
            s.optional_list_i32 = to_thrift_list(["list", "with", "different", "type"])

        # Boundary check for integral types
        with self.assertRaises(OverflowError):
            s.optional_list_i32 = to_thrift_list([max_i32 + 1])

    def test_serialization_round_trip(self) -> None:
        s = TestStructAllThriftPrimitiveTypesMutable()
        s.unqualified_string = "Hello world!"
        s.optional_string = "Hello optional!"
        s.unqualified_i32 = 19
        s.optional_i32 = 23
        s.unqualified_double = 2.1
        s.optional_double = 1.3
        s.unqualified_bool = True
        s.optional_bool = False
        _thrift_serialization_round_trip(self, mutable_serializer, s)
        _pickle_round_trip(self, s)

        s_default_value = TestStructAllThriftPrimitiveTypesWithDefaultValuesMutable()
        _thrift_serialization_round_trip(self, mutable_serializer, s_default_value)
        _pickle_round_trip(self, s_default_value)

    def test_create_and_assign_for_list(self) -> None:
        s = TestStructAllThriftContainerTypesMutable(
            unqualified_list_i32=to_thrift_list([1, 2, 3])
        )

        self.assertEqual(3, len(s.unqualified_list_i32))
        self.assertEqual([1, 2, 3], s.unqualified_list_i32)

        # Assigning to a list field
        s.unqualified_list_i32 = to_thrift_list([1, 2, 3, 4, 5])
        self.assertEqual([1, 2, 3, 4, 5], s.unqualified_list_i32)
        s.unqualified_list_i32 = to_thrift_list([1, 2, 3])
        self.assertEqual([1, 2, 3], s.unqualified_list_i32)

        s.unqualified_list_i32[0] = 2
        self.assertEqual([2, 2, 3], s.unqualified_list_i32)

        with self.assertRaisesRegex(IndexError, "list index out of range"):
            s.unqualified_list_i32[4]

        with self.assertRaisesRegex(IndexError, "list assignment index out of range"):
            s.unqualified_list_i32[4] = 2

        with self.assertRaisesRegex(TypeError, "is not a <class 'int'>"):
            # pyre-ignore[6]: Intentional for test
            s.unqualified_list_i32[4] = "Not integer"

        self.assertEqual([2, 2, 3], s.unqualified_list_i32)

        lst1 = s.unqualified_list_i32
        lst2 = s.unqualified_list_i32

        # lists are instance of thrift.python.mutable_containers.MutableList
        self.assertTrue(isinstance(lst1, MutableList))
        self.assertTrue(isinstance(lst2, MutableList))

        # lst1 and lst2 are the same instances
        self.assertTrue(lst1 is lst2)

        # Update on any variable is reflected on others
        self.assertEqual([2, 2, 3], s.unqualified_list_i32)
        self.assertEqual([2, 2, 3], lst1)
        self.assertEqual([2, 2, 3], lst2)

        lst2[1] = 10

        self.assertEqual([2, 10, 3], s.unqualified_list_i32)
        self.assertEqual([2, 10, 3], lst1)
        self.assertEqual([2, 10, 3], lst2)

        lst1.append(101)

        self.assertEqual([2, 10, 3, 101], s.unqualified_list_i32)
        self.assertEqual([2, 10, 3, 101], lst1)
        self.assertEqual([2, 10, 3, 101], lst2)

        self.assertEqual(101, s.unqualified_list_i32.pop())

        self.assertEqual([2, 10, 3], s.unqualified_list_i32)
        self.assertEqual([2, 10, 3], lst1)
        self.assertEqual([2, 10, 3], lst2)

        lst2.clear()

        self.assertEqual([], s.unqualified_list_i32)
        self.assertEqual([], lst1)
        self.assertEqual([], lst2)

        lst1.extend([11, 12, 13])

        self.assertEqual([11, 12, 13], s.unqualified_list_i32)
        self.assertEqual([11, 12, 13], lst1)
        self.assertEqual([11, 12, 13], lst2)

        with self.assertRaisesRegex(TypeError, "is not a <class 'int'>"):
            # pyre-ignore[6]: Intentional for test
            lst2.extend([14, 15, "16", 17])

        # basic exception safety
        self.assertEqual([11, 12, 13, 14, 15], s.unqualified_list_i32)
        self.assertEqual([11, 12, 13, 14, 15], lst1)
        self.assertEqual([11, 12, 13, 14, 15], lst2)

    def test_assign_for_list(self) -> None:
        s1 = TestStructAllThriftContainerTypesMutable(
            unqualified_list_i32=to_thrift_list([1, 2, 3])
        )

        # It is possible to assign any value that supports `len()` and iteration
        s1.unqualified_list_i32 = to_thrift_list([1, 2, 3])
        self.assertEqual([1, 2, 3], s1.unqualified_list_i32)
        s1.unqualified_list_i32 = to_thrift_list({11, 12, 13})
        self.assertEqual([11, 12, 13], s1.unqualified_list_i32)
        s1.unqualified_list_i32 = to_thrift_list((21, 22, 23))
        self.assertEqual([21, 22, 23], s1.unqualified_list_i32)
        s1.unqualified_list_i32 = to_thrift_list([])
        self.assertEqual([], s1.unqualified_list_i32)

        s2 = TestStructAllThriftContainerTypesMutable(
            unqualified_list_i32=to_thrift_list([])
        )
        # my_list and s2.unqualified_list_i32 are different lists
        my_list = [1, 2, 3]
        s2.unqualified_list_i32 = to_thrift_list(my_list)
        self.assertEqual([1, 2, 3], s2.unqualified_list_i32)
        my_list[0] = 11
        self.assertEqual(1, s2.unqualified_list_i32[0])

        s3 = TestStructAllThriftContainerTypesMutable(
            unqualified_list_i32=to_thrift_list([1, 2, 3])
        )
        # Strong exception safety
        with self.assertRaisesRegex(TypeError, "is not a <class 'int'>"):
            s3.unqualified_list_i32 = to_thrift_list([11, 12, 13, "Not an Integer"])
        self.assertEqual([1, 2, 3], s3.unqualified_list_i32)

    def test_assign_for_set(self) -> None:
        s1 = TestStructAllThriftContainerTypesMutable(
            unqualified_set_string=to_thrift_set({"a", "b", "c"})
        )

        # It is possible to assign any value that supports iteration
        s1.unqualified_set_string = to_thrift_set(["a", "b", "c"])
        self.assertEqual({"a", "b", "c"}, s1.unqualified_set_string)
        s1.unqualified_set_string = to_thrift_set({"aa", "bb", "cc"})
        self.assertEqual({"aa", "bb", "cc"}, s1.unqualified_set_string)
        s1.unqualified_set_string = to_thrift_set(("aaa", "bbb", "ccc"))
        self.assertEqual({"aaa", "bbb", "ccc"}, s1.unqualified_set_string)

        # even from iterator, this is not possible for list field because of
        # `len()` requirement
        my_iter = iter(["x", "y", "z"])
        s1.unqualified_set_string = to_thrift_set(my_iter)
        self.assertEqual({"x", "y", "z"}, s1.unqualified_set_string)

        s2 = TestStructAllThriftContainerTypesMutable(
            unqualified_set_string=to_thrift_set([])
        )
        # my_set and s2.unqualified_set_string are different sets
        my_set = {"a", "b", "c"}
        s2.unqualified_set_string = to_thrift_set(my_set)
        self.assertEqual({"a", "b", "c"}, s2.unqualified_set_string)
        my_set.add("d")
        self.assertEqual(4, len(my_set))
        self.assertEqual(3, len(s2.unqualified_set_string))

        s3 = TestStructAllThriftContainerTypesMutable(
            unqualified_set_string=to_thrift_set({"a", "b", "c"})
        )
        # Strong exception safety
        with self.assertRaisesRegex(TypeError, "Expected type <class 'str'>"):
            # the list is necessary to guarantee the order of the elements
            s3.unqualified_set_string = to_thrift_set(["aa", "bb", "cc", 999])
        self.assertEqual({"a", "b", "c"}, s3.unqualified_set_string)

    def test_assign_for_map(self) -> None:
        s1 = TestStructAllThriftContainerTypesMutable(
            unqualified_map_string_i32=to_thrift_map({"a": 1, "b": 2})
        )

        # It is possible to assign any mapping value that implements `items()`
        s1.unqualified_map_string_i32 = to_thrift_map({"x": 1, "y": 2})
        self.assertEqual({"x": 1, "y": 2}, s1.unqualified_map_string_i32)

        class MyMapping:
            def items(self):
                return (("aa", 11), ("bb", 22))

        s1.unqualified_map_string_i32 = to_thrift_map(MyMapping())
        self.assertEqual({"aa": 11, "bb": 22}, s1.unqualified_map_string_i32)

        s2 = TestStructAllThriftContainerTypesMutable(
            unqualified_map_string_i32=to_thrift_map({})
        )
        # my_map and s2.unqualified_map_string_i32 are different maps
        my_map = {"a": 1, "b": 2}
        s2.unqualified_map_string_i32 = to_thrift_map(my_map)
        my_map["c"] = 3
        self.assertEqual(3, len(my_map))
        self.assertEqual(2, len(s2.unqualified_map_string_i32))

        s3 = TestStructAllThriftContainerTypesMutable(
            unqualified_map_string_i32=to_thrift_map({"a": 1, "b": 2})
        )
        # Strong exception safety
        with self.assertRaisesRegex(TypeError, "is not a <class 'int'>"):
            s3.unqualified_map_string_i32 = to_thrift_map(
                {"x": 1, "y": "Not an Integer"}
            )
        self.assertEqual({"a": 1, "b": 2}, s3.unqualified_map_string_i32)

    def test_adapted_types(self) -> None:
        s = TestStructAdaptedTypesMutable()
        # standard default value for i32 is 0, therefore `fromtimestamp(0)`
        self.assertEqual(
            s.unqualified_adapted_i32_to_datetime, datetime.fromtimestamp(0)
        )
        # from IDL, 3: `string unqualified_adapted_string_to_i32 = "123";`
        self.assertEqual(s.unqualified_adapted_string_to_i32, 123)

        new_date = datetime(2024, 5, 6, 12, 0, 0)
        s.unqualified_adapted_i32_to_datetime = new_date
        self.assertEqual(s.unqualified_adapted_i32_to_datetime, new_date)

        # Thrift simply passes the value to the adapter class. All type
        # checking is performed within the adapter class.
        with self.assertRaisesRegex(
            AttributeError, "'int' object has no attribute 'timestamp'"
        ):
            # pyre-ignore[8]: Intentional for test
            s.unqualified_adapted_i32_to_datetime = 123

        s.unqualified_adapted_string_to_i32 = 999
        self.assertEqual(s.unqualified_adapted_string_to_i32, 999)

    def test_typedef_simple(self) -> None:
        empty = TestStructEmptyMutable()
        empty_alias = TestStructEmptyAliasMutable()
        self.assertEqual(empty, empty_alias)

        struct = TestStructWithTypedefFieldMutable()
        struct.empty_struct = empty_alias
        struct.empty_struct_alias = empty

        _thrift_serialization_round_trip(self, mutable_serializer, struct)
        _pickle_round_trip(self, struct)

    def test_create_and_init_for_set(self) -> None:
        # Initializing the `set` member with an iterable that contains duplicate
        # elements is fine. Thrift removes the duplicates.
        s = TestStructAllThriftContainerTypesMutable(
            unqualified_set_string=to_thrift_set(["1", "2", "2", "3", "3"])
        )
        self.assertEqual(3, len(s.unqualified_set_string))
        self.assertEqual({"1", "2", "3"}, s.unqualified_set_string)

        # Initializing the `set` member with an iterable that contains elements
        # with wrong type raises `TypeError`.
        with self.assertRaisesRegex(
            TypeError, "Expected type <class 'str'>, got: <class 'int'>"
        ):
            s = TestStructAllThriftContainerTypesMutable(
                unqualified_set_string=to_thrift_set(["1", "2", "2", 9999, "3", "3"])
            )

    def test_create_and_assign_for_set(self) -> None:
        s = TestStructAllThriftContainerTypesMutable(
            unqualified_set_string=to_thrift_set(["1", "2", "3"])
        )

        self.assertEqual(3, len(s.unqualified_set_string))
        self.assertEqual({"1", "2", "3"}, s.unqualified_set_string)

        # Assigning to a set field
        s.unqualified_set_string = to_thrift_set({"9", "8", "7"})
        self.assertEqual({"9", "8", "7"}, s.unqualified_set_string)
        s.unqualified_set_string = to_thrift_set({"1", "2", "3"})
        self.assertEqual({"1", "2", "3"}, s.unqualified_set_string)

        # `__contains__()`
        self.assertIn("1", s.unqualified_set_string)
        self.assertIn("2", s.unqualified_set_string)
        self.assertNotIn("11", s.unqualified_set_string)
        self.assertNotIn("12", s.unqualified_set_string)

        # `__contains__()` returns `False` even with the wrong key type
        self.assertNotIn(1, s.unqualified_set_string)

        # `add()`
        s.unqualified_set_string.add("4")
        self.assertEqual({"1", "2", "3", "4"}, s.unqualified_set_string)

        # uniqueness
        s.unqualified_set_string.add("3")
        s.unqualified_set_string.add("4")
        self.assertEqual({"1", "2", "3", "4"}, s.unqualified_set_string)

        with self.assertRaisesRegex(
            TypeError, "Expected type <class 'str'>, got: <class 'int'>"
        ):
            # pyre-ignore[6]: Intentional for test
            s.unqualified_set_string.add(999)

        # `remove()`
        s.unqualified_set_string.remove("1")
        self.assertEqual({"2", "3", "4"}, s.unqualified_set_string)

        with self.assertRaisesRegex(
            TypeError, "Expected type <class 'str'>, got: <class 'int'>"
        ):
            # pyre-ignore[6]: Intentional for test
            s.unqualified_set_string.remove(111)

        # `remove()` raises a `KeyError` if key is absent
        with self.assertRaisesRegex(KeyError, "111"):
            s.unqualified_set_string.remove("111")

        # `discard()`
        s.unqualified_set_string.discard("4")
        self.assertEqual({"2", "3"}, s.unqualified_set_string)

        # `discard()` does not raises a `KeyError` or `TypeError`
        s.unqualified_set_string.discard("111")
        # pyre-ignore[6]: Intentional for test
        s.unqualified_set_string.discard(111)

        set1 = s.unqualified_set_string
        set2 = s.unqualified_set_string

        # sets are instance of thrift.python.mutable_containers.MutableSet
        self.assertTrue(isinstance(set1, MutableSet))
        self.assertTrue(isinstance(set2, MutableSet))

        # set1 and set2 are the same instances
        self.assertIs(set1, set2)

        # Update on any variable is reflected on others
        self.assertEqual({"2", "3"}, s.unqualified_set_string)
        self.assertEqual({"2", "3"}, set1)
        self.assertEqual({"2", "3"}, set2)

        set1.add("1")

        self.assertEqual({"1", "2", "3"}, s.unqualified_set_string)
        self.assertEqual({"1", "2", "3"}, set1)
        self.assertEqual({"1", "2", "3"}, set2)

        # `isdisjoint(iterable)`
        self.assertTrue(set1.isdisjoint(["4", "5"]))
        self.assertTrue(set2.isdisjoint({"4", "5"}))
        self.assertFalse(set1.isdisjoint(["3", "4"]))
        self.assertFalse(set2.isdisjoint({"3", "4"}))

        other = TestStructAllThriftContainerTypesMutable(
            unqualified_set_string=to_thrift_set(["2", "3", "4"])
        )
        other_set = other.unqualified_set_string
        self.assertEqual({"1", "2", "3"}, set1)

        # `__and__()` (both sides are MutableSet)
        result = set1 & other_set
        self.assertIsInstance(result, MutableSet)
        self.assertIsNot(result, set1)
        self.assertIsNot(result, other_set)
        self.assertEqual({"1", "2", "3"}, set1)
        self.assertEqual({"2", "3", "4"}, other_set)
        self.assertEqual({"2", "3"}, result)

        # `__and__()` (left hand side is MutableSet)
        result = set1 & {"2", "3", "4"}
        self.assertIsInstance(result, MutableSet)
        self.assertEqual({"2", "3"}, result)

        # `__rand__() Not implemented yet` (right hand side is MutableSet)
        with self.assertRaisesRegex(TypeError, r"unsupported operand type\(s\) for \&"):
            result = {"2", "3", "4"} & set1

        # `__or__()` (both sides are MutableSet)
        result = set1 | other_set
        self.assertIsInstance(result, MutableSet)
        self.assertIsNot(result, set1)
        self.assertIsNot(result, other_set)
        self.assertEqual({"1", "2", "3"}, set1)
        self.assertEqual({"2", "3", "4"}, other_set)
        self.assertEqual({"1", "2", "3", "4"}, result)

        # `__or__()` (left hand side is MutableSet)
        result = set1 | {"2", "3", "4"}
        self.assertIsInstance(result, MutableSet)
        self.assertEqual({"1", "2", "3", "4"}, result)

        # `__ror__() Not implemented yet` (right hand side is MutableSet)
        with self.assertRaisesRegex(TypeError, r"unsupported operand type\(s\) for \|"):
            result = {"2", "3", "4"} | set1

        # `MutableSet` instances are not hashable
        with self.assertRaisesRegex(
            TypeError, "unhashable type: 'thrift.python.mutable_containers.MutableSet'"
        ):
            hash(set1)

        _thrift_serialization_round_trip(self, mutable_serializer, s)
        _thrift_serialization_round_trip(self, mutable_serializer, other)

        # `clear()`
        set2.clear()
        self.assertEqual(set(), s.unqualified_set_string)
        self.assertEqual(set(), set1)
        self.assertEqual(set(), set2)

    def test_create_and_assign_for_map(self) -> None:
        s = TestStructAllThriftContainerTypesMutable(
            unqualified_map_string_i32=to_thrift_map({"a": 1, "b": 2})
        )

        self.assertEqual(2, len(s.unqualified_map_string_i32))
        self.assertEqual({"a": 1, "b": 2}, s.unqualified_map_string_i32)

        # Assigning to a map field
        s.unqualified_map_string_i32 = to_thrift_map({"x": 1, "y": 2})
        self.assertEqual({"x": 1, "y": 2}, s.unqualified_map_string_i32)
        s.unqualified_map_string_i32 = to_thrift_map({"a": 1, "b": 2})
        self.assertEqual({"a": 1, "b": 2}, s.unqualified_map_string_i32)

        map1 = s.unqualified_map_string_i32
        map2 = s.unqualified_map_string_i32

        # maps are instance of thrift.python.mutable_containers.MutableMap
        self.assertTrue(isinstance(map1, MutableMap))
        self.assertTrue(isinstance(map2, MutableMap))

        # map1 and map2 are the same instances
        self.assertIs(map1, map2)

        # Update on any variable is reflected on others
        map1["c"] = 3
        self.assertEqual({"a": 1, "b": 2, "c": 3}, s.unqualified_map_string_i32)
        self.assertEqual({"a": 1, "b": 2, "c": 3}, map1)
        self.assertEqual({"a": 1, "b": 2, "c": 3}, map2)

        # `__contains__()`
        self.assertIn("a", s.unqualified_map_string_i32)
        self.assertIn("b", s.unqualified_map_string_i32)
        self.assertNotIn("x", s.unqualified_map_string_i32)
        self.assertNotIn("y", s.unqualified_map_string_i32)

        # `__contains__()` returns `False` even with the wrong key type
        self.assertNotIn(1, s.unqualified_map_string_i32)

        # `__getitem__()`
        self.assertEqual(1, s.unqualified_map_string_i32["a"])
        self.assertEqual(2, s.unqualified_map_string_i32["b"])
        self.assertEqual(3, s.unqualified_map_string_i32["c"])

        with self.assertRaises(KeyError):
            s.unqualified_map_string_i32["Not Exists"]

        # `__getitem__()` is type checked
        with self.assertRaisesRegex(
            TypeError, "Expected type <class 'str'>, got: <class 'int'>"
        ):
            # pyre-ignore[6]: Intentional for test
            s.unqualified_map_string_i32[999]

        # `get()`
        self.assertEqual(1, s.unqualified_map_string_i32.get("a"))
        # `get(, default=None)`
        self.assertEqual(None, s.unqualified_map_string_i32.get("Not Exists"))
        self.assertEqual(
            "MyDefaultValue",
            s.unqualified_map_string_i32.get("Not Exists", "MyDefaultValue"),
        )

        # `get()` is type checked
        with self.assertRaisesRegex(
            TypeError, "Expected type <class 'str'>, got: <class 'int'>"
        ):
            # pyre-ignore[6]: Intentional for test
            s.unqualified_map_string_i32.get(999, "MyDefaultValue")

        # `__setitem__()`
        self.assertEqual(1, s.unqualified_map_string_i32["a"])
        s.unqualified_map_string_i32["a"] = 11
        self.assertEqual(11, s.unqualified_map_string_i32["a"])
        s.unqualified_map_string_i32["a"] = 1
        self.assertEqual(1, s.unqualified_map_string_i32["a"])

        # `__setitem__()` is type checked for both key and value
        with self.assertRaisesRegex(
            TypeError, "not a <class 'int'>, is actually of type <class 'str'>"
        ):
            # pyre-ignore[6]: Intentional for test
            s.unqualified_map_string_i32["a"] = "Not an integer"

        with self.assertRaisesRegex(
            TypeError, "Expected type <class 'str'>, got: <class 'int'>"
        ):
            # pyre-ignore[6]: Intentional for test
            s.unqualified_map_string_i32[999] = 11

        # `__iter__()`
        python_set = {"a", "b", "c"}
        for key in s.unqualified_map_string_i32:
            # `remove()` raises a `KeyError` if key is absent
            python_set.remove(key)

        self.assertEqual(0, len(python_set))

        # `keys()`
        self.assertEqual(3, len(s.unqualified_map_string_i32.keys()))

        python_set = {"a", "b", "c"}
        for key in s.unqualified_map_string_i32.keys():
            # `remove()` raises a `KeyError` if key is absent
            python_set.remove(key)

        # `keys()` returns a view
        keys = s.unqualified_map_string_i32.keys()
        self.assertEqual(3, len(keys))
        self.assertEqual({"a", "b", "c"}, set(keys))

        s.unqualified_map_string_i32["d"] = 4

        self.assertEqual(4, len(keys))
        self.assertEqual({"a", "b", "c", "d"}, set(keys))

        # `values()`
        self.assertEqual(4, len(s.unqualified_map_string_i32.values()))

        python_list = [1, 2, 3, 4]
        for value in s.unqualified_map_string_i32.values():
            python_list.remove(value)

        # `values()` returns a view
        values = s.unqualified_map_string_i32.values()
        self.assertEqual(4, len(values))
        self.assertEqual([1, 2, 3, 4], sorted(values))

        s.unqualified_map_string_i32["e"] = 5

        self.assertEqual(5, len(values))
        self.assertEqual([1, 2, 3, 4, 5], sorted(values))

        # `items()`
        self.assertEqual(5, len(s.unqualified_map_string_i32.items()))
        self.assertEqual(
            [("a", 1), ("b", 2), ("c", 3), ("d", 4), ("e", 5)],
            sorted(s.unqualified_map_string_i32.items()),
        )

        # `items()` returns a view
        items = s.unqualified_map_string_i32.items()
        self.assertEqual(5, len(items))
        self.assertEqual(
            [("a", 1), ("b", 2), ("c", 3), ("d", 4), ("e", 5)],
            sorted(items),
        )

        s.unqualified_map_string_i32["f"] = 6

        self.assertEqual(6, len(items))
        self.assertEqual(
            [("a", 1), ("b", 2), ("c", 3), ("d", 4), ("e", 5), ("f", 6)],
            sorted(items),
        )

        # `MutableMap` instances are not hashable
        with self.assertRaisesRegex(
            TypeError, "unhashable type: 'thrift.python.mutable_containers.MutableMap'"
        ):
            hash(s.unqualified_map_string_i32)

        _thrift_serialization_round_trip(self, mutable_serializer, s)

        # `clear()`
        s.unqualified_map_string_i32.clear()
        self.assertEqual({}, s.unqualified_map_string_i32)
        self.assertEqual({}, map1)
        self.assertEqual({}, map2)

    def test_constants(self) -> None:
        s = TestStructAllThriftPrimitiveTypesMutable()

        self.assertEqual(False, s.unqualified_bool)
        s.unqualified_bool = bool_constant
        self.assertEqual(True, s.unqualified_bool)

        self.assertEqual(0, s.unqualified_byte)
        s.unqualified_byte = byte_constant
        self.assertEqual(-10, s.unqualified_byte)

        self.assertEqual(0, s.unqualified_i16)
        s.unqualified_i16 = i16_constant
        self.assertEqual(200, s.unqualified_i16)

        self.assertEqual(0, s.unqualified_i32)
        s.unqualified_i32 = i32_constant
        self.assertEqual(0xFA12EE, s.unqualified_i32)

        self.assertEqual(0, s.unqualified_i64)
        s.unqualified_i64 = i64_constant
        self.assertEqual(0xFFFFFFFFFF, s.unqualified_i64)

        self.assertEqual(0, s.unqualified_float)
        s.unqualified_float = float_constant
        self.assertEqual(2.718281828459, s.unqualified_float)

        self.assertEqual(0, s.unqualified_double)
        s.unqualified_double = double_constant
        self.assertEqual(2.718281828459, s.unqualified_double)

        self.assertEqual("", s.unqualified_string)
        s.unqualified_string = string_constant
        self.assertEqual("June 28, 2017", s.unqualified_string)

        s = TestStructAllThriftContainerTypesMutable()

        self.assertEqual([], s.unqualified_list_i32)
        s.unqualified_list_i32.extend(list_constant)
        self.assertEqual([2, 3, 5, 7], s.unqualified_list_i32)

        s = TestStructAllThriftContainerTypesMutable(
            # pyre-ignore[6]: Fixme: type error to be addressed later
            unqualified_set_string=set_constant
        )
        self.assertEqual({"foo", "bar", "baz"}, s.unqualified_set_string)

        s = TestStructAllThriftContainerTypesMutable(
            # pyre-ignore[6]: Fixme: type error to be addressed later
            unqualified_map_string_i32=map_constant
        )
        self.assertEqual({"foo": 1, "bar": 2}, s.unqualified_map_string_i32)

    def test_nested_structs_init(self) -> None:
        """
        struct TestStructNested_2 {
          1: i32 i32_field;
        }

        struct TestStructNested_1 {
          1: i32 i32_field;
          2: TestStructNested_2 nested_2;
        }

        struct TestStructNested_0 {
          1: i32 i32_field;
          2: TestStructNested_1 nested_1;
        }
        """
        s2 = TestStructNested_2_Mutable(i32_field=2)
        s1 = TestStructNested_1_Mutable(i32_field=3, nested_2=s2)
        s0 = TestStructNested_0_Mutable(i32_field=5, nested_1=s1)

        self.assertEqual(s2, s0.nested_1.nested_2)
        self.assertEqual(s1, s0.nested_1)

        self.assertEqual(2, s0.nested_1.nested_2.i32_field)
        self.assertEqual(3, s0.nested_1.i32_field)

        # Update on `s2` updates both `s1` and `s2`
        self.assertEqual(2, s0.nested_1.nested_2.i32_field)
        self.assertEqual(2, s1.nested_2.i32_field)
        s2.i32_field = 7
        self.assertEqual(7, s0.nested_1.nested_2.i32_field)
        self.assertEqual(7, s1.nested_2.i32_field)

        # Accessing the same field returns the same instances
        my_s2_var1 = s0.nested_1.nested_2
        my_s2_var2 = s0.nested_1.nested_2

        self.assertIs(my_s2_var1, my_s2_var2)

        # Update on `my_s2_var1` or `my_s2_var2` updates all references
        my_s2_var1.i32_field = 11
        self.assertEqual(11, s0.nested_1.nested_2.i32_field)
        self.assertEqual(11, s1.nested_2.i32_field)
        self.assertEqual(11, s2.i32_field)

        # Unfortunately, this is not intuitive, `my_s2_var1` is not `s1`
        self.assertIs(my_s2_var1, my_s2_var2)
        self.assertIsNot(s1, my_s2_var2)

    def test_exception(self) -> None:
        s = TestStructAllThriftPrimitiveTypesMutable()
        self.assertIsInstance(s, MutableStruct)
        self.assertNotIsInstance(s, MutableGeneratedError)

        # `Error` <- `MutableGeneratedError`
        # is an independent type hierarchy than
        # `MutableStructOrUnion` <- `MutableStruct`
        e = TestExceptionAllThriftPrimitiveTypesMutable()
        self.assertNotIsInstance(e, MutableStructOrUnion)
        self.assertNotIsInstance(e, MutableStruct)
        self.assertIsInstance(e, Error)
        self.assertIsInstance(e, MutableGeneratedError)

    def test_call_as_deepcopy(self) -> None:
        """
        struct TestStructCopy {
          1: i32 unqualified_i32;
          2: optional i32 optional_i32;

          3: string unqualified_string;
          4: optional string optional_string;

          5: list<i32> unqualified_list_i32;
          6: set<string> unqualified_set_string;
          7: map<string, i32> unqualified_map_string_i32;

          8: optional TestStructCopy recursive_struct;

          @cpp.Type{name = "folly::IOBuf"}
          9: binary unqualified_binary;
        }
        """
        s = TestStructCopyMutable(
            unqualified_i32=2,
            optional_i32=3,
            unqualified_string="thrift",
            optional_string="python",
            unqualified_list_i32=to_thrift_list([1, 2, 3]),
            unqualified_set_string=to_thrift_set({"1", "2", "3"}),
            unqualified_map_string_i32=to_thrift_map({"a": 1, "b": 2, "c": 3}),
            unqualified_binary=IOBuf(b"abc"),
        )
        s_clone = s()

        self.assertIsNot(s, s_clone)

        self.assertEqual(s.unqualified_list_i32, s_clone.unqualified_list_i32)
        self.assertIsNot(s.unqualified_list_i32, s_clone.unqualified_list_i32)

        # Although `assertIsNot()` appears to check the deepcopy result, the
        # best way to test it is to mutate `s_clone` (due to implementation
        # details) and verify that it doesn't affect `s`.
        s_clone.unqualified_list_i32.append(4)
        self.assertNotEqual(s.unqualified_list_i32, s_clone.unqualified_list_i32)

        self.assertEqual(s.unqualified_set_string, s_clone.unqualified_set_string)
        self.assertIsNot(s.unqualified_set_string, s_clone.unqualified_set_string)
        s_clone.unqualified_set_string.add("4")
        self.assertNotEqual(s.unqualified_set_string, s_clone.unqualified_set_string)

        self.assertEqual(
            s.unqualified_map_string_i32, s_clone.unqualified_map_string_i32
        )
        self.assertIsNot(
            s.unqualified_map_string_i32, s_clone.unqualified_map_string_i32
        )
        s_clone.unqualified_map_string_i32["d"] = 4
        self.assertNotEqual(
            s.unqualified_map_string_i32, s_clone.unqualified_map_string_i32
        )
        self.assertEqual(b"abc", bytes(s_clone.unqualified_binary))
        self.assertIs(s.unqualified_binary, s_clone.unqualified_binary)

    def test_call_reset_field_to_standard_default(self) -> None:
        s1 = TestStructCopyMutable(
            unqualified_i32=2,
            optional_i32=3,
            unqualified_string="thrift",
            optional_string="python",
            unqualified_list_i32=to_thrift_list([1, 2, 3]),
            unqualified_set_string=to_thrift_set({"1", "2", "3"}),
            unqualified_map_string_i32=to_thrift_map({"a": 1, "b": 2, "c": 3}),
        )

        # Assigning `None` to a field is resetting the field to its standard
        # default value
        s2 = s1(
            unqualified_i32=None,
            optional_string=None,
            unqualified_set_string=None,
            unqualified_map_string_i32=to_thrift_map({"d": 4}),
        )

        self.assertEqual(0, s2.unqualified_i32)
        self.assertEqual(3, s2.optional_i32)
        self.assertEqual("thrift", s2.unqualified_string)
        self.assertIsNone(s2.optional_string)
        self.assertEqual([1, 2, 3], s2.unqualified_list_i32)
        self.assertEqual(set(), s2.unqualified_set_string)
        self.assertEqual({"d": 4}, s2.unqualified_map_string_i32)

    def test_exception_deepcopy(self) -> None:
        """
        exception TestExceptionCopy {
          1: i32 unqualified_i32;
          2: optional i32 optional_i32;

          3: string unqualified_string;
          4: optional string optional_string;

          5: list<i32> unqualified_list_i32;
          6: optional list<i32> optional_list_i32;

          7: set<string> unqualified_set_string;
          8: optional set<string> optional_set_string;

          9: map<string, i32> unqualified_map_string_i32;
          10: optional TestExceptionCopy recursive_exception;
        }
        """
        e = TestExceptionCopyMutable(
            unqualified_i32=2,
            optional_i32=3,
            unqualified_string="thrift",
            optional_string="python",
            unqualified_list_i32=to_thrift_list([1, 2, 3]),
            optional_list_i32=to_thrift_list([4, 5, 6]),
            unqualified_set_string=to_thrift_set({"1", "2", "3"}),
            unqualified_map_string_i32=to_thrift_map({"a": 1, "b": 2, "c": 3}),
        )
        e_clone = copy.deepcopy(e)

        self.assertIsNot(e, e_clone)

        self.assertEqual(e.unqualified_i32, e_clone.unqualified_i32)
        self.assertEqual(e.optional_i32, e_clone.optional_i32)
        self.assertEqual(e.unqualified_string, e_clone.unqualified_string)
        self.assertEqual(e.optional_string, e_clone.optional_string)

        e_clone.optional_i32 = None
        self.assertIsNone(e_clone.optional_i32)
        self.assertIsNotNone(e.optional_i32)

        self.assertEqual(e.unqualified_list_i32, e_clone.unqualified_list_i32)
        self.assertIsNot(e.unqualified_list_i32, e_clone.unqualified_list_i32)
        self.assertEqual(e.optional_list_i32, e_clone.optional_list_i32)
        self.assertIsNot(e.optional_list_i32, e_clone.optional_list_i32)

        # Although `assertIsNot()` appears to check the deepcopy result, the
        # best way to test it is to mutate `e_clone` (due to implementation
        # details) and verify that it doesn't affect `e`.
        e_clone.unqualified_list_i32.append(4)
        self.assertNotEqual(e.unqualified_list_i32, e_clone.unqualified_list_i32)
        if e_clone.optional_list_i32 is not None:
            e_clone.optional_list_i32.append(4)

        self.assertNotEqual(e.optional_list_i32, e_clone.optional_list_i32)

        self.assertEqual(e.unqualified_set_string, e_clone.unqualified_set_string)
        self.assertIsNot(e.unqualified_set_string, e_clone.unqualified_set_string)
        e_clone.unqualified_set_string.add("4")
        self.assertNotEqual(e.unqualified_set_string, e_clone.unqualified_set_string)

        self.assertIsNone(e_clone.optional_set_string)

        self.assertEqual(
            e.unqualified_map_string_i32, e_clone.unqualified_map_string_i32
        )
        self.assertIsNot(
            e.unqualified_map_string_i32, e_clone.unqualified_map_string_i32
        )
        e_clone.unqualified_map_string_i32["d"] = 4
        self.assertNotEqual(
            e.unqualified_map_string_i32, e_clone.unqualified_map_string_i32
        )

    def test_struct_constant(self) -> None:
        """
        const TestStructConstant struct_constant = {
          "unqualified_i32": 42,
          "unqualified_string": "Hello world!",
          "unqualified_list_i32": [1, 2, 3],
        };
        """
        # It is just a MutableStruct
        self.assertIsInstance(struct_constant, MutableStruct)

        self.assertEqual(42, struct_constant.unqualified_i32)
        self.assertEqual("Hello world!", struct_constant.unqualified_string)
        self.assertEqual([1, 2, 3], struct_constant.unqualified_list_i32)

        # It is `const` but it is possible to mutate
        struct_constant.unqualified_list_i32.append(4)
        self.assertEqual([1, 2, 3, 4], struct_constant.unqualified_list_i32)

    def test_list_container_assignment(self) -> None:
        """
        struct TestStructAsListElement {
          1: string string_field;
          2: list<i32> list_int;
        }

        struct TestStructContainerAssignment {
          1: list<i32> list_int;
          2: list<i32> list_int_2;
          3: list<list<i32>> list_list_int;
          4: list<list<i32>> list_list_int_2;
          5: list<TestStructAsListElement> list_struct;
        }
        """
        TypeErrorMsg = (
            "Expected values to be an instance of Thrift mutable list with "
            r"matching element type, or the result of `to_thrift_list\(\)`, "
            "but got type <class 'list'>."
        )

        # Cannot initialize a struct with python list.
        with self.assertRaisesRegex(TypeError, TypeErrorMsg):
            # pyre-ignore[6]: Intentional for test
            _ = TestStructContainerAssignmentMutable(list_int=[1, 2, 3])

        my_list = [1, 2, 3]
        # Valid initialization with `to_thrift_list()`.
        s1 = TestStructContainerAssignmentMutable(list_int=to_thrift_list(my_list))

        # Initializing with `to_thrift_list()` uses value semantics, so `s` is
        # initialized with a copy of `my_list`.
        self.assertEqual([1, 2, 3], s1.list_int)
        my_list.append(4)
        self.assertEqual([1, 2, 3], s1.list_int)

        # It is possible to initialize a struct with `MutableList[T]`, but T
        # must be an `int` and within `i32` boundaries.
        s2 = TestStructContainerAssignmentMutable(list_int=s1.list_int)
        self.assertEqual([1, 2, 3], s1.list_int)
        self.assertEqual([1, 2, 3], s2.list_int)

        # Initializing with `MutableList[T]` uses a reference semantics.
        s1.list_int.append(4)
        self.assertEqual([1, 2, 3, 4], s1.list_int)
        self.assertEqual([1, 2, 3, 4], s2.list_int)

        # Heads-up: Thrift doesn't provide an `is` check guarantee, even though
        # they are the "same" list.
        self.assertIsNot(s1.list_int, s2.list_int)

        # `to_thrift_list()` doesn't disable type-checking for element types.
        # Even though pyre will not emit a type error in the example below,
        # the runtime will throw a `TypeError`.
        with self.assertRaisesRegex(
            TypeError, "is not a <class 'int'>, is actually of type <class 'str'>"
        ):
            _ = TestStructContainerAssignmentMutable(
                list_int=to_thrift_list(["1", "2", "3"])
            )

        # `__call__()` is similar to `__init__()`, it doesn't accept python list.
        with self.assertRaisesRegex(TypeError, TypeErrorMsg):
            # pyre-ignore[6]: Intentional for test
            _ = s1(list_int=[1, 2, 3])

        # `__call__()` semantics are a deep copy of the object and assign the
        # fields provided in the argument list.
        #
        # my_list = [1]
        # s2 = s1(list_int=to_thrift_list(my_list))
        #
        # can be thought as
        #
        # s2 = copy.deepcopy(s1)
        # s2.list_int = to_thrift_list(my_list)
        s1 = TestStructContainerAssignmentMutable(list_int=to_thrift_list([1, 2, 3]))
        my_list = [1]
        s2 = s1(list_int=to_thrift_list(my_list))
        self.assertNotEqual(s1, s2)
        self.assertNotEqual(s1.list_int, s2.list_int)
        self.assertEqual([1, 2, 3], s1.list_int)
        self.assertEqual([1], s2.list_int)

        # If `__call__()` argument is `MutableList[T]`, then it uses reference
        # semantics. Thinking of `__call__()` as a deep copy first and then
        # assigning the fields provided in the argument list makes the
        # reasoning easier.
        s3 = s1(list_int=s2.list_int)
        self.assertNotEqual(s1.list_int, s3.list_int)
        self.assertEqual([1, 2, 3], s1.list_int)
        # At this point, `s2.list_int` and `s3.list_int` are the "same" MutableLists.
        self.assertEqual([1], s2.list_int)
        self.assertEqual([1], s3.list_int)
        s2.list_int.append(2)
        self.assertEqual([1, 2], s2.list_int)
        self.assertEqual([1, 2], s3.list_int)
        s3.list_int.append(3)
        self.assertEqual([1, 2, 3], s2.list_int)
        self.assertEqual([1, 2, 3], s3.list_int)

        # Even `[]` is not special, it should be assigned with `to_thrift_list()`.
        with self.assertRaisesRegex(TypeError, TypeErrorMsg):
            # pyre-ignore[8]: Intentional for test
            s3.list_int = []

        # `s2.list_int` and `s3.list_int` are not "same" anymore.
        s3.list_int = to_thrift_list([])
        self.assertEqual([1, 2, 3], s2.list_int)
        self.assertEqual([], s3.list_int)

        # Similar for nested lists, cannot be initialized with a python list.
        x = [1, 2]
        y = [3, 4]
        my_list_of_lists = [x, y, s2.list_int]
        with self.assertRaisesRegex(TypeError, TypeErrorMsg):
            # pyre-ignore[8]: Intentional for test
            s1.list_list_int = my_list_of_lists

        s1.list_list_int = to_thrift_list(my_list_of_lists)

        # Putting `s2.list_int` in `my_list_of_lists` doesn't introduce partial
        # reference semantics. All the containers inside `to_thrift_list()` are
        # copied.
        self.assertEqual([1, 2, 3], s2.list_int)
        self.assertEqual([[1, 2], [3, 4], [1, 2, 3]], s1.list_list_int)

        s2.list_int.append(4)
        self.assertEqual([1, 2, 3, 4], s2.list_int)
        self.assertEqual([[1, 2], [3, 4], [1, 2, 3]], s1.list_list_int)

        # `to_thrift_list()` always copies the containers inside it. However,
        # if the container element is a Thrift struct, union or exception,
        # assignment uses reference semantics for them.
        s_elem1 = TestStructAsListElementMutable(
            string_field="elem1", list_int=to_thrift_list([1])
        )
        s_elem2 = TestStructAsListElementMutable(
            string_field="elem2", list_int=to_thrift_list([2])
        )
        s_elem3 = TestStructAsListElementMutable(
            string_field="elem3", list_int=to_thrift_list([3])
        )

        # Cannot initialize a struct with python list.
        with self.assertRaisesRegex(TypeError, TypeErrorMsg):
            _ = TestStructContainerAssignmentMutable(
                # pyre-ignore[6]: Intentional for test
                list_struct=[s_elem1, s_elem2, s_elem3]
            )

        s = TestStructContainerAssignmentMutable(
            list_struct=to_thrift_list([s_elem1, s_elem2, s_elem3])
        )

        # `s_elem1` and `s.list_struct[0]` are the same structs
        # `s_elem2` and `s.list_struct[1]` are the same structs
        # `s_elem3` and `s.list_struct[2]` are the same structs
        self.assertEqual(s_elem1, s.list_struct[0])
        self.assertEqual([1], s_elem1.list_int)
        self.assertEqual([1], s.list_struct[0].list_int)
        s_elem1.list_int.append(2)
        self.assertEqual([1, 2], s_elem1.list_int)
        self.assertEqual([1, 2], s.list_struct[0].list_int)
        s.list_struct[0].list_int.append(3)
        self.assertEqual([1, 2, 3], s_elem1.list_int)
        self.assertEqual([1, 2, 3], s.list_struct[0].list_int)

    def test_set_container_assignment(self) -> None:
        """
        struct TestStructContainerAssignment {
          ...
          6: set<string> set_string;
          7: set<string> set_string_2;
        }
        """
        TypeErrorMsg = (
            "Expected values to be an instance of Thrift mutable set with "
            r"matching element type, or the result of `to_thrift_set\(\)`, "
            "but got type <class 'set'>."
        )

        # Cannot initialize a struct with python set.
        with self.assertRaisesRegex(TypeError, TypeErrorMsg):
            # pyre-ignore[6]: Intentional for test
            _ = TestStructContainerAssignmentMutable(set_string={"1", "2", "3"})

        my_set = {"1", "2", "3"}
        # Valid initialization with `to_thrift_set()`.
        s1 = TestStructContainerAssignmentMutable(set_string=to_thrift_set(my_set))

        # Initializing with `to_thrift_set()` uses value semantics, so `s` is
        # initialized with a copy of `my_set`.
        self.assertEqual({"1", "2", "3"}, s1.set_string)
        my_set.add("4")
        self.assertEqual({"1", "2", "3"}, s1.set_string)

        # It is possible to initialize a struct with `MutableSet[T]`, but T
        # must be an `string`.
        s2 = TestStructContainerAssignmentMutable(set_string=s1.set_string)
        self.assertEqual({"1", "2", "3"}, s1.set_string)
        self.assertEqual({"1", "2", "3"}, s2.set_string)

        # Initializing with `MutableSet[T]` uses a reference semantics.
        s1.set_string.add("4")
        self.assertEqual({"1", "2", "3", "4"}, s1.set_string)
        self.assertEqual({"1", "2", "3", "4"}, s2.set_string)

        # Heads-up: Thrift doesn't provide an `is` check guarantee, even though
        # they are the "same" set.
        self.assertIsNot(s1.set_string, s2.set_string)

        # `to_thrift_set()` doesn't disable type-checking for element types.
        # Even though pyre will not emit a type error in the example below,
        # the runtime will throw a `TypeError`.
        with self.assertRaisesRegex(
            TypeError, "Expected type <class 'str'>, got: <class 'int'>"
        ):
            _ = TestStructContainerAssignmentMutable(
                set_string=to_thrift_set({1, 2, 3})
            )

        # `__call__()` is similar to `__init__()`, it doesn't accept python set.
        with self.assertRaisesRegex(TypeError, TypeErrorMsg):
            # pyre-ignore[6]: Intentional for test
            _ = s1(set_string={"1", "2", "3"})

        # `__call__()` semantics are a deep copy of the object and assign the
        # fields provided in the argument list.
        #
        # my_set = {"1"}
        # s2 = s1(set_string=to_thrift_set(my_set))
        #
        # can be thought as
        #
        # s2 = copy.deepcopy(s1)
        # s2.set_string = to_thrift_set(my_set)
        s1 = TestStructContainerAssignmentMutable(
            set_string=to_thrift_set({"1", "2", "3"})
        )
        my_set = {"1"}
        s2 = s1(set_string=to_thrift_set(my_set))
        self.assertNotEqual(s1, s2)
        self.assertNotEqual(s1.set_string, s2.set_string)
        self.assertEqual({"1", "2", "3"}, s1.set_string)
        self.assertEqual({"1"}, s2.set_string)

        # If `__call__()` argument is `MutableSet[T]`, then it uses reference
        # semantics. Thinking of `__call__()` as a deep copy first and then
        # assigning the fields provided in the argument list makes the
        # reasoning easier.
        s3 = s1(set_string=s2.set_string)
        self.assertNotEqual(s1.set_string, s3.set_string)
        self.assertEqual({"1", "2", "3"}, s1.set_string)
        # At this point, `s2.set_string` and `s3.set_string` are the "same" MutableSets.
        self.assertEqual({"1"}, s2.set_string)
        self.assertEqual({"1"}, s3.set_string)
        s2.set_string.add("2")
        self.assertEqual({"1", "2"}, s2.set_string)
        self.assertEqual({"1", "2"}, s3.set_string)
        s2.set_string.add("3")
        self.assertEqual({"1", "2", "3"}, s2.set_string)
        self.assertEqual({"1", "2", "3"}, s3.set_string)

        # Even `set()` is not special, it should be assigned with `to_thrift_set()`.
        with self.assertRaisesRegex(TypeError, TypeErrorMsg):
            # pyre-ignore[8]: Intentional for test
            s3.set_string = set()

        # `s2.set_string` and `s3.set_string` are not "same" anymore.
        s3.set_string = to_thrift_set(set())
        self.assertEqual({"1", "2", "3"}, s2.set_string)
        self.assertEqual(set(), s3.set_string)

    def test_map_container_assignment(self) -> None:
        """
        struct TestStructContainerAssignment {
          ...
          8: map<i32, list<i32>> map_int_to_list_int;
        }
        """
        TypeErrorMsg = (
            "Expected values to be an instance of Thrift mutable map with matching "
            r"key type and value type, or the result of `to_thrift_map\(\)`, "
            "but got type <class 'dict'>"
        )

        # Cannot initialize a struct with python dict.
        with self.assertRaisesRegex(TypeError, TypeErrorMsg):
            # pyre-ignore[6]: Intentional for test
            _ = TestStructContainerAssignmentMutable(map_int_to_list_int={1: [1]})

        my_map = {1: [1]}
        # Valid initialization with `to_thrift_map()`.
        s1 = TestStructContainerAssignmentMutable(
            map_int_to_list_int=to_thrift_map(my_map)
        )

        # Initializing with `to_thrift_map()` uses value semantics, so `s` is
        # initialized with a copy of `my_map`.
        self.assertEqual({1: [1]}, s1.map_int_to_list_int)
        my_map[2] = [2]
        self.assertEqual({1: [1]}, s1.map_int_to_list_int)

        # It is possible to initialize a struct with `MutableMap[K, V]`, but K
        # and V must match.
        s2 = TestStructContainerAssignmentMutable(
            map_int_to_list_int=s1.map_int_to_list_int
        )
        self.assertEqual({1: [1]}, s1.map_int_to_list_int)
        self.assertEqual({1: [1]}, s2.map_int_to_list_int)

        # Initializing with `MutableMap[K, V]` uses a reference semantics.
        s1.map_int_to_list_int[2] = to_thrift_list([2])
        self.assertEqual({1: [1], 2: [2]}, s1.map_int_to_list_int)
        self.assertEqual({1: [1], 2: [2]}, s2.map_int_to_list_int)

        # Heads-up: Thrift doesn't provide an `is` check guarantee, even though
        # they are the "same" list.
        self.assertIsNot(s1.map_int_to_list_int, s2.map_int_to_list_int)

        # `to_thrift_map()` doesn't disable type-checking for element types.
        # Even though pyre will not emit a type error in the example below,
        # the runtime will throw a `TypeError`.
        with self.assertRaisesRegex(
            TypeError, "is not a <class 'int'>, is actually of type <class 'str'>"
        ):
            _ = TestStructContainerAssignmentMutable(
                map_int_to_list_int=to_thrift_map({"1": [1]})
            )

        # `__call__()` is similar to `__init__()`, it doesn't accept python dict.
        with self.assertRaisesRegex(TypeError, TypeErrorMsg):
            # pyre-ignore[6]: Intentional for test
            _ = TestStructContainerAssignmentMutable(map_int_to_list_int={"1": [1]})

        # `__call__()` semantics are a deep copy of the object and assign the
        # fields provided in the argument list.
        #
        # my_map = {1: [1]}
        # s2 = s1(map_int_to_list_int=to_thrift_map(my_map))
        #
        # can be thought as
        #
        # s2 = copy.deepcopy(s1)
        # s2.map_int_to_list_int = to_thrift_map(my_map)
        s1 = TestStructContainerAssignmentMutable(
            map_int_to_list_int=to_thrift_map({1: [1]})
        )
        my_map = {2: [2]}
        s2 = s1(map_int_to_list_int=to_thrift_map(my_map))
        self.assertNotEqual(s1, s2)
        self.assertNotEqual(s1.map_int_to_list_int, s2.map_int_to_list_int)
        self.assertEqual({1: [1]}, s1.map_int_to_list_int)
        self.assertEqual({2: [2]}, s2.map_int_to_list_int)

        # If `__call__()` argument is `MutableMap[K, V]`, then it uses reference
        # semantics. Thinking of `__call__()` as a deep copy first and then
        # assigning the fields provided in the argument list makes the
        # reasoning easier.
        s3 = s1(map_int_to_list_int=s2.map_int_to_list_int)
        self.assertNotEqual(s1.map_int_to_list_int, s3.map_int_to_list_int)
        self.assertEqual({1: [1]}, s1.map_int_to_list_int)
        # At this point, `s2.map_int_to_list_int` and `s3.map_int_to_list_int`
        # are the "same" MutableMaps.
        self.assertEqual({2: [2]}, s2.map_int_to_list_int)
        self.assertEqual({2: [2]}, s2.map_int_to_list_int)
        s2.map_int_to_list_int[2].append(3)
        self.assertEqual({2: [2, 3]}, s2.map_int_to_list_int)
        self.assertEqual({2: [2, 3]}, s2.map_int_to_list_int)
        s3.map_int_to_list_int[2].append(4)
        self.assertEqual({2: [2, 3, 4]}, s2.map_int_to_list_int)
        self.assertEqual({2: [2, 3, 4]}, s2.map_int_to_list_int)

        # Even `{}` is not special, it should be assigned with `to_thrift_map()`.
        with self.assertRaisesRegex(TypeError, TypeErrorMsg):
            # pyre-ignore[8]: Intentional for test
            s3.map_int_to_list_int = {}

        # `s2.map_int_to_list_int` and `s3.map_int_to_list_int` are not "same"
        # anymore.
        s3.map_int_to_list_int = to_thrift_map({})
        self.assertEqual({2: [2, 3, 4]}, s2.map_int_to_list_int)
        self.assertEqual({}, s3.map_int_to_list_int)

    def test_match(self) -> None:
        # Canonical case: match fields
        match TestStructMutable(unqualified_string="Hello, world!"):
            case TestStructMutable(unqualified_string=x, optional_string=None):
                self.assertEqual(x, "Hello, world!")
            case _:
                self.fail("Expected match, got none.")

        # Any instance will match a Class pattern with no argument:
        match TestStructMutable(unqualified_string="Hello, ", optional_string="world!"):
            case TestStructMutable():
                pass  # Expected
            case _:
                self.fail("Expected match, got none.")

        # Capturing value of unset optional field (i.e., None):
        match TestStructMutable(unqualified_string="Hello, world!"):
            case TestStructMutable(optional_string=x):
                self.assertIsNone(x)
            case _:
                self.fail("Expected match, got none.")

        # Do not match if values differ
        match TestStructMutable(unqualified_string="Hello"):
            case TestStructMutable(unqualified_string="world!", optional_string=x):
                self.fail(f"Unexpected match: {x}")
            case _:
                pass  # Expected

        # Match default values
        match TestStructWithDefaultValuesMutable():
            case TestStructWithDefaultValuesMutable(
                unqualified_integer=x,
                unqualified_list_i32=y,
                optional_integer=None,
            ):
                self.assertEqual(x, 42)
                self.assertEqual(y, [1, 2, 3])
            case _:
                self.fail("Expected match, got none.")

        # Same, but using sequence capture pattern
        match TestStructWithDefaultValuesMutable():
            case TestStructWithDefaultValuesMutable(
                unqualified_integer=x,
                unqualified_list_i32=[a, b, c],
                optional_integer=None,
            ):
                self.assertEqual(x, 42)
                self.assertEqual(a, 1)
                self.assertEqual(b, 2)
                self.assertEqual(c, 3)
            case _:
                self.fail("Expected match, got none.")

        # Match nested struct
        match TestStructWithDefaultValuesMutable():
            case TestStructWithDefaultValuesMutable(unqualified_struct=x):
                self.assertEqual(x, TestStructMutable(unqualified_string="hello"))
            case _:
                self.fail("Expected match, got none.")

        # Match nested struct, capture nested field
        match TestStructWithDefaultValuesMutable():
            case TestStructWithDefaultValuesMutable(
                unqualified_struct=TestStructMutable(unqualified_string=x)
            ):
                self.assertEqual(x, "hello")
            case _:
                self.fail("Expected match, got none.")

        # Match adapted types
        match TestStructAdaptedTypesMutable(
            unqualified_adapted_i32_to_datetime=datetime.fromtimestamp(1733556290)
        ):
            case TestStructAdaptedTypesMutable(unqualified_adapted_i32_to_datetime=x):
                self.assertIsInstance(x, datetime)
                self.assertEqual(x, datetime.fromtimestamp(1733556290))
            case _:
                self.fail("Expected match, got none.")

        # Cases above are similar to immutable structs. Checking with mutations next...

        # Similar to canonical case above, but field is changed after initialization.
        w = TestStructWithDefaultValuesMutable(
            unqualified_integer=1, optional_integer=2
        )
        w.unqualified_integer = 3
        w.optional_integer = None
        match w:
            case TestStructWithDefaultValuesMutable(
                optional_integer=None, unqualified_integer=x
            ):
                self.assertEqual(x, 3)
            case _:
                self.fail("Expected match, got none.")

        w.optional_integer = 4
        match w:
            case TestStructWithDefaultValuesMutable(
                optional_integer=None, unqualified_integer=x
            ):
                self.fail(f"Unexpected match: {x}")
            case TestStructWithDefaultValuesMutable(
                optional_integer=y, unqualified_integer=x
            ):
                self.assertEqual(y, 4)
                self.assertEqual(x, 3)
            case _:
                self.fail("Expected match, got none.")

        # Identity of captured struct field is the same as the field itself:
        sub_w = TestStructMutable(optional_string="Lorem ipsum")
        w.optional_struct = sub_w
        match w:
            case TestStructWithDefaultValuesMutable(optional_struct=x):
                self.assertIs(x, sub_w)
            case _:
                self.fail("Expected match, got none.")

        # Match after resetting
        w.fbthrift_reset()
        match w:
            case TestStructWithDefaultValuesMutable(
                unqualified_integer=x, optional_struct=y
            ):
                self.assertEqual(x, 42)
                self.assertIsNone(y)
            case _:
                self.fail("Expected match, got none.")

    def test_default_enum_access(self) -> None:
        s_default = TestStructWithDefaultValuesMutable()
        field_value = s_default.unqualified_enum
        self.assertEqual(field_value, TestEnum.ARM1)
        # for most types, this would verify that the field was cached,
        # but enums are always singletons, so can only verify with benchmark
        self.assertIs(s_default.unqualified_enum, field_value)
        # unset optional field is always None even if default provided
        self.assertIsNone(s_default.optional_enum)

        s_default.unqualified_enum = TestEnum.ARM2
        s_default.optional_enum = TestEnum.ARM4
        self.assertEqual(s_default.unqualified_enum, TestEnum.ARM2)
        self.assertEqual(s_default.optional_enum, TestEnum.ARM4)

        s_default.optional_enum = None
        self.assertIsNone(s_default.optional_enum)

        with self.assertRaises(TypeError):
            # pyre-ignore[8]: Intentional for test
            s_default.unqualified_enum = None

    def test_fbthrift_copy_from(self) -> None:
        """
        `lhs.fbthrift_copy_from(rhs)` copies the content of the `rhs` struct
        into the lhs struct. It is semantically equivalent to assigning each
        field of `rhs` to `lhs`."
        """
        # Struct with primitive fields
        s1 = TestStructAllThriftPrimitiveTypesMutable(
            unqualified_bool=True,
            optional_byte=0,
            unqualified_i16=1,
            optional_i32=2,
            unqualified_i64=3,
            optional_float=4.0,
            unqualified_double=5.0,
            optional_string="abc",
        )

        s2 = TestStructAllThriftPrimitiveTypesMutable()
        self.assertNotEqual(s1, s2)
        s2.fbthrift_copy_from(s1)
        self.assertEqual(s1, s2)

        # Struct with container fields
        s3 = TestStructAllThriftContainerTypesMutable(
            unqualified_list_i32=to_thrift_list([1, 2, 3]),
            optional_set_string=to_thrift_set({"a", "b", "c"}),
            unqualified_map_string_i32=to_thrift_map({"a": 1, "b": 2}),
        )

        s4 = TestStructAllThriftContainerTypesMutable()
        self.assertNotEqual(s3, s4)
        s4.fbthrift_copy_from(s3)
        self.assertEqual(s3, s4)

        # Container assignment is refernce semantics, after `fbthrift_copy_from()`
        # s3 and s4 container fields are the "same" containers.
        self.assertEqual([1, 2, 3], s3.unqualified_list_i32)
        self.assertEqual([1, 2, 3], s4.unqualified_list_i32)

        s3.unqualified_list_i32.append(4)

        self.assertEqual([1, 2, 3, 4], s3.unqualified_list_i32)
        self.assertEqual([1, 2, 3, 4], s4.unqualified_list_i32)

        # Struct with struct fields
        n2 = TestStructNested_2_Mutable(i32_field=2)
        n1 = TestStructNested_1_Mutable(i32_field=3, nested_2=n2)
        s5 = TestStructNested_0_Mutable(i32_field=5, nested_1=n1)

        s6 = TestStructNested_0_Mutable()
        self.assertNotEqual(s5, s6)
        s6.fbthrift_copy_from(s5)
        self.assertEqual(s5, s6)

        # Struct assignment is refernce semantics, after `fbthrift_copy_from()`
        # s5 and s4 struct fields are the "same" structs.
        self.assertEqual(3, s5.nested_1.i32_field)
        self.assertEqual(3, s6.nested_1.i32_field)

        s5.nested_1.i32_field = 33

        self.assertEqual(33, s5.nested_1.i32_field)
        self.assertEqual(33, s6.nested_1.i32_field)

        # `lhs` and `rhs` must be the same type
        s7 = TestStructAllThriftPrimitiveTypesMutable()
        with self.assertRaisesRegex(
            TypeError,
            "Cannot copy from.*TestStructAllThriftContainerTypes.*to"
            ".*TestStructAllThriftPrimitiveTypes",
        ):
            s7.fbthrift_copy_from(TestStructAllThriftContainerTypesMutable())
