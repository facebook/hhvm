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

import math
import unittest

from apache.thrift.protocol.detail.protocol_detail.thrift_types import Object, Value
from folly.iobuf import IOBuf
from thrift.lib.python.schema import syntax_graph as _ast
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
    InvalidRecordError,
    ListRecord,
    MapRecord,
    SetRecord,
    TextRecord,
)
from thrift.lib.python.schema.type_system_bridge import (
    _bridge_annotations,
    _record_from_annotation_value,
    _record_from_protocol_value,
)


# ---------------------------------------------------------------------------
# SerializableRecord construction / validation
# ---------------------------------------------------------------------------


class SerializableRecordValidationTest(unittest.TestCase):
    def test_float_rejects_nan(self) -> None:
        with self.assertRaises(InvalidRecordError):
            Float32Record(math.nan)
        with self.assertRaises(InvalidRecordError):
            Float64Record(math.nan)

    def test_float_rejects_negative_zero(self) -> None:
        with self.assertRaises(InvalidRecordError):
            Float32Record(-0.0)
        with self.assertRaises(InvalidRecordError):
            Float64Record(-0.0)

    def test_float_accepts_positive_zero_and_infinity(self) -> None:
        # +0.0 and +/-inf have deterministic bit patterns, so they are allowed.
        self.assertEqual(Float64Record(0.0).value, 0.0)
        self.assertEqual(Float64Record(math.inf).value, math.inf)
        self.assertEqual(Float64Record(-math.inf).value, -math.inf)

    def test_text_rejects_invalid_utf8(self) -> None:
        # A lone surrogate cannot be encoded as UTF-8.
        with self.assertRaises(InvalidRecordError):
            TextRecord("\ud800")

    def test_text_accepts_unicode(self) -> None:
        self.assertEqual(TextRecord("héllo→").value, "héllo→")

    def test_set_rejects_duplicate_elements(self) -> None:
        with self.assertRaises(InvalidRecordError):
            SetRecord([Int32Record(1), Int32Record(1)])
        # Distinct elements are fine.
        self.assertEqual(len(SetRecord([Int32Record(1), Int32Record(2)]).elements), 2)

    def test_map_rejects_duplicate_keys(self) -> None:
        with self.assertRaises(InvalidRecordError):
            MapRecord(
                [
                    (Int32Record(1), TextRecord("a")),
                    (Int32Record(1), TextRecord("b")),
                ]
            )
        # Distinct keys are fine (same value is allowed).
        self.assertEqual(
            len(
                MapRecord(
                    [
                        (Int32Record(1), TextRecord("a")),
                        (Int32Record(2), TextRecord("a")),
                    ]
                ).entries
            ),
            2,
        )

    def test_field_set_fields_is_read_only_view(self) -> None:
        record = FieldSetRecord({1: Int32Record(7)})
        # The accessor hands back a read-only view, not the backing dict.
        with self.assertRaises(TypeError):
            record.fields[2] = Int32Record(9)  # pyre-ignore[16]: read-only view
        self.assertEqual(dict(record.fields), {1: Int32Record(7)})


