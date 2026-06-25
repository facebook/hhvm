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

import unittest
from typing import Callable

import thrift.python.serializer as serializer
import thrift.python.types
from test_thrift.thrift_types import (
    File,
    Integers,
    Kind,
    mixed,
    OptionalFile,
    Optionals,
    SimpleStruct,
    UnusedError,
)
from testing.dependency.thrift_types import IncludedStruct
from thrift.python.exceptions import GeneratedError
from thrift.python.types import StructOrUnion

# `isset_DEPRECATED` is intentionally excluded from `types.pyi`, so reference it
# through the module to keep the suppression in a single place.
isset_DEPRECATED: Callable[[StructOrUnion | GeneratedError], dict[str, bool]] = (
    thrift.python.types.isset_DEPRECATED  # pyre-ignore[16]: not declared in types.pyi
)


class StructIssetDeprecatedTest(unittest.TestCase):
    """`isset_DEPRECATED()` only supports immutable structs, and only when the
    struct's thrift_library is compiled with the `enable_isset_deprecated_unsafe`
    compiler option. `test_thrift` enables it; the `dependency` library does not.
    """

    def test_isset_Struct(self) -> None:
        to_serialize = OptionalFile(name="/dev/null", type=8)
        serialized = serializer.serialize_iobuf(to_serialize)
        file = serializer.deserialize(File, serialized)
        self.assertTrue(isset_DEPRECATED(file)["type"])
        self.assertFalse(isset_DEPRECATED(file)["permissions"])

        to_serialize = OptionalFile(name="/dev/null")
        serialized = serializer.serialize_iobuf(to_serialize)
        file = serializer.deserialize(File, serialized)
        self.assertEqual(file.type, Kind.REGULAR)
        self.assertFalse(isset_DEPRECATED(file)["type"])

    def test_isset_Error(self) -> None:
        e = UnusedError(message="ACK")
        self.assertTrue(isset_DEPRECATED(e)["message"])

    def test_isset_Union(self) -> None:
        i = Integers(large=2)
        with self.assertRaises(TypeError):
            isset_DEPRECATED(i)["large"]

    def test_isset_defaulted_optional_field(self) -> None:
        def assert_isset_all_false(m: mixed) -> None:
            isset = isset_DEPRECATED(m)
            for fld_name, _ in mixed:
                if not fld_name.startswith("opt_"):
                    continue
                self.assertFalse(isset[fld_name], fld_name)

        # constructor
        m = mixed()
        assert_isset_all_false(m)

        # call operator
        m = m(some_field_="don't care")
        assert_isset_all_false(m)

        # serialization round-trip
        m = serializer.deserialize(mixed, serializer.serialize(m))
        assert_isset_all_false(m)

        # explicit `None` set
        m = mixed(opt_field=None)
        self.assertFalse(isset_DEPRECATED(m)["opt_field"])

        m = m(opt_field=None)
        self.assertFalse(isset_DEPRECATED(m)["opt_field"])

        m = serializer.deserialize(mixed, serializer.serialize(m))
        self.assertFalse(isset_DEPRECATED(m)["opt_field"])

    def test_isset_preserved_on_single_field_struct_copy(self) -> None:
        # Regression test for the copy-then-update `Struct.__call__`
        # reimplementation. The "isset" flags are copied with
        # `PyBytes_FromStringAndSize(ptr, n)` and then mutated in place by
        # `setStructIsset`. For a struct whose isset array is a single byte
        # (exactly one field), that call returns one of CPython's interned
        # single-byte `bytes` singletons instead of a fresh buffer. Two such
        # structs then share the same isset object, so mutating one corrupts the
        # other: the field stays readable in memory but is silently dropped on
        # serialization.
        #
        # `Optionals` has exactly one (optional) field, so its isset array is a
        # single byte. Deriving two values from the same instance -- one setting
        # the field, one clearing it -- triggers the shared-singleton clobber.
        base = Optionals()
        with_values = base(values=["a", "b", "c"])
        # Clearing the field on a sibling copy must not disturb `with_values`.
        _ = base(values=None)

        # The isset flag must still be set ...
        self.assertTrue(isset_DEPRECATED(with_values)["values"])
        # ... so the value survives a serialization round trip.
        serialized = serializer.serialize_iobuf(with_values)
        roundtrip = serializer.deserialize(Optionals, serialized)
        self.assertIsNotNone(roundtrip.values)
        self.assertEqual(list(roundtrip.values), ["a", "b", "c"])

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
        self.assertNotEqual(isset_DEPRECATED(s1), isset_DEPRECATED(s2))

        # s1 has fields explicitly set
        self.assertTrue(isset_DEPRECATED(s1)["value"])
        self.assertTrue(isset_DEPRECATED(s1)["name"])
        self.assertTrue(isset_DEPRECATED(s1)["city"])
        self.assertEqual(
            isset_DEPRECATED(s1), {"name": True, "value": True, "city": True}
        )

        # s2 has fields unset (using defaults)
        self.assertFalse(isset_DEPRECATED(s2)["value"])
        self.assertFalse(isset_DEPRECATED(s2)["name"])
        self.assertTrue(isset_DEPRECATED(s2)["city"])
        self.assertEqual(
            isset_DEPRECATED(s2), {"name": False, "value": False, "city": True}
        )

    def test_isset_unsupported_without_compiler_option(self) -> None:
        # `IncludedStruct` comes from the `dependency` library, which is NOT
        # compiled with the `enable_isset_deprecated_unsafe` compiler option, so
        # `isset_DEPRECATED()` is unsupported for it.
        with self.assertRaises(AttributeError):
            isset_DEPRECATED(IncludedStruct())
