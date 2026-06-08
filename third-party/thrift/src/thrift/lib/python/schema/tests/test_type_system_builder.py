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

# Importing the fixture module makes its URIs discoverable by the registry.
import thrift.lib.python.schema.tests.type_system_bridge_test.thrift_types  # noqa: F401
from thrift.lib.python.schema.schema_registry import SchemaRegistry
from thrift.lib.python.schema.type_system import (
    EnumerableTypeSystem,
    EnumNode,
    EnumTypeRef,
    EnumValue,
    FieldIdentity,
    InvalidTypeError,
    ListTypeRef,
    MapTypeRef,
    OpaqueAliasNode,
    OpaqueAliasTypeRef,
    PresenceQualifier,
    Primitive,
    PrimitiveTypeRef,
    SetTypeRef,
    StructNode,
    StructTypeRef,
    TypeRef,
    UnionNode,
    UnionTypeRef,
)
from thrift.lib.python.schema.type_system_builder import (
    build_pruned,
    FieldSpec,
    PruneOptions,
    ref,
    TypeInput,
    TypeSystemBuilder,
)


def _field(
    field_id: int,
    name: str,
    type_input: TypeInput,
    presence: PresenceQualifier = PresenceQualifier.UNQUALIFIED,
) -> FieldSpec:
    return FieldSpec(FieldIdentity(field_id, name), presence, type_input)


I32 = PrimitiveTypeRef(Primitive.I32)
STRING = PrimitiveTypeRef(Primitive.STRING)

# `Outer` references `Inner` (directly, via a typedef-to-struct, and inside
# a `list`), so its closure is {Outer, Inner}.
_BRIDGE_URI = "thrift.com/python/schema/ts_bridge"


def _referenced_uris(type_ref: TypeRef) -> list[str]:
    """The user-defined URIs reachable from a ``TypeRef`` (test-local walker used
    to assert that a pruned type system is self-contained)."""
    if isinstance(type_ref, (ListTypeRef, SetTypeRef)):
        return _referenced_uris(type_ref.element_type)
    if isinstance(type_ref, MapTypeRef):
        return _referenced_uris(type_ref.key_type) + _referenced_uris(
            type_ref.value_type
        )
    if isinstance(
        type_ref, (StructTypeRef, UnionTypeRef, EnumTypeRef, OpaqueAliasTypeRef)
    ):
        return [type_ref.node.uri]
    return []


def _field_shape(
    node: StructNode | UnionNode,
) -> list[tuple[FieldIdentity, PresenceQualifier, TypeRef]]:
    return [(f.identity, f.presence, f.type) for f in node.fields]


class BuildBasicTest(unittest.TestCase):
    def test_build_struct_from_scratch(self) -> None:
        # No .thrift, no SyntaxGraph, no sourceInfo.
        ts = TypeSystemBuilder().add_struct("test/Foo", [_field(1, "bar", I32)]).build()
        foo = ts.get_user_defined_type("test/Foo")
        assert isinstance(foo, StructNode)
        bar = foo.field_by_name("bar")
        assert bar is not None
        self.assertEqual(bar.identity.id, 1)
        self.assertEqual(bar.presence, PresenceQualifier.UNQUALIFIED)
        self.assertEqual(bar.type, PrimitiveTypeRef(Primitive.I32))

    def test_field_referencing_user_type_by_uri(self) -> None:
        builder = TypeSystemBuilder()
        builder.add_struct("test/Bar", [_field(1, "x", I32)])
        builder.add_struct("test/Foo", [_field(1, "bar", "test/Bar")])
        ts = builder.build()

        foo = ts.get_user_defined_type("test/Foo")
        assert isinstance(foo, StructNode)
        bar_node = ts.get_user_defined_type("test/Bar")
        assert isinstance(bar_node, StructNode)

        bar_field = foo.field_by_name("bar")
        assert bar_field is not None
        bar_edge = bar_field.type
        assert isinstance(bar_edge, StructTypeRef)
        # Edge holds the resolved node (same object) ...
        self.assertIs(bar_edge.node, bar_node)
        # ... and the TypeRef is structurally equal to one built by URI.
        self.assertEqual(bar_edge, StructTypeRef(bar_node))

    def test_unresolved_uri_raises(self) -> None:
        builder = TypeSystemBuilder()
        builder.add_struct("test/Foo", [_field(1, "bar", "test/Missing")])
        with self.assertRaises(InvalidTypeError):
            builder.build()

    def test_field_with_container_of_user_type(self) -> None:
        builder = TypeSystemBuilder()
        builder.add_struct("test/Bar", [_field(1, "x", I32)])
        builder.add_struct(
            "test/Foo", [_field(1, "bars", ListTypeRef(ref("test/Bar")))]
        )
        ts = builder.build()

        foo = ts.get_user_defined_type("test/Foo")
        assert isinstance(foo, StructNode)
        bar_node = ts.get_user_defined_type("test/Bar")
        assert isinstance(bar_node, StructNode)

        bars = foo.field_by_name("bars")
        assert bars is not None
        bars_edge = bars.type
        self.assertEqual(bars_edge, ListTypeRef(StructTypeRef(bar_node)))
        assert isinstance(bars_edge, ListTypeRef)
        element = bars_edge.element_type
        assert isinstance(element, StructTypeRef)
        self.assertIs(element.node, bar_node)


