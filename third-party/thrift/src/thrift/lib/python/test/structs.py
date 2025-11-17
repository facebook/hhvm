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


from __future__ import annotations

import copy
import json
import math
import sys
import types
import unittest
from typing import Callable, cast as typing_cast, Type, TypeVar
from unittest import mock

import testing.thrift_mutable_types as mutable_test_types
import testing.thrift_types as immutable_test_types

import thrift.python.mutable_serializer as mutable_serializer
import thrift.python.serializer as immutable_serializer

import thrift.python.types

from folly.iobuf import IOBuf

from parameterized import parameterized_class

from testing.thrift_mutable_types import (
    _Reserved as _ReservedMutable,
    DefaultedFields as MutableDefaultedFields,
    LatLon as MutableLatLon,
)

from testing.thrift_types import (
    _Reserved,
    Color,
    ComplexRef,
    customized,
    DefaultedFields,
    easy,
    EmptyStruct,
    File,
    Integers,
    IOBufListStruct,
    Kind,
    ListTypes,
    mixed,
    Nested1,
    Nested2,
    Nested3,
    NestedStructContainers,
    numerical,
    OptionalFile,
    Optionals,
    Perm,
    Reserved,
    Runtime,
    SimpleStruct,
    StringBucket,
    StructDisabledFieldCache,
    StructuredAnnotation,
    StructWithMap,
    UnusedError,
)
from thrift.python.exceptions import GeneratedError
from thrift.python.mutable_types import (
    _isset as mutable_isset,
    _ThriftListWrapper,
    _ThriftMapWrapper,
    _ThriftSetWrapper,
    to_thrift_list,
    to_thrift_map,
    to_thrift_set,
)
from thrift.python.protocol import Protocol
from thrift.python.types import isset, Struct, StructOrUnion, update_nested_field

ListT = TypeVar("ListT")
SetT = TypeVar("SetT")
MapKey = TypeVar("MapKey")
MapValue = TypeVar("MapValue")


