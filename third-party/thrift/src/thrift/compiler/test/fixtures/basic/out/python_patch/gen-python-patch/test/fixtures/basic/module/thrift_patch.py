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
    DynamicPatch
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
            int,
            I64Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I64Patch:
            return patch.as_i64_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.typeinfo_i64)

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


    @property
    def MyStringField(self) -> UnqualifiedFieldPatch[
            str,
            StringPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> StringPatch:
            return patch.as_string_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
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


    @property
    def myEnum(self) -> UnqualifiedFieldPatch[
            _fbthrift__test__fixtures__basic__module__thrift_types.MyEnum,
            _fbthrift__test__fixtures__basic__module__thrift_types.MyEnum]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> _fbthrift__test__fixtures__basic__module__thrift_types.MyEnum:
            return patch.as_enum_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            4,
            _fbthrift_python_types.EnumTypeInfo(_fbthrift__test__fixtures__basic__module__thrift_types.MyEnum))

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


    @property
    def oneway(self) -> UnqualifiedFieldPatch[
            bool,
            BoolPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> BoolPatch:
            return patch.as_bool_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            5,
            _fbthrift_python_types.typeinfo_bool)

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


    @property
    def readonly(self) -> UnqualifiedFieldPatch[
            bool,
            BoolPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> BoolPatch:
            return patch.as_bool_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            6,
            _fbthrift_python_types.typeinfo_bool)

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


    @property
    def idempotent(self) -> UnqualifiedFieldPatch[
            bool,
            BoolPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> BoolPatch:
            return patch.as_bool_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            7,
            _fbthrift_python_types.typeinfo_bool)

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


    @property
    def floatSet(self) -> UnqualifiedFieldPatch[
            _typing.AbstractSet[float],
            SetPatch[float]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> SetPatch[float]:
            return SetPatch(patch.as_set_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            8,
            _fbthrift_python_types.SetTypeInfo(_fbthrift_python_types.typeinfo_float))

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


    @property
    def no_hack_codegen_field(self) -> UnqualifiedFieldPatch[
            str,
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



class ContainersPatch(
    BaseStructPatch[_fbthrift__test__fixtures__basic__module__thrift_types.Containers]
):
    pass
    @property
    def I32List(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[int],
            ListPatch[int]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[int]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.typeinfo_i32))

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


    @property
    def StringSet(self) -> UnqualifiedFieldPatch[
            _typing.AbstractSet[str],
            SetPatch[str]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> SetPatch[str]:
            return SetPatch(patch.as_set_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.SetTypeInfo(_fbthrift_python_types.typeinfo_string))

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


    @property
    def StringToI64Map(self) -> UnqualifiedFieldPatch[
            _typing.Mapping[str, int],
            MapPatch[str, int, I64Patch]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MapPatch[str, int, I64Patch]:
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



class MyDataItemPatch(
    BaseStructPatch[_fbthrift__test__fixtures__basic__module__thrift_types.MyDataItem]
):
    pass

class MyUnionPatch(
    BaseUnionPatch[_fbthrift__test__fixtures__basic__module__thrift_types.MyUnion]
):
    pass
    @property
    def myEnum(self) -> OptionalFieldPatch[
            _fbthrift__test__fixtures__basic__module__thrift_types.MyEnum,
            _fbthrift__test__fixtures__basic__module__thrift_types.MyEnum]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> _fbthrift__test__fixtures__basic__module__thrift_types.MyEnum:
            return patch.as_enum_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.EnumTypeInfo(_fbthrift__test__fixtures__basic__module__thrift_types.MyEnum))

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


    @property
    def floatSet(self) -> OptionalFieldPatch[
            _typing.AbstractSet[float],
            SetPatch[float]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> SetPatch[float]:
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



class MyExceptionPatch(
    BaseStructPatch[_fbthrift__test__fixtures__basic__module__thrift_types.MyException]
):
    pass
    @property
    def MyIntField(self) -> UnqualifiedFieldPatch[
            int,
            I64Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I64Patch:
            return patch.as_i64_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.typeinfo_i64)

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


    @property
    def MyStringField(self) -> UnqualifiedFieldPatch[
            str,
            StringPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> StringPatch:
            return patch.as_string_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.typeinfo_string)

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



class MyExceptionWithMessagePatch(
    BaseStructPatch[_fbthrift__test__fixtures__basic__module__thrift_types.MyExceptionWithMessage]
):
    pass
    @property
    def MyIntField(self) -> UnqualifiedFieldPatch[
            int,
            I64Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I64Patch:
            return patch.as_i64_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.typeinfo_i64)

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


    @property
    def MyStringField(self) -> UnqualifiedFieldPatch[
            str,
            StringPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> StringPatch:
            return patch.as_string_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.typeinfo_string)

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



class ReservedKeywordPatch(
    BaseStructPatch[_fbthrift__test__fixtures__basic__module__thrift_types.ReservedKeyword]
):
    pass
    @property
    def reserved_field(self) -> UnqualifiedFieldPatch[
            int,
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



class UnionToBeRenamedPatch(
    BaseUnionPatch[_fbthrift__test__fixtures__basic__module__thrift_types.UnionToBeRenamed]
):
    pass
    @property
    def reserved_field(self) -> OptionalFieldPatch[
            int,
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


