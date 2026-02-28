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

import enum
import sys
import types
import unittest
from typing import Any, Type, TypeVar

import testing.thrift_mutable_types as mutable_types
import testing.thrift_types as immutable_types
import thrift.python.mutable_serializer as mutable_serializer
import thrift.python.serializer as immutable_serializer
from parameterized import parameterized_class
from testing.thrift_types import ComplexUnion, Digits, Integers, ReservedUnion
from thrift.python.mutable_types import _ThriftListWrapper, MutableUnion, to_thrift_list
from thrift.python.serializer import deserialize, serialize_iobuf
from thrift.python.types import Union

ListT = TypeVar("ListT")


class UnionTestImmutable(unittest.TestCase):
    def test_hashability(self) -> None:
        hash(Integers())

    def test_integers_fromValue(self) -> None:
        tiny = 2**7 - 1
        small = 2**15 - 1
        medium = 2**31 - 1
        large = 2**63 - 1
        string = "123"
        union = Integers.fromValue(tiny)
        self.assertEqual(union.type, Integers.Type.tiny)
        self.assertEqual(union.fbthrift_current_field, Integers.Type.tiny)
        self.assertEqual(union.fbthrift_current_value, tiny)
        union = Integers.fromValue(small)
        self.assertEqual(union.fbthrift_current_field, Integers.Type.small)
        self.assertEqual(union.fbthrift_current_value, small)
        union = Integers.fromValue(medium)
        self.assertEqual(union.fbthrift_current_field, Integers.Type.medium)
        self.assertEqual(union.fbthrift_current_value, medium)
        union = Integers.fromValue(large)
        self.assertEqual(union.fbthrift_current_field, Integers.Type.large)
        self.assertEqual(union.fbthrift_current_value, large)
        union = Integers.fromValue(string)
        self.assertEqual(union.fbthrift_current_field, Integers.Type.unbounded)
        self.assertEqual(union.fbthrift_current_value, string)
        union = Integers.fromValue(
            Digits(data=[Integers(tiny=1), Integers(unbounded="123")])
        )
        self.assertEqual(union.type, Integers.Type.digits)
        self.assertEqual(union.fbthrift_current_field, Integers.Type.digits)
        self.assertEqual(
            union.fbthrift_current_value,
            Digits(data=[Integers(tiny=1), Integers(unbounded="123")]),
        )


