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
Canonical SHA-256 digest of the **runtime** TypeSystem model.

This path hashes the pure-Python runtime model (``type_system.py`` / ``_record.py``)
**directly** -- it never materializes the wire ``Serializable*`` form.

The output is **byte-identical** to the serialized digest over the equivalent
``SerializableTypeSystem``: the native discriminant->field-id maps here are held
equal to the wire-keyed maps in ``type_system_digest.py``, guarded by
``test_digest_record_parity.py`` plus the C++ goldens and the
``runtime_type_system_digest == type_system_digest(...)`` oracle in
``test_type_system_digest.py``.

``DigestMode`` is re-exported here for ergonomics.
"""

from __future__ import annotations

from collections.abc import Mapping
from typing import assert_never

from thrift.lib.python.schema._digest_common import (
    _Hasher,
    DigestMode,
    is_standard_annotation,
    TYPE_SYSTEM_DIGEST_VERSION,
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
    SerializableRecord,
    SerializableRecordBase,
    SetRecord,
    TextRecord,
)
from thrift.lib.python.schema.type_system import (
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

__all__ = [
    "DigestMode",
    "runtime_type_system_digest",
    "definition_digest",
    "type_ref_digest",
]


# ---------------------------------------------------------------------------
# Native discriminant -> field-id maps. These MUST stay equal to the wire-keyed
# maps in ``type_system_digest.py`` (``_TYPE_ID_FIELD_ID`` / ``_RECORD_FIELD_ID``
# / ``_DEFINITION_FIELD_ID``).
# ---------------------------------------------------------------------------

# TypeId arms, keyed by the native ``Primitive`` / container kind.
_NATIVE_PRIMITIVE_FIELD_ID: dict[Primitive, int] = {
    Primitive.BOOL: 1,
    Primitive.BYTE: 2,
    Primitive.I16: 3,
    Primitive.I32: 4,
    Primitive.I64: 5,
    Primitive.FLOAT: 6,
    Primitive.DOUBLE: 7,
    Primitive.STRING: 8,
    Primitive.BINARY: 9,
    Primitive.ANY: 10,
}
_NATIVE_TYPE_ID_USER_DEFINED = 11
_NATIVE_TYPE_ID_LIST = 12
_NATIVE_TYPE_ID_SET = 13
_NATIVE_TYPE_ID_MAP = 14

# Native ``SerializableRecord`` arms (note the gap: thrift field id 4 is unused
# in ``record.thrift`` -- ``int32Datum`` is 5).
_NATIVE_RECORD_FIELD_ID: dict[type[SerializableRecordBase], int] = {
    BoolRecord: 1,
    Int8Record: 2,
    Int16Record: 3,
    Int32Record: 5,
    Int64Record: 6,
    Float32Record: 7,
    Float64Record: 8,
    TextRecord: 9,
    ByteArrayRecord: 10,
    FieldSetRecord: 11,
    ListRecord: 12,
    SetRecord: 13,
    MapRecord: 14,
}

# Definition discriminants (match ``type_system.thrift`` field ids).
_NATIVE_DEF_STRUCT = 1
_NATIVE_DEF_UNION = 2
_NATIVE_DEF_ENUM = 3
_NATIVE_DEF_OPAQUE_ALIAS = 4


# ---------------------------------------------------------------------------
# Native leaf hashers (TypeId / SerializableRecord / annotations).
# ---------------------------------------------------------------------------


def _hash_type_id_native(h: _Hasher, type_ref: TypeRefBase) -> None:
    """Hash a resolved ``TypeRef``'s identity directly (no wire ``TypeId``)."""
    if not isinstance(type_ref, TypeRef):
        raise InvalidTypeError(
            f"{type(type_ref).__name__} is not a resolved type edge (TypeRef)."
        )
    match type_ref:
        case PrimitiveTypeRef():
            h.hash_i32(_NATIVE_PRIMITIVE_FIELD_ID[type_ref.primitive])
        case ListTypeRef():
            h.hash_i32(_NATIVE_TYPE_ID_LIST)
            _hash_type_id_native(h, type_ref.element_type)
        case SetTypeRef():
            h.hash_i32(_NATIVE_TYPE_ID_SET)
            _hash_type_id_native(h, type_ref.element_type)
        case MapTypeRef():
            h.hash_i32(_NATIVE_TYPE_ID_MAP)
            _hash_type_id_native(h, type_ref.key_type)
            _hash_type_id_native(h, type_ref.value_type)
        case StructTypeRef() | UnionTypeRef() | EnumTypeRef() | OpaqueAliasTypeRef():
            h.hash_i32(_NATIVE_TYPE_ID_USER_DEFINED)
            h.hash_str(type_ref.node.uri)
        case _:
            assert_never(type_ref)


