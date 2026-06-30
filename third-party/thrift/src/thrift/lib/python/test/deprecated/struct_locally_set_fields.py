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
from typing import Any, Callable, cast as typing_cast

import thrift.python.serializer as serializer
import thrift.python.types
from parameterized import parameterized
from pyre_extensions import none_throws
from testing.dependency.thrift_types import IncludedStruct
from thrift.python.exceptions import GeneratedError
from thrift.python.test.deprecated.locally_set_fields_deprecated.thrift_types import (
    File,
    HasContainers,
    Integers,
    Kind,
    mixed,
    Nested,
    OptionalFile,
    Optionals,
    SimpleStruct,
    StructWithAnnotatedChild,
    StructWithIssetInspection,
    StructWithoutIssetInspection,
    StructWithUnannotatedChild,
    UnusedError,
)
from thrift.python.types import get_locally_set_fields, StructOrUnion

# `isset_DEPRECATED` is intentionally excluded from `types.pyi`, so reference it
# through the module to keep the suppression in a single place.
isset_DEPRECATED: Callable[[StructOrUnion | GeneratedError], dict[str, bool]] = (
    thrift.python.types.isset_DEPRECATED  # pyre-ignore[16]: not declared in types.pyi
)


class StructLocallySetFieldsTest(unittest.TestCase):
    """`isset_DEPRECATED()` only supports immutable structs, and only when the
    struct's thrift_library is compiled with the `enable_isset_deprecated_unsafe`
    compiler option. This library enables it; the `dependency` library does not.
    """

    @parameterized.expand(
        [
            (
                "all_fields",
                {
                    "int_field": 42,
                    "opt_str_field": "hello",
                    "bool_field": True,
                    "opt_list_field": [1, 2, 3],
                },
                {"int_field", "opt_str_field", "bool_field", "opt_list_field"},
            ),
            ("int_field", {"int_field": 42}, {"int_field"}),
            ("no_fields", {}, set()),
            (
                "falsey_values",
                {"int_field": 0, "bool_field": False},
                {"int_field", "bool_field"},
            ),
            ("explicit_none", {"int_field": 42, "opt_str_field": None}, {"int_field"}),
        ]
    )
    def test_get_locally_set_fields_constructor(
        self, _name: str, kwargs: dict[str, Any], expected: set[str]
    ) -> None:
        s = StructWithIssetInspection(**kwargs)
        self.assertEqual(get_locally_set_fields(s), frozenset(expected))

    @parameterized.expand(
        [
            (
                "set_new_field",
                {"int_field": 42},
                {"bool_field": True},
                {"int_field", "bool_field"},
            ),
            (
                "clear_optional_field",
                {"int_field": 42, "opt_str_field": "hello", "bool_field": True},
                {"opt_str_field": None},
                {"int_field", "bool_field"},
            ),
        ]
    )
    def test_get_locally_set_fields_call(
        self,
        _name: str,
        init_kwargs: dict[str, Any],
        call_kwargs: dict[str, Any],
        expected: set[str],
    ) -> None:
        s = StructWithIssetInspection(**init_kwargs)
        self.assertEqual(
            get_locally_set_fields(s(**call_kwargs)),
            frozenset(expected),
        )

    def test_get_locally_set_fields_call_preserves_original(self) -> None:
        s = StructWithIssetInspection(int_field=42, opt_str_field="hello")
        s2 = s(int_field=99)
        self.assertEqual(
            get_locally_set_fields(s), frozenset({"int_field", "opt_str_field"})
        )
        self.assertEqual(
            get_locally_set_fields(s2), frozenset({"int_field", "opt_str_field"})
        )

    def test_get_locally_set_fields_uses_isset_bits(self) -> None:
        s = StructWithIssetInspection(int_field=42, opt_str_field=None)
        self.assertEqual(get_locally_set_fields(s), frozenset({"int_field"}))
        self.assertEqual(
            isset_DEPRECATED(s),
            {
                "int_field": True,
                "opt_str_field": False,
                "bool_field": False,
                "opt_list_field": False,
            },
        )

    @parameterized.expand(
        [
            (
                "default_constructed",
                lambda: StructWithUnannotatedChild(
                    child=StructWithoutIssetInspection()
                ),
            ),
            (
                "constructed_with_kwargs",
                lambda: StructWithUnannotatedChild(
                    child=StructWithoutIssetInspection(
                        int_field=42,
                        opt_str_field="hello",
                    )
                ),
            ),
            (
                "serialize_deserialized",
                lambda: serializer.deserialize(
                    StructWithUnannotatedChild,
                    serializer.serialize_iobuf(
                        StructWithUnannotatedChild(
                            child=StructWithoutIssetInspection(
                                int_field=42,
                                opt_str_field="hello",
                            )
                        )
                    ),
                ),
            ),
        ]
    )
    def test_get_locally_set_fields_annotated_parent_with_unannotated_child(
        self, _name: str, create_struct: Callable[[], StructWithUnannotatedChild]
    ) -> None:
        s = create_struct()

        self.assertEqual(get_locally_set_fields(s), frozenset({"child"}))
        with self.assertRaisesRegex(
            AttributeError, "does not support locally set field inspection"
        ):
            get_locally_set_fields(s.child)

    @parameterized.expand(
        [
            (
                "default_constructed",
                lambda: StructWithAnnotatedChild(child=StructWithIssetInspection()),
                frozenset(),
            ),
            (
                "constructed_with_kwargs",
                lambda: StructWithAnnotatedChild(
                    child=StructWithIssetInspection(
                        int_field=42,
                        opt_str_field="hello",
                    )
                ),
                frozenset({"int_field", "opt_str_field"}),
            ),
            (
                "serialize_deserialized",
                lambda: serializer.deserialize(
                    StructWithAnnotatedChild,
                    serializer.serialize_iobuf(
                        StructWithAnnotatedChild(
                            child=StructWithIssetInspection(
                                int_field=42,
                                opt_str_field="hello",
                            )
                        )
                    ),
                ),
                frozenset({"int_field", "opt_str_field", "bool_field"}),
            ),
        ]
    )
    def test_get_locally_set_fields_unannotated_parent_with_annotated_child(
        self,
        _name: str,
        create_struct: Callable[[], StructWithAnnotatedChild],
        expected: frozenset[str],
    ) -> None:
        s = create_struct()

        with self.assertRaisesRegex(
            AttributeError, "does not support locally set field inspection"
        ):
            get_locally_set_fields(s)
        self.assertEqual(
            get_locally_set_fields(s.child),
            expected,
        )

    def test_get_locally_set_fields_generated_error_not_supported(self) -> None:
        e = UnusedError(message="oops")
        with self.assertRaisesRegex(
            AttributeError, "does not support locally set field inspection"
        ):
            get_locally_set_fields(e)

    def test_get_locally_set_fields_not_annotated_raises(self) -> None:
        with self.assertRaisesRegex(
            AttributeError, "does not support locally set field inspection"
        ):
            get_locally_set_fields(SimpleStruct(value=42))

    def test_get_locally_set_fields_deserialized_uses_isset_bits(self) -> None:
        s = StructWithIssetInspection(int_field=42, opt_str_field="hello")
        serialized = serializer.serialize_iobuf(s)
        deserialized = serializer.deserialize(StructWithIssetInspection, serialized)
        self.assertEqual(
            get_locally_set_fields(deserialized),
            frozenset({"int_field", "opt_str_field", "bool_field"}),
        )

    def test_get_locally_set_fields_tracks_top_level_provenance(self) -> None:
        local = StructWithIssetInspection(int_field=42)
        serialized = serializer.serialize_iobuf(local)
        deserialized = serializer.deserialize(StructWithIssetInspection, serialized)

        self.assertTrue(typing_cast(Any, local)._fbthrift_is_locally_constructed)
        self.assertTrue(
            typing_cast(Any, local(bool_field=True))._fbthrift_is_locally_constructed
        )
        self.assertFalse(
            typing_cast(Any, deserialized)._fbthrift_is_locally_constructed
        )
        self.assertFalse(
            typing_cast(
                Any, deserialized(bool_field=True)
            )._fbthrift_is_locally_constructed
        )

    def test_get_locally_set_fields_call_from_deserialized_updates_isset_bits(
        self,
    ) -> None:
        s = StructWithIssetInspection(int_field=42)
        serialized = serializer.serialize_iobuf(s)
        deserialized = serializer.deserialize(StructWithIssetInspection, serialized)
        s2 = deserialized(bool_field=True)
        self.assertEqual(
            get_locally_set_fields(s2),
            frozenset({"int_field", "bool_field"}),
        )

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
        # `isset_DEPRECATED()` is not supported for exceptions.
        e = UnusedError(message="ACK")
        with self.assertRaises(AttributeError):
            isset_DEPRECATED(e)

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

    def test_isset_nested_struct(self) -> None:
        # A struct reached through a nested field of a locally-constructed parent
        # keeps its own isset.
        c = HasContainers(nested=Nested(num=1))
        self.assertTrue(isset_DEPRECATED(c.nested)["num"])
        self.assertFalse(isset_DEPRECATED(c.nested)["label"])

    def test_isset_list_of_structs(self) -> None:
        c = HasContainers(nested_list=[Nested(num=1), Nested(num=2, label="x")])
        self.assertTrue(isset_DEPRECATED(c.nested_list[0])["num"])
        self.assertFalse(isset_DEPRECATED(c.nested_list[0])["label"])
        self.assertTrue(isset_DEPRECATED(c.nested_list[1])["label"])

    def test_isset_map_of_structs(self) -> None:
        c = HasContainers(nested_map={5: Nested(num=1), 6: Nested(num=2, label="x")})
        self.assertTrue(isset_DEPRECATED(c.nested_map[5])["num"])
        self.assertFalse(isset_DEPRECATED(c.nested_map[5])["label"])
        self.assertTrue(isset_DEPRECATED(c.nested_map[6])["label"])

    def test_isset_all_containers_in_one_struct(self) -> None:
        # The combined shape: a struct holding a nested struct, a list<struct>,
        # and a map<key, struct> all at once.
        c = HasContainers(
            nested=Nested(num=1),
            nested_list=[Nested(num=2, label="a")],
            nested_map={7: Nested(num=3)},
        )
        self.assertTrue(isset_DEPRECATED(c.nested)["num"])
        self.assertTrue(isset_DEPRECATED(c.nested_list[0])["label"])
        self.assertFalse(isset_DEPRECATED(c.nested_map[7])["label"])

    def test_isset_nested_containers(self) -> None:
        # list<list<struct>> and map<key, list<struct>>: the shadow is applied
        # recursively through both container layers.
        c = HasContainers(
            nested_list_of_lists=[[Nested(num=1, label="x")]],
            nested_map_of_lists={9: [Nested(num=2)]},
        )
        self.assertTrue(isset_DEPRECATED(c.nested_list_of_lists[0][0])["label"])
        self.assertFalse(isset_DEPRECATED(c.nested_map_of_lists[9][0])["label"])

    def test_isset_call_preserves_container_isset(self) -> None:
        # `__call__` rebuilds the shadow, so nested/container structs reached
        # from the copy still report isset.
        c = HasContainers(nested_list=[Nested(num=1)])
        c2 = c(nested=Nested(num=2, label="y"))
        self.assertTrue(isset_DEPRECATED(c2.nested)["label"])
        self.assertTrue(isset_DEPRECATED(c2.nested_list[0])["num"])
        self.assertFalse(isset_DEPRECATED(c2.nested_list[0])["label"])

    def test_isset_optional_container_fields_none(self) -> None:
        # Optional list/map struct fields left unset (None), including deeply
        # nested in a sub-struct, must not break the shadow build, and isset
        # must report them as unset.
        c = HasContainers(nested=Nested(num=1))
        isset = isset_DEPRECATED(c.nested)
        self.assertTrue(isset["num"])
        self.assertFalse(isset["maybe_children"])
        self.assertFalse(isset["maybe_map"])

        # Optional container set, with a child whose own optional containers are
        # None — the nested-None case reached through a list/map.
        c2 = HasContainers(
            nested=Nested(
                num=2,
                maybe_children=[Nested(num=3)],
                maybe_map={5: Nested(num=4)},
            )
        )
        self.assertTrue(isset_DEPRECATED(c2.nested)["maybe_children"])
        maybe_children = none_throws(c2.nested.maybe_children)
        self.assertTrue(isset_DEPRECATED(maybe_children[0])["num"])
        self.assertFalse(isset_DEPRECATED(maybe_children[0])["maybe_children"])
        maybe_map = none_throws(c2.nested.maybe_map)
        self.assertTrue(isset_DEPRECATED(maybe_map[5])["num"])
        self.assertFalse(isset_DEPRECATED(maybe_map[5])["maybe_map"])

    def test_isset_unsupported_without_compiler_option(self) -> None:
        # `IncludedStruct` comes from the `dependency` library, which is NOT
        # compiled with the `enable_isset_deprecated_unsafe` compiler option, so
        # `isset_DEPRECATED()` is unsupported for it.
        with self.assertRaises(AttributeError):
            isset_DEPRECATED(IncludedStruct())
