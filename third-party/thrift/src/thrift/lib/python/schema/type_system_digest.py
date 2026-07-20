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
Canonical, deterministic SHA-256 digest of a ``SerializableTypeSystem``.

The output is **byte-identical** to the C++
(``thrift/lib/cpp2/dynamic/TypeSystemDigest.{h,cpp}``) and Rust
(``thrift/lib/rust/dynamic/type_system_digest``) implementations, enabling
cross-language cache invalidation, version checks, and deduplication.

It consumes the canonical wire form produced by ``_serializable.py`` and
**never** hashes ``SyntaxGraph`` objects. The hasher uses only the stdlib
``hashlib``.

Properties:

* equivalent type systems always produce the same digest;
* order-independent: URI / field / annotation ordering do not matter;
* ``sourceInfo`` is excluded (file paths are not semantically significant);
* floats are hashed by their IEEE-754 bit pattern -- safe because
  ``SerializableRecord`` rejects NaN and ``-0.0`` at construction;
* :class:`DigestMode` selects the coverage -- ``FULL`` (default) hashes
  everything, ``STRUCTURAL`` skips annotations and custom default values.

Encoding (little-endian throughout):

* the single version byte ``TYPE_SYSTEM_DIGEST_VERSION`` is hashed first;
* ``bool`` -> one byte; integers -> width-exact LE; floats -> ``to_bits`` LE;
* ``str`` / ``bytes`` -> ``u32`` length prefix + raw bytes;
* ordered: top-level types by URI, struct/union fields by id, enum values by
  datum, ``fieldSetDatum`` by field id;