def _hash_record_native(h: _Hasher, record: SerializableRecord) -> None:
    """Hash a native ``SerializableRecord`` directly (no wire conversion)."""
    field_id = _NATIVE_RECORD_FIELD_ID.get(type(record))
    if field_id is None:
        raise InvalidTypeError(f"Cannot digest unknown record: {type(record).__name__}")
    h.hash_i32(field_id)
    match record:
        case BoolRecord():
            h.hash_bool(record.value)
        case Int8Record():
            h.hash_i8(record.value)
        case Int16Record():
            h.hash_i16(record.value)
        case Int32Record():
            h.hash_i32(record.value)
        case Int64Record():
            h.hash_i64(record.value)
        case Float32Record():
            h.hash_f32(record.value)
        case Float64Record():
            h.hash_f64(record.value)
        case TextRecord():
            h.hash_str(record.value)
        case ByteArrayRecord():
            h.hash_bytes(record.value)
        case FieldSetRecord():
            # Field-id order: hash each i16 id, then its value record.
            for field_id_key in sorted(record.fields):
                h.hash_i16(field_id_key)
                _hash_record_native(h, record.fields[field_id_key])
        case ListRecord():
            for element in record.elements:
                _hash_record_native(h, element)
        case SetRecord():
            h.hash_unordered_by_digest(record.elements, _hash_record_native)
        case MapRecord():
            h.hash_map_by_key_digest(
                record.entries,
                _hash_map_entry_key_native,
                _hash_map_entry_full_native,
            )
        case _:
            assert_never(record)


def _hash_map_entry_key_native(
    h: _Hasher, entry: tuple[SerializableRecord, SerializableRecord]
) -> None:
    _hash_record_native(h, entry[0])


def _hash_map_entry_full_native(
    h: _Hasher, entry: tuple[SerializableRecord, SerializableRecord]
) -> None:
    _hash_record_native(h, entry[0])
    _hash_record_native(h, entry[1])


def _hash_annotation_entry_native(
    h: _Hasher, entry: tuple[str, SerializableRecord]
) -> None:
    uri, record = entry
    h.hash_str(uri)
    _hash_record_native(h, record)


def _hash_annotations_native(
    h: _Hasher, annotations: Mapping[str, SerializableRecord]
) -> None:
    """Hash a native annotation map directly, dropping standard
    ``facebook.com/thrift/annotation/*`` annotations (the same rule the wire
    export applies). STRUCTURAL emits NOTHING -- not even the ``u32`` count
    prefix (matches the wire ``_hash_annotations`` early return)."""
    if not h.include_annotations_and_defaults():
        return
    kept = [
        (uri, record)
        for uri, record in annotations.items()
        if not is_standard_annotation(uri)
    ]
    h.hash_unordered_by_digest(kept, _hash_annotation_entry_native)


# ---------------------------------------------------------------------------
# Native node walkers.
# ---------------------------------------------------------------------------


def _hash_field_node(h: _Hasher, field: FieldDefinition) -> None:
    h.hash_i16(field.identity.id)
    h.hash_str(field.identity.name)
    h.hash_i32(field.presence.value)
    _hash_type_id_native(h, field.type)
    if h.include_annotations_and_defaults():
        default = field.custom_default
        if default is not None:
            _hash_record_native(h, default)
    _hash_annotations_native(h, field.annotations)


