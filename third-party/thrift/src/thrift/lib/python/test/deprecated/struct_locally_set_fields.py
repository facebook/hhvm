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
from parameterized import parameterized
from pyre_extensions import none_throws
from testing.dependency.thrift_types import IncludedStruct
from thrift.python.test.deprecated.locally_set_fields.thrift_types import (
    File,
    HasContainers,
    Integers,
    Kind,
    mixed,
    Nested,
    OptionalFile,
    Optionals,
    SimpleStruct,
    StructWithIssetInspection,
    UnusedError,
)
from thrift.python.test.deprecated.locally_set_fields_annotation.thrift_types import (
    AnnotatedStructWithIssetInspection,
    AnnotatedStructWithUnannotatedChild,
    UnannotatedStructWithAnnotatedChild,
    UnannotatedStructWithoutIssetInspection,
)
from thrift.python.types import get_locally_set_fields


class StructLocallySetFieldsTest(unittest.TestCase):
    """Each struct in this library is annotated with
    `@python.EnableUnsafeIssetInspection`, enabling `get_locally_set_fields()`
    for it.
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
                frozenset(
                    {"int_field", "opt_str_field", "bool_field", "opt_list_field"}
                ),
            ),
            ("int_field", {"int_field": 42}, frozenset({"int_field"})),
            ("no_fields", {}, frozenset()),
            (
                "falsey_values",
                {"int_field": 0, "bool_field": False},
                frozenset({"int_field", "bool_field"}),
            ),
            (
                "explicit_none",
                {"int_field": 42, "opt_str_field": None},
                frozenset({"int_field"}),
            ),
        ]
    )
    def test_get_locally_set_fields_constructor(
        self, _name: str, kwargs: dict[str, Any], expected: frozenset[str]
    ) -> None:
        s = StructWithIssetInspection(**kwargs)
        self.assertEqual(get_locally_set_fields(s), expected)

    @parameterized.expand(
        [
            (
                "set_new_field",
                {"int_field": 42},
                {"bool_field": True},
                frozenset({"int_field", "bool_field"}),
            ),
            (
                "clear_optional_field",
                {"int_field": 42, "opt_str_field": "hello", "bool_field": True},
                {"opt_str_field": None},
                frozenset({"int_field", "bool_field"}),
            ),
        ]
    )
    def test_get_locally_set_fields_call(
        self,
        _name: str,
        init_kwargs: dict[str, Any],
        call_kwargs: dict[str, Any],
        expected: frozenset[str],
    ) -> None:
        s = StructWithIssetInspection(**init_kwargs)
        self.assertEqual(
            get_locally_set_fields(s(**call_kwargs)),
            expected,
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

    @parameterized.expand(
        [
            (
                "default_constructed",
                lambda: AnnotatedStructWithUnannotatedChild(
                    child=UnannotatedStructWithoutIssetInspection()
                ),
            ),
            (
                "constructed_with_kwargs",
                lambda: AnnotatedStructWithUnannotatedChild(
                    child=UnannotatedStructWithoutIssetInspection(
                        int_field=42,
                        opt_str_field="hello",
                    )
                ),
            ),
            (
                "serialize_deserialized",
                lambda: serializer.deserialize(
                    AnnotatedStructWithUnannotatedChild,
                    serializer.serialize_iobuf(
                        AnnotatedStructWithUnannotatedChild(
                            child=UnannotatedStructWithoutIssetInspection(
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
        self,
        _name: str,
        create_struct: Callable[[], AnnotatedStructWithUnannotatedChild],
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
                lambda: UnannotatedStructWithAnnotatedChild(
                    child=AnnotatedStructWithIssetInspection()
                ),
                frozenset(),
            ),
            (
                "constructed_with_kwargs",
                lambda: UnannotatedStructWithAnnotatedChild(
                    child=AnnotatedStructWithIssetInspection(
                        int_field=42,
                        opt_str_field="hello",
                    )
                ),
                frozenset({"int_field", "opt_str_field"}),
            ),
            (
                "serialize_deserialized",
                lambda: serializer.deserialize(
                    UnannotatedStructWithAnnotatedChild,
                    serializer.serialize_iobuf(
                        UnannotatedStructWithAnnotatedChild(
                            child=AnnotatedStructWithIssetInspection(
                                int_field=42,
                                opt_str_field="hello",
                            )
                        )
                    ),
                ),
                frozenset({"int_field", "opt_str_field"}),
            ),
        ]
    )
    def test_get_locally_set_fields_unannotated_parent_with_annotated_child(
        self,
        _name: str,
        create_struct: Callable[[], UnannotatedStructWithAnnotatedChild],
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

    def test_get_locally_set_fields_deserialized_tracks_present_fields(self) -> None:
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

    def test_get_locally_set_fields_call_from_deserialized_updates_present_fields(
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

    def test_compiler_option_enables_unannotated_structs(self) -> None:
        s = SimpleStruct(value=42)
        self.assertEqual(get_locally_set_fields(s), frozenset({"value"}))

    def test_deserialized_optional_fields(self) -> None:
        to_serialize = OptionalFile(name="/dev/null", type=8)
        serialized = serializer.serialize_iobuf(to_serialize)
        file = serializer.deserialize(File, serialized)
        self.assertIn("type", get_locally_set_fields(file))
        self.assertNotIn("permissions", get_locally_set_fields(file))

        to_serialize = OptionalFile(name="/dev/null")
        serialized = serializer.serialize_iobuf(to_serialize)
        file = serializer.deserialize(File, serialized)
        self.assertEqual(file.type, Kind.REGULAR)
        self.assertNotIn("type", get_locally_set_fields(file))

    def test_union_not_supported(self) -> None:
        i = Integers(large=2)
        with self.assertRaises(TypeError):
            get_locally_set_fields(typing_cast(Any, i))

    def test_defaulted_optional_fields_absent(self) -> None:
        def assert_optional_fields_absent(m: mixed) -> None:
            locally_set_fields = get_locally_set_fields(m)
            for fld_name, _ in mixed:
                if not fld_name.startswith("opt_"):
                    continue
                self.assertNotIn(fld_name, locally_set_fields)

        m = mixed()
        assert_optional_fields_absent(m)

        m = m(some_field_="don't care")
        assert_optional_fields_absent(m)

        m = serializer.deserialize(mixed, serializer.serialize(m))
        assert_optional_fields_absent(m)

        m = mixed(opt_field=None)
        self.assertNotIn("opt_field", get_locally_set_fields(m))

        m = m(opt_field=None)
        self.assertNotIn("opt_field", get_locally_set_fields(m))

        m = serializer.deserialize(mixed, serializer.serialize(m))
        self.assertNotIn("opt_field", get_locally_set_fields(m))

    def test_single_field_struct_copy_preserves_locally_set_fields(self) -> None:
        # Regression test for the copy-then-update `Struct.__call__`
        # reimplementation. The locally set field state is copied with
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

        self.assertIn("values", get_locally_set_fields(with_values))
        serialized = serializer.serialize_iobuf(with_values)
        roundtrip = serializer.deserialize(Optionals, serialized)
        self.assertIsNotNone(roundtrip.values)
        self.assertEqual(list(roundtrip.values), ["a", "b", "c"])

    def test_locally_set_fields_do_not_affect_equality_and_hash(self) -> None:
        s1 = SimpleStruct(value=0, name="", city="NY")
        s2 = SimpleStruct(city="NY")

        self.assertEqual(s1, s2)
        self.assertEqual(hash(s1), hash(s2))
        self.assertEqual(len({s1, s2}), 1)
        self.assertEqual(
            get_locally_set_fields(s1), frozenset({"name", "value", "city"})
        )
        self.assertEqual(get_locally_set_fields(s2), frozenset({"city"}))

    def test_isset_deprecated_equality_uses_value_comparison(self) -> None:
        # Fields explicitly set to their defaults vs left unset: equal by value
        # but differing in isset. __eq__ must compare values, not the raw
        # isset-bearing internal tuple.
        a = StructWithIssetInspection(int_field=0, bool_field=False)
        b = StructWithIssetInspection()

        self.assertEqual(a, b)
        self.assertEqual(hash(a), hash(b))
        self.assertEqual(len({a, b}), 1)
        self.assertEqual(
            get_locally_set_fields(a), frozenset({"int_field", "bool_field"})
        )
        self.assertEqual(get_locally_set_fields(b), frozenset())

    def test_unannotated_parent_with_annotated_child_uses_value_comparison(
        self,
    ) -> None:
        # The parent isn't isset-tracking itself but transitively contains an
        # isset-tracking struct, so it must compare by value, not by raw tuple:
        # children equal by value but differing in isset must still be equal.
        a = UnannotatedStructWithAnnotatedChild(
            child=AnnotatedStructWithIssetInspection(int_field=0)
        )
        b = UnannotatedStructWithAnnotatedChild(
            child=AnnotatedStructWithIssetInspection()
        )

        # Children are equal by value but differ in isset.
        self.assertEqual(a.child, b.child)
        self.assertEqual(get_locally_set_fields(a.child), frozenset({"int_field"}))
        self.assertEqual(get_locally_set_fields(b.child), frozenset())

        # ... so the parents must compare equal (and hash/dedup consistently).
        self.assertEqual(a, b)
        self.assertEqual(hash(a), hash(b))
        self.assertEqual(len({a, b}), 1)

    def test_nested_and_container_structs(self) -> None:
        c = HasContainers(nested_list=[Nested(num=1), Nested(num=2, label="x")])
        self.assertIn("num", get_locally_set_fields(c.nested_list[0]))
        self.assertNotIn("label", get_locally_set_fields(c.nested_list[0]))
        self.assertIn("label", get_locally_set_fields(c.nested_list[1]))

        c = HasContainers(nested_map={5: Nested(num=1), 6: Nested(num=2, label="x")})
        self.assertIn("num", get_locally_set_fields(c.nested_map[5]))
        self.assertNotIn("label", get_locally_set_fields(c.nested_map[5]))
        self.assertIn("label", get_locally_set_fields(c.nested_map[6]))

        c = HasContainers(
            nested=Nested(num=1),
            nested_list_of_lists=[[Nested(num=1, label="x")]],
            nested_map_of_lists={9: [Nested(num=2)]},
        )
        self.assertIn("num", get_locally_set_fields(c.nested))
        self.assertIn("label", get_locally_set_fields(c.nested_list_of_lists[0][0]))
        self.assertNotIn("label", get_locally_set_fields(c.nested_map_of_lists[9][0]))

    def test_call_preserves_container_locally_set_fields(self) -> None:
        c = HasContainers(nested_list=[Nested(num=1)])
        c2 = c(nested=Nested(num=2, label="y"))
        self.assertIn("label", get_locally_set_fields(c2.nested))
        self.assertIn("num", get_locally_set_fields(c2.nested_list[0]))
        self.assertNotIn("label", get_locally_set_fields(c2.nested_list[0]))

    def test_optional_container_fields_none(self) -> None:
        # Optional list/map struct fields left unset (None), including deeply
        # nested in a sub-struct, must not break locally-set field tracking.
        c = HasContainers(nested=Nested(num=1))
        locally_set_fields = get_locally_set_fields(c.nested)
        self.assertIn("num", locally_set_fields)
        self.assertNotIn("maybe_children", locally_set_fields)
        self.assertNotIn("maybe_map", locally_set_fields)

        # Optional container set, with a child whose own optional containers are
        # None — the nested-None case reached through a list/map.
        c2 = HasContainers(
            nested=Nested(
                num=2,
                maybe_children=[Nested(num=3)],
                maybe_map={5: Nested(num=4)},
            )
        )
        self.assertIn("maybe_children", get_locally_set_fields(c2.nested))
        maybe_children = none_throws(c2.nested.maybe_children)
        self.assertIn("num", get_locally_set_fields(maybe_children[0]))
        self.assertNotIn("maybe_children", get_locally_set_fields(maybe_children[0]))
        maybe_map = none_throws(c2.nested.maybe_map)
        self.assertIn("num", get_locally_set_fields(maybe_map[5]))
        self.assertNotIn("maybe_map", get_locally_set_fields(maybe_map[5]))

    def test_unsupported_without_compiler_option(self) -> None:
        # `IncludedStruct` comes from the `dependency` library, which is NOT
        # compiled with the `enable_isset_deprecated_unsafe` compiler option, so
        # `get_locally_set_fields()` is unsupported for it.
        with self.assertRaises(AttributeError):
            get_locally_set_fields(IncludedStruct())

    def test_set_of_structs(self) -> None:
        # A struct reached through a `set<struct>` container tracks its own
        # locally set fields, just like list/map elements.
        c = HasContainers(nested_set={Nested(num=1, label="x")})
        (elem,) = tuple(c.nested_set)
        locally_set = get_locally_set_fields(elem)
        self.assertIn("num", locally_set)
        self.assertIn("label", locally_set)

        # Optional `set<struct>` left unset (None) is reported as absent, and a
        # populated optional set is reported as present; its element tracks its
        # own fields.
        self.assertNotIn("maybe_set", get_locally_set_fields(Nested(num=2)))

        parent = HasContainers(nested=Nested(num=3, maybe_set={Nested(num=4)}))
        self.assertIn("maybe_set", get_locally_set_fields(parent.nested))
        maybe_set = none_throws(parent.nested.maybe_set)
        (child,) = tuple(maybe_set)
        self.assertIn("num", get_locally_set_fields(child))

        # Tracking survives a serialize/deserialize round trip too.
        rt = serializer.deserialize(HasContainers, serializer.serialize_iobuf(c))
        (rt_elem,) = tuple(rt.nested_set)
        self.assertIn("num", get_locally_set_fields(rt_elem))
        self.assertIn("label", get_locally_set_fields(rt_elem))
