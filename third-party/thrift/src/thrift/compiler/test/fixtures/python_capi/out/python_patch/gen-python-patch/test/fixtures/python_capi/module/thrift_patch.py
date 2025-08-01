# @generated
from __future__ import annotations

import typing
import typing as _typing

from common.thrift.patch.detail.dynamic_patch import (
    BaseStructPatch,
    BaseUnionPatch,
    ListPatch,
    SetPatch,
    MapPatch,
    OptionalFieldPatch,
    UnqualifiedFieldPatch,
)

from common.thrift.patch.detail.py_bindings.DynamicPatch import (
    BoolPatch,
    BytePatch,
    I16Patch,
    I32Patch,
    I64Patch,
    FloatPatch,
    DoublePatch,
    StringPatch,
    BinaryPatch,
    StructPatch as DynamicStructPatch,
    UnionPatch as DynamicUnionPatch,
    DynamicPatch,
    EnumPatch
)

import thrift.python.types as _fbthrift_python_types
import folly.iobuf as _fbthrift_iobuf

import test.fixtures.python_capi.module.thrift_types as _fbthrift__test__fixtures__python_capi__module__thrift_types
import test.fixtures.python_capi.gen_safe_patch_module.thrift_types as _fbthrift_safe_patch_types


import test.fixtures.python_capi.serialized_dep.thrift_types as _fbthrift__test__fixtures__python_capi__serialized_dep__thrift_types
import test.fixtures.python_capi.serialized_dep.thrift_patch

import test.fixtures.python_capi.thrift_dep.thrift_types as _fbthrift__test__fixtures__python_capi__thrift_dep__thrift_types
import test.fixtures.python_capi.thrift_dep.thrift_patch


class MyStructPatch(
    BaseStructPatch[_fbthrift__test__fixtures__python_capi__module__thrift_types.MyStruct]
):
    pass
    @property
    def inty(self) -> UnqualifiedFieldPatch[
            builtins.int,
            I64Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I64Patch:
            return patch.as_i64_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.typeinfo_i64)
    @property
    def stringy(self) -> UnqualifiedFieldPatch[
            builtins.str,
            StringPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> StringPatch:
            return patch.as_string_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.typeinfo_string)
    @property
    def myItemy(self) -> UnqualifiedFieldPatch[
            _fbthrift__test__fixtures__python_capi__module__thrift_types.MyDataItem,
            MyDataItemPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MyDataItemPatch:
            return MyDataItemPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__test__fixtures__python_capi__module__thrift_types.MyDataItem))
    @property
    def myEnumy(self) -> UnqualifiedFieldPatch[
            _fbthrift__test__fixtures__python_capi__module__thrift_types.MyEnum,
            EnumPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> EnumPatch:
            return patch.as_enum_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            4,
            _fbthrift_python_types.EnumTypeInfo(_fbthrift__test__fixtures__python_capi__module__thrift_types.MyEnum))
    @property
    def booly(self) -> UnqualifiedFieldPatch[
            builtins.bool,
            BoolPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> BoolPatch:
            return patch.as_bool_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            5,
            _fbthrift_python_types.typeinfo_bool)
    @property
    def floatListy(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[builtins.float],
            ListPatch[builtins.float]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[builtins.float]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            6,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.typeinfo_float))
    @property
    def strMappy(self) -> UnqualifiedFieldPatch[
            _typing.Mapping[builtins.bytes, builtins.str],
            MapPatch[builtins.bytes, builtins.str, StringPatch]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MapPatch[builtins.bytes, builtins.str, StringPatch]:
            def cast_dynamic_patch_to_typed_map_value_patch(patch: DynamicPatch, type_info) -> StringPatch:
                return patch.as_string_patch()
            return MapPatch(cast_dynamic_patch_to_typed_map_value_patch, patch.as_map_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            7,
            _fbthrift_python_types.MapTypeInfo(_fbthrift_python_types.typeinfo_binary, _fbthrift_python_types.typeinfo_string))
    @property
    def intSetty(self) -> UnqualifiedFieldPatch[
            _typing.AbstractSet[builtins.int],
            SetPatch[builtins.int]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> SetPatch[builtins.int]:
            return SetPatch(patch.as_set_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            8,
            _fbthrift_python_types.SetTypeInfo(_fbthrift_python_types.typeinfo_i32))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.MyStructSafePatch:
        return _fbthrift_safe_patch_types.MyStructSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.MyStructSafePatch) -> MyStructPatch:
        patch = MyStructPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch

    def merge(self, other: MyStructPatch) -> None:
        self._patch.merge(other._patch)


class MyDataItemPatch(
    BaseStructPatch[_fbthrift__test__fixtures__python_capi__module__thrift_types.MyDataItem]
):
    pass
    @property
    def s(self) -> UnqualifiedFieldPatch[
            builtins.str,
            StringPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> StringPatch:
            return patch.as_string_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.typeinfo_string)

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.MyDataItemSafePatch:
        return _fbthrift_safe_patch_types.MyDataItemSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.MyDataItemSafePatch) -> MyDataItemPatch:
        patch = MyDataItemPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch

    def merge(self, other: MyDataItemPatch) -> None:
        self._patch.merge(other._patch)


