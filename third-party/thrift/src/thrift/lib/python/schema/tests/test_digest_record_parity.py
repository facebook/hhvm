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

"""Parity tests guaranteeing the **runtime** digest's native leaf hashers emit
byte-identical bytes to the **serialized** digest's wire leaf hashers."""

import unittest
from collections.abc import Callable
from typing import Any

from thrift.lib.python.schema._digest_common import _Hasher, DigestMode
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
from thrift.lib.python.schema._serializable import (
    to_type_id,
    to_wire_annotations,
    to_wire_record,
)

# Native runtime hashers (under test), now in the wire-free runtime_digest module.
from thrift.lib.python.schema.runtime_digest import (
    _hash_annotations_native,
    _hash_record_native,
    _hash_type_id_native,
)
from thrift.lib.python.schema.type_system import (
    ListTypeRef,
    MapTypeRef,
    Primitive,
    PrimitiveTypeRef,
    SetTypeRef,
    StructNode,
    StructTypeRef,
    TypeRef,
)

# Their wire counterparts (the oracle), in the serialized digest module.
from thrift.lib.python.schema.type_system_digest import (
    _hash_annotations,
    _hash_record,
    _hash_type_id,
)

# Every native SerializableRecord arm, including nesting, empties, and float /
# byte-array / text edge cases.
_RECORDS: list[SerializableRecord] = [
    BoolRecord(True),
    BoolRecord(False),
    Int8Record(-128),
    Int8Record(127),
    Int16Record(-32768),
    Int16Record(12345),
    Int32Record(0),
    Int32Record(-2147483648),
    Int64Record(2**62),
    Float32Record(1.5),
    Float32Record(float("inf")),
    Float64Record(3.141592653589793),
    Float64Record(-2.0),
    TextRecord(""),
    TextRecord("héllo 世界"),
    ByteArrayRecord(b""),
    ByteArrayRecord(b"\x00\x01\xff\xfe"),
    FieldSetRecord({}),
    FieldSetRecord({1: Int32Record(5), 2: TextRecord("x"), 7: BoolRecord(True)}),
    ListRecord([]),
    ListRecord([Int32Record(1), Int32Record(2), Int32Record(3)]),
    SetRecord([]),
    SetRecord([Int32Record(1), Int32Record(2)]),
    MapRecord([]),
    MapRecord([(TextRecord("a"), Int32Record(1)), (TextRecord("b"), Int32Record(2))]),
    # Deeply nested: field-set of list-of-set and map.
    FieldSetRecord(
        {
            1: ListRecord([SetRecord([Int32Record(1), Int32Record(2)])]),
            2: MapRecord([(Int32Record(1), TextRecord("v"))]),
        }
    ),
]


def _user_node() -> StructNode:
    return StructNode(uri="meta.com/test/Foo")


# Representative TypeRefs: every primitive, nested containers, and a user-defined
# edge (which collapses to the userDefinedType URI arm).
_TYPE_REFS: list[TypeRef] = [
    *[PrimitiveTypeRef(p) for p in Primitive],
    ListTypeRef(PrimitiveTypeRef(Primitive.I32)),
    SetTypeRef(PrimitiveTypeRef(Primitive.STRING)),
    MapTypeRef(PrimitiveTypeRef(Primitive.I32), PrimitiveTypeRef(Primitive.STRING)),
    ListTypeRef(
        MapTypeRef(
            PrimitiveTypeRef(Primitive.STRING),
            SetTypeRef(PrimitiveTypeRef(Primitive.I32)),
        )
    ),
    StructTypeRef(_user_node()),
]

_STANDARD_ANNO = "facebook.com/thrift/annotation/Cpp"
_NON_STANDARD_ANNO = "meta.com/test/Keep"


def _run(
    hash_fn: Callable[[_Hasher, Any], None],
    value: Any,
    mode: DigestMode = DigestMode.FULL,
) -> bytes:
    h = _Hasher(mode)
    hash_fn(h, value)
    return h.finalize()


class RecordHashParityTest(unittest.TestCase):
    def test_record_hashers_agree(self) -> None:
        for record in _RECORDS:
            with self.subTest(record=repr(record)):
                self.assertEqual(
                    _run(_hash_record_native, record),
                    _run(_hash_record, to_wire_record(record)),
                )

    def test_distinct_records_distinct_native_digests(self) -> None:
        # Guards against a degenerate native hasher that ignores its input: every
        # distinct record must hash distinctly (the discriminant + payload).
        digests = {_run(_hash_record_native, r) for r in _RECORDS}
        self.assertEqual(len(digests), len(_RECORDS))

    def test_int_width_distinguished_natively(self) -> None:
        # Same payload, different width arm -> different digest (the id-4 gap and
        # per-arm field id must be honored natively).
        self.assertNotEqual(
            _run(_hash_record_native, Int32Record(1)),
            _run(_hash_record_native, Int64Record(1)),
        )


class TypeIdHashParityTest(unittest.TestCase):
    def test_type_id_hashers_agree(self) -> None:
        for type_ref in _TYPE_REFS:
            with self.subTest(type_ref=repr(type_ref)):
                self.assertEqual(
                    _run(_hash_type_id_native, type_ref),
                    _run(_hash_type_id, to_type_id(type_ref)),
                )

    def test_distinct_type_refs_distinct_native_digests(self) -> None:
        digests = {_run(_hash_type_id_native, r) for r in _TYPE_REFS}
        self.assertEqual(len(digests), len(_TYPE_REFS))


class AnnotationHashParityTest(unittest.TestCase):
    def _maps(self) -> list[dict[str, SerializableRecord]]:
        return [
            {},
            {_NON_STANDARD_ANNO: FieldSetRecord({1: Int32Record(7)})},
            # Standard annotation must be dropped by BOTH paths; non-standard kept.
            {
                _STANDARD_ANNO: FieldSetRecord({1: Int32Record(1)}),
                _NON_STANDARD_ANNO: FieldSetRecord({}),
            },
        ]

    def test_annotation_hashers_agree_full(self) -> None:
        for anns in self._maps():
            with self.subTest(anns=sorted(anns)):
                self.assertEqual(
                    _run(_hash_annotations_native, anns),
                    _run(_hash_annotations, to_wire_annotations(anns)),
                )

    def test_standard_annotation_dropped_natively(self) -> None:
        only_standard: dict[str, SerializableRecord] = {
            _STANDARD_ANNO: FieldSetRecord({1: Int32Record(1)})
        }
        # Dropping the lone standard annotation natively must equal hashing an
        # empty annotation map.
        self.assertEqual(
            _run(_hash_annotations_native, only_standard),
            _run(_hash_annotations_native, {}),
        )

    def test_structural_mode_emits_nothing_natively(self) -> None:
        anns: dict[str, SerializableRecord] = {
            _NON_STANDARD_ANNO: FieldSetRecord({1: Int32Record(7)})
        }
        # STRUCTURAL emits nothing for annotations -- not even a count prefix --
        # so a fresh hasher's digest is unchanged, and it matches the wire path.
        empty = _Hasher(DigestMode.STRUCTURAL).finalize()
        self.assertEqual(
            _run(_hash_annotations_native, anns, DigestMode.STRUCTURAL), empty
        )
        self.assertEqual(
            _run(_hash_annotations_native, anns, DigestMode.STRUCTURAL),
            _run(_hash_annotations, to_wire_annotations(anns), DigestMode.STRUCTURAL),
        )