class TwoPhaseCycleTest(unittest.TestCase):
    def test_mutually_recursive_structs(self) -> None:
        builder = TypeSystemBuilder()
        builder.add_struct(
            "test/A",
            [_field(1, "b", "test/B", PresenceQualifier.OPTIONAL)],
        )
        builder.add_struct(
            "test/B",
            [_field(1, "a", "test/A", PresenceQualifier.OPTIONAL)],
        )
        ts = builder.build()

        a = ts.get_user_defined_type("test/A")
        b = ts.get_user_defined_type("test/B")
        assert isinstance(a, StructNode)
        assert isinstance(b, StructNode)

        a_to_b = a.field_by_name("b")
        assert a_to_b is not None
        a_edge = a_to_b.type
        assert isinstance(a_edge, StructTypeRef)
        b_to_a = b.field_by_name("a")
        assert b_to_a is not None
        b_edge = b_to_a.type
        assert isinstance(b_edge, StructTypeRef)

        # Stable node identity across the cycle.
        self.assertIs(a_edge.node, b)
        self.assertIs(b_edge.node, a)

    def test_self_recursive_struct(self) -> None:
        builder = TypeSystemBuilder()
        builder.add_struct(
            "test/Node",
            [_field(1, "next", "test/Node", PresenceQualifier.OPTIONAL)],
        )
        ts = builder.build()
        node = ts.get_user_defined_type("test/Node")
        assert isinstance(node, StructNode)
        next_field = node.field_by_name("next")
        assert next_field is not None
        next_edge = next_field.type
        assert isinstance(next_edge, StructTypeRef)
        self.assertIs(next_edge.node, node)


class UnionTest(unittest.TestCase):
    def test_union_forces_all_fields_optional(self) -> None:
        builder = TypeSystemBuilder()
        builder.add_union(
            "test/U",
            [
                _field(1, "a", I32, PresenceQualifier.UNQUALIFIED),
                _field(2, "b", STRING, PresenceQualifier.TERSE),
            ],
        )
        ts = builder.build()
        union = ts.get_user_defined_type("test/U")
        assert isinstance(union, UnionNode)
        self.assertEqual(len(union.fields), 2)
        for field in union.fields:
            self.assertEqual(field.presence, PresenceQualifier.OPTIONAL)


class ValidatorTest(unittest.TestCase):
    def test_duplicate_field_id_rejected(self) -> None:
        builder = TypeSystemBuilder()
        builder.add_struct("test/Foo", [_field(1, "a", I32), _field(1, "b", I32)])
        with self.assertRaises(InvalidTypeError):
            builder.build()

    def test_duplicate_field_name_rejected(self) -> None:
        builder = TypeSystemBuilder()
        builder.add_struct("test/Foo", [_field(1, "a", I32), _field(2, "a", I32)])
        with self.assertRaises(InvalidTypeError):
            builder.build()

    def test_duplicate_enum_name_rejected(self) -> None:
        builder = TypeSystemBuilder()
        builder.add_enum("test/E", [EnumValue("A", 0), EnumValue("A", 1)])
        with self.assertRaises(InvalidTypeError):
            builder.build()

    def test_duplicate_enum_datum_rejected(self) -> None:
        builder = TypeSystemBuilder()
        builder.add_enum("test/E", [EnumValue("A", 0), EnumValue("B", 0)])
        with self.assertRaises(InvalidTypeError):
            builder.build()

    def test_duplicate_uri_rejected(self) -> None:
        builder = TypeSystemBuilder()
        builder.add_struct("test/Foo", [_field(1, "a", I32)])
        with self.assertRaises(InvalidTypeError):
            builder.add_struct("test/Foo", [_field(1, "b", I32)])

    def test_empty_uri_rejected(self) -> None:
        builder = TypeSystemBuilder()
        with self.assertRaises(InvalidTypeError):
            builder.add_struct("", [_field(1, "a", I32)])