class TransitiveDoublerPatch(
    BaseStructPatch[_fbthrift__test__fixtures__python_capi__module__thrift_types.TransitiveDoubler]
):
    pass

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.TransitiveDoublerSafePatch:
        return _fbthrift_safe_patch_types.TransitiveDoublerSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.TransitiveDoublerSafePatch) -> TransitiveDoublerPatch:
        patch = TransitiveDoublerPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch

    def merge(self, other: TransitiveDoublerPatch) -> None:
        self._patch.merge(other._patch)


class DoubledPairPatch(
    BaseStructPatch[_fbthrift__test__fixtures__python_capi__module__thrift_types.DoubledPair]
):
    pass
    @property
    def s(self) -> UnqualifiedFieldPatch[
            builtins.str,
            StringPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> StringPatch:
            return patch.as_string_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.typeinfo_string)
    @property
    def x(self) -> UnqualifiedFieldPatch[
            builtins.int,
            I32Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I32Patch:
            return patch.as_i32_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.typeinfo_i32)

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.DoubledPairSafePatch:
        return _fbthrift_safe_patch_types.DoubledPairSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.DoubledPairSafePatch) -> DoubledPairPatch:
        patch = DoubledPairPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch

    def merge(self, other: DoubledPairPatch) -> None:
        self._patch.merge(other._patch)


class StringPairPatch(
    BaseStructPatch[_fbthrift__test__fixtures__python_capi__module__thrift_types.StringPair]
):
    pass
    @property
    def normal(self) -> UnqualifiedFieldPatch[
            builtins.str,
            StringPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> StringPatch:
            return patch.as_string_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.typeinfo_string)
    @property
    def doubled(self) -> UnqualifiedFieldPatch[
            builtins.str,
            StringPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> StringPatch:
            return patch.as_string_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.typeinfo_string)

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.StringPairSafePatch:
        return _fbthrift_safe_patch_types.StringPairSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.StringPairSafePatch) -> StringPairPatch:
        patch = StringPairPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch

    def merge(self, other: StringPairPatch) -> None:
        self._patch.merge(other._patch)


class EmptyStructPatch(
    BaseStructPatch[_fbthrift__test__fixtures__python_capi__module__thrift_types.EmptyStruct]
):
    pass

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.EmptyStructSafePatch:
        return _fbthrift_safe_patch_types.EmptyStructSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.EmptyStructSafePatch) -> EmptyStructPatch:
        patch = EmptyStructPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch

    def merge(self, other: EmptyStructPatch) -> None:
        self._patch.merge(other._patch)


