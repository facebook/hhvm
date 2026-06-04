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

from thrift.lib.python.schema.type_system import (
    EnumNode,
    EnumValue,
    FieldIdentity,
    InvalidTypeError,
    ListTypeRef,
    OpaqueAliasNode,
    PresenceQualifier,
    Primitive,
    PrimitiveTypeRef,
    StructNode,
    StructTypeRef,
    UnionNode,
)
from thrift.lib.python.schema.type_system_builder import (
    FieldSpec,
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