class EnumTest(unittest.TestCase):
    def test_enum_values_preserved_in_order(self) -> None:
        builder = TypeSystemBuilder()
        builder.add_enum(
            "test/Color",
            [EnumValue("RED", 0), EnumValue("GREEN", 1), EnumValue("BLUE", 2)],
        )
        ts = builder.build()
        color = ts.get_user_defined_type("test/Color")
        assert isinstance(color, EnumNode)
        self.assertEqual(
            color.values,
            (EnumValue("RED", 0), EnumValue("GREEN", 1), EnumValue("BLUE", 2)),
        )


class OpaqueAliasTest(unittest.TestCase):
    def test_alias_to_primitive(self) -> None:
        builder = TypeSystemBuilder()
        builder.add_opaque_alias("test/MyId", PrimitiveTypeRef(Primitive.I64))
        ts = builder.build()
        alias = ts.get_user_defined_type("test/MyId")
        assert isinstance(alias, OpaqueAliasNode)
        self.assertEqual(alias.target_type, PrimitiveTypeRef(Primitive.I64))

    def test_alias_to_container(self) -> None:
        builder = TypeSystemBuilder()
        builder.add_opaque_alias("test/IntList", ListTypeRef(I32))
        ts = builder.build()
        alias = ts.get_user_defined_type("test/IntList")
        assert isinstance(alias, OpaqueAliasNode)
        self.assertEqual(
            alias.target_type, ListTypeRef(PrimitiveTypeRef(Primitive.I32))
        )

    def test_alias_to_user_defined_target_rejected(self) -> None:
        builder = TypeSystemBuilder()
        builder.add_struct("test/S", [_field(1, "a", I32)])
        builder.add_opaque_alias("test/AliasToS", "test/S")
        with self.assertRaises(InvalidTypeError):
            builder.build()


class BuildPrunedFromBuilderTest(unittest.TestCase):
    """``build_pruned`` over a builder-made ``IndexedTypeSystem`` source."""

    def _source(self) -> EnumerableTypeSystem:
        builder = TypeSystemBuilder()
        builder.add_struct("test/Bar", [_field(1, "x", I32)])
        builder.add_enum("test/Color", [EnumValue("RED", 0), EnumValue("GREEN", 1)])
        builder.add_struct("test/Unrelated", [_field(1, "y", I32)])
        builder.add_struct(
            "test/Foo",
            [
                _field(1, "bar", "test/Bar"),  # field type -> Bar
                _field(2, "colors", SetTypeRef(ref("test/Color"))),  # set elt -> Color
                _field(
                    3,
                    "lookup",
                    MapTypeRef(ref("test/Color"), ref("test/Bar")),  # key/value -> deps
                ),
            ],
        )
        return builder.build()

    def test_closure_pulls_field_container_and_map_deps(self) -> None:
        source = self._source()
        pruned = build_pruned(source, ["test/Foo"])
        self.assertEqual(
            pruned.get_known_uris(),
            frozenset({"test/Foo", "test/Bar", "test/Color"}),
        )
        # Unrelated types are not pulled into the closure.
        self.assertIsNone(pruned.get_user_defined_type("test/Unrelated"))

    def test_result_is_self_contained(self) -> None:
        pruned = build_pruned(self._source(), ["test/Foo"])
        known = pruned.get_known_uris()
        assert known is not None
        for uri in known:
            node = pruned.get_user_defined_type(uri)
            if isinstance(node, (StructNode, UnionNode)):
                for field in node.fields:
                    for ref_uri in _referenced_uris(field.type):
                        self.assertIn(ref_uri, known)

    def test_opaque_alias_target_closure(self) -> None:
        builder = TypeSystemBuilder()
        builder.add_struct("test/Bar", [_field(1, "x", I32)])
        # An opaque alias to `list<Bar>` -- the target transitively references a
        # user-defined type, which the closure must follow.
        builder.add_opaque_alias("test/BarList", ListTypeRef(ref("test/Bar")))
        source = builder.build()

        pruned = build_pruned(source, ["test/BarList"])
        self.assertEqual(
            pruned.get_known_uris(), frozenset({"test/BarList", "test/Bar"})
        )

    def test_enum_root_has_empty_closure(self) -> None:
        pruned = build_pruned(self._source(), ["test/Color"])
        self.assertEqual(pruned.get_known_uris(), frozenset({"test/Color"}))

    def test_root_absent_from_source_raises(self) -> None:
        with self.assertRaises(InvalidTypeError):
            build_pruned(self._source(), ["test/Missing"])

    def test_independent_copy_is_default(self) -> None:
        source = self._source()
        pruned = build_pruned(source, ["test/Foo"])

        # Copied nodes are distinct objects ...
        self.assertIsNot(
            pruned.get_user_defined_type("test/Bar"),
            source.get_user_defined_type("test/Bar"),
        )
        # ... and the copied graph is internally rewired to its own copies.
        pruned_foo = pruned.get_user_defined_type("test/Foo")
        pruned_bar = pruned.get_user_defined_type("test/Bar")
        assert isinstance(pruned_foo, StructNode)
        bar_field = pruned_foo.field_by_name("bar")
        assert bar_field is not None
        bar_edge = bar_field.type
        assert isinstance(bar_edge, StructTypeRef)
        self.assertIs(bar_edge.node, pruned_bar)
        self.assertIsNot(bar_edge.node, source.get_user_defined_type("test/Bar"))

    def test_mutating_source_leaves_independent_copy_intact(self) -> None:
        source = self._source()
        pruned = build_pruned(source, ["test/Foo"])

        # Wipe the source's Bar fields; the independent copy must be unaffected.
        src_bar = source.get_user_defined_type("test/Bar")
        assert isinstance(src_bar, StructNode)
        src_bar._set_fields([])
        self.assertEqual(len(src_bar.fields), 0)

        pruned_bar = pruned.get_user_defined_type("test/Bar")
        assert isinstance(pruned_bar, StructNode)
        self.assertEqual(len(pruned_bar.fields), 1)

    def test_share_reuses_source_nodes(self) -> None:
        source = self._source()
        shared = build_pruned(source, ["test/Foo"], share=True)
        self.assertIs(
            shared.get_user_defined_type("test/Foo"),
            source.get_user_defined_type("test/Foo"),
        )
        self.assertIs(
            shared.get_user_defined_type("test/Bar"),
            source.get_user_defined_type("test/Bar"),
        )
        # The shared subset is still bounded to the closure.
        self.assertEqual(
            shared.get_known_uris(),
            frozenset({"test/Foo", "test/Bar", "test/Color"}),
        )

    def test_include_source_info_false_leaves_structure_unchanged(self) -> None:
        source = self._source()
        default = build_pruned(source, ["test/Foo"])
        no_source_info = build_pruned(
            source, ["test/Foo"], PruneOptions(include_source_info=False)
        )
        self.assertEqual(default.get_known_uris(), no_source_info.get_known_uris())

        a = default.get_user_defined_type("test/Foo")
        b = no_source_info.get_user_defined_type("test/Foo")
        assert isinstance(a, StructNode) and isinstance(b, StructNode)
        self.assertEqual(_field_shape(a), _field_shape(b))