@parameterized_class(
    ("test_types", "serializer_module"),
    [
        (immutable_test_types, immutable_serializer),
        (mutable_test_types, mutable_serializer),
    ],
)
class StructTestsParameterized(unittest.TestCase):
    def setUp(self) -> None:
        """
        The `setUp` method performs these assignments with type hints to enable
        pyre when using 'parameterized'. Otherwise, Pyre cannot deduce the types
        behind `test_types`.
        """
        # pyre-ignore[16]: has no attribute `test_types`
        self.StringBucket: Type[StringBucket] = self.test_types.StringBucket
        self.OptionalFile: Type[OptionalFile] = self.test_types.OptionalFile
        self.File: Type[File] = self.test_types.File
        self.Kind: Type[Kind] = self.test_types.Kind
        self.UnusedError: Type[UnusedError] = self.test_types.UnusedError
        self.Integers: Type[Integers] = self.test_types.Integers
        self.easy: Type[easy] = self.test_types.easy
        self.Optionals: Type[Optionals] = self.test_types.Optionals
        self.Runtime: Type[Runtime] = self.test_types.Runtime
        self.Color: Type[Color] = self.test_types.Color
        self.Reserved: Type[Reserved] = self.test_types.Reserved
        self.StructuredAnnotation: Type[StructuredAnnotation] = (
            self.test_types.StructuredAnnotation
        )
        self.mixed: Type[mixed] = self.test_types.mixed
        self.is_mutable_run: bool = self.test_types.__name__.endswith(
            "thrift_mutable_types"
        )
        # pyre-ignore[8]: Intentional for test
        self._Reserved: Type[_Reserved] = (
            _Reserved if not self.is_mutable_run else _ReservedMutable
        )
        # pyre-ignore[16]: has no attribute `serializer_module`
        self.serializer: types.ModuleType = self.serializer_module
        # pyre-ignore[8]: has no attribute `serializer_module`
        self.isset: Callable[[StructOrUnion | GeneratedError], dict[str, bool]] = (
            mutable_isset if self.is_mutable_run else isset
        )

    def to_list(self, list_data: list[ListT]) -> list[ListT] | _ThriftListWrapper:
        return to_thrift_list(list_data) if self.is_mutable_run else list_data

    def test_isset_Struct(self) -> None:
        to_serialize = self.OptionalFile(name="/dev/null", type=8)
        serialized = self.serializer.serialize_iobuf(to_serialize)
        file = self.serializer.deserialize(self.File, serialized)
        self.assertTrue(self.isset(file)["type"])
        self.assertFalse(self.isset(file)["permissions"])

        to_serialize = self.OptionalFile(name="/dev/null")
        serialized = self.serializer.serialize_iobuf(to_serialize)
        file = self.serializer.deserialize(self.File, serialized)
        self.assertEqual(file.type, self.Kind.REGULAR)
        self.assertFalse(self.isset(file)["type"])

    def test_isset_Error(self) -> None:
        e = self.UnusedError(message="ACK")
        self.assertTrue(self.isset(e)["message"])

    def test_isset_Union(self) -> None:
        i = self.Integers(large=2)
        with self.assertRaises(TypeError):
            self.isset(i)["large"]

    def test_no_dict(self) -> None:
        with self.assertRaises(AttributeError):
            self.easy().__dict__
        # TODO: make it a tuple instead of a list
        # self.assertIsInstance(easy().__class__.__slots__, tuple)
        self.assertIs(self.easy().__class__.__slots__, self.easy().__slots__)
        self.assertIs(self.easy(val=5).__slots__, self.easy(name="yo").__slots__)

    def test_optional_struct_creation(self) -> None:
        with self.assertRaises(TypeError):
            # pyre-ignore[19]: for test
            self.easy(1, [1, 1], "test", self.Integers(tiny=1))
        self.easy(val=1, an_int=self.Integers(small=500))
        with self.assertRaises(TypeError):
            # pyre-ignore[6]: for test
            self.easy(name=b"binary")
        # Only Required Fields don't accept None
        self.easy(val=5, an_int=None)

    def test_call_replace_container(self) -> None:
        # pyre-ignore[6]: TODO: Thrift-Container init
        x = self.Optionals(values=self.to_list(["a", "b", "c"]))
        # pyre-ignore[6]: TODO: Thrift-Container init
        z = x(values=self.to_list(["b", "c"]))
        y = z(values=None)
        self.assertIsNone(y.values)

    def test_str_subclass_param(self) -> None:
        class MyStr(str):
            pass

        e = self.easy(name=MyStr("foo"))
        self.assertIsInstance(e.name, str)
        self.assertIs(type(e.name), str)
        self.assertEqual(e.name, "foo")

    def test_runtime_checks(self) -> None:
        x = self.Runtime()
        with self.assertRaises(TypeError):
            # pyre-ignore[6]: for test
            x(bool_val=5)

        with self.assertRaises(TypeError):
            # pyre-ignore[6]: for test
            self.Runtime(bool_val=5)

        with self.assertRaises(TypeError):
            # pyre-ignore[6]: for test
            x(enum_val=2)

        with self.assertRaises(TypeError):
            # pyre-ignore[6]: for test
            self.Runtime(enum_val=2)

        with self.assertRaises(TypeError):
            # pyre-ignore[6]: for test
            x(int_list_val=["foo", "bar", "baz"])

        with self.assertRaises(TypeError):
            # pyre-ignore[6]: for test
            self.Runtime(int_list_val=["foo", "bar", "baz"])

        with self.assertRaises(TypeError):
            self.Runtime(
                bool_val=True,
                enum_val=self.Color.red,
                # pyre-ignore[6]: for test
                int_list_val=[1, 2, "foo"],
            )

    def test_ordering(self) -> None:
        x = self.Runtime(
            bool_val=False,
            enum_val=self.Color.red,
            # pyre-ignore[6]: TODO: Thrift-Container init
            int_list_val=self.to_list([64, 128]),
        )
        y = x(bool_val=True)
        self.assertLess(x, y)
        self.assertLessEqual(x, y)
        self.assertGreater(y, x)
        self.assertGreaterEqual(y, x)
        self.assertEqual([x, y], sorted([y, x]))

    def test_init_with_invalid_field(self) -> None:
        with self.assertRaises(TypeError):
            # pyre-ignore[28]: for test
            self.easy(
                val=1,
                an_int=self.Integers(small=300),
                name="foo",
                val_lists=[1, 2, 3, 4],
            )

    def test_init_with_invalid_field_value(self) -> None:
        with self.assertRaisesRegex(
            TypeError, "Cannot create internal string data representation"
        ):
            # pyre-ignore[6]: name is string, but under test
            self.easy(val=1, an_int=self.Integers(small=300), name=1)

    def test_iterate(self) -> None:
        # pyre-ignore[28]: proving __mangled_str works
        x = self.Reserved(
            from_="hello",
            nonlocal_=3,
            ok="bye",
            is_cpdef=True,
            move="Qh4xe1",
            inst="foo",
            changes="bar",
            __mangled_str="secret",
            _Reserved__mangled_int=42,
        )
        self.assertEqual(
            list(x),
            [
                ("from_", "hello"),
                ("nonlocal_", 3),
                ("ok", "bye"),
                ("is_cpdef", True),
                ("move", "Qh4xe1"),
                ("inst", "foo"),
                ("changes", "bar"),
                ("_Reserved__mangled_str", "secret"),
                ("_Reserved__mangled_int", 42),
            ],
        )
        self.assertEqual(
            [k for k, _ in self.Reserved],
            [
                "from_",
                "nonlocal_",
                "ok",
                "is_cpdef",
                "move",
                "inst",
                "changes",
                "_Reserved__mangled_str",
                "_Reserved__mangled_int",
            ],
        )

    def test_recursive_init(self) -> None:
        self.StructuredAnnotation()

    def test_call_replace(self) -> None:
        x = self.easy(val=1, an_int=self.Integers(small=300), name="foo")
        y1 = x(name="bar")
        self.assertEqual(x.name, "foo")
        self.assertEqual(y1.name, "bar")

        if sys.version_info >= (3, 13):
            y2 = copy.replace(x, name="bar")
            self.assertEqual(x.name, "foo")
            self.assertEqual(y2.name, "bar")

        z1 = x(an_int=None, val=4)
        self.assertEqual(x.an_int, self.Integers(small=300))
        self.assertEqual(z1.an_int, self.Integers())
        self.assertEqual(x.val, 1)
        self.assertEqual(z1.val, 4)

        if sys.version_info >= (3, 13):
            z2 = copy.replace(x, an_int=None, val=4)
            self.assertEqual(x.an_int, self.Integers(small=300))
            self.assertEqual(z2.an_int, self.Integers())
            self.assertEqual(x.val, 1)
            self.assertEqual(z2.val, 4)

    def test_call_bad_key(self) -> None:
        x = self.easy()

        err_type = AttributeError if self.is_mutable_run else TypeError
        with self.assertRaisesRegex(
            err_type, "'easy' .*(attribute|initialization).* 'bad_key'"
        ):
            # pyre-ignore[28]: bad key for test
            x = x(bad_key="foo")

        if sys.version_info >= (3, 13):
            with self.assertRaisesRegex(
                err_type, "'easy' .*(attribute|initialization).* 'bad_key'"
            ):
                x = copy.replace(x, bad_key="foo")

    def test_reserved(self) -> None:
        # pyre-ignore[28]: proving the mangling is optional
        x = self.Reserved(
            from_="hello",
            nonlocal_=3,
            ok="bye",
            is_cpdef=True,
            move="Qh4xe1",
            inst="foo",
            changes="bar",
            _Reserved__mangled_str="secret",
            __mangled_int=42,
        )
        self.assertEqual(x.from_, "hello")
        self.assertEqual(x.nonlocal_, 3)
        self.assertEqual(x.ok, "bye")
        self.assertEqual(x.is_cpdef, True)
        self.assertEqual(x.move, "Qh4xe1")
        self.assertEqual(x.inst, "foo")
        self.assertEqual(x.changes, "bar")
        self.assertEqual(x._Reserved__mangled_str, "secret")
        self.assertEqual(x._Reserved__mangled_int, 42)

        self.assertEqual(x, x)

        y = self._Reserved(
            _Reserved__mangled_str="secret",
            _Reserved__mangled_int=42,
        )
        self.assertEqual(y._Reserved__mangled_str, "secret")
        self.assertEqual(y._Reserved__mangled_int, 42)

    def test_dir(self) -> None:
        expected = ["__iter__", "an_int", "name", "py3_hidden", "val", "val_list"]
        self.assertEqual(expected, dir(self.easy()))
        self.assertEqual(expected, dir(self.easy))

    def test_repr(self) -> None:
        if self.is_mutable_run:
            self.assertEqual(
                "easy(val=42, val_list=to_thrift_list([]), name=None, an_int=Integers(EMPTY=None), py3_hidden=0)",
                repr(self.easy(val=42)),
            )
        else:
            self.assertEqual(
                "easy(val=42, val_list=i[], name=None, an_int=Integers(EMPTY=None), py3_hidden=0)",
                repr(self.easy(val=42)),
            )

    def test_autospec_iterable(self) -> None:
        for _ in mock.create_autospec(self.easy):
            pass
        for _ in mock.create_autospec(self.easy()):
            pass

    def test_copy(self) -> None:
        x = self.easy(
            val=1,
            an_int=self.Integers(small=300),
            name="foo",
            # pyre-ignore[6]: TODO: Thrift-Container init
            val_list=self.to_list([1, 2, 3, 4]),
        )
        dif_list = copy.copy(x.val_list)
        self.assertEqual(x.val_list, dif_list)
        dif_int = copy.copy(x.an_int)
        self.assertEqual(x.an_int, dif_int)

    def test_defaulted_optional_field(self) -> None:
        def assert_mixed(m: mixed) -> None:
            self.assertIsNone(m.opt_field)
            self.assertIsNone(m.opt_float)
            self.assertIsNone(m.opt_int)
            self.assertIsNone(m.opt_enum)

        def assert_isset(m: mixed) -> None:
            isset = self.isset(m)
            for fld_name, _ in mixed:
                if not fld_name.startswith("opt_"):
                    continue
                self.assertFalse(isset[fld_name], fld_name)

        # constructor
        m = self.mixed()
        assert_mixed(m)
        assert_isset(m)

        # call operator
        m = m(some_field_="don't care")
        assert_mixed(m)
        assert_isset(m)

        # serialization round-trip
        m = self.serializer.deserialize(self.mixed, self.serializer.serialize(m))
        assert_mixed(m)
        assert_isset(m)

        ### Now with explicit `None` set
        m = self.mixed(opt_field=None)
        self.assertIsNone(m.opt_field)
        self.assertFalse(self.isset(m)["opt_field"])

        m = m(opt_field=None)
        self.assertIsNone(m.opt_field)
        self.assertFalse(self.isset(m)["opt_field"])

        m = self.serializer.deserialize(self.mixed, self.serializer.serialize(m))
        self.assertIsNone(m.opt_field)
        self.assertFalse(self.isset(m)["opt_field"])

    def test_getattr(self) -> None:
        e = self.easy(val=1, an_int=self.Integers(small=300), name="foo")
        for name, value in e:
            self.assertEqual(getattr(e, name), value)

    def test_getattr_disable_cached(self) -> None:
        # check the argument is set
        self.assertTrue(
            hasattr(
                StructDisabledFieldCache, "_fbthrift_disable_field_cache_DO_NOT_USE"
            )
        )

        s = StructDisabledFieldCache(
            int_field=2,
            set_field=({"1", "2", "3"}),
            list_field=[1, 2, 3],
            map_field={"key": 3},
            empty_struct_field=EmptyStruct(),
            easy_field=easy(name="easy"),
        )
        # to access the non-primitive fields and ensureing they have the right value
        self.assertEqual(len(s.set_field), 3)
        self.assertEqual(s.list_field[0], 1)
        self.assertEqual(s.map_field["key"], 3)
        self.assertEqual(s.easy_field.name, "easy")

        # get the non-primitive fields
        first_int = s.int_field
        first_empty = s.empty_struct_field
        first_easy = s.easy_field
        first_set = s.set_field
        first_list = s.list_field
        first_map = s.map_field

        # pyre-ignore[16]: private method only for thrift tests
        if thrift.python.types._fbthrift__runtime_is_cinder():
            # cinder will cache the field
            return

        self.assertIs(first_int, s.int_field)
        # Get the different non-primitive objects when calling get() because now the struct field is not cached.
        self.assertIsNot(first_empty, s.empty_struct_field)
        self.assertIsNot(first_easy, s.easy_field)
        self.assertIsNot(first_set, s.set_field)
        self.assertIsNot(first_list, s.list_field)
        self.assertIsNot(first_map, s.map_field)

    def test_compare_optional(self) -> None:
        x = self.StringBucket()
        y = self.StringBucket()

        # Both are default so they are equal and neither are greater
        self.assertFalse(x < y)
        self.assertFalse(x > y)
        self.assertTrue(x <= y)
        self.assertTrue(x >= y)

        x = self.StringBucket(one="one")

        # x has a field set so it's greater
        self.assertFalse(x < y)
        self.assertTrue(x > y)
        self.assertFalse(x <= y)
        self.assertTrue(x >= y)

        # x has an optional field set so even though it's empty string, "" > None
        x = self.StringBucket(two="")
        self.assertFalse(x < y)
        self.assertTrue(x > y)
        self.assertFalse(x <= y)
        self.assertTrue(x >= y)

        # comparisons happen in field order so because y.one > x.one, y > x
        y = self.StringBucket(one="one")
        self.assertTrue(x < y)
        self.assertFalse(x > y)
        self.assertTrue(x <= y)
        self.assertFalse(x >= y)

        z = self.easy()
        with self.assertRaises(TypeError):
            # TODO(ffrancet): pyre should complain about this
            z < y  # noqa: B015

    def test_instance_base_class(self) -> None:
        self.assertIsInstance(Nested1(), Struct)
        self.assertIsInstance(Nested1(), Nested1)
        self.assertNotIsInstance(Nested1(), Nested2)
        self.assertNotIsInstance(3, Nested1)
        self.assertNotIsInstance(3, Struct)
        self.assertTrue(issubclass(Nested1, Struct))
        self.assertFalse(issubclass(int, Struct))
        self.assertFalse(issubclass(int, Nested1))
        self.assertFalse(issubclass(Struct, Nested1))
        self.assertFalse(issubclass(Nested1, Nested2))


