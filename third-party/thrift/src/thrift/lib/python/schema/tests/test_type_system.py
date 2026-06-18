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
from typing import assert_never

from thrift.lib.python.schema._record import Int32Record
from thrift.lib.python.schema.type_system import (
    DefinitionNode,
    EnumNode,
    EnumTypeRef,
    EnumValue,
    FieldDefinition,
    FieldIdentity,
    IndexedTypeSystem,
    InvalidTypeError,
    ListTypeRef,
    MapTypeRef,
    OpaqueAliasNode,
    OpaqueAliasTypeRef,
    PresenceQualifier,
    Primitive,
    PrimitiveTypeRef,
    SetTypeRef,
    SourceInfo,
    StructNode,
    StructTypeRef,
    TypeRef,
    UnionNode,
    UnionTypeRef,
)


def _kind_name(type_ref: TypeRef) -> str:
    match type_ref:
        case PrimitiveTypeRef():
            return "primitive"
        case ListTypeRef():
            return "list"
        case SetTypeRef():
            return "set"
        case MapTypeRef():
            return "map"
        case StructTypeRef():
            return "struct"
        case UnionTypeRef():
            return "union"
        case EnumTypeRef():
            return "enum"
        case OpaqueAliasTypeRef():
            return "opaque_alias"
        case _:
            assert_never(type_ref)


def _i32_field(field_id: int, name: str) -> FieldDefinition:
    return FieldDefinition(
        identity=FieldIdentity(field_id, name),
        presence=PresenceQualifier.UNQUALIFIED,
        type=PrimitiveTypeRef(Primitive.I32),
    )


class AnnotationViewTest(unittest.TestCase):
    def test_field_annotations_is_read_only_view(self) -> None:
        field = FieldDefinition(
            identity=FieldIdentity(1, "f"),
            presence=PresenceQualifier.UNQUALIFIED,
            type=PrimitiveTypeRef(Primitive.I32),
            annotations={"a": Int32Record(1)},
        )
        with self.assertRaises(TypeError):
            field.annotations["b"] = Int32Record(2)  # pyre-ignore[16]: read-only
        self.assertEqual(dict(field.annotations), {"a": Int32Record(1)})

    def test_node_annotations_is_read_only_view(self) -> None:
        node = StructNode(uri="test/S")
        node._set_annotations({"a": Int32Record(1)})
        with self.assertRaises(TypeError):
            node.annotations["b"] = Int32Record(2)  # pyre-ignore[16]: read-only
        self.assertEqual(dict(node.annotations), {"a": Int32Record(1)})


class TypeRefDiscriminationTest(unittest.TestCase):
    def test_isinstance_and_match_over_all_kinds(self) -> None:
        struct_node = StructNode(uri="test/S")
        union_node = UnionNode(uri="test/U")
        enum_node = EnumNode(uri="test/E")
        alias_node = OpaqueAliasNode(
            uri="test/A", target_type=PrimitiveTypeRef(Primitive.I64)
        )

        cases: list[tuple[TypeRef, str]] = [
            (PrimitiveTypeRef(Primitive.I32), "primitive"),
            (ListTypeRef(PrimitiveTypeRef(Primitive.I32)), "list"),
            (SetTypeRef(PrimitiveTypeRef(Primitive.STRING)), "set"),
            (
                MapTypeRef(
                    PrimitiveTypeRef(Primitive.STRING),
                    PrimitiveTypeRef(Primitive.I32),
                ),
                "map",
            ),
            (StructTypeRef(struct_node), "struct"),
            (UnionTypeRef(union_node), "union"),
            (EnumTypeRef(enum_node), "enum"),
            (OpaqueAliasTypeRef(alias_node), "opaque_alias"),
        ]
        for type_ref, expected in cases:
            with self.subTest(kind=expected):
                self.assertEqual(_kind_name(type_ref), expected)

    def test_match_extracts_attributes(self) -> None:
        struct_node = StructNode(uri="test/S")
        primitive = PrimitiveTypeRef(Primitive.I32)
        match primitive:
            case PrimitiveTypeRef(primitive=p):
                self.assertEqual(p, Primitive.I32)
            case _:
                self.fail("expected PrimitiveTypeRef")

        match StructTypeRef(struct_node):
            case StructTypeRef(node=n):
                self.assertIs(n, struct_node)
            case _:
                self.fail("expected StructTypeRef")

        match MapTypeRef(
            PrimitiveTypeRef(Primitive.STRING), PrimitiveTypeRef(Primitive.I32)
        ):
            case MapTypeRef(key_type=k, value_type=v):
                self.assertEqual(k, PrimitiveTypeRef(Primitive.STRING))
                self.assertEqual(v, PrimitiveTypeRef(Primitive.I32))
            case _:
                self.fail("expected MapTypeRef")


