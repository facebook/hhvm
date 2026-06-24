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

"""
The canonical ``SerializableTypeSystem`` wire boundary -- the export half of the
TypeSystem layer.

It materializes the runtime model (``type_system.py`` / ``_record.py``) into the
generated thrift-python wire types for ``type_id.thrift`` / ``record.thrift`` /
``type_system.thrift``:

* ``to_type_id`` / ``resolve_type_id`` -- the ``TypeRef`` <-> ``TypeId`` pair;
* ``to_wire_record`` / ``from_wire_record`` -- the ``SerializableRecord`` <-> wire
  union pair (re-validates NaN / ``-0.0`` / UTF-8 / dup on import);
* ``to_serializable_definition`` -- a single ``DefinitionNode`` -> wire
  ``SerializableTypeDefinition``;
* ``build_serializable_type_system`` -- a root-pruned export of a whole
  ``TypeSystem`` to a ``SerializableTypeSystem``, walking the type-structure
  closure **plus non-standard annotation dependencies** and **dropping standard
  ``facebook.com/thrift/annotation/*`` annotations**.

The import half (``from_serializable``) and overlay composition
(``build_derived_from``) live in ``type_system_builder.py`` and consume
``resolve_type_id`` / ``from_wire_record`` / ``annotations_from_wire`` from here.
This module never imports ``type_system_builder`` (one-way dependency).
"""

from __future__ import annotations

from collections.abc import Callable, Iterator, Mapping, Sequence
from typing import assert_never

from apache.thrift.type_system.record.thrift_types import (
    SerializableRecord as WireRecord,
    SerializableRecordMapEntry,
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
    PresenceQualifier as WirePresenceQualifier,
    SerializableEnumDefinition,
    SerializableEnumValueDefinition,
    SerializableFieldDefinition,
    SerializableOpaqueAliasDefinition,
    SerializableStructDefinition,
    SerializableThriftSourceInfo,
    SerializableTypeDefinition,
    SerializableTypeDefinitionEntry,
    SerializableTypeSystem,
    SerializableUnionDefinition,
)
from folly.iobuf import IOBuf
from thrift.lib.python.schema._digest_common import is_standard_annotation
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
    SerializableRecord,
    SetRecord,
    TextRecord,
)
from thrift.lib.python.schema.type_system import (
    _collect_closure,
    _index_by_source,
    _type_ref_for_node,
    _type_ref_uris,
    DefinitionNode,
    EnumNode,
    EnumTypeRef,
    FieldDefinition,
    InvalidTypeError,
    ListTypeRef,
    MapTypeRef,
    OpaqueAliasNode,
    OpaqueAliasTypeRef,
    Primitive,
    PrimitiveTypeRef,
    SetTypeRef,
    StructNode,
    StructTypeRef,
    TypeRef,
    TypeRefBase,
    TypeSystem,
    UnionNode,
    UnionTypeRef,
)


# ---------------------------------------------------------------------------
# TypeRef <-> TypeId
# ---------------------------------------------------------------------------

# Each primitive's empty-struct ``TypeId`` arm, built fresh per call.
_PRIMITIVE_TYPE_ID_FACTORY: dict[Primitive, Callable[[], TypeId]] = {
    Primitive.BOOL: lambda: TypeId(boolType=BoolTypeId()),
    Primitive.BYTE: lambda: TypeId(byteType=ByteTypeId()),
    Primitive.I16: lambda: TypeId(i16Type=I16TypeId()),
    Primitive.I32: lambda: TypeId(i32Type=I32TypeId()),
    Primitive.I64: lambda: TypeId(i64Type=I64TypeId()),
    Primitive.FLOAT: lambda: TypeId(floatType=FloatTypeId()),
    Primitive.DOUBLE: lambda: TypeId(doubleType=DoubleTypeId()),
    Primitive.STRING: lambda: TypeId(stringType=StringTypeId()),
    Primitive.BINARY: lambda: TypeId(binaryType=BinaryTypeId()),
    Primitive.ANY: lambda: TypeId(anyType=AnyTypeId()),
}

# The inverse: a ``TypeId`` primitive arm -> ``Primitive``.
_ARM_TO_PRIMITIVE: dict[TypeId.Type, Primitive] = {
    TypeId.Type.boolType: Primitive.BOOL,
    TypeId.Type.byteType: Primitive.BYTE,
    TypeId.Type.i16Type: Primitive.I16,
    TypeId.Type.i32Type: Primitive.I32,
    TypeId.Type.i64Type: Primitive.I64,
    TypeId.Type.floatType: Primitive.FLOAT,
    TypeId.Type.doubleType: Primitive.DOUBLE,
    TypeId.Type.stringType: Primitive.STRING,
    TypeId.Type.binaryType: Primitive.BINARY,
    TypeId.Type.anyType: Primitive.ANY,
}