class SerializableRecordEqualityTest(unittest.TestCase):
    def test_int_width_distinguishes_records(self) -> None:
        # Same payload, different arm -> not equal.
        self.assertNotEqual(Int8Record(1), Int16Record(1))
        self.assertNotEqual(Int32Record(1), Int64Record(1))
        self.assertEqual(Int32Record(5), Int32Record(5))
        self.assertEqual(hash(Int32Record(5)), hash(Int32Record(5)))

    def test_float_width_distinguishes_records(self) -> None:
        self.assertNotEqual(Float32Record(1.5), Float64Record(1.5))

    def test_set_equality_is_order_independent(self) -> None:
        a = SetRecord([Int32Record(1), Int32Record(2)])
        b = SetRecord([Int32Record(2), Int32Record(1)])
        self.assertEqual(a, b)
        self.assertEqual(hash(a), hash(b))

    def test_map_equality_is_order_independent(self) -> None:
        a = MapRecord(
            [(Int32Record(1), TextRecord("a")), (Int32Record(2), TextRecord("b"))]
        )
        b = MapRecord(
            [(Int32Record(2), TextRecord("b")), (Int32Record(1), TextRecord("a"))]
        )
        self.assertEqual(a, b)
        self.assertEqual(hash(a), hash(b))

    def test_list_equality_is_order_sensitive(self) -> None:
        a = ListRecord([Int32Record(1), Int32Record(2)])
        b = ListRecord([Int32Record(2), Int32Record(1)])
        self.assertNotEqual(a, b)
        self.assertEqual(a, ListRecord([Int32Record(1), Int32Record(2)]))

    def test_field_set_equality_is_key_order_independent(self) -> None:
        a = FieldSetRecord({1: Int32Record(1), 2: TextRecord("x")})
        b = FieldSetRecord({2: TextRecord("x"), 1: Int32Record(1)})
        self.assertEqual(a, b)
        self.assertEqual(hash(a), hash(b))

    def test_nested_records_are_hashable(self) -> None:
        rec = MapRecord([(TextRecord("k"), ListRecord([SetRecord([Int32Record(1)])]))])
        # Usable as a set element / dict key (recursive structural hashing).
        self.assertIn(rec, {rec})


# ---------------------------------------------------------------------------
# Protocol Value -> SerializableRecord (the custom-default path, value-directed)
# ---------------------------------------------------------------------------


class ProtocolValueConversionTest(unittest.TestCase):
    def test_scalar_arms_map_to_matching_record(self) -> None:
        cases = [
            (Value(boolValue=True), BoolRecord(True)),
            (Value(byteValue=5), Int8Record(5)),
            (Value(i16Value=5), Int16Record(5)),
            (Value(i32Value=42), Int32Record(42)),
            (Value(i64Value=42), Int64Record(42)),
            (Value(floatValue=1.5), Float32Record(1.5)),
            (Value(doubleValue=2.5), Float64Record(2.5)),
            (Value(stringValue=b"hi"), TextRecord("hi")),
            # binaryValue is a ByteBuffer (IOBuf in thrift-python), not raw bytes.
            (Value(binaryValue=IOBuf(b"\x00\x01")), ByteArrayRecord(b"\x00\x01")),
        ]
        for value, expected in cases:
            with self.subTest(kind=type(expected).__name__):
                self.assertEqual(_record_from_protocol_value(value), expected)

    def test_list_value(self) -> None:
        value = Value(listValue=[Value(i32Value=1), Value(i32Value=2)])
        self.assertEqual(
            _record_from_protocol_value(value),
            ListRecord([Int32Record(1), Int32Record(2)]),
        )

    def test_map_value(self) -> None:
        value = Value(mapValue={Value(stringValue=b"k"): Value(i32Value=9)})
        self.assertEqual(
            _record_from_protocol_value(value),
            MapRecord([(TextRecord("k"), Int32Record(9))]),
        )

    def test_object_value_is_field_id_keyed(self) -> None:
        # protocol Object.members is map<i16, Value> -- already field-id keyed,
        # so it maps straight to a FieldSetRecord.
        value = Value(
            objectValue=Object(
                members={1: Value(i32Value=7), 2: Value(boolValue=False)}
            )
        )
        self.assertEqual(
            _record_from_protocol_value(value),
            FieldSetRecord({1: Int32Record(7), 2: BoolRecord(False)}),
        )


# ---------------------------------------------------------------------------
# Annotation value -> SerializableRecord (type-directed, name->id re-resolution)
# ---------------------------------------------------------------------------


def _field(field_id: int, name: str, type_ref: _ast.TypeRef) -> _ast.FieldNode:
    return _ast.FieldNode(
        id=field_id,
        name=name,
        type=type_ref,
        qualifier=_ast.FieldQualifier.Default,
        doc_block=None,
        annotations=[],
    )


def _annotation_struct(
    resolver: _ast._Resolver,
    *,
    uri: str,
    key: bytes,
    fields: list[_ast.FieldNode] | None = None,
) -> _ast.StructNode:
    node = _ast.StructNode(
        uri=uri,
        fields=fields or [],
        name=uri.rsplit("/", 1)[-1],
        definition_key=key,
        resolver=resolver,
        doc_block=None,
        annotations=[],
    )
    resolver.register(key, node)
    return node