class BuildPrunedFromRegistryTest(unittest.TestCase):
    """``build_pruned`` over the lazy ``SchemaRegistry`` view as the source."""

    def setUp(self) -> None:
        SchemaRegistry._reset()
        self.registry = SchemaRegistry()

    def _uri(self, name: str) -> str:
        return f"{_BRIDGE_URI}/{name}"

    def test_closure_equals_roots_plus_transitive_deps(self) -> None:
        pruned = build_pruned(self.registry, [self._uri("Outer")])
        self.assertEqual(
            pruned.get_known_uris(),
            frozenset({self._uri("Outer"), self._uri("Inner")}),
        )

    def test_mix_and_match_multiple_roots(self) -> None:
        pruned = build_pruned(self.registry, [self._uri("Outer"), self._uri("Color")])
        self.assertEqual(
            pruned.get_known_uris(),
            frozenset({self._uri("Outer"), self._uri("Inner"), self._uri("Color")}),
        )

    def test_result_is_self_contained(self) -> None:
        pruned = build_pruned(self.registry, [self._uri("Outer")])
        known = pruned.get_known_uris()
        assert known is not None
        for uri in known:
            node = pruned.get_user_defined_type(uri)
            if isinstance(node, (StructNode, UnionNode)):
                for field in node.fields:
                    for ref_uri in _referenced_uris(field.type):
                        self.assertIn(ref_uri, known)

    def test_root_not_in_registry_raises(self) -> None:
        with self.assertRaises(InvalidTypeError):
            build_pruned(self.registry, ["does.not/Exist"])

    def test_independent_copy_is_default(self) -> None:
        pruned = build_pruned(self.registry, [self._uri("Outer")])
        self.assertIsNot(
            pruned.get_user_defined_type(self._uri("Outer")),
            self.registry.get_user_defined_type(self._uri("Outer")),
        )
        outer = pruned.get_user_defined_type(self._uri("Outer"))
        inner = pruned.get_user_defined_type(self._uri("Inner"))
        assert isinstance(outer, StructNode)
        inner_field = outer.field_by_name("inner")
        assert inner_field is not None
        inner_edge = inner_field.type
        assert isinstance(inner_edge, StructTypeRef)
        self.assertIs(inner_edge.node, inner)

    def test_share_reuses_registry_nodes(self) -> None:
        pruned = build_pruned(self.registry, [self._uri("Outer")], share=True)
        self.assertIs(
            pruned.get_user_defined_type(self._uri("Outer")),
            self.registry.get_user_defined_type(self._uri("Outer")),
        )
