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