class TypeRefEqualityTest(unittest.TestCase):
    def test_primitive_structural_equality(self) -> None:
        self.assertEqual(
            PrimitiveTypeRef(Primitive.I32), PrimitiveTypeRef(Primitive.I32)
        )
        self.assertEqual(
            hash(PrimitiveTypeRef(Primitive.I32)),
            hash(PrimitiveTypeRef(Primitive.I32)),
        )
        self.assertNotEqual(
            PrimitiveTypeRef(Primitive.I32), PrimitiveTypeRef(Primitive.I64)
        )

    def test_container_structural_equality(self) -> None:
        self.assertEqual(
            ListTypeRef(PrimitiveTypeRef(Primitive.I32)),
            ListTypeRef(PrimitiveTypeRef(Primitive.I32)),
        )
        self.assertEqual(
            MapTypeRef(
                PrimitiveTypeRef(Primitive.STRING),
                PrimitiveTypeRef(Primitive.I32),
            ),
            MapTypeRef(
                PrimitiveTypeRef(Primitive.STRING),
                PrimitiveTypeRef(Primitive.I32),
            ),
        )
        # Same element type but different container kind must not be equal.
        self.assertNotEqual(
            ListTypeRef(PrimitiveTypeRef(Primitive.I32)),
            SetTypeRef(PrimitiveTypeRef(Primitive.I32)),
        )

    def test_user_typeref_equality_is_by_uri(self) -> None:
        # Two distinct node objects with the same URI yield equal TypeRefs.
        node_a = StructNode(uri="test/Same")
        node_b = StructNode(uri="test/Same")
        self.assertIsNot(node_a, node_b)
        self.assertEqual(StructTypeRef(node_a), StructTypeRef(node_b))
        self.assertEqual(hash(StructTypeRef(node_a)), hash(StructTypeRef(node_b)))
        self.assertNotEqual(
            StructTypeRef(node_a), StructTypeRef(StructNode(uri="test/Other"))
        )


class DefinitionNodeIdentityTest(unittest.TestCase):
    def test_node_equality_is_by_uri(self) -> None:
        node_a = StructNode(uri="test/Same")
        node_b = StructNode(uri="test/Same")
        self.assertEqual(node_a, node_b)
        self.assertEqual(hash(node_a), hash(node_b))
        # ...but object identity still distinguishes them.
        self.assertIsNot(node_a, node_b)
        self.assertNotEqual(node_a, StructNode(uri="test/Other"))

    def test_nodes_are_usable_as_dict_keys(self) -> None:
        node_a = StructNode(uri="test/Same")
        node_b = StructNode(uri="test/Same")
        index: dict[DefinitionNode, int] = {node_a: 1}
        index[node_b] = 2  # same URI -> overwrites
        self.assertEqual(index[node_a], 2)
        self.assertEqual(len(index), 1)


