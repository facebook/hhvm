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

from thrift.python.reflection.constants_reflection import (
    ConstantEnumSpec,
    ConstantListSpec,
    ConstantMapSpec,
    ConstantSetSpec,
    ConstantSpec,
    ConstantStructSpec,
    ConstantUnionSpec,
    ThriftType,
)
from thrift.python.reflection.types_reflection import (
    FieldSpec,
    inspect,
    inspectable,
    ListSpec,
    MapSpec,
    SetSpec,
    StructSpec,
)
from thrift.python.reflection_enums import NumberType, Qualifier, StructType


class ConstantSpecTest(unittest.TestCase):
    def test_primitive_bool(self) -> None:
        spec = ConstantSpec(value=True, thrift_type=ThriftType.BOOL)
        self.assertEqual(spec.value, True)
        self.assertEqual(spec.thrift_type, ThriftType.BOOL)

    def test_primitive_int(self) -> None:
        spec = ConstantSpec(value=42, thrift_type=ThriftType.I32)
        self.assertEqual(spec.value, 42)
        self.assertEqual(spec.thrift_type, ThriftType.I32)

    def test_primitive_string(self) -> None:
        spec = ConstantSpec(value="hello", thrift_type=ThriftType.STRING)
        self.assertEqual(spec.value, "hello")
        self.assertEqual(spec.thrift_type, ThriftType.STRING)

    def test_list_constant(self) -> None:
        list_val = ConstantListSpec(
            value=[
                ConstantSpec(value=1, thrift_type=ThriftType.I32),
                ConstantSpec(value=2, thrift_type=ThriftType.I32),
            ]
        )
        spec = ConstantSpec(value=list_val, thrift_type=ThriftType.LIST)
        val = spec.value
        assert isinstance(val, ConstantListSpec)
        self.assertEqual(len(val.value), 2)
        self.assertEqual(val.value[0].value, 1)

    def test_set_constant(self) -> None:
        set_val = ConstantSetSpec(
            value=[
                ConstantSpec(value="a", thrift_type=ThriftType.STRING),
                ConstantSpec(value="b", thrift_type=ThriftType.STRING),
            ]
        )
        spec = ConstantSpec(value=set_val, thrift_type=ThriftType.SET)
        val = spec.value
        assert isinstance(val, ConstantSetSpec)
        self.assertEqual(len(val.value), 2)
        self.assertIsInstance(val.value, frozenset)

    def test_map_constant(self) -> None:
        key_spec = ConstantSpec(value="key", thrift_type=ThriftType.STRING)
        val_spec = ConstantSpec(value=42, thrift_type=ThriftType.I32)
        map_val = ConstantMapSpec(value={key_spec: val_spec})
        spec = ConstantSpec(value=map_val, thrift_type=ThriftType.MAP)
        val = spec.value
        assert isinstance(val, ConstantMapSpec)
        self.assertEqual(len(val.value), 1)
        self.assertEqual(val.value[key_spec], val_spec)
        self.assertIsInstance(val.value, MappingProxyType)

    def test_struct_constant(self) -> None:
        class FakeStruct:
            pass

        struct_val = ConstantStructSpec(
            struct_type=FakeStruct,  # pyre-fixme[6]
            fields={
                "name": ConstantSpec(value="test", thrift_type=ThriftType.STRING),
                "id": ConstantSpec(value=123, thrift_type=ThriftType.I32),
            },
        )
        spec = ConstantSpec(value=struct_val, thrift_type=ThriftType.STRUCT)
        val = spec.value
        assert isinstance(val, ConstantStructSpec)
        self.assertEqual(val.struct_type, FakeStruct)
        self.assertEqual(val.fields["name"].value, "test")
        self.assertEqual(val.fields["id"].value, 123)
        self.assertIsInstance(val.fields, MappingProxyType)

    def test_union_constant(self) -> None:
        class FakeUnion:
            pass

        active_value = ConstantSpec(value=97, thrift_type=ThriftType.I32)
        union_val = ConstantUnionSpec(
            union_type=FakeUnion,  # pyre-fixme[6]
            field="i",
            value=active_value,
        )
        spec = ConstantSpec(value=union_val, thrift_type=ThriftType.UNION)
        val = spec.value
        assert isinstance(val, ConstantUnionSpec)
        self.assertEqual(val.union_type, FakeUnion)
        self.assertEqual(val.field, "i")
        assert val.value is not None
        self.assertEqual(val.value.value, 97)

    def test_union_constant_empty(self) -> None:
        class FakeUnion:
            pass

        union_val = ConstantUnionSpec(
            union_type=FakeUnion,  # pyre-fixme[6]
        )
        self.assertIsNone(union_val.field)
        self.assertIsNone(union_val.value)

    def test_union_constant_validation(self) -> None:
        class FakeUnion:
            pass

        with self.assertRaises(ValueError):
            ConstantUnionSpec(
                union_type=FakeUnion,  # pyre-fixme[6]
                field="i",
            )
        with self.assertRaises(ValueError):
            ConstantUnionSpec(
                union_type=FakeUnion,  # pyre-fixme[6]
                value=ConstantSpec(value=1, thrift_type=ThriftType.I32),
            )

    def test_enum_constant(self) -> None:
        import enum

        class Company(enum.Enum):
            FACEBOOK = 0
            INSTAGRAM = 3

        enum_val = ConstantEnumSpec(
            enum_type=Company,  # pyre-fixme[6]
            value=Company.INSTAGRAM,  # pyre-fixme[6]
        )
        spec = ConstantSpec(value=enum_val, thrift_type=ThriftType.ENUM)
        val = spec.value
        assert isinstance(val, ConstantEnumSpec)
        self.assertEqual(val.enum_type, Company)
        self.assertEqual(val.value, Company.INSTAGRAM)

    def test_equality(self) -> None:
        a = ConstantSpec(value=42, thrift_type=ThriftType.I32)
        b = ConstantSpec(value=42, thrift_type=ThriftType.I32)
        c = ConstantSpec(value=42, thrift_type=ThriftType.I64)
        self.assertEqual(a, b)
        self.assertNotEqual(a, c)
        self.assertNotEqual(a, "not a ConstantSpec")

    def test_repr(self) -> None:
        spec = ConstantSpec(value=42, thrift_type=ThriftType.I32)
        self.assertIn("42", repr(spec))
        self.assertIn("ThriftType.I32", repr(spec))