* unordered (set / map / annotation map): each element to a 32-byte sub-digest,
  a ``u32`` count prefix, then the lexicographically-sorted sub-digests (maps
  sort by the key's sub-digest, then hash full key+value pairs).
"""

from __future__ import annotations

from collections.abc import Mapping

from apache.thrift.type_system.record.thrift_types import (
    SerializableRecord as WireRecord,
    SerializableRecordMapEntry,
)
from apache.thrift.type_system.type_id.thrift_types import TypeId
from apache.thrift.type_system.type_system.thrift_types import (
    SerializableEnumDefinition,
    SerializableEnumValueDefinition,
    SerializableFieldDefinition,
    SerializableOpaqueAliasDefinition,
    SerializableStructDefinition,
    SerializableTypeDefinition,
    SerializableTypeSystem,
    SerializableUnionDefinition,
)
from thrift.lib.python.schema._digest_common import (
    _Hasher,
    DigestMode,
    TYPE_SYSTEM_DIGEST_VERSION,
)
from thrift.lib.python.schema.type_system import InvalidTypeError


# ---------------------------------------------------------------------------
# Wire-union arm -> thrift field id. These integers are the canonical
# discriminants the digest hashes; they must match the IDL field ids exactly
# (see ``type_id.thrift`` / ``record.thrift`` / ``type_system.thrift``).
# ---------------------------------------------------------------------------

_TYPE_ID_FIELD_ID: dict[TypeId.Type, int] = {
    TypeId.Type.boolType: 1,
    TypeId.Type.byteType: 2,
    TypeId.Type.i16Type: 3,
    TypeId.Type.i32Type: 4,
    TypeId.Type.i64Type: 5,
    TypeId.Type.floatType: 6,
    TypeId.Type.doubleType: 7,
    TypeId.Type.stringType: 8,
    TypeId.Type.binaryType: 9,
    TypeId.Type.anyType: 10,
    TypeId.Type.userDefinedType: 11,
    TypeId.Type.listType: 12,
    TypeId.Type.setType: 13,
    TypeId.Type.mapType: 14,
}

# Note the gap: thrift field id 4 is unused in ``record.thrift`` (int32Datum=5).
_RECORD_FIELD_ID: dict[WireRecord.Type, int] = {
    WireRecord.Type.boolDatum: 1,
    WireRecord.Type.int8Datum: 2,
    WireRecord.Type.int16Datum: 3,
    WireRecord.Type.int32Datum: 5,
    WireRecord.Type.int64Datum: 6,
    WireRecord.Type.float32Datum: 7,
    WireRecord.Type.float64Datum: 8,
    WireRecord.Type.textDatum: 9,
    WireRecord.Type.byteArrayDatum: 10,
    WireRecord.Type.fieldSetDatum: 11,
    WireRecord.Type.listDatum: 12,
    WireRecord.Type.setDatum: 13,
    WireRecord.Type.mapDatum: 14,
}

_DEFINITION_FIELD_ID: dict[SerializableTypeDefinition.Type, int] = {
    SerializableTypeDefinition.Type.structDef: 1,
    SerializableTypeDefinition.Type.unionDef: 2,
    SerializableTypeDefinition.Type.enumDef: 3,
    SerializableTypeDefinition.Type.opaqueAliasDef: 4,
}


def _iobuf_to_bytes(buf: object) -> bytes:
    """Flatten a ``byteArrayDatum`` (a ``folly.iobuf.IOBuf`` in thrift-python) to
    ``bytes``; tolerate a plain ``bytes`` too."""
    if isinstance(buf, (bytes, bytearray, memoryview)):
        return bytes(buf)
    # pyrefly: ignore [bad-argument-type]
    return b"".join(buf)  # IOBuf iterates its (memoryview) chunks


# ---------------------------------------------------------------------------
# TypeId
# ---------------------------------------------------------------------------


def _hash_type_id(h: _Hasher, type_id: TypeId) -> None:
    field_id = _TYPE_ID_FIELD_ID.get(type_id.type)
    if field_id is None:
        # An empty/unset union -> a single ``0u8`` (matches the C++ empty-TypeId
        # path and the Rust ``None`` boxed-child encoding).
        h.hash_u8(0)
        return
    h.hash_i32(field_id)
    kind = type_id.type
    if kind == TypeId.Type.userDefinedType:
        h.hash_str(type_id.userDefinedType)
    elif kind == TypeId.Type.listType:
        _hash_type_id_child(h, type_id.listType.elementType)
    elif kind == TypeId.Type.setType:
        _hash_type_id_child(h, type_id.setType.elementType)
    elif kind == TypeId.Type.mapType:
        _hash_type_id_child(h, type_id.mapType.keyType)
        _hash_type_id_child(h, type_id.mapType.valueType)
    # Primitives carry no additional data -- the discriminant distinguishes them.


def _hash_type_id_child(h: _Hasher, child: TypeId | None) -> None:
    """A boxed container child: absent (``None``) is encoded as a single
    ``0u8``."""
    if child is None:
        h.hash_u8(0)
    else:
        _hash_type_id(h, child)


# ---------------------------------------------------------------------------
# SerializableRecord (custom defaults + annotation values)
# ---------------------------------------------------------------------------


def _hash_record(h: _Hasher, record: WireRecord) -> None:
    field_id = _RECORD_FIELD_ID.get(record.type)
    if field_id is None:
        raise InvalidTypeError(f"Cannot digest empty/unknown record: {record.type!r}")
    h.hash_i32(field_id)
    kind = record.type
    if kind == WireRecord.Type.fieldSetDatum:
        # Field-id order: hash each i16 id, then its value record.
        field_set = record.fieldSetDatum
        for field_id_key in sorted(field_set):
            h.hash_i16(field_id_key)
            _hash_record(h, field_set[field_id_key])
    elif kind == WireRecord.Type.listDatum:
        for element in record.listDatum:
            _hash_record(h, element)
    elif kind == WireRecord.Type.setDatum:
        h.hash_unordered_by_digest(record.setDatum, _hash_record_element)
    elif kind == WireRecord.Type.mapDatum:
        h.hash_map_by_key_digest(
            record.mapDatum, _hash_map_entry_key, _hash_map_entry_full
        )
    else:
        _hash_scalar_record(h, kind, record)


def _hash_scalar_record(h: _Hasher, kind: WireRecord.Type, record: WireRecord) -> None:
    """Hash a non-container record value (the discriminant is hashed by the
    caller). Integer width is exact per arm (int8/16/32/64); floats use their
    IEEE-754 bit pattern."""
    if kind == WireRecord.Type.boolDatum:
        h.hash_bool(record.boolDatum)
    elif kind == WireRecord.Type.int8Datum:
        h.hash_i8(record.int8Datum)
    elif kind == WireRecord.Type.int16Datum:
        h.hash_i16(record.int16Datum)
    elif kind == WireRecord.Type.int32Datum:
        h.hash_i32(record.int32Datum)
    elif kind == WireRecord.Type.int64Datum:
        h.hash_i64(record.int64Datum)
    elif kind == WireRecord.Type.float32Datum:
        h.hash_f32(record.float32Datum)
    elif kind == WireRecord.Type.float64Datum:
        h.hash_f64(record.float64Datum)
    elif kind == WireRecord.Type.textDatum:
        h.hash_str(record.textDatum)
    elif kind == WireRecord.Type.byteArrayDatum:
        h.hash_bytes(_iobuf_to_bytes(record.byteArrayDatum))
    else:
        raise InvalidTypeError(f"Cannot digest unhandled scalar record: {kind!r}")


def _hash_record_element(h: _Hasher, element: WireRecord) -> None:
    _hash_record(h, element)


def _hash_map_entry_key(h: _Hasher, entry: SerializableRecordMapEntry) -> None:
    _hash_record(h, entry.key)


def _hash_map_entry_full(h: _Hasher, entry: SerializableRecordMapEntry) -> None:
    _hash_record(h, entry.key)
    _hash_record(h, entry.value)


# ---------------------------------------------------------------------------
# Annotations (URI -> value), order-independent.
# ---------------------------------------------------------------------------


def _hash_annotation_entry(h: _Hasher, entry: tuple[str, WireRecord]) -> None:
    uri, record = entry
    h.hash_str(uri)
    _hash_record(h, record)


def _hash_annotations(h: _Hasher, annotations: Mapping[str, WireRecord]) -> None:
    # Structural mode emits NOTHING for annotations -- not even the u32 count
    # prefix.
    if not h.include_annotations_and_defaults():
        return
    h.hash_unordered_by_digest(annotations.items(), _hash_annotation_entry)


# ---------------------------------------------------------------------------
# Definitions
# ---------------------------------------------------------------------------


def _hash_field(h: _Hasher, field: SerializableFieldDefinition) -> None:
    h.hash_i16(field.identity.id)
    h.hash_str(field.identity.name)
    h.hash_i32(field.presence.value)
    _hash_type_id(h, field.type)
    if h.include_annotations_and_defaults():
        default = field.customDefaultPartialRecord
        if default is not None:
            _hash_record(h, default)
    _hash_annotations(h, field.annotations)


def _hash_struct_def(h: _Hasher, definition: SerializableStructDefinition) -> None:
    for field in sorted(definition.fields, key=_field_id):
        _hash_field(h, field)
    h.hash_bool(definition.isSealed)
    _hash_annotations(h, definition.annotations)


def _hash_union_def(h: _Hasher, definition: SerializableUnionDefinition) -> None:
    for field in sorted(definition.fields, key=_field_id):
        _hash_field(h, field)
    h.hash_bool(definition.isSealed)
    _hash_annotations(h, definition.annotations)


def _hash_enum_def(h: _Hasher, definition: SerializableEnumDefinition) -> None:
    for value in sorted(definition.values, key=_enum_datum):
        h.hash_str(value.name)
        h.hash_i32(value.datum)
        _hash_annotations(h, value.annotations)
    _hash_annotations(h, definition.annotations)


def _hash_opaque_alias_def(
    h: _Hasher, definition: SerializableOpaqueAliasDefinition
) -> None:
    _hash_type_id(h, definition.targetType)
    _hash_annotations(h, definition.annotations)


def _field_id(field: SerializableFieldDefinition) -> int:
    return field.identity.id


def _enum_datum(value: SerializableEnumValueDefinition) -> int:
    return value.datum


def _hash_definition(h: _Hasher, definition: SerializableTypeDefinition) -> None:
    field_id = _DEFINITION_FIELD_ID.get(definition.type)
    if field_id is None:
        raise InvalidTypeError(
            f"Cannot digest empty/unknown type definition: {definition.type!r}"
        )
    h.hash_i32(field_id)
    kind = definition.type
    if kind == SerializableTypeDefinition.Type.structDef:
        _hash_struct_def(h, definition.structDef)
    elif kind == SerializableTypeDefinition.Type.unionDef:
        _hash_union_def(h, definition.unionDef)
    elif kind == SerializableTypeDefinition.Type.enumDef:
        _hash_enum_def(h, definition.enumDef)
    elif kind == SerializableTypeDefinition.Type.opaqueAliasDef:
        _hash_opaque_alias_def(h, definition.opaqueAliasDef)


# ---------------------------------------------------------------------------
# Public API
# ---------------------------------------------------------------------------


def type_system_digest(
    type_system: SerializableTypeSystem, mode: DigestMode = DigestMode.FULL
) -> bytes:
    """The canonical 32-byte SHA-256 digest of ``type_system``.

    Hashes the version byte first, then every type in URI order (excluding
    ``sourceInfo``). Byte-identical to the C++/Rust implementations.

    With :attr:`DigestMode.STRUCTURAL`, annotations and custom default values
    are excluded, so two type systems that differ only in those produce the same
    digest. Defaults to :attr:`DigestMode.FULL`."""
    h = _Hasher(mode)
    h.hash_u8(TYPE_SYSTEM_DIGEST_VERSION)
    types = type_system.types
    for uri in sorted(types, key=lambda u: u.encode("utf-8")):
        h.hash_str(uri)
        _hash_definition(h, types[uri].definition)
    return h.finalize()


def type_id_digest(type_id: TypeId) -> bytes:
    """The 32-byte SHA-256 digest of a single ``TypeId`` (no version byte).

    Only the structural identity is hashed, so it is suitable for keying a single
    type reference -- not a whole type system."""
    h = _Hasher()
    _hash_type_id(h, type_id)
    return h.finalize()