class PrimitiveStructPatch(
    BaseStructPatch[_fbthrift__test__fixtures__python_capi__module__thrift_types.PrimitiveStruct]
):
    pass
    @property
    def booly(self) -> UnqualifiedFieldPatch[
            builtins.bool,
            BoolPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> BoolPatch:
            return patch.as_bool_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.typeinfo_bool)
    @property
    def charry(self) -> UnqualifiedFieldPatch[
            builtins.int,
            BytePatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> BytePatch:
            return patch.as_byte_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.typeinfo_byte)
    @property
    def shorty(self) -> UnqualifiedFieldPatch[
            builtins.int,
            I16Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I16Patch:
            return patch.as_i16_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.typeinfo_i16)
    @property
    def inty(self) -> UnqualifiedFieldPatch[
            builtins.int,
            I32Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I32Patch:
            return patch.as_i32_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            5,
            _fbthrift_python_types.typeinfo_i32)
    @property
    def longy(self) -> UnqualifiedFieldPatch[
            builtins.int,
            I64Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I64Patch:
            return patch.as_i64_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            7,
            _fbthrift_python_types.typeinfo_i64)
    @property
    def floaty(self) -> OptionalFieldPatch[
            builtins.float,
            FloatPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> FloatPatch:
            return patch.as_float_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            8,
            _fbthrift_python_types.typeinfo_float)
    @property
    def dubby(self) -> OptionalFieldPatch[
            builtins.float,
            DoublePatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> DoublePatch:
            return patch.as_double_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            9,
            _fbthrift_python_types.typeinfo_double)
    @property
    def stringy(self) -> OptionalFieldPatch[
            builtins.str,
            StringPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> StringPatch:
            return patch.as_string_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            12,
            _fbthrift_python_types.typeinfo_string)
    @property
    def bytey(self) -> OptionalFieldPatch[
            builtins.bytes,
            BinaryPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> BinaryPatch:
            return patch.as_binary_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            13,
            _fbthrift_python_types.typeinfo_binary)
    @property
    def buffy(self) -> UnqualifiedFieldPatch[
            _fbthrift_iobuf.IOBuf,
            BinaryPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> BinaryPatch:
            return patch.as_binary_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            14,
            _fbthrift_python_types.typeinfo_iobuf)
    @property
    def pointbuffy(self) -> UnqualifiedFieldPatch[
            _fbthrift_iobuf.IOBuf,
            BinaryPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> BinaryPatch:
            return patch.as_binary_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            15,
            _fbthrift_python_types.typeinfo_iobuf)
    @property
    def patched_struct(self) -> UnqualifiedFieldPatch[
            _fbthrift__test__fixtures__python_capi__module__thrift_types.MyStruct,
            MyStructPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MyStructPatch:
            return MyStructPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            18,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__test__fixtures__python_capi__module__thrift_types.MyStruct))
    @property
    def empty_struct(self) -> UnqualifiedFieldPatch[
            _fbthrift__test__fixtures__python_capi__module__thrift_types.EmptyStruct,
            EmptyStructPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> EmptyStructPatch:
            return EmptyStructPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            19,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__test__fixtures__python_capi__module__thrift_types.EmptyStruct))
    @property
    def fbstring(self) -> UnqualifiedFieldPatch[
            builtins.bytes,
            BinaryPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> BinaryPatch:
            return patch.as_binary_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            20,
            _fbthrift_python_types.typeinfo_binary)
    @property
    def managed_string_view(self) -> UnqualifiedFieldPatch[
            builtins.str,
            StringPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> StringPatch:
            return patch.as_string_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            21,
            _fbthrift_python_types.typeinfo_string)
    @property
    def some_error(self) -> UnqualifiedFieldPatch[
            _fbthrift__test__fixtures__python_capi__thrift_dep__thrift_types.SomeError,
            test.fixtures.python_capi.thrift_dep.thrift_patch.SomeErrorPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> test.fixtures.python_capi.thrift_dep.thrift_patch.SomeErrorPatch:
            return test.fixtures.python_capi.thrift_dep.thrift_patch.SomeErrorPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            22,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__test__fixtures__python_capi__thrift_dep__thrift_types.SomeError))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.PrimitiveStructSafePatch:
        return _fbthrift_safe_patch_types.PrimitiveStructSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.PrimitiveStructSafePatch) -> PrimitiveStructPatch:
        patch = PrimitiveStructPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch

    def merge(self, other: PrimitiveStructPatch) -> None:
        self._patch.merge(other._patch)


class AdaptedFieldsPatch(
    BaseStructPatch[_fbthrift__test__fixtures__python_capi__module__thrift_types.AdaptedFields]
):
    pass
    @property
    def adapted_int(self) -> UnqualifiedFieldPatch[
            builtins.int,
            I64Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I64Patch:
            return patch.as_i64_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.typeinfo_i64)
    @property
    def list_adapted_int(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[builtins.int],
            ListPatch[builtins.int]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[builtins.int]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.typeinfo_i16))
    @property
    def set_adapted_int(self) -> UnqualifiedFieldPatch[
            _typing.AbstractSet[builtins.int],
            SetPatch[builtins.int]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> SetPatch[builtins.int]:
            return SetPatch(patch.as_set_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.SetTypeInfo(_fbthrift_python_types.typeinfo_i64))
    @property
    def inline_adapted_int(self) -> UnqualifiedFieldPatch[
            builtins.int,
            I64Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I64Patch:
            return patch.as_i64_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            4,
            _fbthrift_python_types.typeinfo_i64)

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.AdaptedFieldsSafePatch:
        return _fbthrift_safe_patch_types.AdaptedFieldsSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.AdaptedFieldsSafePatch) -> AdaptedFieldsPatch:
        patch = AdaptedFieldsPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch

    def merge(self, other: AdaptedFieldsPatch) -> None:
        self._patch.merge(other._patch)