class FieldSpecTest(unittest.TestCase):
    def test_basic_field(self) -> None:
        field = FieldSpec(
            id=1,
            name="my_field",
            py_name="my_field",
            type=int,
            thrift_type=ThriftType.I32,
            qualifier=Qualifier.UNQUALIFIED,
            default=0,
        )
        self.assertEqual(field.id, 1)
        self.assertEqual(field.name, "my_field")
        self.assertEqual(field.py_name, "my_field")
        self.assertEqual(field.type, int)
        self.assertEqual(field.thrift_type, ThriftType.I32)
        self.assertEqual(field.qualifier, Qualifier.UNQUALIFIED)
        self.assertEqual(field.default, 0)
        self.assertEqual(field.structured_annotations, {})

    def test_field_with_annotations(self) -> None:
        annotations = {
            "my.Annotation": ConstantSpec(value=True, thrift_type=ThriftType.BOOL),
        }
        field = FieldSpec(
            id=2,
            name="annotated",
            py_name="annotated_",
            type=str,
            thrift_type=ThriftType.STRING,
            qualifier=Qualifier.OPTIONAL,
            structured_annotations=annotations,
        )
        self.assertEqual(field.py_name, "annotated_")
        self.assertEqual(field.qualifier, Qualifier.OPTIONAL)
        self.assertIn("my.Annotation", field.structured_annotations)
        self.assertEqual(field.structured_annotations["my.Annotation"].value, True)
        # structured_annotations is immutable
        self.assertIsInstance(field.structured_annotations, MappingProxyType)

    def test_equality(self) -> None:
        f1 = FieldSpec(
            id=1,
            name="x",
            py_name="x",
            type=int,
            thrift_type=ThriftType.I32,
            qualifier=Qualifier.UNQUALIFIED,
        )
        f2 = FieldSpec(
            id=1,
            name="x",
            py_name="x",
            type=int,
            thrift_type=ThriftType.I32,
            qualifier=Qualifier.UNQUALIFIED,
        )
        self.assertEqual(f1, f2)

    def test_inequality_different_thrift_type(self) -> None:
        f1 = FieldSpec(
            id=1,
            name="x",
            py_name="x",
            type=int,
            thrift_type=ThriftType.I32,
            qualifier=Qualifier.UNQUALIFIED,
        )
        f2 = FieldSpec(
            id=1,
            name="x",
            py_name="x",
            type=int,
            thrift_type=ThriftType.I64,
            qualifier=Qualifier.UNQUALIFIED,
        )
        self.assertNotEqual(f1, f2)