@parameterized_class(
    ("test_types", "serializer_module"),
    [
        (immutable_types, immutable_serializer),
        (mutable_types, mutable_serializer),
    ],
)
class UnionTests(unittest.TestCase):
    def setUp(self) -> None:
        """
        The `setUp` method performs these assignments with type hints to enable
        pyre when using 'parameterized'. Otherwise, Pyre cannot deduce the types
        behind `test_types`.
        """
        # pyre-ignore[16]: has no attribute `test_types`
        self.Integers: Type[Integers] = self.test_types.Integers
        self.Digits: Type[Digits] = self.test_types.Digits
        self.ReservedUnion: Type[ReservedUnion] = self.test_types.ReservedUnion
        self.ComplexUnion: Type[ComplexUnion] = self.test_types.ComplexUnion
        self.is_mutable_run: bool = self.test_types.__name__.endswith(
            "thrift_mutable_types"
        )
        # pyre-ignore[16]: has no attribute `serializer_module`
        self.serializer: types.ModuleType = self.serializer_module
        self.maxDiff = None

    def to_list(self, list_data: list[ListT]) -> list[ListT] | _ThriftListWrapper:
        return to_thrift_list(list_data) if self.is_mutable_run else list_data

    # pyre-ignore[2, 3]: no type sepcified
    def get_type(self, thrift_union):
        """
        Returns Union.type or MutableUnion.fbthrift_current_field
        """
        return (
            thrift_union.type
            if not self.is_mutable_run
            else thrift_union.fbthrift_current_field
        )

    # pyre-ignore[2, 3]: no type sepcified
    def get_value(self, thrift_union):
        """
        Returns Union.value or MutableUnion.fbthrift_current_value
        """
        return (
            thrift_union.value
            if not self.is_mutable_run
            else thrift_union.fbthrift_current_value
        )

    # pyre-ignore[2, 3]: no type sepcified
    def get_enum(self, thrift_union):
        """
        Returns Union.Type or MutableUnion.FbThriftUnionFieldEnum
        """
        return (
            thrift_union.Type
            if not self.is_mutable_run
            else thrift_union.FbThriftUnionFieldEnum
        )

    def test_constructor(self) -> None:
        self.assertEqual(
            self.get_type(self.Integers(small=2)),
            self.get_enum(self.Integers).small,
        )
        self.assertEqual(
            self.get_type(self.Integers(unbounded="123")),
            self.get_enum(self.Integers).unbounded,
        )

        self.assertEqual(
            self.get_type(
                self.Integers(
                    digits=self.Digits(
                        # pyre-ignore[6]: TODO: Thrift-Container init
                        data=self.to_list(
                            [
                                self.Integers(tiny=1),
                                self.Integers(small=2),
                                self.Integers(large=3),
                            ]
                        )
                    )
                )
            ),
            self.get_enum(self.Integers).digits,
        )

    def test_union_dir(self) -> None:
        expected = [
            "digits",
            "large",
            "medium",
            "name_",
            "small",
            "tiny",
            "unbounded",
        ] + (
            ["type", "value", "fbthrift_current_field", "fbthrift_current_value"]
            if not self.is_mutable_run
            else ["fbthrift_current_field", "fbthrift_current_value"]
        )
        expected.sort()
        self.assertEqual(expected, dir(self.Integers()))
        self.assertEqual(expected, dir(self.Integers))

    def test_union_enum_dir(self) -> None:
        # TODO: fixme(T188685508)
        if sys.version_info.minor > 10:
            return
        contents = dir(self.get_enum(self.Integers))
        self.assertEqual(len(contents), 4 + len(self.get_enum(self.Integers)))
        self.assertIn("__module__", contents)
        self.assertIn("__class__", contents)
        self.assertIn("__doc__", contents)
        self.assertIn("__members__", contents)
        for itype in iter(self.get_enum(self.Integers)):
            self.assertTrue(itype.name in contents)

    def test_union_subclass(self) -> None:
        if self.is_mutable_run:
            self.assertIsInstance(
                self.Integers(tiny=2).fbthrift_current_field, enum.Enum
            )
            self.assertTrue(issubclass(self.Integers.FbThriftUnionFieldEnum, enum.Enum))
        else:
            self.assertIsInstance(self.Integers(tiny=2).type, enum.Enum)
            self.assertTrue(issubclass(self.Integers.Type, enum.Enum))

    def test_no_dict(self) -> None:
        with self.assertRaises(AttributeError):
            self.Integers().__dict__
        # TODO: make it a tuple instead of a list
        # self.assertIsInstance(self.Integer().__class__.__slots__, tuple)
        self.assertIs(self.Integers().__class__.__slots__, self.Integers().__slots__)
        self.assertIs(
            self.Integers(small=5).__slots__, self.Integers(large=500).__slots__
        )

    def test_union_no_dynamic(self) -> None:
        i = self.Integers(small=42)
        with self.assertRaisesRegex(
            AttributeError, "object attribute '.+' is read-only"
        ):
            setattr(
                i,
                "Type" if not self.is_mutable_run else "FbThriftUnionFieldEnum",
                self.get_enum(self.Integers).large,
            )

        with self.assertRaisesRegex(
            AttributeError, "object attribute '.+' is read-only"
        ):
            setattr(
                i,
                "Type" if not self.is_mutable_run else "FbThriftUnionFieldEnum",
                self.get_enum(self.Integers).small,
            )

    def test_union_enum_members(self) -> None:
        members = self.get_enum(self.Integers).__members__
        # Alias can't happen in this enum so these should always equal
        self.assertEqual(len(members), len(self.get_enum(self.Integers)))
        for type in self.get_enum(self.Integers):
            self.assertIn(type.name, members)
            self.assertIs(type, members[type.name])

    def test_deserialize_empty(self) -> None:
        x = self.serializer.deserialize(
            self.Integers, self.serializer.serialize_iobuf(self.Integers())
        )
        self.assertEqual(self.get_type(x), self.get_enum(self.Integers).EMPTY)

    def test_deserialize_nonempty(self) -> None:
        x = self.serializer.deserialize(
            self.Integers, self.serializer.serialize_iobuf(self.Integers(tiny=42))
        )
        self.assertEqual(self.get_type(x), self.get_enum(self.Integers).tiny)

    def test_union_usage(self) -> None:
        value = hash("i64")
        x = self.Integers(large=value)
        self.assertIsInstance(x, Union if not self.is_mutable_run else MutableUnion)
        self.assertEqual(self.get_type(x), x.get_type())
        self.assertEqual(self.get_type(x), self.get_enum(self.Integers).large)
        self.assertEqual(self.get_value(x), value)

        if not self.is_mutable_run:
            # Hashing Works for immutable types
            s = {x}
            self.assertIn(x, s)

        # Repr is useful
        rx = repr(x)
        self.assertIn(str(value), rx)
        self.assertIn(self.get_type(x).name, rx)

    def test_multiple_values(self) -> None:
        with self.assertRaises(TypeError):
            self.Integers(small=1, large=2)

    def test_wrong_type(self) -> None:
        x = self.Integers(small=1)
        with self.assertRaises(AttributeError):
            x.large
        x.small

    def test_reserved_union(self) -> None:
        x = self.ReservedUnion(from_="foo")
        self.assertIsInstance(x, Union if not self.is_mutable_run else MutableUnion)
        self.assertEqual(self.get_type(x), self.get_enum(self.ReservedUnion).from_)
        self.assertEqual(self.get_value(x), "foo")
        self.assertEqual(x.from_, "foo")

        x = self.ReservedUnion(nonlocal_=3)
        self.assertIsInstance(x, Union if not self.is_mutable_run else MutableUnion)
        self.assertEqual(self.get_type(x), self.get_enum(self.ReservedUnion).nonlocal_)
        self.assertEqual(self.get_value(x), 3)
        self.assertEqual(x.nonlocal_, 3)

        x = self.ReservedUnion(ok="bar")
        self.assertIsInstance(x, Union if not self.is_mutable_run else MutableUnion)
        self.assertEqual(self.get_type(x), self.get_enum(self.ReservedUnion).ok)
        self.assertEqual(self.get_value(x), "bar")
        self.assertEqual(x.ok, "bar")

    def test_union_ordering(self) -> None:
        x = self.Integers(tiny=4)
        y = self.ComplexUnion(tiny=1)
        with self.assertRaises(TypeError):
            # flake8: noqa: B015 intentionally introduced for test
            x < y
        # same type, compare value
        y = self.Integers(tiny=2)
        self.assertGreater(x, y)
        self.assertLess(y, x)
        # different type
        y = self.Integers(small=2)
        self.assertLess(x, y)
        self.assertGreater(y, x)
        # equality
        y = self.Integers(tiny=4)
        self.assertEqual(x, y)

    def test_instance_base_class(self) -> None:
        self.assertIsInstance(ComplexUnion(tiny=1), Union)
        self.assertIsInstance(ComplexUnion(tiny=1), ComplexUnion)
        self.assertNotIsInstance(ComplexUnion(tiny=1), ReservedUnion)
        self.assertNotIsInstance(3, Union)
        self.assertNotIsInstance(3, ComplexUnion)
        self.assertTrue(issubclass(ComplexUnion, Union))
        self.assertFalse(issubclass(int, Union))
        self.assertFalse(issubclass(int, ComplexUnion))
        self.assertFalse(issubclass(Union, ComplexUnion))
        self.assertFalse(issubclass(ComplexUnion, ReservedUnion))