class ListStructPatch(
    BaseStructPatch[_fbthrift__test__fixtures__python_capi__module__thrift_types.ListStruct]
):
    pass
    @property
    def boolz(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[builtins.bool],
            ListPatch[builtins.bool]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[builtins.bool]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.typeinfo_bool))
    @property
    def intz(self) -> OptionalFieldPatch[
            _typing.Sequence[builtins.int],
            ListPatch[builtins.int]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[builtins.int]:
            return ListPatch(patch.as_list_patch(), type_info)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.typeinfo_i64))
    @property
    def stringz(self) -> OptionalFieldPatch[
            _typing.Sequence[builtins.str],
            ListPatch[builtins.str]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[builtins.str]:
            return ListPatch(patch.as_list_patch(), type_info)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.typeinfo_string))
    @property
    def encoded(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[builtins.bytes],
            ListPatch[builtins.bytes]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[builtins.bytes]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            4,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.typeinfo_binary))
    @property
    def uidz(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[builtins.int],
            ListPatch[builtins.int]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[builtins.int]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            5,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.typeinfo_i64))
    @property
    def matrix(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[_typing.Sequence[builtins.float]],
            ListPatch[_typing.Sequence[builtins.float]]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[_typing.Sequence[builtins.float]]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            6,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.typeinfo_double)))
    @property
    def ucharz(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[_typing.Sequence[builtins.int]],
            ListPatch[_typing.Sequence[builtins.int]]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[_typing.Sequence[builtins.int]]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            7,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.typeinfo_byte)))
    @property
    def voxels(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[_typing.Sequence[_typing.Sequence[builtins.int]]],
            ListPatch[_typing.Sequence[_typing.Sequence[builtins.int]]]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[_typing.Sequence[_typing.Sequence[builtins.int]]]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            8,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.typeinfo_byte))))
    @property
    def buf_ptrs(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[_fbthrift_iobuf.IOBuf],
            ListPatch[_fbthrift_iobuf.IOBuf]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[_fbthrift_iobuf.IOBuf]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            9,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.typeinfo_iobuf))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.ListStructSafePatch:
        return _fbthrift_safe_patch_types.ListStructSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.ListStructSafePatch) -> ListStructPatch:
        patch = ListStructPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch

    def merge(self, other: ListStructPatch) -> None:
        self._patch.merge(other._patch)


