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

import test.fixtures.basic.module.thrift_types as _fbthrift__test__fixtures__basic__module__thrift_types
import test.fixtures.basic.gen_safe_patch_module.thrift_types as _fbthrift_safe_patch_types



class MyStructPatch(
    BaseStructPatch[_fbthrift__test__fixtures__basic__module__thrift_types.MyStruct]
):
    pass
    @property
    def MyIntField(self) -> UnqualifiedFieldPatch[
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
    def MyStringField(self) -> UnqualifiedFieldPatch[
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
    def MyDataField(self) -> UnqualifiedFieldPatch[
            _fbthrift__test__fixtures__basic__module__thrift_types.MyDataItem,
            MyDataItemPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MyDataItemPatch:
            return MyDataItemPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__test__fixtures__basic__module__thrift_types.MyDataItem))
    @property
    def myEnum(self) -> UnqualifiedFieldPatch[
            _fbthrift__test__fixtures__basic__module__thrift_types.MyEnum,
            EnumPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> EnumPatch:
            return patch.as_enum_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            4,
            _fbthrift_python_types.EnumTypeInfo(_fbthrift__test__fixtures__basic__module__thrift_types.MyEnum))
    @property
    def oneway(self) -> UnqualifiedFieldPatch[
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
    def readonly(self) -> UnqualifiedFieldPatch[
            builtins.bool,
            BoolPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> BoolPatch:
            return patch.as_bool_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            6,
            _fbthrift_python_types.typeinfo_bool)
    @property
    def idempotent(self) -> UnqualifiedFieldPatch[
            builtins.bool,
            BoolPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> BoolPatch:
            return patch.as_bool_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            7,
            _fbthrift_python_types.typeinfo_bool)
    @property
    def floatSet(self) -> UnqualifiedFieldPatch[
            _typing.AbstractSet[builtins.float],
            SetPatch[builtins.float]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> SetPatch[builtins.float]:
            return SetPatch(patch.as_set_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            8,
            _fbthrift_python_types.SetTypeInfo(_fbthrift_python_types.typeinfo_float))
    @property
    def no_hack_codegen_field(self) -> UnqualifiedFieldPatch[
            builtins.str,
            StringPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> StringPatch:
            return patch.as_string_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            9,
            _fbthrift_python_types.typeinfo_string)

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


class ContainersPatch(
    BaseStructPatch[_fbthrift__test__fixtures__basic__module__thrift_types.Containers]
):
    pass
    @property
    def I32List(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[builtins.int],
            ListPatch[builtins.int]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[builtins.int]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.typeinfo_i32))
    @property
    def StringSet(self) -> UnqualifiedFieldPatch[
            _typing.AbstractSet[builtins.str],
            SetPatch[builtins.str]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> SetPatch[builtins.str]:
            return SetPatch(patch.as_set_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.SetTypeInfo(_fbthrift_python_types.typeinfo_string))
    @property
    def StringToI64Map(self) -> UnqualifiedFieldPatch[
            _typing.Mapping[builtins.str, builtins.int],
            MapPatch[builtins.str, builtins.int, I64Patch]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MapPatch[builtins.str, builtins.int, I64Patch]:
            def cast_dynamic_patch_to_typed_map_value_patch(patch: DynamicPatch, type_info) -> I64Patch:
                return patch.as_i64_patch()
            return MapPatch(cast_dynamic_patch_to_typed_map_value_patch, patch.as_map_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.MapTypeInfo(_fbthrift_python_types.typeinfo_string, _fbthrift_python_types.typeinfo_i64))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.ContainersSafePatch:
        return _fbthrift_safe_patch_types.ContainersSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.ContainersSafePatch) -> ContainersPatch:
        patch = ContainersPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch

    def merge(self, other: ContainersPatch) -> None:
        self._patch.merge(other._patch)


class MyDataItemPatch(
    BaseStructPatch[_fbthrift__test__fixtures__basic__module__thrift_types.MyDataItem]
):
    pass

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


class MyUnionPatch(
    BaseUnionPatch[_fbthrift__test__fixtures__basic__module__thrift_types.MyUnion]
):
    pass
    @property
    def myEnum(self) -> OptionalFieldPatch[
            _fbthrift__test__fixtures__basic__module__thrift_types.MyEnum,
            EnumPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> EnumPatch:
            return patch.as_enum_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.EnumTypeInfo(_fbthrift__test__fixtures__basic__module__thrift_types.MyEnum))
    @property
    def myStruct(self) -> OptionalFieldPatch[
            _fbthrift__test__fixtures__basic__module__thrift_types.MyStruct,
            MyStructPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MyStructPatch:
            return MyStructPatch(patch)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__test__fixtures__basic__module__thrift_types.MyStruct))
    @property
    def myDataItem(self) -> OptionalFieldPatch[
            _fbthrift__test__fixtures__basic__module__thrift_types.MyDataItem,
            MyDataItemPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MyDataItemPatch:
            return MyDataItemPatch(patch)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__test__fixtures__basic__module__thrift_types.MyDataItem))
    @property
    def floatSet(self) -> OptionalFieldPatch[
            _typing.AbstractSet[builtins.float],
            SetPatch[builtins.float]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> SetPatch[builtins.float]:
            return SetPatch(patch.as_set_patch(), type_info)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            4,
            _fbthrift_python_types.SetTypeInfo(_fbthrift_python_types.typeinfo_float))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.MyUnionSafePatch:
        return _fbthrift_safe_patch_types.MyUnionSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.MyUnionSafePatch) -> MyUnionPatch:
        patch = MyUnionPatch()
        DynamicPatch = DynamicUnionPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch

    def merge(self, other: MyUnionPatch) -> None:
        self._patch.merge(other._patch)