def to_type_id(type_ref: TypeRefBase) -> TypeId:
    """Convert a resolved ``TypeRef`` to its serializable ``TypeId``.

    All four user-defined kinds collapse to the ``userDefinedType`` (URI) arm --
    the kind is recovered only by resolving the URI in a type system.

    Raises ``InvalidTypeError`` for an unresolved/unknown ``TypeRef`` (e.g. a
    builder-input ``UnresolvedTypeRef``), which has no wire ``TypeId``."""
    if not isinstance(type_ref, TypeRef):
        raise InvalidTypeError(
            f"{type(type_ref).__name__} is not a resolved type edge (TypeRef)."
        )
    match type_ref:
        case PrimitiveTypeRef():
            return _PRIMITIVE_TYPE_ID_FACTORY[type_ref.primitive]()
        case ListTypeRef():
            return TypeId(
                listType=ListTypeId(elementType=to_type_id(type_ref.element_type))
            )
        case SetTypeRef():
            return TypeId(
                setType=SetTypeId(elementType=to_type_id(type_ref.element_type))
            )
        case MapTypeRef():
            return TypeId(
                mapType=MapTypeId(
                    keyType=to_type_id(type_ref.key_type),
                    valueType=to_type_id(type_ref.value_type),
                )
            )
        case StructTypeRef() | UnionTypeRef() | EnumTypeRef() | OpaqueAliasTypeRef():
            return TypeId(userDefinedType=type_ref.node.uri)
        case _:
            assert_never(type_ref)


def resolve_type_id_with_lookup(
    type_id: TypeId, resolve_uri: Callable[[str], DefinitionNode | None]
) -> TypeRef:
    """Resolve a ``TypeId`` to a ``TypeRef``, resolving user-defined URIs via
    ``resolve_uri``.

    A missing user-defined URI, or an absent (``None``) boxed container child,
    raises ``InvalidTypeError``."""
    kind = type_id.type
    primitive = _ARM_TO_PRIMITIVE.get(kind)
    if primitive is not None:
        return PrimitiveTypeRef(primitive)
    arm = TypeId.Type
    if kind == arm.userDefinedType:
        uri = type_id.userDefinedType
        node = resolve_uri(uri)
        if node is None:
            raise InvalidTypeError(
                f"Unresolvable TypeId: user-defined URI {uri!r} is not defined"
            )
        return _type_ref_for_node(node)
    if kind == arm.listType:
        element = type_id.listType.elementType
        if element is None:
            raise InvalidTypeError("Empty TypeId: list element type is missing")
        return ListTypeRef(resolve_type_id_with_lookup(element, resolve_uri))
    if kind == arm.setType:
        element = type_id.setType.elementType
        if element is None:
            raise InvalidTypeError("Empty TypeId: set element type is missing")
        return SetTypeRef(resolve_type_id_with_lookup(element, resolve_uri))
    if kind == arm.mapType:
        key = type_id.mapType.keyType
        value = type_id.mapType.valueType
        if key is None or value is None:
            raise InvalidTypeError("Empty TypeId: map key/value type is missing")
        return MapTypeRef(
            resolve_type_id_with_lookup(key, resolve_uri),
            resolve_type_id_with_lookup(value, resolve_uri),
        )
    raise InvalidTypeError(f"Unknown TypeId arm: {kind!r}")


def resolve_type_id(type_id: TypeId, source: TypeSystem) -> TypeRef:
    """Resolve a ``TypeId`` to a ``TypeRef``, resolving user-defined URIs against
    ``source``. The inverse of :func:`to_type_id`."""
    return resolve_type_id_with_lookup(type_id, source.get_user_defined_type)


# ---------------------------------------------------------------------------
# SerializableRecord <-> wire union
# ---------------------------------------------------------------------------


def _iobuf_to_bytes(buf: object) -> bytes:
    """Flatten a ``byteArrayDatum`` (a ``folly.iobuf.IOBuf`` in thrift-python)
    to ``bytes``; tolerate a plain ``bytes`` too."""
    if isinstance(buf, (bytes, bytearray, memoryview)):
        return bytes(buf)
    return b"".join(buf)  # IOBuf iterates its (memoryview) chunks