class StructFieldIndexTest(unittest.TestCase):
    def test_field_by_id_and_name_use_precomputed_indexes(self) -> None:
        f1 = _i32_field(1, "alpha")
        f2 = FieldDefinition(
            identity=FieldIdentity(7, "beta"),
            presence=PresenceQualifier.OPTIONAL,
            type=PrimitiveTypeRef(Primitive.STRING),
        )
        struct_node = StructNode(uri="test/S", fields=[f1, f2])

        self.assertEqual(struct_node.fields, (f1, f2))
        self.assertIs(struct_node.field_by_id(1), f1)
        beta = struct_node.field_by_name("beta")
        self.assertIs(beta, f2)
        assert beta is not None  # narrow for pyre
        self.assertEqual(beta.type, PrimitiveTypeRef(Primitive.STRING))

    def test_field_lookup_misses_return_none(self) -> None:
        struct_node = StructNode(uri="test/S", fields=[_i32_field(1, "alpha")])
        self.assertIsNone(struct_node.field_by_id(99))
        self.assertIsNone(struct_node.field_by_name("missing"))

    def test_is_sealed_defaults_false(self) -> None:
        self.assertFalse(StructNode(uri="test/S").is_sealed)
        self.assertTrue(StructNode(uri="test/S", is_sealed=True).is_sealed)


class UnionNodeTest(unittest.TestCase):
    def test_optional_fields_are_accepted(self) -> None:
        f = FieldDefinition(
            identity=FieldIdentity(1, "a"),
            presence=PresenceQualifier.OPTIONAL,
            type=PrimitiveTypeRef(Primitive.I32),
        )
        self.assertEqual(UnionNode(uri="test/U", fields=[f]).fields, (f,))

    def test_non_optional_field_is_rejected(self) -> None:
        # Union fields must be OPTIONAL.
        # `_i32_field` builds an UNQUALIFIED field.
        with self.assertRaises(InvalidTypeError):
            UnionNode(uri="test/U", fields=[_i32_field(1, "a")])

    def test_empty_union_is_allowed(self) -> None:
        self.assertEqual(UnionNode(uri="test/U").fields, ())


class EnumNodeTest(unittest.TestCase):
    def test_values_returned_as_immutable_tuple(self) -> None:
        red, blue = EnumValue("RED", 0), EnumValue("BLUE", 1)
        enum_node = EnumNode(uri="test/Color", values=[red, blue])
        self.assertEqual(enum_node.values, (red, blue))

    def test_values_default_to_empty_tuple(self) -> None:
        self.assertEqual(EnumNode(uri="test/Empty").values, ())


class EnumValueAnnotationTest(unittest.TestCase):
    def test_value_carries_annotations(self) -> None:
        value = EnumValue("RED", 0, annotations={"a": Int32Record(1)})
        self.assertEqual(dict(value.annotations), {"a": Int32Record(1)})

    def test_value_annotations_default_to_empty(self) -> None:
        self.assertEqual(dict(EnumValue("RED", 0).annotations), {})

    def test_value_annotations_is_read_only_view(self) -> None:
        value = EnumValue("RED", 0, annotations={"a": Int32Record(1)})
        with self.assertRaises(TypeError):
            value.annotations["b"] = Int32Record(2)  # pyre-ignore[16]: read-only
        self.assertEqual(dict(value.annotations), {"a": Int32Record(1)})

    def test_value_equality_ignores_annotations(self) -> None:
        self.assertEqual(
            EnumValue("RED", 0, annotations={"a": Int32Record(1)}),
            EnumValue("RED", 0),
        )
        self.assertEqual(
            hash(EnumValue("RED", 0, annotations={"a": Int32Record(1)})),
            hash(EnumValue("RED", 0)),
        )