class MyExceptionPatch(
    BaseStructPatch[_fbthrift__test__fixtures__basic__module__thrift_types.MyException]
):
    pass
    @property
    def MyIntField(self) -> UnqualifiedFieldPatch[
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
    def MyStringField(self) -> UnqualifiedFieldPatch[
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
    def myStruct(self) -> UnqualifiedFieldPatch[
            _fbthrift__test__fixtures__basic__module__thrift_types.MyStruct,
            MyStructPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MyStructPatch:
            return MyStructPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__test__fixtures__basic__module__thrift_types.MyStruct))
    @property
    def myUnion(self) -> UnqualifiedFieldPatch[
            _fbthrift__test__fixtures__basic__module__thrift_types.MyUnion,
            MyUnionPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MyUnionPatch:
            return MyUnionPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            4,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__test__fixtures__basic__module__thrift_types.MyUnion))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.MyExceptionSafePatch:
        return _fbthrift_safe_patch_types.MyExceptionSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.MyExceptionSafePatch) -> MyExceptionPatch:
        patch = MyExceptionPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch

    def merge(self, other: MyExceptionPatch) -> None:
        self._patch.merge(other._patch)


class MyExceptionWithMessagePatch(
    BaseStructPatch[_fbthrift__test__fixtures__basic__module__thrift_types.MyExceptionWithMessage]
):
    pass
    @property
    def MyIntField(self) -> UnqualifiedFieldPatch[
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
    def MyStringField(self) -> UnqualifiedFieldPatch[
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
    def myStruct(self) -> UnqualifiedFieldPatch[
            _fbthrift__test__fixtures__basic__module__thrift_types.MyStruct,
            MyStructPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MyStructPatch:
            return MyStructPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__test__fixtures__basic__module__thrift_types.MyStruct))
    @property
    def myUnion(self) -> UnqualifiedFieldPatch[
            _fbthrift__test__fixtures__basic__module__thrift_types.MyUnion,
            MyUnionPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MyUnionPatch:
            return MyUnionPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            4,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__test__fixtures__basic__module__thrift_types.MyUnion))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.MyExceptionWithMessageSafePatch:
        return _fbthrift_safe_patch_types.MyExceptionWithMessageSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.MyExceptionWithMessageSafePatch) -> MyExceptionWithMessagePatch:
        patch = MyExceptionWithMessagePatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch

    def merge(self, other: MyExceptionWithMessagePatch) -> None:
        self._patch.merge(other._patch)


class ReservedKeywordPatch(
    BaseStructPatch[_fbthrift__test__fixtures__basic__module__thrift_types.ReservedKeyword]
):
    pass
    @property
    def reserved_field(self) -> UnqualifiedFieldPatch[
            builtins.int,
            I32Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I32Patch:
            return patch.as_i32_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.typeinfo_i32)

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.ReservedKeywordSafePatch:
        return _fbthrift_safe_patch_types.ReservedKeywordSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.ReservedKeywordSafePatch) -> ReservedKeywordPatch:
        patch = ReservedKeywordPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch

    def merge(self, other: ReservedKeywordPatch) -> None:
        self._patch.merge(other._patch)


class UnionToBeRenamedPatch(
    BaseUnionPatch[_fbthrift__test__fixtures__basic__module__thrift_types.UnionToBeRenamed]
):
    pass
    @property
    def reserved_field(self) -> OptionalFieldPatch[
            builtins.int,
            I32Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I32Patch:
            return patch.as_i32_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.typeinfo_i32)

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.UnionToBeRenamedSafePatch:
        return _fbthrift_safe_patch_types.UnionToBeRenamedSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.UnionToBeRenamedSafePatch) -> UnionToBeRenamedPatch:
        patch = UnionToBeRenamedPatch()
        DynamicPatch = DynamicUnionPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch

    def merge(self, other: UnionToBeRenamedPatch) -> None:
        self._patch.merge(other._patch)