class StructSpecTest(unittest.TestCase):
    def test_struct(self) -> None:
        field = FieldSpec(
            id=1,
            name="val",
            py_name="val",
            type=int,
            thrift_type=ThriftType.I32,
            qualifier=Qualifier.UNQUALIFIED,
            default=0,
        )
        spec = StructSpec(
            name="MyStruct",
            kind=StructType.STRUCT,
            fields=[field],
        )
        self.assertEqual(spec.name, "MyStruct")
        self.assertEqual(spec.kind, StructType.STRUCT)
        self.assertEqual(len(spec.fields), 1)
        self.assertEqual(spec.fields[0].name, "val")
        self.assertEqual(spec.structured_annotations, {})

    def test_union(self) -> None:
        spec = StructSpec(
            name="MyUnion",
            kind=StructType.UNION,
        )
        self.assertEqual(spec.kind, StructType.UNION)
        self.assertEqual(len(spec.fields), 0)

    def test_exception(self) -> None:
        spec = StructSpec(
            name="MyError",
            kind=StructType.EXCEPTION,
        )
        self.assertEqual(spec.kind, StructType.EXCEPTION)

    def test_add_field(self) -> None:
        spec = StructSpec(name="S", kind=StructType.STRUCT)
        self.assertEqual(len(spec.fields), 0)

        field = FieldSpec(
            id=1,
            name="f",
            py_name="f",
            type=str,
            thrift_type=ThriftType.STRING,
            qualifier=Qualifier.UNQUALIFIED,
        )
        spec.add_field(field)
        self.assertEqual(len(spec.fields), 1)
        self.assertEqual(spec.fields[0].name, "f")

    def test_fields_returns_tuple(self) -> None:
        spec = StructSpec(name="S", kind=StructType.STRUCT)
        self.assertIsInstance(spec.fields, tuple)

    def test_structured_annotations(self) -> None:
        annotations = {
            "my.Ann": ConstantSpec(
                value=ConstantStructSpec(
                    struct_type=type,  # pyre-fixme[6]
                    fields={"x": ConstantSpec(value=1, thrift_type=ThriftType.I32)},
                ),
                thrift_type=ThriftType.STRUCT,
            ),
        }
        spec = StructSpec(
            name="Annotated",
            kind=StructType.STRUCT,
            structured_annotations=annotations,
        )
        self.assertIn("my.Ann", spec.structured_annotations)
        self.assertIsInstance(spec.structured_annotations, MappingProxyType)

    def test_equality(self) -> None:
        s1 = StructSpec(name="S", kind=StructType.STRUCT)
        s2 = StructSpec(name="S", kind=StructType.STRUCT)
        self.assertEqual(s1, s2)

        s3 = StructSpec(name="S", kind=StructType.UNION)
        self.assertNotEqual(s1, s3)


class ListSpecTest(unittest.TestCase):
    def test_basic(self) -> None:
        spec = ListSpec(value=int, thrift_type=ThriftType.I32)
        self.assertEqual(spec.value, int)
        self.assertEqual(spec.thrift_type, ThriftType.I32)

    def test_equality(self) -> None:
        a = ListSpec(value=int, thrift_type=ThriftType.I32)
        b = ListSpec(value=int, thrift_type=ThriftType.I32)
        c = ListSpec(value=int, thrift_type=ThriftType.I64)
        self.assertEqual(a, b)
        self.assertNotEqual(a, c)
        self.assertNotEqual(a, "not a ListSpec")


class SetSpecTest(unittest.TestCase):
    def test_basic(self) -> None:
        spec = SetSpec(value=str, thrift_type=ThriftType.STRING)
        self.assertEqual(spec.value, str)
        self.assertEqual(spec.thrift_type, ThriftType.STRING)

    def test_equality(self) -> None:
        a = SetSpec(value=str, thrift_type=ThriftType.STRING)
        b = SetSpec(value=str, thrift_type=ThriftType.STRING)
        self.assertEqual(a, b)


class MapSpecTest(unittest.TestCase):
    def test_basic(self) -> None:
        spec = MapSpec(
            key=str,
            key_thrift_type=ThriftType.STRING,
            value=int,
            value_thrift_type=ThriftType.I64,
        )
        self.assertEqual(spec.key, str)
        self.assertEqual(spec.key_thrift_type, ThriftType.STRING)
        self.assertEqual(spec.value, int)
        self.assertEqual(spec.value_thrift_type, ThriftType.I64)

    def test_equality(self) -> None:
        a = MapSpec(
            key=str,
            key_thrift_type=ThriftType.STRING,
            value=int,
            value_thrift_type=ThriftType.I64,
        )
        b = MapSpec(
            key=str,
            key_thrift_type=ThriftType.STRING,
            value=int,
            value_thrift_type=ThriftType.I64,
        )
        c = MapSpec(
            key=str,
            key_thrift_type=ThriftType.STRING,
            value=int,
            value_thrift_type=ThriftType.I32,
        )
        self.assertEqual(a, b)
        self.assertNotEqual(a, c)