class StructTestsImmutable(unittest.TestCase):
    """
    Unittest only valid for immutable types
    """

    def test_hashability(self) -> None:
        hash(easy())
        hash(EmptyStruct())

    def test_to_python(self) -> None:
        e = easy()
        self.assertEqual(e, e._to_python())

    def test_immutability(self) -> None:
        e = easy()
        with self.assertRaises(AttributeError):
            # pyre-ignore[41]: Cannot reassign final attribute `name`.
            e.name = "foo"

    def test_setattr(self) -> None:
        e = easy(val=1, an_int=Integers(small=300), name="foo")

        vals = {
            "an_int": Integers(small=3),
            "val": 2,
            "name": "bar",
        }

        # run-around for linter error on setattr usage
        for field, val in vals.items():
            with self.assertRaisesRegex(AttributeError, field):
                setattr(e, field, val)

            # for both cinder and cpython, normal setattr is blocked
            with self.assertRaisesRegex(AttributeError, field):
                setattr(e, field, val)

            # pyre-ignore[16]: private method only for thrift tests
            if thrift.python.types._fbthrift__runtime_is_cinder():
                # cinder cached property doesn't let us override __set__
                continue

            # we can block object.__setattr__ backdoor with
            # our own property descriptor impl
            with self.assertRaisesRegex(AttributeError, field):
                object.__setattr__(e, field, val)


