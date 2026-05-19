#!/usr/bin/env python3
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
from types import MappingProxyType
from typing import Any

from parameterized import parameterized
from test_thrift.thrift_types import (
    AnnotatedForReflection,
    Color,
    ComplexUnion,
    CrossModuleRef,
    Digits,
    easy,
    EmptyStruct,
    HardError,
    I32List,
    Integers,
    ListTypes,
    Messy,
    SetI32,
    SimpleStruct,
    StrI32ListMap,
    StructuredAnnotation,
)
from testing.sub_dependency.thrift_types import Basic as SubDepBasic
from thrift.python.reflection.constants_reflection import (
    ConstantListSpec,
    ConstantMapSpec,
    ConstantSpec,
    ConstantStructSpec,
    ThriftType,
)
from thrift.python.reflection.types_reflection import (
    inspect,
    inspectable,
    ListSpec,
    MapSpec,
    SetSpec,
    StructSpec,
)
from thrift.python.reflection_enums import NumberType, Qualifier, StructType
from thrift.python.types import (
    List as _fbthrift_List,
    Map as _fbthrift_Map,
    Set as _fbthrift_Set,
)


def _inspect_struct(cls: type[Any]) -> StructSpec:
    spec = inspect(cls)
    assert isinstance(spec, StructSpec)
    return spec


class InspectSimpleStructTest(unittest.TestCase):
    def test_returns_struct_spec(self) -> None:
        spec = inspect(SimpleStruct)
        # structured_annotations is immutable
        self.assertIsInstance(spec, StructSpec)

    def test_name(self) -> None:
        spec = _inspect_struct(SimpleStruct)
        self.assertEqual(spec.name, "SimpleStruct")

    def test_kind(self) -> None:
        spec = _inspect_struct(SimpleStruct)
        self.assertEqual(spec.kind, StructType.STRUCT)

    def test_field_count(self) -> None:
        spec = _inspect_struct(SimpleStruct)
        self.assertEqual(len(spec.fields), 3)

    def test_field_name(self) -> None:
        spec = _inspect_struct(SimpleStruct)
        fields_by_name = {f.name: f for f in spec.fields}

        f = fields_by_name["name"]
        self.assertEqual(f.id, 1)
        self.assertEqual(f.py_name, "name")
        self.assertEqual(f.type, str)
        self.assertEqual(f.thrift_type, ThriftType.STRING)
        self.assertEqual(f.qualifier, Qualifier.UNQUALIFIED)

    def test_field_value(self) -> None:
        spec = _inspect_struct(SimpleStruct)
        fields_by_name = {f.name: f for f in spec.fields}

        f = fields_by_name["value"]
        self.assertEqual(f.id, 2)
        self.assertEqual(f.py_name, "value")
        self.assertEqual(f.type, int)
        self.assertEqual(f.thrift_type, ThriftType.I32)
        self.assertEqual(f.qualifier, Qualifier.UNQUALIFIED)

    def test_field_city(self) -> None:
        spec = _inspect_struct(SimpleStruct)
        fields_by_name = {f.name: f for f in spec.fields}

        f = fields_by_name["city"]
        self.assertEqual(f.id, 3)
        self.assertEqual(f.py_name, "city")
        self.assertEqual(f.type, str)
        self.assertEqual(f.thrift_type, ThriftType.STRING)
        self.assertEqual(f.qualifier, Qualifier.UNQUALIFIED)

    def test_fields_ordered_by_id(self) -> None:
        spec = _inspect_struct(SimpleStruct)
        ids = [f.id for f in spec.fields]
        self.assertEqual(ids, sorted(ids))

    def test_inspect_instance(self) -> None:
        instance = SimpleStruct(name="test", value=42, city="NYC")
        spec = _inspect_struct(type(instance))
        self.assertEqual(spec.name, "SimpleStruct")
        self.assertEqual(spec.kind, StructType.STRUCT)

    def test_inspect_caches(self) -> None:
        spec1 = inspect(SimpleStruct)
        spec2 = inspect(SimpleStruct)
        self.assertIs(spec1, spec2)

    def test_struct_spec_immutable(self) -> None:
        spec = _inspect_struct(SimpleStruct)
        with self.assertRaises(AttributeError):
            spec.name = "T"  # type: ignore[misc]

    def test_field_spec_immutable(self) -> None:
        spec = _inspect_struct(SimpleStruct)
        field = spec.fields[0]
        with self.assertRaises(AttributeError):
            field.name = "y"  # type: ignore[misc]

    def test_struct_spec_iter(self) -> None:
        name, fields, kind, annotations = inspect(SimpleStruct)
        self.assertEqual(name, "SimpleStruct")
        self.assertIsInstance(fields, tuple)
        self.assertEqual(kind, StructType.STRUCT)
        self.assertIsInstance(annotations, dict)

    def test_field_spec_iter(self) -> None:
        spec = _inspect_struct(SimpleStruct)
        field = next(f for f in spec.fields if f.name == "value")
        fid, name, typ, kind, qualifier, default, annotations = field
        self.assertEqual(fid, 2)
        self.assertEqual(name, "value")
        self.assertEqual(typ, int)
        self.assertEqual(kind, NumberType.I32)
        self.assertEqual(qualifier, Qualifier.UNQUALIFIED)
        self.assertIsInstance(annotations, dict)

    def test_fields_returns_tuple(self) -> None:
        spec = _inspect_struct(SimpleStruct)
        self.assertIsInstance(spec.fields, tuple)

    def test_annotations_empty_without_structured_annotations(self) -> None:
        spec = _inspect_struct(SimpleStruct)
        self.assertEqual(spec.annotations, {})
        for f in spec.fields:
            self.assertEqual(f.annotations, {})