class IndexedTypeSystemTest(unittest.TestCase):
    def _make_ts(self) -> tuple[IndexedTypeSystem, StructNode, EnumNode]:
        struct_node = StructNode(uri="test/Foo", fields=[_i32_field(1, "alpha")])
        enum_node = EnumNode(
            uri="test/Color", values=[EnumValue("RED", 0), EnumValue("BLUE", 1)]
        )
        ts = IndexedTypeSystem({"test/Foo": struct_node, "test/Color": enum_node})
        return ts, struct_node, enum_node

    def test_get_known_uris_returns_exact_frozenset(self) -> None:
        ts, _, _ = self._make_ts()
        self.assertEqual(ts.get_known_uris(), frozenset({"test/Foo", "test/Color"}))

    def test_get_user_defined_type(self) -> None:
        ts, struct_node, enum_node = self._make_ts()
        self.assertIs(ts.get_user_defined_type("test/Foo"), struct_node)
        self.assertIs(ts.get_user_defined_type("test/Color"), enum_node)
        self.assertIsNone(ts.get_user_defined_type("test/Missing"))

    def test_get_user_defined_type_or_throw(self) -> None:
        ts, struct_node, _ = self._make_ts()
        self.assertIs(ts.get_user_defined_type_or_throw("test/Foo"), struct_node)
        with self.assertRaises(InvalidTypeError):
            ts.get_user_defined_type_or_throw("test/Missing")

    def test_key_must_match_node_uri(self) -> None:
        # A key that disagrees with the node's own uri is rejected, so URI
        # lookups can never return a node that identifies as a different uri.
        with self.assertRaises(InvalidTypeError):
            IndexedTypeSystem({"test/Foo": StructNode(uri="test/Bar")})


class SourceInfoTest(unittest.TestCase):
    def test_exposes_locator_and_name(self) -> None:
        info = SourceInfo("file://a.thrift", "Foo")
        self.assertEqual(info.locator, "file://a.thrift")
        self.assertEqual(info.name, "Foo")

    def test_equality_and_hash_by_locator_and_name(self) -> None:
        self.assertEqual(
            SourceInfo("file://a.thrift", "Foo"),
            SourceInfo("file://a.thrift", "Foo"),
        )
        self.assertEqual(
            hash(SourceInfo("file://a.thrift", "Foo")),
            hash(SourceInfo("file://a.thrift", "Foo")),
        )
        # Differs by locator.
        self.assertNotEqual(
            SourceInfo("file://a.thrift", "Foo"),
            SourceInfo("file://b.thrift", "Foo"),
        )
        # Differs by name.
        self.assertNotEqual(
            SourceInfo("file://a.thrift", "Foo"),
            SourceInfo("file://a.thrift", "Bar"),
        )

    def test_repr_is_informative(self) -> None:
        rendered = repr(SourceInfo("file://a.thrift", "Foo"))
        self.assertIn("file://a.thrift", rendered)
        self.assertIn("Foo", rendered)


class NodeSourceInfoTest(unittest.TestCase):
    def _all_node_kinds(self, info: SourceInfo | None) -> list[DefinitionNode]:
        return [
            StructNode(uri="test/S", source_info=info),
            UnionNode(uri="test/U", source_info=info),
            EnumNode(uri="test/E", source_info=info),
            OpaqueAliasNode(
                uri="test/A",
                target_type=PrimitiveTypeRef(Primitive.I64),
                source_info=info,
            ),
        ]

    def test_each_node_kind_carries_source_info(self) -> None:
        info = SourceInfo("file://a.thrift", "X")
        for node in self._all_node_kinds(info):
            with self.subTest(kind=type(node).__name__):
                self.assertEqual(node.source_info, info)

    def test_source_info_defaults_to_none(self) -> None:
        for node in self._all_node_kinds(None):
            with self.subTest(kind=type(node).__name__):
                self.assertIsNone(node.source_info)

    def test_source_info_is_identity_neutral(self) -> None:
        # Two same-URI nodes with *different* source_info (and one with None)
        # are still equal and hash equally.
        a = StructNode(uri="test/Same", source_info=SourceInfo("file://a", "A"))
        b = StructNode(uri="test/Same", source_info=SourceInfo("file://b", "B"))
        c = StructNode(uri="test/Same")  # no source_info
        self.assertEqual(a, b)
        self.assertEqual(a, c)
        self.assertEqual(hash(a), hash(b))
        self.assertEqual(hash(a), hash(c))


