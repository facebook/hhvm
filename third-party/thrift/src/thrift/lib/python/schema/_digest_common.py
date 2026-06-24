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
Wire-free machinery shared by the two TypeSystem digest paths -- the serialized
digest (``type_system_digest.py``, over the generated ``Serializable*`` wire
types) and the runtime digest (``runtime_digest.py``, over the native
``type_system.py`` / ``_record.py`` model).

Encoding (little-endian throughout):

* ``bool`` -> one byte; integers -> width-exact LE; floats -> ``to_bits`` LE;
* ``str`` / ``bytes`` -> ``u32`` length prefix + raw bytes;
* unordered (set / map / annotation map): each element to a 32-byte sub-digest,
  a ``u32`` count prefix, then the lexicographically-sorted sub-digests (maps
  sort by the key's sub-digest, then hash full key+value pairs).
"""

from __future__ import annotations

import enum
import hashlib
import struct
from collections.abc import Callable, Iterable
from typing import TypeVar

# Current hash algorithm version. Bumped only for backwards-incompatible
# changes to the digest format.
TYPE_SYSTEM_DIGEST_VERSION = 2

# Standard annotations are dropped from the digest (and the wire export) -- they
# are not bundled in serializable type systems (circular-dependency concerns).
_STANDARD_ANNOTATION_PREFIX = "facebook.com/thrift/annotation/"

_T = TypeVar("_T")


def is_standard_annotation(uri: str) -> bool:
    """Whether ``uri`` names a standard ``facebook.com/thrift/annotation/*``
    annotation, which is dropped on export and excluded from the digest."""
    return uri.startswith(_STANDARD_ANNOTATION_PREFIX)


class DigestMode(enum.Enum):
    # Hash the complete definition, including annotations and custom default
    # values.
    FULL = 0
    # Hash only the wire-compatible structure (fields, ids, types, enum values),
    # skipping annotations and custom default values. Two definitions with the
    # same structure but differing annotations/defaults produce the same digest.
    STRUCTURAL = 1


class _Hasher:
    """A streaming SHA-256 hasher with the canonical primitive encodings.

    A fresh ``_Hasher`` carries no version byte -- the version is hashed only by
    the top-level digest entry points. Sub-hashers (for order-independent
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