class InspectExceptionTest(unittest.TestCase):
    def test_kind_is_exception(self) -> None:
        spec = _inspect_struct(HardError)
        self.assertEqual(spec.kind, StructType.EXCEPTION)

    def test_name(self) -> None:
        spec = _inspect_struct(HardError)
        self.assertEqual(spec.name, "HardError")

    def test_field_count(self) -> None:
        spec = _inspect_struct(HardError)
        self.assertEqual(len(spec.fields), 2)

    def test_fields(self) -> None:
        spec = _inspect_struct(HardError)
        fields_by_name = {f.name: f for f in spec.fields}

        errortext = fields_by_name["errortext"]
        self.assertEqual(errortext.id, 1)
        self.assertEqual(errortext.type, str)
        self.assertEqual(errortext.thrift_type, ThriftType.STRING)

        code = fields_by_name["code"]
        self.assertEqual(code.id, 2)
        self.assertEqual(code.type, int)
        self.assertEqual(code.thrift_type, ThriftType.I32)

    def test_inspect_instance(self) -> None:
        err = HardError(errortext="boom", code=500)
        spec = _inspect_struct(type(err))
        self.assertEqual(spec.kind, StructType.EXCEPTION)


class InspectUnionTest(unittest.TestCase):
    def test_kind_is_union(self) -> None:
        spec = _inspect_struct(Integers)
        self.assertEqual(spec.kind, StructType.UNION)

    def test_name(self) -> None:
        spec = _inspect_struct(Integers)
        self.assertEqual(spec.name, "Integers")

    def test_field_count(self) -> None:
        spec = _inspect_struct(Integers)
        self.assertEqual(len(spec.fields), 7)

    def test_byte_field(self) -> None:
        spec = _inspect_struct(Integers)
        fields_by_name = {f.name: f for f in spec.fields}
        f = fields_by_name["tiny"]
        self.assertEqual(f.id, 1)
        self.assertEqual(f.type, int)
        self.assertEqual(f.thrift_type, ThriftType.BYTE)

    def test_i16_field(self) -> None:
        spec = _inspect_struct(Integers)
        fields_by_name = {f.name: f for f in spec.fields}
        f = fields_by_name["small"]
        self.assertEqual(f.id, 2)
        self.assertEqual(f.type, int)
        self.assertEqual(f.thrift_type, ThriftType.I16)

    def test_i32_field(self) -> None:
        spec = _inspect_struct(Integers)
        fields_by_name = {f.name: f for f in spec.fields}
        f = fields_by_name["medium"]
        self.assertEqual(f.id, 3)
        self.assertEqual(f.type, int)
        self.assertEqual(f.thrift_type, ThriftType.I32)

    def test_i64_field(self) -> None:
        spec = _inspect_struct(Integers)
        fields_by_name = {f.name: f for f in spec.fields}
        f = fields_by_name["large"]
        self.assertEqual(f.id, 4)
        self.assertEqual(f.type, int)
        self.assertEqual(f.thrift_type, ThriftType.I64)

    def test_string_field(self) -> None:
        spec = _inspect_struct(Integers)
        fields_by_name = {f.name: f for f in spec.fields}
        f = fields_by_name["unbounded"]
        self.assertEqual(f.id, 5)
        self.assertEqual(f.type, str)
        self.assertEqual(f.thrift_type, ThriftType.STRING)

    def test_renamed_field(self) -> None:
        spec = _inspect_struct(Integers)
        fields_by_name = {f.name: f for f in spec.fields}
        f = fields_by_name["name"]
        self.assertEqual(f.id, 6)
        self.assertEqual(f.name, "name")
        self.assertEqual(f.py_name, "name_")
        self.assertEqual(f.type, str)
        self.assertEqual(f.thrift_type, ThriftType.STRING)

    def test_struct_typed_field(self) -> None:
        spec = _inspect_struct(Integers)
        fields_by_name = {f.name: f for f in spec.fields}
        f = fields_by_name["digits"]
        self.assertEqual(f.id, 7)
        self.assertEqual(f.type, Digits)
        self.assertEqual(f.thrift_type, ThriftType.STRUCT)

    def test_fields_ordered_by_id(self) -> None:
        spec = _inspect_struct(Integers)
        ids = [f.id for f in spec.fields]
        self.assertEqual(ids, sorted(ids))