class StructTests(unittest.TestCase):
    def test_update_nested_fields(self) -> None:
        n = Nested1(a=Nested2(b=Nested3(c=easy(val=42, name="foo"))))
        n = update_nested_field(n, {"a.b.c": easy(val=128)})
        self.assertEqual(n.a.b.c.val, 128)

    def test_update_multiple_nested_fields(self) -> None:
        n = Nested1(a=Nested2(b=Nested3(c=easy(val=42, name="foo"))))
        n = update_nested_field(
            n,
            {
                "a.b.c.name": "bar",
                "a.b.c.val": 256,
            },
        )
        self.assertEqual(n.a.b.c.name, "bar")
        self.assertEqual(n.a.b.c.val, 256)

    def test_update_invalid_nested_fields(self) -> None:
        n = Nested1(a=Nested2(b=Nested3(c=easy(val=42, name="foo"))))
        with self.assertRaises(ValueError):
            update_nested_field(n, {"": 0})
        with self.assertRaises(ValueError):
            update_nested_field(n, {"e": 0})
        with self.assertRaises(ValueError):
            update_nested_field(n, {"a.b.e": 0})
        with self.assertRaises(ValueError):
            update_nested_field(n, {"a.e.f": 0})

    def test_update_conflicting_nested_fields(self) -> None:
        n = Nested1(a=Nested2(b=Nested3(c=easy(val=42, name="foo"))))
        with self.assertRaises(ValueError):
            n = update_nested_field(
                n,
                {
                    "a.b.c": easy(val=128),
                    "a.b.c.val": 256,
                },
            )

    def test_isset_struct_for_equality_and_hash(self) -> None:
        """
        Test that isset flags don't affect struct equality and hash.
        """
        # Create s1 with some fields explicitly set to default value (isset=true)
        s1 = SimpleStruct(value=0, name="", city="NY")

        # Create s2 without setting the value and name (isset=false, uses default)
        s2 = SimpleStruct(city="NY")

        self.assertEqual(s1, s2)
        self.assertEqual(hash(s1), hash(s2))
        self.assertEqual(len({s1, s2}), 1)

        # But isset flags should differ
        self.assertNotEqual(isset(s1), isset(s2))

        # s1 has fields explicitly set
        self.assertTrue(isset(s1)["value"])
        self.assertTrue(isset(s1)["name"])
        self.assertTrue(isset(s1)["city"])
        self.assertEqual(isset(s1), {"name": True, "value": True, "city": True})

        # s2 has fields unset (using defaults)
        self.assertFalse(isset(s2)["value"])
        self.assertFalse(isset(s2)["name"])
        self.assertTrue(isset(s2)["city"])
        self.assertEqual(isset(s2), {"name": False, "value": False, "city": True})

    def test_struct_with_map_field_insertion_order(self) -> None:
        """
        Test that demonstrates map field insertion order affects struct hash
        but not equality. Maps with same content but different insertion order
        should be equal but currently have different hash values.

        This test documents the current behavior where insertion order affects
        the hash value.
        """
        s1 = StructWithMap(data={"a": 1, "b": 2})
        s2 = StructWithMap(data={"b": 2, "a": 1})

        self.assertEqual(s1, s2)

        # However, hash values are currently different (insertion order dependent)
        self.assertNotEqual(hash(s1), hash(s2))

        struct_set = {s1, s2}
        self.assertEqual(len(struct_set), 2)