class SourceIdentifierLookupTest(unittest.TestCase):
    _A = "file://test/a.thrift"
    _B = "file://test/b.thrift"

    def _make_ts(
        self,
    ) -> tuple[IndexedTypeSystem, StructNode, EnumNode, StructNode, StructNode]:
        # `foo` and `color` share locator `_A`; `bar` is at `_B`; `nosrc`
        # carries no source_info (so it is absent from the source index).
        foo = StructNode(
            uri="test/Foo",
            fields=[_i32_field(1, "x")],
            source_info=SourceInfo(self._A, "Foo"),
        )
        color = EnumNode(
            uri="test/Color",
            values=[EnumValue("RED", 0)],
            source_info=SourceInfo(self._A, "Color"),
        )
        bar = StructNode(uri="test/Bar", source_info=SourceInfo(self._B, "Bar"))
        nosrc = StructNode(uri="test/NoSrc")
        ts = IndexedTypeSystem(
            {
                "test/Foo": foo,
                "test/Color": color,
                "test/Bar": bar,
                "test/NoSrc": nosrc,
            }
        )
        return ts, foo, color, bar, nosrc

    def test_by_source_identifier_resolves(self) -> None:
        ts, foo, color, bar, _ = self._make_ts()
        self.assertIs(
            ts.get_user_defined_type_by_source_identifier(self._A, "Foo"), foo
        )
        self.assertIs(
            ts.get_user_defined_type_by_source_identifier(self._A, "Color"), color
        )
        self.assertIs(
            ts.get_user_defined_type_by_source_identifier(self._B, "Bar"), bar
        )

    def test_by_source_identifier_miss_returns_none(self) -> None:
        ts, _, _, _, _ = self._make_ts()
        # Unknown name at a known locator.
        self.assertIsNone(
            ts.get_user_defined_type_by_source_identifier(self._A, "Nope")
        )
        # Unknown locator.
        self.assertIsNone(
            ts.get_user_defined_type_by_source_identifier("file://test/zzz", "Foo")
        )

    def test_at_location_returns_name_to_node_map(self) -> None:
        ts, foo, color, bar, _ = self._make_ts()
        self.assertEqual(
            dict(ts.get_user_defined_types_at_location(self._A)),
            {"Foo": foo, "Color": color},
        )
        self.assertEqual(
            dict(ts.get_user_defined_types_at_location(self._B)), {"Bar": bar}
        )

    def test_at_location_unknown_is_empty(self) -> None:
        ts, _, _, _, _ = self._make_ts()
        self.assertEqual(dict(ts.get_user_defined_types_at_location("file://nope")), {})

    def test_node_without_source_info_is_absent_from_index(self) -> None:
        ts, _, _, _, nosrc = self._make_ts()
        # The no-source node resolves by URI ...
        self.assertIs(ts.get_user_defined_type("test/NoSrc"), nosrc)
        # ... but never appears in any source-location map.
        all_indexed = {
            *ts.get_user_defined_types_at_location(self._A).values(),
            *ts.get_user_defined_types_at_location(self._B).values(),
        }
        self.assertNotIn(nosrc, all_indexed)

    def test_at_location_view_is_read_only(self) -> None:
        ts, _, _, _, _ = self._make_ts()
        view = ts.get_user_defined_types_at_location(self._A)
        bar = ts.get_user_defined_type_or_throw("test/Bar")
        with self.assertRaises(TypeError):
            view["Bar"] = bar  # pyre-ignore[16]: read-only view

    def test_duplicate_source_identifier_raises(self) -> None:
        # Two distinct URIs sharing one (locator, name) is a malformed type
        # system; the source index rejects it.
        dup = SourceInfo(self._A, "Dup")
        ts = IndexedTypeSystem(
            {
                "test/One": StructNode(uri="test/One", source_info=dup),
                "test/Two": StructNode(uri="test/Two", source_info=dup),
            }
        )
        with self.assertRaises(InvalidTypeError):
            ts.get_user_defined_type_by_source_identifier(self._A, "Dup")