class InspectEmptyStructTest(unittest.TestCase):
    def test_empty_struct(self) -> None:
        spec = _inspect_struct(EmptyStruct)
        self.assertEqual(spec.name, "EmptyStruct")
        self.assertEqual(spec.kind, StructType.STRUCT)
        self.assertEqual(len(spec.fields), 0)


class InspectableTest(unittest.TestCase):
    def test_inspectable_struct(self) -> None:
        self.assertTrue(inspectable(SimpleStruct))

    def test_inspectable_instance(self) -> None:
        self.assertTrue(inspectable(SimpleStruct()))

    def test_inspectable_union(self) -> None:
        self.assertTrue(inspectable(Integers))

    def test_inspectable_exception(self) -> None:
        self.assertTrue(inspectable(HardError))

    def test_not_inspectable_builtin(self) -> None:
        self.assertFalse(inspectable(int))
        self.assertFalse(inspectable(42))
        self.assertFalse(inspectable(None))

    def test_inspect_raises_for_non_thrift(self) -> None:
        with self.assertRaises(TypeError):
            inspect(int)  # pyre-fixme[6]: intentionally invalid


class NumberTypeEnumTest(unittest.TestCase):
    def test_i08_is_alias_for_byte(self) -> None:
        self.assertIs(NumberType.I08, NumberType.BYTE)

    def test_values(self) -> None:
        self.assertEqual(NumberType.NOT_A_NUMBER.value, 0)
        self.assertEqual(NumberType.BYTE.value, 1)
        self.assertEqual(NumberType.I16.value, 2)
        self.assertEqual(NumberType.I32.value, 3)
        self.assertEqual(NumberType.I64.value, 4)
        self.assertEqual(NumberType.FLOAT.value, 5)
        self.assertEqual(NumberType.DOUBLE.value, 6)