class AnnotationValueConversionTest(unittest.TestCase):
    def test_names_re_resolved_to_field_ids(self) -> None:
        resolver = _ast._Resolver()
        anno = _annotation_struct(
            resolver,
            uri="test.com/RecordAnno",
            key=b"k-anno",
            fields=[
                _field(1, "count", _ast.PrimitiveTypeRef(_ast.Primitive.I32)),
                _field(2, "label", _ast.PrimitiveTypeRef(_ast.Primitive.STRING)),
            ],
        )
        # The decoded annotation value is keyed by field *name*.
        record = _record_from_annotation_value(
            anno.as_type(), {"count": 7, "label": "hi"}
        )
        # ...and is re-resolved to a field-*id* keyed FieldSetRecord.
        self.assertEqual(
            record, FieldSetRecord({1: Int32Record(7), 2: TextRecord("hi")})
        )

    def test_primitive_arm_chosen_by_declared_type(self) -> None:
        # The decoded payload (int 5) is wrapped using the declared field width.
        resolver = _ast._Resolver()
        anno = _annotation_struct(
            resolver,
            uri="test.com/Widths",
            key=b"k-widths",
            fields=[
                _field(1, "b", _ast.PrimitiveTypeRef(_ast.Primitive.BYTE)),
                _field(2, "s", _ast.PrimitiveTypeRef(_ast.Primitive.I16)),
                _field(3, "big", _ast.PrimitiveTypeRef(_ast.Primitive.I64)),
            ],
        )
        record = _record_from_annotation_value(
            anno.as_type(), {"b": 5, "s": 5, "big": 5}
        )
        self.assertEqual(
            record,
            FieldSetRecord({1: Int8Record(5), 2: Int16Record(5), 3: Int64Record(5)}),
        )

    def test_container_annotation_value(self) -> None:
        resolver = _ast._Resolver()
        anno = _annotation_struct(
            resolver,
            uri="test.com/WithList",
            key=b"k-list",
            fields=[
                _field(
                    1,
                    "nums",
                    _ast.ListTypeRef(_ast.PrimitiveTypeRef(_ast.Primitive.I32)),
                ),
            ],
        )
        record = _record_from_annotation_value(anno.as_type(), {"nums": [1, 2, 3]})
        self.assertEqual(
            record,
            FieldSetRecord(
                {1: ListRecord([Int32Record(1), Int32Record(2), Int32Record(3)])}
            ),
        )


class AnnotationFilteringTest(unittest.TestCase):
    def _annotation(
        self, resolver: _ast._Resolver, uri: str, key: bytes
    ) -> _ast.Annotation:
        struct = _annotation_struct(
            resolver,
            uri=uri,
            key=key,
            fields=[_field(1, "n", _ast.PrimitiveTypeRef(_ast.Primitive.I32))],
        )
        return _ast.Annotation(struct.as_type(), {"n": 1})

    def test_compiler_consumed_dropped_others_retained(self) -> None:
        resolver = _ast._Resolver()
        annotations = [
            # Exactly the Uri annotation -> dropped.
            self._annotation(resolver, "facebook.com/thrift/annotation/Uri", b"k-uri"),
            # Anything under the scope/ sub-namespace -> dropped.
            self._annotation(
                resolver, "facebook.com/thrift/annotation/scope/Struct", b"k-scope"
            ),
            # A sibling standard annotation that is NOT Uri/scope -> retained
            # (broader standard-annotation stripping is an export-time concern).
            self._annotation(resolver, "facebook.com/thrift/annotation/Cpp", b"k-cpp"),
            # A user-defined annotation -> retained.
            self._annotation(resolver, "test.com/Custom", b"k-custom"),
        ]
        result = _bridge_annotations(annotations)
        self.assertEqual(
            set(result),
            {"facebook.com/thrift/annotation/Cpp", "test.com/Custom"},
        )
        self.assertEqual(result["test.com/Custom"], FieldSetRecord({1: Int32Record(1)}))
