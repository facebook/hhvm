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
Native, pure-Python ``SerializableRecord`` -- the value layer that carries field
custom defaults and structured-annotation values in the runtime TypeSystem.

A closed hierarchy discriminated by ``isinstance`` / ``match``. The arms are:

    BoolRecord  Int8Record  Int16Record  Int32Record  Int64Record
    Float32Record  Float64Record  TextRecord  ByteArrayRecord
    FieldSetRecord  ListRecord  SetRecord  MapRecord

This layer enforces data-model invariants that pure Thrift schema cannot
express. ``record.thrift`` documents the set/map rules in prose; the float and
text rules match the canonical (C++/Rust) record validation:

* floats reject ``NaN`` and ``-0.0`` (so their bit patterns are deterministic),
* text must be valid UTF-8,
* sets reject duplicate elements and maps reject duplicate keys.

Equality and hashing are *structural and order-independent* for the unordered
arms (``FieldSetRecord`` / ``SetRecord`` / ``MapRecord``) and order-preserving for
``ListRecord``. This module deliberately depends on nothing else in the package.
"""

from __future__ import annotations

import math
from abc import ABC
from collections.abc import Mapping, Sequence
from types import MappingProxyType
from typing import ClassVar


class InvalidRecordError(ValueError):
    """Raised when a ``SerializableRecord`` is constructed from an invalid datum:
    ``NaN`` / ``-0.0`` floats, non-UTF-8 text, duplicate set elements, or
    duplicate map keys."""


def _ensure_valid_float(value: float) -> None:
    """Reject ``NaN`` and ``-0.0``. ``+/-inf`` are *not* rejected."""
    if math.isnan(value):
        raise InvalidRecordError("NaN is not a valid Thrift datum")
    if value == 0.0 and math.copysign(1.0, value) < 0.0:
        raise InvalidRecordError("-0.0 is not a valid Thrift datum")


def _ensure_utf8(value: str) -> None:
    """Reject text that cannot be encoded as UTF-8 (e.g. lone surrogates)."""
    try:
        value.encode("utf-8")
    except UnicodeEncodeError as e:
        raise InvalidRecordError(f"text datum is not valid UTF-8: {value!r}") from e


# ---------------------------------------------------------------------------
# SerializableRecord hierarchy -- discriminate with isinstance / match.
# Equality and hashing are structural (by ``_key``); object identity via ``is``.
# ---------------------------------------------------------------------------


class SerializableRecord:
    """Base of the record-value hierarchy.

    Two records are equal iff they denote the same datum; subclasses implement
    ``_key`` and inherit ``__eq__`` / ``__hash__``. ``_key`` returns a hashable,
    structural key whose first element is a per-arm tag, so records of different
    arms with the same payload (e.g. ``Int8Record(1)`` vs ``Int16Record(1)``)
    never compare equal."""

    __slots__: tuple[str, ...] = ()

    def _key(self) -> tuple[object, ...]:
        raise NotImplementedError

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, SerializableRecord):
            return NotImplemented
        return self._key() == other._key()

    def __hash__(self) -> int:
        return hash(self._key())

    def __repr__(self) -> str:
        return f"{type(self).__name__}()"


class BoolRecord(SerializableRecord):
    """A ``bool`` datum (``boolDatum``)."""

    __slots__ = ("_value",)
    __match_args__ = ("value",)
    _value: bool

    def __init__(self, value: bool) -> None:
        self._value = value

    @property
    def value(self) -> bool:
        return self._value

    def _key(self) -> tuple[object, ...]:
        return ("bool", self._value)

    def __repr__(self) -> str:
        return f"BoolRecord({self._value!r})"


class _IntRecord(SerializableRecord, ABC):
    """Shared implementation for the four fixed-width integer arms. Each leaf
    supplies its own ``_TAG`` so equal payloads of different widths are still
    distinct records (abstract: only the concrete arms set ``_TAG``)."""

    __slots__: tuple[str, ...] = ("_value",)
    __match_args__ = ("value",)
    _TAG: ClassVar[str]
    _value: int

    def __init__(self, value: int) -> None:
        self._value = value

    @property
    def value(self) -> int:
        return self._value

    def _key(self) -> tuple[object, ...]:
        return (self._TAG, self._value)

    def __repr__(self) -> str:
        return f"{type(self).__name__}({self._value!r})"


class Int8Record(_IntRecord):
    """An 8-bit integer datum (``int8Datum``)."""

    __slots__ = ()
    _TAG = "int8"


class Int16Record(_IntRecord):
    """A 16-bit integer datum (``int16Datum``)."""

    __slots__ = ()
    _TAG = "int16"


class Int32Record(_IntRecord):
    """A 32-bit integer datum (``int32Datum``). Enum values also use this arm."""

    __slots__ = ()
    _TAG = "int32"


class Int64Record(_IntRecord):
    """A 64-bit integer datum (``int64Datum``)."""

    __slots__ = ()
    _TAG = "int64"


class _FloatRecord(SerializableRecord, ABC):
    """Shared implementation for the two floating-point arms. ``NaN`` / ``-0.0``
    are rejected at construction; each leaf supplies its own ``_TAG`` (abstract:
    only the concrete arms set ``_TAG``)."""

    __slots__: tuple[str, ...] = ("_value",)
    __match_args__ = ("value",)
    _TAG: ClassVar[str]
    _value: float

    def __init__(self, value: float) -> None:
        _ensure_valid_float(value)
        self._value = value

    @property
    def value(self) -> float:
        return self._value

    def _key(self) -> tuple[object, ...]:
        return (self._TAG, self._value)

    def __repr__(self) -> str:
        return f"{type(self).__name__}({self._value!r})"


class Float32Record(_FloatRecord):
    """A 32-bit floating-point datum (``float32Datum``)."""

    __slots__ = ()
    _TAG = "float32"


class Float64Record(_FloatRecord):
    """A 64-bit floating-point datum (``float64Datum``)."""

    __slots__ = ()
    _TAG = "float64"


class TextRecord(SerializableRecord):
    """A UTF-8 text datum (``textDatum``). Non-UTF-8 input is rejected."""

    __slots__ = ("_value",)
    __match_args__ = ("value",)
    _value: str

    def __init__(self, value: str) -> None:
        _ensure_utf8(value)
        self._value = value

    @property
    def value(self) -> str:
        return self._value

    def _key(self) -> tuple[object, ...]:
        return ("text", self._value)

    def __repr__(self) -> str:
        return f"TextRecord({self._value!r})"


class ByteArrayRecord(SerializableRecord):
    """A raw byte-array datum (``byteArrayDatum``). Not UTF-8 validated."""

    __slots__ = ("_value",)
    __match_args__ = ("value",)
    _value: bytes

    def __init__(self, value: bytes) -> None:
        self._value = bytes(value)

    @property
    def value(self) -> bytes:
        return self._value

    def _key(self) -> tuple[object, ...]:
        return ("byte_array", self._value)

    def __repr__(self) -> str:
        return f"ByteArrayRecord({self._value!r})"


class FieldSetRecord(SerializableRecord):
    """A field-id-keyed record (``fieldSetDatum``) -- the shape of a struct/union
    value and of a structured-annotation value. Keys are i16 field ids."""

    __slots__ = ("_fields",)
    __match_args__ = ("fields",)
    _fields: dict[int, SerializableRecord]

    def __init__(self, fields: Mapping[int, SerializableRecord]) -> None:
        self._fields = dict(fields)

    @property
    def fields(self) -> Mapping[int, SerializableRecord]:
        """The field-id -> record map, as a read-only view (mutation raises)."""
        return MappingProxyType(self._fields)

    def _key(self) -> tuple[object, ...]:
        return (
            "field_set",
            tuple(sorted((fid, r._key()) for fid, r in self._fields.items())),
        )

    def __repr__(self) -> str:
        return f"FieldSetRecord({self._fields!r})"


class ListRecord(SerializableRecord):
    """An ordered list datum (``listDatum``). Order is significant."""

    __slots__ = ("_elements",)
    __match_args__ = ("elements",)
    _elements: tuple[SerializableRecord, ...]

    def __init__(self, elements: Sequence[SerializableRecord]) -> None:
        self._elements = tuple(elements)

    @property
    def elements(self) -> tuple[SerializableRecord, ...]:
        return self._elements

    def _key(self) -> tuple[object, ...]:
        return ("list", tuple(e._key() for e in self._elements))

    def __repr__(self) -> str:
        return f"ListRecord({list(self._elements)!r})"


class SetRecord(SerializableRecord):
    """An unordered set datum (``setDatum``). Duplicate elements are rejected;
    equality is order-independent."""

    __slots__ = ("_elements",)
    __match_args__ = ("elements",)
    _elements: tuple[SerializableRecord, ...]

    def __init__(self, elements: Sequence[SerializableRecord]) -> None:
        result = tuple(elements)
        seen: set[tuple[object, ...]] = set()
        for element in result:
            key = element._key()
            if key in seen:
                raise InvalidRecordError(f"duplicate element in set datum: {element!r}")
            seen.add(key)
        self._elements = result

    @property
    def elements(self) -> tuple[SerializableRecord, ...]:
        return self._elements

    def _key(self) -> tuple[object, ...]:
        return ("set", frozenset(e._key() for e in self._elements))

    def __repr__(self) -> str:
        return f"SetRecord({list(self._elements)!r})"


class MapRecord(SerializableRecord):
    """An unordered map datum (``mapDatum``) of ``(key, value)`` record entries.
    Duplicate keys are rejected; equality is order-independent."""

    __slots__ = ("_entries",)
    __match_args__ = ("entries",)
    _entries: tuple[tuple[SerializableRecord, SerializableRecord], ...]

    def __init__(
        self, entries: Sequence[tuple[SerializableRecord, SerializableRecord]]
    ) -> None:
        result = tuple((key, value) for key, value in entries)
        seen: set[tuple[object, ...]] = set()
        for key, _value in result:
            key_key = key._key()
            if key_key in seen:
                raise InvalidRecordError(f"duplicate key in map datum: {key!r}")
            seen.add(key_key)
        self._entries = result

    @property
    def entries(self) -> tuple[tuple[SerializableRecord, SerializableRecord], ...]:
        return self._entries

    def _key(self) -> tuple[object, ...]:
        return ("map", frozenset((k._key(), v._key()) for k, v in self._entries))

    def __repr__(self) -> str:
        return f"MapRecord({list(self._entries)!r})"


# Static-exhaustiveness union alias (for ``match`` + ``assert_never``), mirroring
# ``TypeRefT`` in ``type_system.py``. Defined after the classes so the runtime
# ``X | Y`` union can be built.
SerializableRecordT = (
    BoolRecord
    | Int8Record
    | Int16Record
    | Int32Record
    | Int64Record
    | Float32Record
    | Float64Record
    | TextRecord
    | ByteArrayRecord
    | FieldSetRecord
    | ListRecord
    | SetRecord
    | MapRecord
)