class SetStructPatch(
    BaseStructPatch[_fbthrift__test__fixtures__python_capi__module__thrift_types.SetStruct]
):
    pass
    @property
    def enumz(self) -> UnqualifiedFieldPatch[
            _typing.AbstractSet[_fbthrift__test__fixtures__python_capi__module__thrift_types.MyEnum],
            SetPatch[_fbthrift__test__fixtures__python_capi__module__thrift_types.MyEnum]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> SetPatch[_fbthrift__test__fixtures__python_capi__module__thrift_types.MyEnum]:
            return SetPatch(patch.as_set_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.SetTypeInfo(_fbthrift_python_types.EnumTypeInfo(_fbthrift__test__fixtures__python_capi__module__thrift_types.MyEnum)))
    @property
    def intz(self) -> OptionalFieldPatch[
            _typing.AbstractSet[builtins.int],
            SetPatch[builtins.int]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> SetPatch[builtins.int]:
            return SetPatch(patch.as_set_patch(), type_info)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.SetTypeInfo(_fbthrift_python_types.typeinfo_i32))
    @property
    def binnaz(self) -> OptionalFieldPatch[
            _typing.AbstractSet[builtins.bytes],
            SetPatch[builtins.bytes]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> SetPatch[builtins.bytes]:
            return SetPatch(patch.as_set_patch(), type_info)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.SetTypeInfo(_fbthrift_python_types.typeinfo_binary))
    @property
    def encoded(self) -> UnqualifiedFieldPatch[
            _typing.AbstractSet[builtins.bytes],
            SetPatch[builtins.bytes]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> SetPatch[builtins.bytes]:
            return SetPatch(patch.as_set_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            4,
            _fbthrift_python_types.SetTypeInfo(_fbthrift_python_types.typeinfo_binary))
    @property
    def uidz(self) -> UnqualifiedFieldPatch[
            _typing.AbstractSet[builtins.int],
            SetPatch[builtins.int]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> SetPatch[builtins.int]:
            return SetPatch(patch.as_set_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            5,
            _fbthrift_python_types.SetTypeInfo(_fbthrift_python_types.typeinfo_i64))
    @property
    def charz(self) -> UnqualifiedFieldPatch[
            _typing.AbstractSet[builtins.int],
            SetPatch[builtins.int]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> SetPatch[builtins.int]:
            return SetPatch(patch.as_set_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            6,
            _fbthrift_python_types.SetTypeInfo(_fbthrift_python_types.typeinfo_byte))
    @property
    def setz(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[_typing.AbstractSet[builtins.int]],
            ListPatch[_typing.AbstractSet[builtins.int]]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[_typing.AbstractSet[builtins.int]]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            7,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.SetTypeInfo(_fbthrift_python_types.typeinfo_i64)))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.SetStructSafePatch:
        return _fbthrift_safe_patch_types.SetStructSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.SetStructSafePatch) -> SetStructPatch:
        patch = SetStructPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch

    def merge(self, other: SetStructPatch) -> None:
        self._patch.merge(other._patch)


class MapStructPatch(
    BaseStructPatch[_fbthrift__test__fixtures__python_capi__module__thrift_types.MapStruct]
):
    pass
    @property
    def enumz(self) -> UnqualifiedFieldPatch[
            _typing.Mapping[_fbthrift__test__fixtures__python_capi__module__thrift_types.MyEnum, builtins.str],
            MapPatch[_fbthrift__test__fixtures__python_capi__module__thrift_types.MyEnum, builtins.str, StringPatch]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MapPatch[_fbthrift__test__fixtures__python_capi__module__thrift_types.MyEnum, builtins.str, StringPatch]:
            def cast_dynamic_patch_to_typed_map_value_patch(patch: DynamicPatch, type_info) -> StringPatch:
                return patch.as_string_patch()
            return MapPatch(cast_dynamic_patch_to_typed_map_value_patch, patch.as_map_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.MapTypeInfo(_fbthrift_python_types.EnumTypeInfo(_fbthrift__test__fixtures__python_capi__module__thrift_types.MyEnum), _fbthrift_python_types.typeinfo_string))
    @property
    def intz(self) -> OptionalFieldPatch[
            _typing.Mapping[builtins.int, builtins.str],
            MapPatch[builtins.int, builtins.str, StringPatch]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MapPatch[builtins.int, builtins.str, StringPatch]:
            def cast_dynamic_patch_to_typed_map_value_patch(patch: DynamicPatch, type_info) -> StringPatch:
                return patch.as_string_patch()
            return MapPatch(cast_dynamic_patch_to_typed_map_value_patch, patch.as_map_patch(), type_info)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.MapTypeInfo(_fbthrift_python_types.typeinfo_i32, _fbthrift_python_types.typeinfo_string))
    @property
    def binnaz(self) -> OptionalFieldPatch[
            _typing.Mapping[builtins.bytes, _fbthrift__test__fixtures__python_capi__module__thrift_types.PrimitiveStruct],
            MapPatch[builtins.bytes, _fbthrift__test__fixtures__python_capi__module__thrift_types.PrimitiveStruct, PrimitiveStructPatch]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MapPatch[builtins.bytes, _fbthrift__test__fixtures__python_capi__module__thrift_types.PrimitiveStruct, PrimitiveStructPatch]:
            def cast_dynamic_patch_to_typed_map_value_patch(patch: DynamicPatch, type_info) -> PrimitiveStructPatch:
                return PrimitiveStructPatch(patch)
            return MapPatch(cast_dynamic_patch_to_typed_map_value_patch, patch.as_map_patch(), type_info)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.MapTypeInfo(_fbthrift_python_types.typeinfo_binary, _fbthrift_python_types.StructTypeInfo(_fbthrift__test__fixtures__python_capi__module__thrift_types.PrimitiveStruct)))
    @property
    def encoded(self) -> UnqualifiedFieldPatch[
            _typing.Mapping[builtins.str, builtins.float],
            MapPatch[builtins.str, builtins.float, DoublePatch]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MapPatch[builtins.str, builtins.float, DoublePatch]:
            def cast_dynamic_patch_to_typed_map_value_patch(patch: DynamicPatch, type_info) -> DoublePatch:
                return patch.as_double_patch()
            return MapPatch(cast_dynamic_patch_to_typed_map_value_patch, patch.as_map_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            4,
            _fbthrift_python_types.MapTypeInfo(_fbthrift_python_types.typeinfo_string, _fbthrift_python_types.typeinfo_double))
    @property
    def flotz(self) -> UnqualifiedFieldPatch[
            _typing.Mapping[builtins.int, builtins.float],
            MapPatch[builtins.int, builtins.float, FloatPatch]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MapPatch[builtins.int, builtins.float, FloatPatch]:
            def cast_dynamic_patch_to_typed_map_value_patch(patch: DynamicPatch, type_info) -> FloatPatch:
                return patch.as_float_patch()
            return MapPatch(cast_dynamic_patch_to_typed_map_value_patch, patch.as_map_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            5,
            _fbthrift_python_types.MapTypeInfo(_fbthrift_python_types.typeinfo_i64, _fbthrift_python_types.typeinfo_float))
    @property
    def map_list(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[_typing.Mapping[builtins.int, builtins.int]],
            ListPatch[_typing.Mapping[builtins.int, builtins.int]]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[_typing.Mapping[builtins.int, builtins.int]]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            6,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.MapTypeInfo(_fbthrift_python_types.typeinfo_i32, _fbthrift_python_types.typeinfo_i64)))
    @property
    def list_map(self) -> UnqualifiedFieldPatch[
            _typing.Mapping[builtins.int, _typing.Sequence[builtins.int]],
            MapPatch[builtins.int, _typing.Sequence[builtins.int], ListPatch[builtins.int]]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MapPatch[builtins.int, _typing.Sequence[builtins.int], ListPatch[builtins.int]]:
            def cast_dynamic_patch_to_typed_map_value_patch(patch: DynamicPatch, type_info) -> ListPatch[builtins.int]:
                return ListPatch(patch.as_list_patch(), type_info)
            return MapPatch(cast_dynamic_patch_to_typed_map_value_patch, patch.as_map_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            7,
            _fbthrift_python_types.MapTypeInfo(_fbthrift_python_types.typeinfo_i32, _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.typeinfo_i64)))
    @property
    def fast_list_map(self) -> UnqualifiedFieldPatch[
            _typing.Mapping[builtins.int, _typing.Sequence[builtins.float]],
            MapPatch[builtins.int, _typing.Sequence[builtins.float], ListPatch[builtins.float]]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MapPatch[builtins.int, _typing.Sequence[builtins.float], ListPatch[builtins.float]]:
            def cast_dynamic_patch_to_typed_map_value_patch(patch: DynamicPatch, type_info) -> ListPatch[builtins.float]:
                return ListPatch(patch.as_list_patch(), type_info)
            return MapPatch(cast_dynamic_patch_to_typed_map_value_patch, patch.as_map_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            8,
            _fbthrift_python_types.MapTypeInfo(_fbthrift_python_types.typeinfo_i32, _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.typeinfo_double)))
    @property
    def buf_map(self) -> UnqualifiedFieldPatch[
            _typing.Mapping[builtins.bytes, _fbthrift_iobuf.IOBuf],
            MapPatch[builtins.bytes, _fbthrift_iobuf.IOBuf, BinaryPatch]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MapPatch[builtins.bytes, _fbthrift_iobuf.IOBuf, BinaryPatch]:
            def cast_dynamic_patch_to_typed_map_value_patch(patch: DynamicPatch, type_info) -> BinaryPatch:
                return patch.as_binary_patch()
            return MapPatch(cast_dynamic_patch_to_typed_map_value_patch, patch.as_map_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            9,
            _fbthrift_python_types.MapTypeInfo(_fbthrift_python_types.typeinfo_binary, _fbthrift_python_types.typeinfo_iobuf))
    @property
    def unsigned_list_map(self) -> UnqualifiedFieldPatch[
            _typing.Mapping[builtins.int, _typing.Sequence[builtins.int]],
            MapPatch[builtins.int, _typing.Sequence[builtins.int], ListPatch[builtins.int]]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MapPatch[builtins.int, _typing.Sequence[builtins.int], ListPatch[builtins.int]]:
            def cast_dynamic_patch_to_typed_map_value_patch(patch: DynamicPatch, type_info) -> ListPatch[builtins.int]:
                return ListPatch(patch.as_list_patch(), type_info)
            return MapPatch(cast_dynamic_patch_to_typed_map_value_patch, patch.as_map_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            10,
            _fbthrift_python_types.MapTypeInfo(_fbthrift_python_types.typeinfo_i64, _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.typeinfo_i64)))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.MapStructSafePatch:
        return _fbthrift_safe_patch_types.MapStructSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.MapStructSafePatch) -> MapStructPatch:
        patch = MapStructPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch

    def merge(self, other: MapStructPatch) -> None:
        self._patch.merge(other._patch)


class ComposeStructPatch(
    BaseStructPatch[_fbthrift__test__fixtures__python_capi__module__thrift_types.ComposeStruct]
):
    pass
    @property
    def enum_(self) -> UnqualifiedFieldPatch[
            _fbthrift__test__fixtures__python_capi__module__thrift_types.MyEnum,
            EnumPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> EnumPatch:
            return patch.as_enum_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.EnumTypeInfo(_fbthrift__test__fixtures__python_capi__module__thrift_types.MyEnum))
    @property
    def renamed_(self) -> UnqualifiedFieldPatch[
            _fbthrift__test__fixtures__python_capi__module__thrift_types.AnnoyingEnum,
            EnumPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> EnumPatch:
            return patch.as_enum_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.EnumTypeInfo(_fbthrift__test__fixtures__python_capi__module__thrift_types.AnnoyingEnum))
    @property
    def primitive(self) -> UnqualifiedFieldPatch[
            _fbthrift__test__fixtures__python_capi__module__thrift_types.PrimitiveStruct,
            PrimitiveStructPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> PrimitiveStructPatch:
            return PrimitiveStructPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__test__fixtures__python_capi__module__thrift_types.PrimitiveStruct))
    @property
    def aliased(self) -> UnqualifiedFieldPatch[
            _fbthrift__test__fixtures__python_capi__module__thrift_types.ListStruct,
            ListStructPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListStructPatch:
            return ListStructPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            4,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__test__fixtures__python_capi__module__thrift_types.ListStruct))
    @property
    def xenum(self) -> UnqualifiedFieldPatch[
            _fbthrift__test__fixtures__python_capi__thrift_dep__thrift_types.DepEnum,
            EnumPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> EnumPatch:
            return patch.as_enum_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            5,
            _fbthrift_python_types.EnumTypeInfo(_fbthrift__test__fixtures__python_capi__thrift_dep__thrift_types.DepEnum))
    @property
    def xstruct(self) -> UnqualifiedFieldPatch[
            _fbthrift__test__fixtures__python_capi__thrift_dep__thrift_types.DepStruct,
            test.fixtures.python_capi.thrift_dep.thrift_patch.DepStructPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> test.fixtures.python_capi.thrift_dep.thrift_patch.DepStructPatch:
            return test.fixtures.python_capi.thrift_dep.thrift_patch.DepStructPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            6,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__test__fixtures__python_capi__thrift_dep__thrift_types.DepStruct))
    @property
    def friends(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[_fbthrift__test__fixtures__python_capi__thrift_dep__thrift_types.DepStruct],
            ListPatch[_fbthrift__test__fixtures__python_capi__thrift_dep__thrift_types.DepStruct]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[_fbthrift__test__fixtures__python_capi__thrift_dep__thrift_types.DepStruct]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            7,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.StructTypeInfo(_fbthrift__test__fixtures__python_capi__thrift_dep__thrift_types.DepStruct)))
    @property
    def serial_struct(self) -> UnqualifiedFieldPatch[
            _fbthrift__test__fixtures__python_capi__serialized_dep__thrift_types.SerializedStruct,
            test.fixtures.python_capi.serialized_dep.thrift_patch.SerializedStructPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> test.fixtures.python_capi.serialized_dep.thrift_patch.SerializedStructPatch:
            return test.fixtures.python_capi.serialized_dep.thrift_patch.SerializedStructPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            8,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__test__fixtures__python_capi__serialized_dep__thrift_types.SerializedStruct))
    @property
    def serial_union(self) -> UnqualifiedFieldPatch[
            _fbthrift__test__fixtures__python_capi__serialized_dep__thrift_types.SerializedUnion,
            test.fixtures.python_capi.serialized_dep.thrift_patch.SerializedUnionPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> test.fixtures.python_capi.serialized_dep.thrift_patch.SerializedUnionPatch:
            return test.fixtures.python_capi.serialized_dep.thrift_patch.SerializedUnionPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            9,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__test__fixtures__python_capi__serialized_dep__thrift_types.SerializedUnion))
    @property
    def serial_error(self) -> UnqualifiedFieldPatch[
            _fbthrift__test__fixtures__python_capi__serialized_dep__thrift_types.SerializedError,
            test.fixtures.python_capi.serialized_dep.thrift_patch.SerializedErrorPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> test.fixtures.python_capi.serialized_dep.thrift_patch.SerializedErrorPatch:
            return test.fixtures.python_capi.serialized_dep.thrift_patch.SerializedErrorPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            10,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__test__fixtures__python_capi__serialized_dep__thrift_types.SerializedError))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.ComposeStructSafePatch:
        return _fbthrift_safe_patch_types.ComposeStructSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.ComposeStructSafePatch) -> ComposeStructPatch:
        patch = ComposeStructPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch

    def merge(self, other: ComposeStructPatch) -> None:
        self._patch.merge(other._patch)