class InspectFieldKindTest(unittest.TestCase):
    @parameterized.expand(
        [
            ("byte", Integers, "tiny", NumberType.BYTE),
            ("i16", Integers, "small", NumberType.I16),
            ("i32", Integers, "medium", NumberType.I32),
            ("i64", Integers, "large", NumberType.I64),
            ("float", ComplexUnion, "float_val", NumberType.FLOAT),
            ("double", ComplexUnion, "double_val", NumberType.DOUBLE),
            ("string", Integers, "unbounded", NumberType.NOT_A_NUMBER),
            ("struct", Integers, "digits", NumberType.NOT_A_NUMBER),
            ("enum", ComplexUnion, "color", NumberType.NOT_A_NUMBER),
            ("bool", ComplexUnion, "truthy", NumberType.NOT_A_NUMBER),
        ]
    )
    def test_field_kind(
        self, _name: str, cls: type[Any], field_name: str, expected: NumberType
    ) -> None:
        spec = _inspect_struct(cls)
        fields_by_name = {f.name: f for f in spec.fields}
        self.assertEqual(fields_by_name[field_name].kind, expected)

    def test_optional_qualifier(self) -> None:
        spec = _inspect_struct(easy)
        fields_by_name = {f.name: f for f in spec.fields}
        self.assertEqual(fields_by_name["name"].qualifier, Qualifier.OPTIONAL)

    @parameterized.expand(
        [
            ("bool", ComplexUnion, "truthy", bool, ThriftType.BOOL),
            ("float", ComplexUnion, "float_val", float, ThriftType.FLOAT),
            ("double", ComplexUnion, "double_val", float, ThriftType.DOUBLE),
            ("binary", ComplexUnion, "raw", bytes, ThriftType.BINARY),
            ("enum", ComplexUnion, "color", Color, ThriftType.ENUM),
        ]
    )
    def test_field_type(
        self,
        _name: str,
        cls: type[Any],
        field_name: str,
        expected_type: type[Any],
        expected_thrift_type: ThriftType,
    ) -> None:
        spec = _inspect_struct(cls)
        fields_by_name = {f.name: f for f in spec.fields}
        f = fields_by_name[field_name]
        self.assertEqual(f.type, expected_type)
        self.assertEqual(f.thrift_type, expected_thrift_type)

    def test_container_typedef_field_uses_typedef_class(self) -> None:
        spec = _inspect_struct(easy)
        fields_by_name = {f.name: f for f in spec.fields}
        self.assertIs(fields_by_name["val_list"].type, I32List)

    @parameterized.expand(
        [
            (
                "list",
                "float_list",
                _fbthrift_List,
                "List__ComplexUnion_float_list",
                ThriftType.LIST,
            ),
            (
                "set",
                "float_set",
                _fbthrift_Set,
                "Set__ComplexUnion_float_set",
                ThriftType.SET,
            ),
            (
                "map",
                "float_map",
                _fbthrift_Map,
                "Map__ComplexUnion_float_map",
                ThriftType.MAP,
            ),
        ]
    )
    def test_raw_container_field(
        self,
        _name: str,
        field_name: str,
        base_cls: type[Any],
        expected_type_name: str,
        expected_thrift_type: ThriftType,
    ) -> None:
        spec = _inspect_struct(ComplexUnion)
        fields_by_name = {f.name: f for f in spec.fields}
        f = fields_by_name[field_name]
        self.assertTrue(issubclass(f.type, base_cls))
        self.assertEqual(f.type.__name__, expected_type_name)
        self.assertEqual(f.type.__module__, "test_thrift.thrift_types")
        self.assertEqual(f.thrift_type, expected_thrift_type)
        instance = f.type()
        self.assertEqual(len(instance), 0)

    def test_nested_container_inspect_returns_list_spec(self) -> None:
        spec = _inspect_struct(ListTypes)
        fields_by_name = {f.name: f for f in spec.fields}
        outer_type = fields_by_name["third"].type
        outer_spec = inspect(outer_type)
        self.assertIsInstance(outer_spec, ListSpec)

    def test_nested_container_inner_type_is_default_constructible(self) -> None:
        spec = _inspect_struct(ListTypes)
        fields_by_name = {f.name: f for f in spec.fields}
        outer_type = fields_by_name["third"].type
        outer_spec = inspect(outer_type)
        assert isinstance(outer_spec, ListSpec)
        self.assertEqual(outer_spec.thrift_type, ThriftType.LIST)
        inner_type = outer_spec.value
        self.assertTrue(issubclass(inner_type, _fbthrift_List))
        inner_instance = inner_type()
        self.assertEqual(len(inner_instance), 0)

    def test_container_typedef_thrift_type(self) -> None:
        spec = _inspect_struct(easy)
        fields_by_name = {f.name: f for f in spec.fields}
        self.assertEqual(fields_by_name["val_list"].thrift_type, ThriftType.LIST)

    def test_cross_module_field_type(self) -> None:
        spec = _inspect_struct(CrossModuleRef)
        fields_by_name = {f.name: f for f in spec.fields}
        f = fields_by_name["basic"]
        self.assertIs(f.type, SubDepBasic)
        self.assertEqual(f.thrift_type, ThriftType.STRUCT)