def _hash_struct_or_union_node(h: _Hasher, node: StructNode | UnionNode) -> None:
    for field in sorted(node.fields, key=lambda f: f.identity.id):
        _hash_field_node(h, field)
    h.hash_bool(node.is_sealed)
    _hash_annotations_native(h, node.annotations)


def _hash_enum_node(h: _Hasher, node: EnumNode) -> None:
    for value in sorted(node.values, key=lambda v: v.datum):
        h.hash_str(value.name)
        h.hash_i32(value.datum)
        _hash_annotations_native(h, value.annotations)
    _hash_annotations_native(h, node.annotations)


def _hash_opaque_alias_node(h: _Hasher, node: OpaqueAliasNode) -> None:
    if node.target_type is None:
        raise InvalidTypeError(
            f"Cannot digest opaque alias {node.uri!r} with no target type"
        )
    _hash_type_id_native(h, node.target_type)
    _hash_annotations_native(h, node.annotations)


def _hash_definition_node(h: _Hasher, node: DefinitionNode) -> None:
    """Emit the definition discriminant + body for a runtime node, matching the
    wire ``_hash_definition`` byte-for-byte."""
    if isinstance(node, StructNode):
        h.hash_i32(_NATIVE_DEF_STRUCT)
        _hash_struct_or_union_node(h, node)
    elif isinstance(node, UnionNode):
        h.hash_i32(_NATIVE_DEF_UNION)
        _hash_struct_or_union_node(h, node)
    elif isinstance(node, EnumNode):
        h.hash_i32(_NATIVE_DEF_ENUM)
        _hash_enum_node(h, node)
    elif isinstance(node, OpaqueAliasNode):
        h.hash_i32(_NATIVE_DEF_OPAQUE_ALIAS)
        _hash_opaque_alias_node(h, node)
    else:
        raise InvalidTypeError(
            f"Cannot digest unknown definition node: {type(node).__name__}"
        )


# ---------------------------------------------------------------------------
# Public runtime-representation entry points.
# Each is byte-identical to the equivalent serialized digest.
# ---------------------------------------------------------------------------


def runtime_type_system_digest(
    source: TypeSystem, mode: DigestMode = DigestMode.FULL
) -> bytes:
    """The digest of a **runtime** ``TypeSystem``.

    Walks every known URI (sorted) directly, hashing each definition node in
    place. Byte-identical to ``type_system_digest`` over the equivalent wire
    form. Raises :class:`InvalidTypeError` if ``source`` cannot enumerate its
    URIs (``get_known_uris()`` is ``None`` -- e.g. the lazy ``SchemaRegistry``,
    which offers the root-pruned ``type_system_digest(roots)`` instead)."""
    uris = source.get_known_uris()
    if uris is None:
        raise InvalidTypeError(
            "Cannot digest a TypeSystem that cannot enumerate its URIs "
            "(get_known_uris() returned None); pass an EnumerableTypeSystem, or "
            "use the root-pruned SchemaRegistry.type_system_digest(roots)."
        )
    h = _Hasher(mode)
    h.hash_u8(TYPE_SYSTEM_DIGEST_VERSION)
    for uri in sorted(uris, key=lambda u: u.encode("utf-8")):
        h.hash_str(uri)
        _hash_definition_node(h, source.get_user_defined_type_or_throw(uri))
    return h.finalize()


def definition_digest(
    node: DefinitionNode, mode: DigestMode = DigestMode.FULL
) -> bytes:
    """The digest of a single runtime definition node.

    Hashes the definition discriminant + body -- no version byte, no URI -- so it
    equals the digest of the equivalent ``SerializableTypeDefinition``."""
    h = _Hasher(mode)
    _hash_definition_node(h, node)
    return h.finalize()


def type_ref_digest(type_ref: TypeRefBase) -> bytes:
    """The digest of a runtime type reference's identity, hashing the native
    ``TypeRef`` directly. Equal to ``type_id_digest(to_type_id(type_ref))``;
    a ``TypeId`` carries no annotations/defaults, so it is mode-invariant."""
    h = _Hasher()
    _hash_type_id_native(h, type_ref)
    return h.finalize()
