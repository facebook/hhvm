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

import enum
import hashlib
import struct
from collections.abc import Callable, Iterable, Mapping
from typing import TypeVar

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
from thrift.lib.python.schema.type_system import InvalidTypeError

# Current hash algorithm version. Bumped only for backwards-incompatible
# changes to the digest format.
TYPE_SYSTEM_DIGEST_VERSION = 2

_T = TypeVar("_T")


class DigestMode(enum.Enum):
    # Hash the complete definition, including annotations and custom default
    # values.
    FULL = 0
    # Hash only the wire-compatible structure (fields, ids, types, enum values),
    # skipping annotations and custom default values. Two definitions with the
    # same structure but differing annotations/defaults produce the same digest.
    STRUCTURAL = 1


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
    return b"".join(buf)  # IOBuf iterates its (memoryview) chunks


# ---------------------------------------------------------------------------
# The streaming SHA-256 hasher.
# ---------------------------------------------------------------------------


class _Hasher:
    """A streaming SHA-256 hasher with the canonical primitive encodings.

    A fresh ``_Hasher`` carries no version byte -- the version is hashed only by
    the top-level :func:`type_system_digest`. Sub-hashers (for order-independent
    collections) carry no version byte either, but inherit the parent's
    :class:`DigestMode`."""

    __slots__ = ("_h", "_mode")
    _h: hashlib._Hash
    _mode: DigestMode

    def __init__(self, mode: DigestMode = DigestMode.FULL) -> None:
        self._h = hashlib.sha256()
        self._mode = mode

    def include_annotations_and_defaults(self) -> bool:
        return self._mode is DigestMode.FULL

    def update(self, data: bytes) -> None:
        self._h.update(data)

    def hash_bool(self, value: bool) -> None:
        self._h.update(b"\x01" if value else b"\x00")

    def hash_u8(self, value: int) -> None:
        self._h.update(struct.pack("<B", value))

    def hash_i8(self, value: int) -> None:
        self._h.update(struct.pack("<b", value))

    def hash_i16(self, value: int) -> None:
        self._h.update(struct.pack("<h", value))

    def hash_i32(self, value: int) -> None:
        self._h.update(struct.pack("<i", value))

    def hash_i64(self, value: int) -> None:
        self._h.update(struct.pack("<q", value))

    def hash_u32(self, value: int) -> None:
        self._h.update(struct.pack("<I", value))

    def hash_f32(self, value: float) -> None:
        self._h.update(struct.pack("<f", value))

    def hash_f64(self, value: float) -> None:
        self._h.update(struct.pack("<d", value))

    def hash_str(self, value: str) -> None:
        self.hash_bytes(value.encode("utf-8"))

    def hash_bytes(self, value: bytes) -> None:
        self.hash_u32(len(value))
        self._h.update(value)

    def finalize(self) -> bytes:
        return self._h.digest()

    def hash_unordered_by_digest(
        self, items: Iterable[_T], hash_fn: Callable[[_Hasher, _T], None]
    ) -> None:
        """Hash ``items`` order-independently: each to its own 32-byte
        sub-digest, then a ``u32`` count prefix, then the sorted sub-digests."""
        digests: list[bytes] = []
        for item in items:
            sub = _Hasher(self._mode)
            hash_fn(sub, item)
            digests.append(sub.finalize())
        self.hash_u32(len(digests))
        digests.sort()
        for sub_digest in digests:
            self._h.update(sub_digest)

    def hash_map_by_key_digest(
        self,
        items: Iterable[_T],
        key_hash_fn: Callable[[_Hasher, _T], None],
        entry_hash_fn: Callable[[_Hasher, _T], None],
    ) -> None:
        """Hash map entries order-independently: sort by the key's sub-digest
        (``u32`` count prefixed), then hash full key+value pairs in that order."""
        sorted_entries: list[tuple[bytes, _T]] = []
        for item in items:
            sub = _Hasher(self._mode)
            key_hash_fn(sub, item)
            sorted_entries.append((sub.finalize(), item))
        self.hash_u32(len(sorted_entries))
        sorted_entries.sort(key=lambda pair: pair[0])
        for _, item in sorted_entries:
            entry_hash_fn(self, item)


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