@parameterized_class(
    ("test_types", "serializer_module"),
    [
        (immutable_test_types, immutable_serializer),
        (mutable_test_types, mutable_serializer),
    ],
)
class NumericalConversionsTests(unittest.TestCase):
    def setUp(self) -> None:
        """
        The `setUp` method performs these assignments with type hints to enable
        pyre when using 'parameterized'. Otherwise, Pyre cannot deduce the types
        behind `test_types`.
        """
        # pyre-ignore[16]: has no attribute `test_types`
        self.numerical: Type[numerical] = self.test_types.numerical
        self.Kind: Type[Kind] = self.test_types.Kind
        self.is_mutable_run: bool = self.test_types.__name__.endswith(
            "thrift_mutable_types"
        )
        # pyre-ignore[16]: has no attribute `serializer_module`
        self.serializer: types.ModuleType = self.serializer_module

    def to_list(self, list_data: list[ListT]) -> list[ListT] | _ThriftListWrapper:
        return to_thrift_list(list_data) if self.is_mutable_run else list_data

    def test_overflow(self) -> None:
        with self.assertRaises(OverflowError):
            self.numerical(float_val=5, int_val=2**63 - 1)

        with self.assertRaises(OverflowError):
            # pyre-ignore[6]: TODO: Thrift-Container init
            self.numerical(float_val=5, int_val=2, int_list=self.to_list([5, 2**32]))

    def test_int_to_float(self) -> None:
        # pyre-ignore[6]: TODO: Thrift-Container init
        x = self.numerical(int_val=5, float_val=5, float_list=self.to_list([1, 5, 6]))
        x(float_val=10)
        # pyre-ignore[6]: TODO: Thrift-Container init
        x(float_list=self.to_list([6, 7, 8]))

    def test_int_to_i64(self) -> None:
        large = 2**63 - 1
        self.numerical(int_val=5, float_val=5, i64_val=int(large))
        too_large = 2**65 - 1
        with self.assertRaises(OverflowError):
            self.numerical(int_val=5, float_val=5, i64_val=int(too_large))

    def test_float_to_int_required_field(self) -> None:
        with self.assertRaises(TypeError):
            # pyre-ignore[6]: for test
            self.numerical(int_val=math.pi, float_val=math.pi)

    def test_float_to_int_unqualified_field(self) -> None:
        with self.assertRaises(TypeError):
            self.numerical(
                float_val=math.pi,
                # pyre-ignore[6]: for test
                int_val=math.pi,
            )

    def test_float_to_int_list(self) -> None:
        with self.assertRaises(TypeError):
            self.numerical(
                int_val=5,
                float_val=math.pi,
                # pyre-ignore[6]: for test
                int_list=[math.pi, math.e],
            )

    def roundtrip(self, x: numerical) -> numerical:
        return self.serializer.deserialize(self.numerical, self.serializer.serialize(x))

    def test_permissive_init_int_with_enum(self) -> None:
        n = self.numerical(int_val=self.Kind.LINK)

        def assert_strict(n: numerical) -> None:
            self.assertIs(type(n.int_val), int)
            self.assertEqual(n.int_val, self.Kind.LINK.value)

        assert_strict(n)
        rt = self.roundtrip(n)
        assert_strict(rt)

    def test_permissive_init_float_with_enum(self) -> None:
        n = self.numerical(float_val=self.Kind.LINK)

        def assert_strict(n: numerical) -> None:
            self.assertIs(type(n.float_val), float)
            self.assertEqual(n.float_val, self.Kind.LINK.value)

        assert_strict(n)
        rt = self.roundtrip(n)
        assert_strict(rt)

    def test_permissive_init_float_with_bool(self) -> None:
        n = self.numerical(float_val=True)

        def assert_strict(n: numerical) -> None:
            self.assertIs(type(n.float_val), float)
            self.assertEqual(n.float_val, float(True))

        assert_strict(n)
        rt = self.roundtrip(n)
        assert_strict(rt)

    def test_permissive_init_float_with_int(self) -> None:
        n = self.numerical(float_val=888)

        def assert_strict(n: numerical) -> None:
            self.assertIs(type(n.float_val), float)
            self.assertEqual(n.float_val, 888.0)

        assert_strict(n)
        rt = self.roundtrip(n)
        assert_strict(rt)