class InspectContainerTypedefTest(unittest.TestCase):
    def test_inspect_list_typedef(self) -> None:
        spec = inspect(I32List)
        self.assertIsInstance(spec, ListSpec)
        self.assertEqual(spec.value, int)
        self.assertEqual(spec.thrift_type, ThriftType.I32)
        self.assertEqual(spec.kind, NumberType.I32)
        value, kind = spec
        self.assertEqual(value, int)
        self.assertEqual(kind, NumberType.I32)

    def test_inspect_list_typedef_instance(self) -> None:
        spec = inspect(I32List([1, 2, 3]))
        self.assertIsInstance(spec, ListSpec)
        self.assertEqual(spec.value, int)

    def test_inspect_set_typedef(self) -> None:
        spec = inspect(SetI32)
        self.assertIsInstance(spec, SetSpec)
        self.assertEqual(spec.value, int)
        self.assertEqual(spec.thrift_type, ThriftType.I32)
        self.assertEqual(spec.kind, NumberType.I32)
        value, kind = spec
        self.assertEqual(value, int)
        self.assertEqual(kind, NumberType.I32)

    def test_inspect_map_typedef(self) -> None:
        spec = inspect(StrI32ListMap)
        assert isinstance(spec, MapSpec)
        self.assertEqual(spec.key, str)
        self.assertEqual(spec.key_thrift_type, ThriftType.STRING)
        self.assertEqual(spec.key_kind, NumberType.NOT_A_NUMBER)
        self.assertTrue(issubclass(spec.value, _fbthrift_List))
        self.assertEqual(spec.value_thrift_type, ThriftType.LIST)
        self.assertEqual(spec.value_kind, NumberType.NOT_A_NUMBER)
        inner = inspect(spec.value)
        self.assertIsInstance(inner, ListSpec)
        self.assertEqual(inner.value, int)
        self.assertEqual(inner.thrift_type, ThriftType.I32)
        self.assertEqual(inner.kind, NumberType.I32)
        key, key_kind, value, value_kind = spec
        self.assertEqual(key, str)
        self.assertEqual(key_kind, NumberType.NOT_A_NUMBER)
        self.assertTrue(issubclass(value, _fbthrift_List))
        self.assertEqual(value_kind, NumberType.NOT_A_NUMBER)

    def test_inspect_caches_container(self) -> None:
        spec1 = inspect(I32List)
        spec2 = inspect(I32List)
        self.assertIs(spec1, spec2)

    def test_inspect_container_field_value(self) -> None:
        val_list = easy().val_list
        spec = inspect(val_list)
        self.assertIsInstance(spec, ListSpec)
        self.assertEqual(spec.value, int)
        self.assertEqual(spec.thrift_type, ThriftType.I32)

    def test_inspectable_container(self) -> None:
        self.assertTrue(inspectable(I32List))
        self.assertTrue(inspectable(SetI32))
        self.assertTrue(inspectable(StrI32ListMap))
        self.assertTrue(inspectable(I32List()))
        self.assertTrue(inspectable(easy().val_list))