class OnionPatch(
    BaseUnionPatch[_fbthrift__test__fixtures__python_capi__module__thrift_types.Onion]
):
    pass
    @property
    def myEnum(self) -> OptionalFieldPatch[
            _fbthrift__test__fixtures__python_capi__module__thrift_types.MyEnum,
            EnumPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> EnumPatch:
            return patch.as_enum_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.EnumTypeInfo(_fbthrift__test__fixtures__python_capi__module__thrift_types.MyEnum))
    @property
    def myStruct(self) -> OptionalFieldPatch[
            _fbthrift__test__fixtures__python_capi__module__thrift_types.PrimitiveStruct,
            PrimitiveStructPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> PrimitiveStructPatch:
            return PrimitiveStructPatch(patch)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__test__fixtures__python_capi__module__thrift_types.PrimitiveStruct))
    @property
    def myString(self) -> OptionalFieldPatch[
            builtins.str,
            StringPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> StringPatch:
            return patch.as_string_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            4,
            _fbthrift_python_types.typeinfo_string)
    @property
    def intSet(self) -> OptionalFieldPatch[
            _typing.AbstractSet[builtins.int],
            SetPatch[builtins.int]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> SetPatch[builtins.int]:
            return SetPatch(patch.as_set_patch(), type_info)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            6,
            _fbthrift_python_types.SetTypeInfo(_fbthrift_python_types.typeinfo_i64))
    @property
    def doubleList(self) -> OptionalFieldPatch[
            _typing.Sequence[builtins.float],
            ListPatch[builtins.float]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[builtins.float]:
            return ListPatch(patch.as_list_patch(), type_info)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            8,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.typeinfo_double))
    @property
    def strMap(self) -> OptionalFieldPatch[
            _typing.Mapping[builtins.bytes, builtins.str],
            MapPatch[builtins.bytes, builtins.str, StringPatch]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MapPatch[builtins.bytes, builtins.str, StringPatch]:
            def cast_dynamic_patch_to_typed_map_value_patch(patch: DynamicPatch, type_info) -> StringPatch:
                return patch.as_string_patch()
            return MapPatch(cast_dynamic_patch_to_typed_map_value_patch, patch.as_map_patch(), type_info)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            9,
            _fbthrift_python_types.MapTypeInfo(_fbthrift_python_types.typeinfo_binary, _fbthrift_python_types.typeinfo_string))
    @property
    def adapted_int(self) -> OptionalFieldPatch[
            builtins.int,
            I64Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I64Patch:
            return patch.as_i64_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            10,
            _fbthrift_python_types.typeinfo_i64)

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.OnionSafePatch:
        return _fbthrift_safe_patch_types.OnionSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.OnionSafePatch) -> OnionPatch:
        patch = OnionPatch()
        DynamicPatch = DynamicUnionPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch

    def merge(self, other: OnionPatch) -> None:
        self._patch.merge(other._patch)


class SomeBinaryPatch(
    BaseUnionPatch[_fbthrift__test__fixtures__python_capi__module__thrift_types.SomeBinary]
):
    pass
    @property
    def iobuf(self) -> OptionalFieldPatch[
            _fbthrift_iobuf.IOBuf,
            BinaryPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> BinaryPatch:
            return patch.as_binary_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.typeinfo_iobuf)
    @property
    def iobuf_ptr(self) -> OptionalFieldPatch[
            _fbthrift_iobuf.IOBuf,
            BinaryPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> BinaryPatch:
            return patch.as_binary_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.typeinfo_iobuf)
    @property
    def iobufRef(self) -> OptionalFieldPatch[
            _fbthrift_iobuf.IOBuf,
            BinaryPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> BinaryPatch:
            return patch.as_binary_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.typeinfo_iobuf)

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.SomeBinarySafePatch:
        return _fbthrift_safe_patch_types.SomeBinarySafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.SomeBinarySafePatch) -> SomeBinaryPatch:
        patch = SomeBinaryPatch()
        DynamicPatch = DynamicUnionPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch

    def merge(self, other: SomeBinaryPatch) -> None:
        self._patch.merge(other._patch)