@parameterized_class(
    ("test_types", "serializer_module"),
    [
        (immutable_test_types, immutable_serializer),
        (mutable_test_types, mutable_serializer),
    ],
)
class StructDeepcopyTests(unittest.TestCase):
    def setUp(self) -> None:
        """
        The `setUp` method performs these assignments with type hints to enable
        pyre when using 'parameterized'. Otherwise, Pyre cannot deduce the types
        behind `test_types`.
        """
        # pyre-ignore[16]: has no attribute `test_types`
        self.easy: Type[easy] = self.test_types.easy
        self.Integers: Type[Integers] = self.test_types.Integers
        self.customized: Type[customized] = self.test_types.customized
        self.ComplexRef: Type[ComplexRef] = self.test_types.ComplexRef
        self.ListTypes: Type[ListTypes] = self.test_types.ListTypes
        self.File: Type[File] = self.test_types.File
        self.Perm: Type[Perm] = self.test_types.Perm
        self.Kind: Type[Kind] = self.test_types.Kind
        self.IOBufListStruct: Type[IOBufListStruct] = self.test_types.IOBufListStruct
        self.StringBucket: Type[StringBucket] = self.test_types.StringBucket
        self.is_mutable_run: bool = self.test_types.__name__.endswith(
            "thrift_mutable_types"
        )
        self.DefaultedFields: Type[DefaultedFields] = self.test_types.DefaultedFields
        self.Nested3: Type[Nested3] = self.test_types.Nested3
        self.NestedStructContainers: Type[NestedStructContainers] = (
            self.test_types.NestedStructContainers
        )
        # pyre-ignore[16]: has no attribute `serializer_module`
        self.serializer: types.ModuleType = self.serializer_module

    def to_list(self, list_data: list[ListT]) -> list[ListT] | _ThriftListWrapper:
        return to_thrift_list(list_data) if self.is_mutable_run else list_data

    def to_set(self, set_data: set[SetT]) -> set[SetT] | _ThriftSetWrapper:
        return to_thrift_set(set_data) if self.is_mutable_run else set_data

    def to_map(
        self, map_data: dict[MapKey, MapValue]
    ) -> dict[MapKey, MapValue] | _ThriftMapWrapper:
        return to_thrift_map(map_data) if self.is_mutable_run else map_data

    def test_deepcopy(self) -> None:
        x = self.easy(
            val=1,
            an_int=self.Integers(small=300),
            name="bar",
            # pyre-ignore[6]: TODO: Thrift-Container init
            val_list=self.to_list([1, 2, 3, 4]),
        )
        dif = copy.deepcopy(x)
        if self.is_mutable_run:
            self.assertIsNot(x, dif)
            self.assertEqual(x, dif)
        else:
            self.assertIs(x, dif)

    def test_nested_in_python_types(self) -> None:
        x = self.easy(
            val=1,
            an_int=self.Integers(small=300),
            name="bar",
            # pyre-ignore[6]: TODO: Thrift-Container init
            val_list=self.to_list([1, 2, 3, 4]),
        )
        nested_in_py = {"a": {"b": {"c": x}}}
        dif = copy.deepcopy(nested_in_py)
        self.assertEqual(nested_in_py, dif)

    def test_list_set_map_types_copy(self) -> None:
        custom = self.customized(
            # pyre-ignore[6]: TODO: Thrift-Container init
            list_template=self.to_list([1, 2, 3, 4]),
            # pyre-ignore[6]: TODO: Thrift-Container init
            set_template=self.to_set({1, 2, 3}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            map_template=self.to_map({0: 1, 2: 3}),
        )
        dif = copy.deepcopy(custom)
        if self.is_mutable_run:
            self.assertIsNot(custom, dif)
            self.assertEqual(custom, dif)
        else:
            self.assertIs(custom, dif)

        # test copying just the map/list field in the thrift object
        dif = copy.deepcopy(custom.list_template)
        if self.is_mutable_run:
            self.assertIsNot(custom.list_template, dif)
            self.assertEqual(custom.list_template, dif)
        else:
            self.assertIs(custom.list_template, dif)

        dif = copy.deepcopy(custom.set_template)
        if self.is_mutable_run:
            self.assertIsNot(custom.set_template, dif)
            self.assertEqual(custom.set_template, dif)
        else:
            self.assertIs(custom.set_template, dif)

        dif = copy.deepcopy(custom.map_template)
        if self.is_mutable_run:
            self.assertIsNot(custom.map_template, dif)
            self.assertEqual(custom.map_template, dif)
        else:
            self.assertIs(custom.map_template, dif)

    def test_list_ref_copy(self) -> None:
        obj = self.ComplexRef(
            name="outer",
            # pyre-ignore[6]: TODO: Thrift-Container init
            list_recursive_ref=self.to_list([self.ComplexRef(name="inner")]),
        )
        dif = copy.deepcopy(obj)
        if self.is_mutable_run:
            self.assertIsNot(obj, dif)
            self.assertEqual(obj, dif)
        else:
            self.assertIs(obj, dif)

    def test_list_string_copy(self) -> None:
        obj = self.ListTypes(
            # pyre-ignore[6]: TODO: Thrift-Container init
            first=self.to_list(["one", "two", "three"]),
            # pyre-ignore[6]: TODO: Thrift-Container init
            second=self.to_list([1, 2, 3]),
            # pyre-ignore[6]: TODO: Thrift-Container init
            third=self.to_list([[1, 2], [3, 4]]),
            # pyre-ignore[6]: TODO: Thrift-Container init
            fourth=self.to_list([{1, 2}, {3, 4}]),
            # pyre-ignore[6]: TODO: Thrift-Container init
            fifth=self.to_list([{1: 2}, {3: 4}]),
        )
        dif = copy.deepcopy(obj)
        if self.is_mutable_run:
            self.assertIsNot(obj, dif)
            self.assertEqual(obj, dif)
        else:
            self.assertIs(obj, dif)

    def test_enum_values_copy(self) -> None:
        file = self.File(
            name="test.txt", permissions=self.Perm.read, type=self.Kind.REGULAR
        )
        dif = copy.deepcopy(file)
        if self.is_mutable_run:
            self.assertIsNot(file, dif)
            self.assertEqual(file, dif)
        else:
            self.assertIs(file, dif)

    def test_binary_values_copy(self) -> None:
        obj = self.IOBufListStruct(
            # pyre-ignore[6]: TODO: Thrift-Container init
            iobufs=self.to_list([IOBuf(b"one"), IOBuf(b"two")])
        )
        # IOBuf does not support deepcopy
        if not self.is_mutable_run:
            dif = copy.deepcopy(obj)
            self.assertIs(obj, dif)

    def test_field_deepcopy(self) -> None:
        d1 = self.DefaultedFields()
        d2 = self.DefaultedFields()
        self.assertEqual(d1, d2)
        for fld_name, d1_value in d1:
            d2_value = getattr(d2, fld_name)
            self.assertEqual(d1_value, d2_value)
            # for mutable, they should be distinct objects, ofc
            # for immutable, the internal data types are references to the same object,
            # but field access creates a new distinct python object (thrift List or Map)
            self.assertIsNot(d1_value, d2_value)

        if not self.is_mutable_run:
            return

        # mutate the first one and verify that values of second don't change
        d1 = typing_cast(MutableDefaultedFields, d1)
        d1.int_list.append(49)
        d1.unicode_set.add("£")
        d1.location_map[38] = to_thrift_list(
            [MutableLatLon(lat=39.7392567, lon=-104.9848600)]
        )
        d1.location_map[47].append(MutableLatLon())
        d1.location_map[47][0].lat += 1e-6

        # d2 should be unchanged
        self.assertEqual(d1.int_list, list(range(11)) + [49])
        self.assertEqual(d2.int_list, list(range(11)))

        self.assertEqual(d1.unicode_set - d2.unicode_set, {"£"})
        self.assertEqual(d2.unicode_set - d1.unicode_set, set())

        self.assertEqual(d1.location_map[31], d2.location_map[31])
        self.assertEqual(len(d1.location_map[38]), 1)
        self.assertNotIn(38, d2.location_map)
        self.assertEqual(len(d1.location_map[47]), 3)
        self.assertEqual(len(d2.location_map[47]), 2)
        self.assertNotEqual(d1.location_map[47][0].lat, d2.location_map[47][0].lat)
        self.assertEqual(d1.location_map[47][0].lon, d2.location_map[47][0].lon)

    def test_nested_struct_deepcopy(self) -> None:
        n = self.Nested3(c=self.easy(name="foo", val=42))

        n_copy = copy.deepcopy(n)
        easy_copy = n_copy.c
        self.assertEqual(n.c, easy_copy)

        if self.is_mutable_run:
            self.assertIsNot(n.c, easy_copy)
        else:
            self.assertIs(n, n_copy)
            return

        assert isinstance(easy_copy, mutable_test_types.easy)
        easy_copy.name = "bar"
        easy_copy.val = 128

        json_copy = json.loads(
            self.serializer.serialize(n_copy, protocol=Protocol.JSON).decode()
        )
        self.assertEqual(n_copy.c.name, "bar")
        self.assertEqual(n_copy.c.val, 128)
        self.assertEqual(json_copy["c"]["name"], "bar")
        self.assertEqual(json_copy["c"]["val"], 128)

    def test_nested_struct_list_deepcopy(self) -> None:
        n = self.NestedStructContainers(
            # pyre-ignore[6]: need to make ThriftListWrapper a Sequence
            easy_list=self.to_list([self.easy(name="foo", val=42)])
        )

        n_copy = copy.deepcopy(n)
        e_copy = n_copy.easy_list[0]
        self.assertEqual(n.easy_list[0], e_copy)

        if self.is_mutable_run:
            self.assertIsNot(n.easy_list[0], e_copy)
        else:
            self.assertIs(n, n_copy)
            return

        assert isinstance(e_copy, mutable_test_types.easy)
        e_copy.name = "bar"
        e_copy.val = 128

        json_copy = json.loads(
            self.serializer.serialize(n_copy, protocol=Protocol.JSON).decode()
        )
        self.assertEqual(n_copy.easy_list[0].name, "bar")
        self.assertEqual(n_copy.easy_list[0].val, 128)
        # BAD: the serialized values don't match the mutated python values
        self.assertEqual(json_copy["easy_list"][0]["name"], "bar")
        self.assertEqual(json_copy["easy_list"][0]["val"], 128)

    def test_nested_struct_map_deepcopy(self) -> None:
        n = self.NestedStructContainers(
            # pyre-ignore[6]: need to make ThriftListWrapper a Sequence
            easy_map=self.to_map({"baz": self.easy(name="foo", val=42)})
        )

        n_copy = copy.deepcopy(n)
        e_copy = n_copy.easy_map["baz"]
        self.assertEqual(n.easy_map["baz"], e_copy)

        if self.is_mutable_run:
            self.assertIsNot(n.easy_map["baz"], e_copy)
        else:
            self.assertIs(n, n_copy)
            return

        assert isinstance(e_copy, mutable_test_types.easy)
        e_copy.name = "bar"
        e_copy.val = 128

        json_copy = json.loads(
            self.serializer.serialize(n_copy, protocol=Protocol.JSON).decode()
        )
        self.assertEqual(n_copy.easy_map["baz"].name, "bar")
        self.assertEqual(n_copy.easy_map["baz"].val, 128)
        # NOTE: after landing a mitigation, these work as expected
        self.assertEqual(json_copy["easy_map"]["baz"]["name"], "bar")
        self.assertEqual(json_copy["easy_map"]["baz"]["val"], 128)