class InspectAnnotationsTest(unittest.TestCase):
    def test_no_annotations(self) -> None:
        spec = _inspect_struct(SimpleStruct)
        self.assertEqual(spec.annotations, {})
        for f in spec.fields:
            self.assertEqual(f.annotations, {})

    def test_struct_deprecated_annotations(self) -> None:
        spec = _inspect_struct(easy)
        self.assertEqual(spec.annotations, {"anno1": "foo", "bar": "1"})

    def test_field_deprecated_annotations(self) -> None:
        spec = _inspect_struct(Messy)
        fields_by_name = {f.name: f for f in spec.fields}
        self.assertEqual(
            fields_by_name["opt_field"].annotations,
            {"a.b.c": "d.e.f", "some": "annotation"},
        )

    def test_mixed_deprecated_and_custom(self) -> None:
        spec = _inspect_struct(AnnotatedForReflection)
        annotations = spec.annotations
        self.assertEqual(annotations["depr_key"], "depr_val")
        self.assertIn("test_thrift.StructuredAnnotation", annotations)
        sa = annotations["test_thrift.StructuredAnnotation"]
        assert isinstance(sa, dict)
        self.assertEqual(sa["second"], 42)

    def test_field_annotation(self) -> None:
        spec = _inspect_struct(AnnotatedForReflection)
        fields_by_name = {f.name: f for f in spec.fields}
        self.assertEqual(
            fields_by_name["annotated_field"].annotations,
            {"field_key": "field_val"},
        )
        self.assertEqual(fields_by_name["plain_field"].annotations, {})

    def test_structured_annotations_raw(self) -> None:
        spec = _inspect_struct(AnnotatedForReflection)
        raw = spec.structured_annotations
        self.assertIsInstance(raw, MappingProxyType)
        self.assertIn("test_thrift.StructuredAnnotation", raw)
        sa_spec = raw["test_thrift.StructuredAnnotation"]
        self.assertIsInstance(sa_spec, ConstantSpec)
        self.assertEqual(sa_spec.thrift_type, ThriftType.STRUCT)
        sa_value = sa_spec.value
        assert isinstance(sa_value, ConstantStructSpec)
        self.assertIs(sa_value.struct_type, StructuredAnnotation)
        self.assertIn("second", sa_value.fields)
        second_spec = sa_value.fields["second"]
        self.assertIsInstance(second_spec, ConstantSpec)
        self.assertEqual(second_spec.value, 42)
        self.assertEqual(second_spec.thrift_type, ThriftType.I64)

    def test_annotation_with_list_value(self) -> None:
        spec = _inspect_struct(AnnotatedForReflection)
        raw = spec.structured_annotations
        sa_value = raw["test_thrift.StructuredAnnotation"].value
        assert isinstance(sa_value, ConstantStructSpec)
        third_spec = sa_value.fields["third"]
        self.assertEqual(third_spec.thrift_type, ThriftType.LIST)
        third_value = third_spec.value
        assert isinstance(third_value, ConstantListSpec)
        self.assertEqual(len(third_value.value), 2)
        self.assertEqual(third_value.value[0].value, "hello")
        self.assertEqual(third_value.value[0].thrift_type, ThriftType.STRING)
        self.assertEqual(third_value.value[1].value, "world")

    def test_annotation_i32_and_float_thrift_types(self) -> None:
        spec = _inspect_struct(AnnotatedForReflection)
        raw = spec.structured_annotations
        self.assertIn("test_thrift.ReflectionAnnotation", raw)
        ra_value = raw["test_thrift.ReflectionAnnotation"].value
        assert isinstance(ra_value, ConstantStructSpec)
        count_spec = ra_value.fields["count"]
        self.assertEqual(count_spec.value, 7)
        self.assertEqual(count_spec.thrift_type, ThriftType.I32)
        ratio_spec = ra_value.fields["ratio"]
        assert isinstance(ratio_spec.value, float)
        self.assertAlmostEqual(ratio_spec.value, 0.5)
        self.assertEqual(ratio_spec.thrift_type, ThriftType.FLOAT)


