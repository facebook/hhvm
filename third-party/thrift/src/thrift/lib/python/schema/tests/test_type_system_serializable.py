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

"""Tests for the wire-form boundary (``_serializable.py`` + builder edits):
``to_type_id`` / ``resolve_type_id`` / ``TypeRef.id()``,
``build_serializable_type_system`` (runtime -> wire),
``from_serializable`` (wire -> runtime), and ``build_derived_from`` (overlay).
"""

import unittest

# Importing the fixture module makes its URIs discoverable by the registry.
import thrift.lib.python.schema.tests.type_system_bridge_test.thrift_types  # noqa: F401
from apache.thrift.type_system.record.thrift_types import (
    SerializableRecord as WireRecord,
)
from apache.thrift.type_system.type_id.thrift_types import (
    AnyTypeId,
    BinaryTypeId,
    BoolTypeId,
    ByteTypeId,
    DoubleTypeId,
    FloatTypeId,
    I16TypeId,
    I32TypeId,
    I64TypeId,
    ListTypeId,
    MapTypeId,
    SetTypeId,
    StringTypeId,
    TypeId,
)
from apache.thrift.type_system.type_system.thrift_types import (
    FieldIdentity as WireFieldIdentity,
    PresenceQualifier as WirePresence,
    SerializableEnumDefinition,
    SerializableEnumValueDefinition,
    SerializableFieldDefinition,
    SerializableOpaqueAliasDefinition,
    SerializableStructDefinition,
    SerializableTypeDefinition,
    SerializableTypeDefinitionEntry,
    SerializableTypeSystem,
    SerializableUnionDefinition,
)
from thrift.lib.python.schema._record import (
    BoolRecord,
    ByteArrayRecord,
    FieldSetRecord,
    Float32Record,
    Float64Record,
    Int16Record,
    Int32Record,
    Int64Record,
    Int8Record,
    ListRecord,
    MapRecord,
    SetRecord,
    TextRecord,
)
from thrift.lib.python.schema._serializable import (
    build_serializable_type_system,
    from_wire_record,
    resolve_type_id,
    to_serializable_definition,
    to_type_id,
    to_wire_record,
)
from thrift.lib.python.schema.schema_registry import SchemaRegistry
from thrift.lib.python.schema.type_system import (
    DefinitionNode,
    EnumNode,
    EnumValue,
    FieldDefinition,
    FieldIdentity,
    IndexedTypeSystem,
    InvalidTypeError,
    ListTypeRef,
    MapTypeRef,
    OpaqueAliasNode,
    PresenceQualifier,
    Primitive,
    PrimitiveTypeRef,
    SetTypeRef,
    SourceInfo,
    StructNode,
    StructTypeRef,
    TypeRef,
    TypeSystem,
    UnionNode,
)
from thrift.lib.python.schema.type_system_builder import (
    build_derived_from,
    build_pruned,
    FieldSpec,
    from_serializable,
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


_BRIDGE_URI = "thrift.com/python/schema/ts_bridge"

# Every primitive paired with its empty-struct ``TypeId`` arm.
_PRIMITIVE_TYPE_IDS: list[tuple[Primitive, TypeId]] = [
    (Primitive.BOOL, TypeId(boolType=BoolTypeId())),
    (Primitive.BYTE, TypeId(byteType=ByteTypeId())),
    (Primitive.I16, TypeId(i16Type=I16TypeId())),
    (Primitive.I32, TypeId(i32Type=I32TypeId())),
    (Primitive.I64, TypeId(i64Type=I64TypeId())),
    (Primitive.FLOAT, TypeId(floatType=FloatTypeId())),
    (Primitive.DOUBLE, TypeId(doubleType=DoubleTypeId())),
    (Primitive.STRING, TypeId(stringType=StringTypeId())),
    (Primitive.BINARY, TypeId(binaryType=BinaryTypeId())),
    (Primitive.ANY, TypeId(anyType=AnyTypeId())),
]


def _assert_nodes_structurally_equal(
    test: unittest.TestCase, a: DefinitionNode, b: DefinitionNode
) -> None:
    """Deep structural equality (node ``==`` is URI-only, so spot-check the
    contents that a round-trip must preserve)."""
    test.assertEqual(a.uri, b.uri)
    test.assertEqual(type(a), type(b))
    if isinstance(a, (StructNode, UnionNode)):
        assert isinstance(b, (StructNode, UnionNode))
        test.assertEqual(a.is_sealed, b.is_sealed)
        test.assertEqual(len(a.fields), len(b.fields))
        for fa, fb in zip(a.fields, b.fields):
            test.assertEqual(fa.identity, fb.identity)
            test.assertEqual(fa.presence, fb.presence)
            test.assertEqual(fa.type, fb.type)
            test.assertEqual(fa.custom_default, fb.custom_default)
            test.assertEqual(dict(fa.annotations), dict(fb.annotations))
        test.assertEqual(dict(a.annotations), dict(b.annotations))
    elif isinstance(a, EnumNode):
        assert isinstance(b, EnumNode)
        test.assertEqual(a.values, b.values)
        # ``EnumValue`` equality is name+datum only, so compare value
        # annotations explicitly (a round-trip must preserve them).
        for va, vb in zip(a.values, b.values):
            test.assertEqual(dict(va.annotations), dict(vb.annotations))
        test.assertEqual(dict(a.annotations), dict(b.annotations))
    elif isinstance(a, OpaqueAliasNode):
        assert isinstance(b, OpaqueAliasNode)
        test.assertEqual(a.target_type, b.target_type)
        test.assertEqual(dict(a.annotations), dict(b.annotations))


def _assert_type_systems_equal(
    test: unittest.TestCase, a: TypeSystem, b: TypeSystem
) -> None:
    test.assertEqual(a.get_known_uris(), b.get_known_uris())
    known = a.get_known_uris()
    assert known is not None
    for uri in known:
        node_a = a.get_user_defined_type(uri)
        node_b = b.get_user_defined_type(uri)
        assert node_a is not None and node_b is not None
        _assert_nodes_structurally_equal(test, node_a, node_b)


class ToTypeIdTest(unittest.TestCase):
    def test_primitives(self) -> None:
        for primitive, expected in _PRIMITIVE_TYPE_IDS:
            self.assertEqual(to_type_id(PrimitiveTypeRef(primitive)), expected)

    def test_list(self) -> None:
        type_id = to_type_id(ListTypeRef(PrimitiveTypeRef(Primitive.I32)))
        self.assertEqual(
            type_id,
            TypeId(listType=ListTypeId(elementType=TypeId(i32Type=I32TypeId()))),
        )

    def test_set(self) -> None:
        type_id = to_type_id(SetTypeRef(PrimitiveTypeRef(Primitive.STRING)))
        self.assertEqual(
            type_id,
            TypeId(setType=SetTypeId(elementType=TypeId(stringType=StringTypeId()))),
        )

    def test_map(self) -> None:
        type_id = to_type_id(
            MapTypeRef(
                PrimitiveTypeRef(Primitive.STRING), PrimitiveTypeRef(Primitive.I64)
            )
        )
        self.assertEqual(
            type_id,
            TypeId(
                mapType=MapTypeId(
                    keyType=TypeId(stringType=StringTypeId()),
                    valueType=TypeId(i64Type=I64TypeId()),
                )
            ),
        )

    def test_nested_container(self) -> None:
        type_id = to_type_id(
            ListTypeRef(
                MapTypeRef(
                    PrimitiveTypeRef(Primitive.I32),
                    SetTypeRef(PrimitiveTypeRef(Primitive.BOOL)),
                )
            )
        )
        self.assertEqual(
            type_id,
            TypeId(
                listType=ListTypeId(
                    elementType=TypeId(
                        mapType=MapTypeId(
                            keyType=TypeId(i32Type=I32TypeId()),
                            valueType=TypeId(
                                setType=SetTypeId(
                                    elementType=TypeId(boolType=BoolTypeId())
                                )
                            ),
                        )
                    )
                )
            ),
        )

    def test_user_defined_uses_uri_arm(self) -> None:
        ts = (
            TypeSystemBuilder()
            .add_struct("test/Foo", [_field(1, "x", PrimitiveTypeRef(Primitive.I32))])
            .build()
        )
        foo = ts.get_user_defined_type("test/Foo")
        assert isinstance(foo, StructNode)
        type_id = to_type_id(StructTypeRef(foo))
        self.assertEqual(type_id.type, TypeId.Type.userDefinedType)
        self.assertEqual(type_id.userDefinedType, "test/Foo")

    def test_typeref_id_method_matches_to_type_id(self) -> None:
        type_ref: TypeRef = ListTypeRef(PrimitiveTypeRef(Primitive.I32))
        self.assertEqual(type_ref.id(), to_type_id(type_ref))


class ResolveTypeIdTest(unittest.TestCase):
    def _source(self) -> TypeSystem:
        return (
            TypeSystemBuilder()
            .add_struct("test/Foo", [_field(1, "x", PrimitiveTypeRef(Primitive.I32))])
            .build()
        )

    def test_primitive_round_trip(self) -> None:
        source = self._source()
        for primitive, _ in _PRIMITIVE_TYPE_IDS:
            type_ref: TypeRef = PrimitiveTypeRef(primitive)
            self.assertEqual(resolve_type_id(type_ref.id(), source), type_ref)

    def test_container_round_trip(self) -> None:
        source = self._source()
        for type_ref in (
            ListTypeRef(PrimitiveTypeRef(Primitive.I32)),
            SetTypeRef(PrimitiveTypeRef(Primitive.STRING)),
            MapTypeRef(
                PrimitiveTypeRef(Primitive.STRING), PrimitiveTypeRef(Primitive.BOOL)
            ),
        ):
            self.assertEqual(resolve_type_id(type_ref.id(), source), type_ref)

    def test_user_defined_round_trip(self) -> None:
        source = self._source()
        foo = source.get_user_defined_type("test/Foo")
        assert isinstance(foo, StructNode)
        type_ref: TypeRef = StructTypeRef(foo)
        resolved = resolve_type_id(type_ref.id(), source)
        self.assertEqual(resolved, type_ref)
        assert isinstance(resolved, StructTypeRef)
        self.assertIs(resolved.node, foo)

    def test_unknown_uri_raises(self) -> None:
        source = self._source()
        with self.assertRaises(InvalidTypeError):
            resolve_type_id(TypeId(userDefinedType="test/Missing"), source)

    def test_none_list_element_raises(self) -> None:
        source = self._source()
        with self.assertRaises(InvalidTypeError):
            resolve_type_id(TypeId(listType=ListTypeId(elementType=None)), source)

    def test_none_map_key_raises(self) -> None:
        source = self._source()
        with self.assertRaises(InvalidTypeError):
            resolve_type_id(
                TypeId(
                    mapType=MapTypeId(
                        keyType=None, valueType=TypeId(i32Type=I32TypeId())
                    )
                ),
                source,
            )


class WireRecordConversionTest(unittest.TestCase):
    """``to_wire_record`` / ``from_wire_record`` -- the ``SerializableRecord`` <->
    wire-union pair."""

    def test_record_round_trip(self) -> None:
        for record in (
            BoolRecord(True),
            Int8Record(7),
            Int16Record(-9),
            Int32Record(123456),
            Int64Record(-(2**40)),
            Float32Record(1.5),
            Float64Record(-2.25),
            TextRecord("héllo"),
            ByteArrayRecord(b"\x00\x01\xff"),
            ListRecord([Int32Record(1), Int32Record(2)]),
            SetRecord([TextRecord("a"), TextRecord("b")]),
            MapRecord([(TextRecord("k"), Int32Record(1))]),
            FieldSetRecord({1: Int32Record(7), 2: BoolRecord(False)}),
        ):
            with self.subTest(record=record):
                self.assertEqual(from_wire_record(to_wire_record(record)), record)

    def test_float32_non_exact_round_trips(self) -> None:
        for value in (0.1, 3.14159265358979, -2.7182818284):
            record = Float32Record(value)
            with self.subTest(value=value):
                self.assertEqual(from_wire_record(to_wire_record(record)), record)

    def test_empty_wire_record_raises(self) -> None:
        with self.assertRaises(InvalidTypeError):
            from_wire_record(WireRecord())


class BuildSerializableTest(unittest.TestCase):
    def test_over_builder_type_system(self) -> None:
        builder = TypeSystemBuilder()
        builder.add_struct(
            "test/Bar", [_field(1, "x", PrimitiveTypeRef(Primitive.I32))]
        )
        builder.add_struct(
            "test/Unrelated", [_field(1, "y", PrimitiveTypeRef(Primitive.I32))]
        )
        builder.add_struct(
            "test/Foo",
            [
                _field(1, "bar", "test/Bar"),
                _field(2, "name", PrimitiveTypeRef(Primitive.STRING)),
            ],
        )
        source = builder.build()

        sts = build_serializable_type_system(source, ["test/Foo"])
        # Closure = {Foo, Bar}; Unrelated is excluded.
        self.assertEqual(set(sts.types.keys()), {"test/Foo", "test/Bar"})
        foo_entry = sts.types["test/Foo"]
        self.assertEqual(
            foo_entry.definition.type, SerializableTypeDefinition.Type.structDef
        )
        self.assertEqual(len(foo_entry.definition.structDef.fields), 2)

    def test_over_registry_pruned_closure(self) -> None:
        SchemaRegistry._reset()
        registry = SchemaRegistry()
        outer = f"{_BRIDGE_URI}/Outer"
        inner = f"{_BRIDGE_URI}/Inner"
        sts = build_serializable_type_system(registry, [outer])
        # `Outer` references `Inner` (directly, via typedef, and in a list).
        self.assertEqual(set(sts.types.keys()), {outer, inner})

    def test_root_absent_raises(self) -> None:
        source = (
            TypeSystemBuilder()
            .add_struct("test/Foo", [_field(1, "x", PrimitiveTypeRef(Primitive.I32))])
            .build()
        )
        with self.assertRaises(InvalidTypeError):
            build_serializable_type_system(source, ["test/Missing"])

    def test_to_serializable_definition_enum(self) -> None:
        builder = TypeSystemBuilder()
        builder.add_enum("test/Color", [EnumValue("RED", 0), EnumValue("GREEN", 1)])
        ts = builder.build()
        color = ts.get_user_defined_type("test/Color")
        assert isinstance(color, EnumNode)
        defn = to_serializable_definition(color)
        self.assertEqual(defn.type, SerializableTypeDefinition.Type.enumDef)
        self.assertEqual(
            [(v.name, v.datum) for v in defn.enumDef.values],
            [("RED", 0), ("GREEN", 1)],
        )


class FromSerializableRoundTripTest(unittest.TestCase):
    def test_round_trip_builder_type_system(self) -> None:
        builder = TypeSystemBuilder()
        builder.add_struct(
            "test/Bar", [_field(1, "x", PrimitiveTypeRef(Primitive.I32))]
        )
        builder.add_enum("test/Color", [EnumValue("RED", 0), EnumValue("GREEN", 1)])
        builder.add_union(
            "test/U",
            [
                _field(
                    1, "a", PrimitiveTypeRef(Primitive.I32), PresenceQualifier.OPTIONAL
                )
            ],
        )
        builder.add_struct(
            "test/Foo",
            [
                _field(1, "bar", "test/Bar"),
                _field(2, "color", "test/Color"),
                _field(3, "u", "test/U", PresenceQualifier.OPTIONAL),
                _field(
                    4, "terse", PrimitiveTypeRef(Primitive.I32), PresenceQualifier.TERSE
                ),
                _field(5, "items", ListTypeRef(ref("test/Bar"))),
            ],
        )
        source = builder.build()
        roots = ["test/Foo"]

        sts = build_serializable_type_system(source, roots)
        rebuilt = from_serializable(sts)

        _assert_type_systems_equal(self, source, rebuilt)
        # Re-exporting the rebuilt type system reproduces the wire form exactly.
        self.assertEqual(build_serializable_type_system(rebuilt, roots), sts)

    def test_round_trip_preserves_presence(self) -> None:
        builder = TypeSystemBuilder()
        builder.add_struct(
            "test/Foo",
            [
                _field(1, "plain", PrimitiveTypeRef(Primitive.I32)),
                _field(
                    2,
                    "opt",
                    PrimitiveTypeRef(Primitive.I32),
                    PresenceQualifier.OPTIONAL,
                ),
                _field(
                    3, "terse", PrimitiveTypeRef(Primitive.I32), PresenceQualifier.TERSE
                ),
            ],
        )
        source = builder.build()
        rebuilt = from_serializable(
            build_serializable_type_system(source, ["test/Foo"])
        )
        foo = rebuilt.get_user_defined_type("test/Foo")
        assert isinstance(foo, StructNode)
        self.assertEqual(
            [(f.identity.name, f.presence) for f in foo.fields],
            [
                ("plain", PresenceQualifier.UNQUALIFIED),
                ("opt", PresenceQualifier.OPTIONAL),
                ("terse", PresenceQualifier.TERSE),
            ],
        )

    def test_round_trip_opaque_alias(self) -> None:
        builder = TypeSystemBuilder()
        builder.add_opaque_alias("test/MyId", PrimitiveTypeRef(Primitive.I64))
        source = builder.build()
        rebuilt = from_serializable(
            build_serializable_type_system(source, ["test/MyId"])
        )
        alias = rebuilt.get_user_defined_type("test/MyId")
        assert isinstance(alias, OpaqueAliasNode)
        self.assertEqual(alias.target_type, PrimitiveTypeRef(Primitive.I64))

    def test_round_trip_cycle(self) -> None:
        builder = TypeSystemBuilder()
        builder.add_struct(
            "test/Node",
            [_field(1, "next", "test/Node", PresenceQualifier.OPTIONAL)],
        )
        source = builder.build()
        rebuilt = from_serializable(
            build_serializable_type_system(source, ["test/Node"])
        )
        node = rebuilt.get_user_defined_type("test/Node")
        assert isinstance(node, StructNode)
        next_field = node.field_by_name("next")
        assert next_field is not None
        edge = next_field.type
        assert isinstance(edge, StructTypeRef)
        # Self-reference rewires to the rebuilt node (stable identity).
        self.assertIs(edge.node, node)

    def test_round_trip_enum_value_annotations(self) -> None:
        # Wire form: an annotation struct + an enum whose RED value carries a
        # (non-standard) annotation; GREEN carries none.
        sts = SerializableTypeSystem(
            types={
                "test/Anno": _wire_struct([_wire_field(1, "count", _I32_ID)]),
                "test/Color": SerializableTypeDefinitionEntry(
                    definition=SerializableTypeDefinition(
                        enumDef=SerializableEnumDefinition(
                            values=[
                                SerializableEnumValueDefinition(
                                    name="RED",
                                    datum=0,
                                    annotations={
                                        "test/Anno": WireRecord(
                                            fieldSetDatum={1: WireRecord(int32Datum=7)}
                                        )
                                    },
                                ),
                                SerializableEnumValueDefinition(
                                    name="GREEN", datum=1, annotations={}
                                ),
                            ],
                            annotations={},
                        )
                    )
                ),
            }
        )
        ts = from_serializable(sts)
        color = ts.get_user_defined_type("test/Color")
        assert isinstance(color, EnumNode)
        red, green = color.values
        self.assertEqual(red.name, "RED")
        self.assertEqual(
            dict(red.annotations),
            {"test/Anno": FieldSetRecord({1: Int32Record(7)})},
        )
        self.assertEqual(dict(green.annotations), {})
        # Re-export reproduces the wire form exactly (round-trip identity).
        self.assertEqual(build_serializable_type_system(ts, ["test/Color"]), sts)


def _wire_struct(
    fields: list[SerializableFieldDefinition],
    *,
    is_sealed: bool = False,
    annotations: dict[str, WireRecord] | None = None,
) -> SerializableTypeDefinitionEntry:
    return SerializableTypeDefinitionEntry(
        definition=SerializableTypeDefinition(
            structDef=SerializableStructDefinition(
                fields=fields, isSealed=is_sealed, annotations=annotations or {}
            )
        )
    )


def _wire_field(
    field_id: int,
    name: str,
    type_id: TypeId,
    presence: WirePresence = WirePresence.UNQUALIFIED,
) -> SerializableFieldDefinition:
    return SerializableFieldDefinition(
        identity=WireFieldIdentity(id=field_id, name=name),
        presence=presence,
        type=type_id,
    )


_I32_ID = TypeId(i32Type=I32TypeId())


class FromSerializableValidatorTest(unittest.TestCase):
    def test_duplicate_field_id(self) -> None:
        sts = SerializableTypeSystem(
            types={
                "test/Foo": _wire_struct(
                    [_wire_field(1, "a", _I32_ID), _wire_field(1, "b", _I32_ID)]
                )
            }
        )
        with self.assertRaises(InvalidTypeError):
            from_serializable(sts)

    def test_duplicate_field_name(self) -> None:
        sts = SerializableTypeSystem(
            types={
                "test/Foo": _wire_struct(
                    [_wire_field(1, "a", _I32_ID), _wire_field(2, "a", _I32_ID)]
                )
            }
        )
        with self.assertRaises(InvalidTypeError):
            from_serializable(sts)

    def test_non_optional_union_field(self) -> None:
        sts = SerializableTypeSystem(
            types={
                "test/U": SerializableTypeDefinitionEntry(
                    definition=SerializableTypeDefinition(
                        unionDef=SerializableUnionDefinition(
                            fields=[
                                _wire_field(1, "a", _I32_ID, WirePresence.UNQUALIFIED)
                            ],
                            isSealed=False,
                            annotations={},
                        )
                    )
                )
            }
        )
        with self.assertRaises(InvalidTypeError):
            from_serializable(sts)

    def test_duplicate_enum_name(self) -> None:
        sts = SerializableTypeSystem(
            types={
                "test/E": SerializableTypeDefinitionEntry(
                    definition=SerializableTypeDefinition(
                        enumDef=SerializableEnumDefinition(
                            values=[
                                SerializableEnumValueDefinition(
                                    name="A", datum=0, annotations={}
                                ),
                                SerializableEnumValueDefinition(
                                    name="A", datum=1, annotations={}
                                ),
                            ],
                            annotations={},
                        )
                    )
                )
            }
        )
        with self.assertRaises(InvalidTypeError):
            from_serializable(sts)

    def test_duplicate_enum_datum(self) -> None:
        sts = SerializableTypeSystem(
            types={
                "test/E": SerializableTypeDefinitionEntry(
                    definition=SerializableTypeDefinition(
                        enumDef=SerializableEnumDefinition(
                            values=[
                                SerializableEnumValueDefinition(
                                    name="A", datum=0, annotations={}
                                ),
                                SerializableEnumValueDefinition(
                                    name="B", datum=0, annotations={}
                                ),
                            ],
                            annotations={},
                        )
                    )
                )
            }
        )
        with self.assertRaises(InvalidTypeError):
            from_serializable(sts)

    def test_opaque_alias_user_defined_target(self) -> None:
        sts = SerializableTypeSystem(
            types={
                "test/Foo": _wire_struct([_wire_field(1, "a", _I32_ID)]),
                "test/Alias": SerializableTypeDefinitionEntry(
                    definition=SerializableTypeDefinition(
                        opaqueAliasDef=SerializableOpaqueAliasDefinition(
                            targetType=TypeId(userDefinedType="test/Foo"),
                            annotations={},
                        )
                    )
                ),
            }
        )
        with self.assertRaises(InvalidTypeError):
            from_serializable(sts)

    def test_unknown_uri_in_field(self) -> None:
        sts = SerializableTypeSystem(
            types={
                "test/Foo": _wire_struct(
                    [_wire_field(1, "missing", TypeId(userDefinedType="test/Nope"))]
                )
            }
        )
        with self.assertRaises(InvalidTypeError):
            from_serializable(sts)


class AnnotationMetadataTest(unittest.TestCase):
    def _anno_struct(self) -> SerializableTypeDefinitionEntry:
        # An annotation type: a struct with a single i32 `count` field.
        return _wire_struct([_wire_field(1, "count", _I32_ID)])

    def test_annotation_uri_must_resolve_to_struct(self) -> None:
        # The annotation URI points at an *enum*, not a struct -> reject.
        sts = SerializableTypeSystem(
            types={
                "test/E": SerializableTypeDefinitionEntry(
                    definition=SerializableTypeDefinition(
                        enumDef=SerializableEnumDefinition(
                            values=[
                                SerializableEnumValueDefinition(
                                    name="A", datum=0, annotations={}
                                )
                            ],
                            annotations={},
                        )
                    )
                ),
                "test/Foo": _wire_struct(
                    [_wire_field(1, "a", _I32_ID)],
                    annotations={
                        "test/E": WireRecord(
                            fieldSetDatum={1: WireRecord(int32Datum=7)}
                        )
                    },
                ),
            }
        )
        with self.assertRaises(InvalidTypeError):
            from_serializable(sts)

    def test_annotation_value_must_be_field_set(self) -> None:
        # The annotation value is a scalar, not a field-set record -> reject.
        sts = SerializableTypeSystem(
            types={
                "test/Anno": self._anno_struct(),
                "test/Foo": _wire_struct(
                    [_wire_field(1, "a", _I32_ID)],
                    annotations={"test/Anno": WireRecord(int32Datum=7)},
                ),
            }
        )
        with self.assertRaises(InvalidTypeError):
            from_serializable(sts)

    def test_valid_annotation_accepted(self) -> None:
        sts = SerializableTypeSystem(
            types={
                "test/Anno": self._anno_struct(),
                "test/Foo": _wire_struct(
                    [_wire_field(1, "a", _I32_ID)],
                    annotations={
                        "test/Anno": WireRecord(
                            fieldSetDatum={1: WireRecord(int32Datum=7)}
                        )
                    },
                ),
            }
        )
        ts = from_serializable(sts)
        foo = ts.get_user_defined_type("test/Foo")
        assert isinstance(foo, StructNode)
        self.assertEqual(
            dict(foo.annotations),
            {"test/Anno": FieldSetRecord({1: Int32Record(7)})},
        )

    def test_standard_annotations_dropped_on_export(self) -> None:
        # Build a hand-rolled type system whose `Foo` carries a standard
        # annotation (dropped on export) and a non-standard one (retained).
        anno = StructNode(
            uri="test/Anno",
            fields=[
                FieldDefinition(
                    identity=FieldIdentity(1, "count"),
                    presence=PresenceQualifier.UNQUALIFIED,
                    type=PrimitiveTypeRef(Primitive.I32),
                )
            ],
        )
        foo = StructNode(
            uri="test/Foo",
            fields=[
                FieldDefinition(
                    identity=FieldIdentity(1, "a"),
                    presence=PresenceQualifier.UNQUALIFIED,
                    type=PrimitiveTypeRef(Primitive.I32),
                )
            ],
            annotations={
                "test/Anno": FieldSetRecord({1: Int32Record(7)}),
                "facebook.com/thrift/annotation/Cpp": FieldSetRecord({}),
            },
        )
        from thrift.lib.python.schema.type_system import IndexedTypeSystem

        source = IndexedTypeSystem({"test/Anno": anno, "test/Foo": foo})
        sts = build_serializable_type_system(source, ["test/Foo"])
        foo_anns = sts.types["test/Foo"].definition.structDef.annotations
        self.assertIn("test/Anno", foo_anns)
        self.assertNotIn("facebook.com/thrift/annotation/Cpp", foo_anns)

    def test_enum_value_annotations_exported_and_pull_closure(self) -> None:
        # An enum whose RED value carries a non-standard annotation (retained,
        # and its backing struct is pulled into the closure) and a standard one
        # (dropped); GREEN carries none.
        from thrift.lib.python.schema.type_system import IndexedTypeSystem

        anno = StructNode(
            uri="test/Anno",
            fields=[
                FieldDefinition(
                    identity=FieldIdentity(1, "count"),
                    presence=PresenceQualifier.UNQUALIFIED,
                    type=PrimitiveTypeRef(Primitive.I32),
                )
            ],
        )
        color = EnumNode(
            uri="test/Color",
            values=[
                EnumValue(
                    "RED",
                    0,
                    annotations={
                        "test/Anno": FieldSetRecord({1: Int32Record(7)}),
                        "facebook.com/thrift/annotation/Cpp": FieldSetRecord({}),
                    },
                ),
                EnumValue("GREEN", 1),
            ],
        )
        source = IndexedTypeSystem({"test/Anno": anno, "test/Color": color})
        sts = build_serializable_type_system(source, ["test/Color"])
        # The value-annotation's backing struct is pulled in: self-contained.
        self.assertEqual(set(sts.types.keys()), {"test/Color", "test/Anno"})
        values = sts.types["test/Color"].definition.enumDef.values
        self.assertIn("test/Anno", values[0].annotations)
        self.assertNotIn("facebook.com/thrift/annotation/Cpp", values[0].annotations)
        self.assertEqual(dict(values[1].annotations), {})

    def test_enum_value_annotation_uri_must_resolve_to_struct(self) -> None:
        # The enum-value annotation URI points at an enum, not a struct -> reject
        # (enum-value annotations go through the same metadata validation).
        sts = SerializableTypeSystem(
            types={
                "test/Color": SerializableTypeDefinitionEntry(
                    definition=SerializableTypeDefinition(
                        enumDef=SerializableEnumDefinition(
                            values=[
                                SerializableEnumValueDefinition(
                                    name="RED",
                                    datum=0,
                                    annotations={
                                        "test/Color": WireRecord(fieldSetDatum={})
                                    },
                                )
                            ],
                            annotations={},
                        )
                    )
                )
            }
        )
        with self.assertRaises(InvalidTypeError):
            from_serializable(sts)


class BuildDerivedFromTest(unittest.TestCase):
    def _base(self) -> TypeSystem:
        return (
            TypeSystemBuilder()
            .add_struct("test/Base", [_field(1, "x", PrimitiveTypeRef(Primitive.I32))])
            .build()
        )

    def test_overlay_union_with_base(self) -> None:
        base = self._base()
        overlay = TypeSystemBuilder()
        overlay.add_struct("test/Overlay", [_field(1, "base", "test/Base")])
        result = build_derived_from(overlay, base)

        self.assertIsNotNone(result.get_user_defined_type("test/Overlay"))
        self.assertIsNotNone(result.get_user_defined_type("test/Base"))
        known = result.get_known_uris()
        self.assertEqual(known, frozenset({"test/Overlay", "test/Base"}))

    def test_overlay_field_resolves_to_base_node(self) -> None:
        base = self._base()
        base_node = base.get_user_defined_type("test/Base")
        overlay = TypeSystemBuilder()
        overlay.add_struct("test/Overlay", [_field(1, "base", "test/Base")])
        result = build_derived_from(overlay, base)

        overlay_node = result.get_user_defined_type("test/Overlay")
        assert isinstance(overlay_node, StructNode)
        base_field = overlay_node.field_by_name("base")
        assert base_field is not None
        edge = base_field.type
        assert isinstance(edge, StructTypeRef)
        self.assertIs(edge.node, base_node)

    def test_delegates_to_base_not_a_copy(self) -> None:
        base = self._base()
        overlay = TypeSystemBuilder()
        overlay.add_struct(
            "test/Overlay", [_field(1, "x", PrimitiveTypeRef(Primitive.I32))]
        )
        result = build_derived_from(overlay, base)
        # The base type is delegated to, not copied.
        self.assertIs(
            result.get_user_defined_type("test/Base"),
            base.get_user_defined_type("test/Base"),
        )

    def test_uri_collision_throws(self) -> None:
        base = self._base()
        overlay = TypeSystemBuilder()
        overlay.add_struct(
            "test/Base", [_field(1, "y", PrimitiveTypeRef(Primitive.I32))]
        )
        with self.assertRaises(InvalidTypeError):
            build_derived_from(overlay, base)


class SourceInfoSerializationTest(unittest.TestCase):
    """``sourceInfo`` emission on export, the import read-back, and preservation
    through the ``build_pruned`` deep-copy."""

    _FOO_INFO: SourceInfo = SourceInfo("file://test/foo.thrift", "Foo")

    def _source(self) -> IndexedTypeSystem:
        # `Foo` carries source_info; `Bare` (an enum) carries none.
        foo = StructNode(
            uri="test/Foo",
            fields=[
                FieldDefinition(
                    identity=FieldIdentity(1, "x"),
                    presence=PresenceQualifier.UNQUALIFIED,
                    type=PrimitiveTypeRef(Primitive.I32),
                )
            ],
            source_info=self._FOO_INFO,
        )
        bare = EnumNode(uri="test/Bare", values=[EnumValue("A", 0)])
        return IndexedTypeSystem({"test/Foo": foo, "test/Bare": bare})

    def test_export_emits_source_info_when_set(self) -> None:
        sts = build_serializable_type_system(self._source(), ["test/Foo", "test/Bare"])
        foo_info = sts.types["test/Foo"].sourceInfo
        assert foo_info is not None, "expected sourceInfo on export of a sourced node"
        self.assertEqual(foo_info.locator, "file://test/foo.thrift")
        self.assertEqual(foo_info.name, "Foo")
        # A node without source_info emits sourceInfo == None.
        self.assertIsNone(sts.types["test/Bare"].sourceInfo)

    def test_export_omits_source_info_when_disabled(self) -> None:
        sts = build_serializable_type_system(
            self._source(),
            ["test/Foo"],
            options=PruneOptions(include_source_info=False),
        )
        self.assertIsNone(sts.types["test/Foo"].sourceInfo)

    def test_round_trip_preserves_source_info(self) -> None:
        source = self._source()
        roots = ["test/Foo", "test/Bare"]
        sts = build_serializable_type_system(source, roots)
        rebuilt = from_serializable(sts)

        foo = rebuilt.get_user_defined_type("test/Foo")
        assert foo is not None
        self.assertEqual(foo.source_info, self._FOO_INFO)
        bare = rebuilt.get_user_defined_type("test/Bare")
        assert bare is not None
        self.assertIsNone(bare.source_info)
        # Re-export reproduces the wire form exactly.
        self.assertEqual(build_serializable_type_system(rebuilt, roots), sts)

    def test_build_pruned_preserves_source_info_by_default(self) -> None:
        pruned = build_pruned(self._source(), ["test/Foo"])
        foo = pruned.get_user_defined_type("test/Foo")
        assert foo is not None
        self.assertEqual(foo.source_info, self._FOO_INFO)

    def test_build_pruned_drops_source_info_when_disabled(self) -> None:
        pruned = build_pruned(
            self._source(),
            ["test/Foo"],
            PruneOptions(include_source_info=False),
        )
        foo = pruned.get_user_defined_type("test/Foo")
        assert foo is not None
        self.assertIsNone(foo.source_info)


if __name__ == "__main__":
    unittest.main()