class InspectTest(unittest.TestCase):
    def test_inspect_with_get_reflection(self) -> None:
        expected_spec = StructSpec(name="Foo", kind=StructType.STRUCT)

        class FakeStruct:
            @staticmethod
            def __get_reflection__() -> StructSpec:
                return expected_spec

        result = inspect(FakeStruct)
        self.assertEqual(result, expected_spec)

    def test_inspect_instance(self) -> None:
        expected_spec = StructSpec(name="Foo", kind=StructType.STRUCT)

        class FakeStruct:
            @staticmethod
            def __get_reflection__() -> StructSpec:
                return expected_spec

        result = inspect(FakeStruct())
        self.assertEqual(result, expected_spec)

    def test_inspect_caches(self) -> None:
        call_count = 0

        class FakeStruct:
            @staticmethod
            def __get_reflection__() -> StructSpec:
                nonlocal call_count
                call_count += 1
                return StructSpec(name="Foo", kind=StructType.STRUCT)

        inspect(FakeStruct)
        inspect(FakeStruct)
        inspect(FakeStruct)
        self.assertEqual(call_count, 1)

    def test_inspect_not_inspectable(self) -> None:
        with self.assertRaises(TypeError):
            inspect(int)

    def test_inspectable_true(self) -> None:
        class FakeStruct:
            @staticmethod
            def __get_reflection__() -> StructSpec:
                return StructSpec(name="Foo", kind=StructType.STRUCT)

        self.assertTrue(inspectable(FakeStruct))
        self.assertTrue(inspectable(FakeStruct()))

    def test_inspectable_false(self) -> None:
        self.assertFalse(inspectable(int))
        self.assertFalse(inspectable(42))
        self.assertFalse(inspectable(None))


class IterTest(unittest.TestCase):
    def test_struct_spec_iter(self) -> None:
        field = FieldSpec(
            id=1,
            name="val",
            py_name="val",
            type=int,
            thrift_type=ThriftType.I32,
            qualifier=Qualifier.UNQUALIFIED,
            default=0,
        )
        spec = StructSpec(
            name="MyStruct",
            kind=StructType.STRUCT,
            fields=[field],
        )
        name, fields, kind, annotations = spec
        self.assertEqual(name, "MyStruct")
        self.assertEqual(len(fields), 1)
        self.assertEqual(kind, StructType.STRUCT)
        self.assertEqual(annotations, {})

    def test_field_spec_iter(self) -> None:
        field = FieldSpec(
            id=1,
            name="x",
            py_name="x",
            type=int,
            thrift_type=ThriftType.I32,
            qualifier=Qualifier.UNQUALIFIED,
            default=0,
        )
        fid, name, typ, kind, qualifier, default, annotations = field
        self.assertEqual(fid, 1)
        self.assertEqual(name, "x")
        self.assertEqual(typ, int)
        self.assertEqual(kind, NumberType.I32)
        self.assertEqual(qualifier, Qualifier.UNQUALIFIED)
        self.assertEqual(default, 0)
        self.assertEqual(annotations, {})

    def test_list_spec_iter(self) -> None:
        spec = ListSpec(value=int, thrift_type=ThriftType.I32)
        value, kind = spec
        self.assertEqual(value, int)
        self.assertEqual(kind, NumberType.I32)

    def test_set_spec_iter(self) -> None:
        spec = SetSpec(value=str, thrift_type=ThriftType.STRING)
        value, kind = spec
        self.assertEqual(value, str)
        self.assertEqual(kind, NumberType.NOT_A_NUMBER)

    def test_map_spec_iter(self) -> None:
        spec = MapSpec(
            key=str,
            key_thrift_type=ThriftType.STRING,
            value=int,
            value_thrift_type=ThriftType.I64,
        )
        key, key_kind, value, value_kind = spec
        self.assertEqual(key, str)
        self.assertEqual(key_kind, NumberType.NOT_A_NUMBER)
        self.assertEqual(value, int)
        self.assertEqual(value_kind, NumberType.I64)


class ImmutabilityTest(unittest.TestCase):
    def test_constant_spec_immutable(self) -> None:
        spec = ConstantSpec(value=42, thrift_type=ThriftType.I32)
        with self.assertRaises(AttributeError):
            spec.value = 99  # type: ignore[misc]

    def test_field_spec_immutable(self) -> None:
        field = FieldSpec(
            id=1,
            name="x",
            py_name="x",
            type=int,
            thrift_type=ThriftType.I32,
            qualifier=Qualifier.UNQUALIFIED,
        )
        with self.assertRaises(AttributeError):
            field.name = "y"  # type: ignore[misc]

    def test_struct_spec_immutable(self) -> None:
        spec = StructSpec(name="S", kind=StructType.STRUCT)
        with self.assertRaises(AttributeError):
            spec.name = "T"  # type: ignore[misc]

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