def to_wire_record(record: SerializableRecord) -> WireRecord:
    """Convert a native ``SerializableRecord`` to its wire union form."""
    match record:
        case BoolRecord():
            return WireRecord(boolDatum=record.value)
        case Int8Record():
            return WireRecord(int8Datum=record.value)
        case Int16Record():
            return WireRecord(int16Datum=record.value)
        case Int32Record():
            return WireRecord(int32Datum=record.value)
        case Int64Record():
            return WireRecord(int64Datum=record.value)
        case Float32Record():
            return WireRecord(float32Datum=record.value)
        case Float64Record():
            return WireRecord(float64Datum=record.value)
        case TextRecord():
            return WireRecord(textDatum=record.value)
        case ByteArrayRecord():
            return WireRecord(byteArrayDatum=IOBuf(record.value))
        case FieldSetRecord():
            return WireRecord(
                fieldSetDatum={
                    fid: to_wire_record(value) for fid, value in record.fields.items()
                }
            )
        case ListRecord():
            return WireRecord(listDatum=[to_wire_record(e) for e in record.elements])
        case SetRecord():
            return WireRecord(setDatum=[to_wire_record(e) for e in record.elements])
        case MapRecord():
            return WireRecord(
                mapDatum=[
                    SerializableRecordMapEntry(
                        key=to_wire_record(key), value=to_wire_record(value)
                    )
                    for key, value in record.entries
                ]
            )
        case _:
            assert_never(record)


def from_wire_record(wire: WireRecord) -> SerializableRecord:
    """Convert a wire ``SerializableRecord`` union to the native record."""
    arm = WireRecord.Type
    kind = wire.type
    if kind == arm.boolDatum:
        return BoolRecord(wire.boolDatum)
    if kind == arm.int8Datum:
        return Int8Record(wire.int8Datum)
    if kind == arm.int16Datum:
        return Int16Record(wire.int16Datum)
    if kind == arm.int32Datum:
        return Int32Record(wire.int32Datum)
    if kind == arm.int64Datum:
        return Int64Record(wire.int64Datum)
    if kind == arm.float32Datum:
        return Float32Record(wire.float32Datum)
    if kind == arm.float64Datum:
        return Float64Record(wire.float64Datum)
    if kind == arm.textDatum:
        return TextRecord(wire.textDatum)
    if kind == arm.byteArrayDatum:
        return ByteArrayRecord(_iobuf_to_bytes(wire.byteArrayDatum))
    if kind == arm.fieldSetDatum:
        return FieldSetRecord(
            {fid: from_wire_record(value) for fid, value in wire.fieldSetDatum.items()}
        )
    if kind == arm.listDatum:
        return ListRecord([from_wire_record(e) for e in wire.listDatum])
    if kind == arm.setDatum:
        return SetRecord([from_wire_record(e) for e in wire.setDatum])
    if kind == arm.mapDatum:
        return MapRecord(
            [
                (from_wire_record(e.key), from_wire_record(e.value))
                for e in wire.mapDatum
            ]
        )
    raise InvalidTypeError(f"Cannot convert wire record (kind={kind!r}) to a record")


# ---------------------------------------------------------------------------
# Annotation conversion (with the standard-annotation drop on export and the
# struct/field-set metadata validation on import).
# ---------------------------------------------------------------------------


def to_wire_annotations(
    annotations: Mapping[str, SerializableRecord],
) -> dict[str, WireRecord]:
    """Convert a node/field annotation map to the wire form, dropping standard
    ``facebook.com/thrift/annotation/*`` annotations."""
    return {
        uri: to_wire_record(record)
        for uri, record in annotations.items()
        if not is_standard_annotation(uri)
    }


def annotations_from_wire(
    wire_annotations: Mapping[str, WireRecord],
    resolve_uri: Callable[[str], DefinitionNode | None],
) -> dict[str, SerializableRecord]:
    """Convert a wire annotation map to native records: each annotation URI must
    resolve to a **struct**, and each value must be a **field-set** record."""
    result: dict[str, SerializableRecord] = {}
    for uri, wire_record in wire_annotations.items():
        match resolve_uri(uri):
            case StructNode():
                pass  # the only valid annotation target
            case None:
                raise InvalidTypeError(
                    f"Annotation URI {uri!r} is not defined in this type system"
                )
            case _:
                raise InvalidTypeError(
                    f"Annotation URI {uri!r} does not resolve to a struct"
                )
        match from_wire_record(wire_record):
            case FieldSetRecord() as record:
                result[uri] = record
            case _:
                raise InvalidTypeError(
                    f"Annotation value for {uri!r} is not a field-set record"
                )
    return result


# ---------------------------------------------------------------------------
# Definition export
# ---------------------------------------------------------------------------


def _to_wire_field(field: FieldDefinition) -> SerializableFieldDefinition:
    return SerializableFieldDefinition(
        identity=WireFieldIdentity(id=field.identity.id, name=field.identity.name),
        presence=WirePresenceQualifier(field.presence.value),
        type=to_type_id(field.type),
        customDefaultPartialRecord=(
            to_wire_record(field.custom_default)
            if field.custom_default is not None
            else None
        ),
        annotations=to_wire_annotations(field.annotations),
    )