class HashContractTest(unittest.TestCase):
    def test_struct_spec_hash_invariant_with_different_insertion_order(self) -> None:
        class FakeStruct:
            pass

        fields_a = {
            "name": ConstantSpec(value="test", thrift_type=ThriftType.STRING),
            "id": ConstantSpec(value=123, thrift_type=ThriftType.I32),
            "flag": ConstantSpec(value=True, thrift_type=ThriftType.BOOL),
        }
        fields_b = {
            "flag": ConstantSpec(value=True, thrift_type=ThriftType.BOOL),
            "id": ConstantSpec(value=123, thrift_type=ThriftType.I32),
            "name": ConstantSpec(value="test", thrift_type=ThriftType.STRING),
        }
        a = ConstantStructSpec(
            struct_type=FakeStruct,  # pyre-fixme[6]
            fields=fields_a,
        )
        b = ConstantStructSpec(
            struct_type=FakeStruct,  # pyre-fixme[6]
            fields=fields_b,
        )
        self.assertEqual(a, b)
        self.assertEqual(hash(a), hash(b))
        self.assertEqual(len({a, b}), 1)

    def test_map_spec_hash_invariant_with_different_insertion_order(self) -> None:
        k1 = ConstantSpec(value="x", thrift_type=ThriftType.STRING)
        v1 = ConstantSpec(value=1, thrift_type=ThriftType.I32)
        k2 = ConstantSpec(value="y", thrift_type=ThriftType.STRING)
        v2 = ConstantSpec(value=2, thrift_type=ThriftType.I32)
        k3 = ConstantSpec(value="z", thrift_type=ThriftType.STRING)
        v3 = ConstantSpec(value=3, thrift_type=ThriftType.I32)

        a = ConstantMapSpec(value={k1: v1, k2: v2, k3: v3})
        b = ConstantMapSpec(value={k3: v3, k1: v1, k2: v2})
        self.assertEqual(a, b)
        self.assertEqual(hash(a), hash(b))
        self.assertEqual(len({a, b}), 1)


class SpecImmutabilityTest(unittest.TestCase):
    def test_constant_spec_immutable(self) -> None:
        spec = ConstantSpec(value=42, thrift_type=ThriftType.I32)
        with self.assertRaises(AttributeError):
            spec.value = 99  # type: ignore[misc]

    def test_list_spec_immutable(self) -> None:
        spec = ListSpec(value=int, thrift_type=ThriftType.I32)
        with self.assertRaises(AttributeError):
            spec.value = str  # type: ignore[misc]

    def test_map_spec_immutable(self) -> None:
        spec = MapSpec(
            key=str,
            key_thrift_type=ThriftType.STRING,
            value=int,
            value_thrift_type=ThriftType.I64,
        )
        with self.assertRaises(AttributeError):
            spec.key = int  # type: ignore[misc]

    def test_constant_spec_hashable(self) -> None:
        a = ConstantSpec(value=42, thrift_type=ThriftType.I32)
        b = ConstantSpec(value=42, thrift_type=ThriftType.I32)
        self.assertEqual(hash(a), hash(b))
        s = {a, b}
        self.assertEqual(len(s), 1)