def to_serializable_definition(node: DefinitionNode) -> SerializableTypeDefinition:
    """Convert a single ``DefinitionNode`` to its wire ``SerializableTypeDefinition``.

    Standard annotations are dropped at every level (node, field, and enum
    value); records are converted to the wire form."""
    match node:
        case StructNode():
            return SerializableTypeDefinition(
                structDef=SerializableStructDefinition(
                    fields=[_to_wire_field(f) for f in node.fields],
                    isSealed=node.is_sealed,
                    annotations=to_wire_annotations(node.annotations),
                )
            )
        case UnionNode():
            return SerializableTypeDefinition(
                unionDef=SerializableUnionDefinition(
                    fields=[_to_wire_field(f) for f in node.fields],
                    isSealed=node.is_sealed,
                    annotations=to_wire_annotations(node.annotations),
                )
            )
        case EnumNode():
            return SerializableTypeDefinition(
                enumDef=SerializableEnumDefinition(
                    values=[
                        SerializableEnumValueDefinition(
                            name=v.name,
                            datum=v.datum,
                            annotations=to_wire_annotations(v.annotations),
                        )
                        for v in node.values
                    ],
                    annotations=to_wire_annotations(node.annotations),
                )
            )
        case OpaqueAliasNode():
            return SerializableTypeDefinition(
                opaqueAliasDef=SerializableOpaqueAliasDefinition(
                    targetType=to_type_id(node.target_type),
                    annotations=to_wire_annotations(node.annotations),
                )
            )
        case _:
            assert_never(node)


# ---------------------------------------------------------------------------
# Export closure (type structure + non-standard annotation dependencies).
# ---------------------------------------------------------------------------


def _non_standard_annotation_uris(
    annotations: Mapping[str, SerializableRecord],
) -> Iterator[str]:
    for uri in annotations:
        if not is_standard_annotation(uri):
            yield uri


def _export_referenced_uris(node: DefinitionNode) -> Iterator[str]:
    """The closure edges for export: field/element/key/value types,
    opaque-alias targets, and **non-standard** annotation types (the annotation's
    backing struct) at every level -- node, field, and enum value. Standard
    annotation types are skipped."""
    yield from _non_standard_annotation_uris(node.annotations)
    match node:
        case StructNode() | UnionNode():
            for field in node.fields:
                yield from _type_ref_uris(field.type)
                yield from _non_standard_annotation_uris(field.annotations)
        case EnumNode():
            for value in node.values:
                yield from _non_standard_annotation_uris(value.annotations)
        case OpaqueAliasNode():
            yield from _type_ref_uris(node.target_type)
        case _:
            assert_never(node)


class PruneOptions:
    """Options for :func:`build_pruned` and :func:`build_serializable_type_system`."""

    __slots__ = ("_include_source_info",)
    _include_source_info: bool

    def __init__(self, include_source_info: bool = True) -> None:
        self._include_source_info = include_source_info

    @property
    def include_source_info(self) -> bool:
        return self._include_source_info

    def __repr__(self) -> str:
        return f"PruneOptions(include_source_info={self._include_source_info})"


def _to_wire_source_info(node: DefinitionNode) -> SerializableThriftSourceInfo | None:
    """A node's ``source_info`` as its wire form, or ``None`` when the node
    carries none (programmatically built nodes have no ``.thrift`` source)."""
    info = node.source_info
    if info is None:
        return None
    return SerializableThriftSourceInfo(locator=info.locator, name=info.name)


def build_serializable_type_system(
    source: TypeSystem,
    root_uris: Sequence[str],
    options: PruneOptions | None = None,
) -> SerializableTypeSystem:
    """Export ``root_uris`` (plus their transitive type-structure + non-standard
    annotation closure) from ``source`` as a canonical ``SerializableTypeSystem``.

    ``source`` may be any ``TypeSystem`` (including the lazy ``SchemaRegistry``
    view or a builder-made ``IndexedTypeSystem``).

    ``options.include_source_info`` (default ``True``) gates whether each entry's
    ``sourceInfo`` is emitted."""
    opts = options if options is not None else PruneOptions()
    closure = _collect_closure(source, root_uris, _export_referenced_uris)
    if opts.include_source_info:
        # Refuse to emit a malformed artifact: no two exported URIs may share one
        # ``(locator, name)`` source identifier. ``_index_by_source`` raises
        # ``InvalidTypeError`` on a duplicate.
        _index_by_source(closure.values())
    types = {
        uri: SerializableTypeDefinitionEntry(
            definition=to_serializable_definition(node),
            sourceInfo=(
                _to_wire_source_info(node) if opts.include_source_info else None
            ),
        )
        for uri, node in closure.items()
    }
    return SerializableTypeSystem(types=types)
