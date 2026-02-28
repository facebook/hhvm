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

import apache.thrift.protocol.detail.protocol_detail.thrift_types as _fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types
import apache.thrift.protocol.detail.gen_safe_patch_protocol_detail.thrift_types as _fbthrift_safe_patch_types



class ObjectPatch(
    BaseStructPatch[_fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Object]
):
    pass
    @property
    def type(self) -> UnqualifiedFieldPatch[
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
    def members(self) -> UnqualifiedFieldPatch[
            _typing.Mapping[builtins.int, _fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value],
            MapPatch[builtins.int, _fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value, ValuePatch]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MapPatch[builtins.int, _fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value, ValuePatch]:
            def cast_dynamic_patch_to_typed_map_value_patch(patch: DynamicPatch, type_info) -> ValuePatch:
                return ValuePatch(patch)
            return MapPatch(cast_dynamic_patch_to_typed_map_value_patch, patch.as_map_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.MapTypeInfo(_fbthrift_python_types.typeinfo_i16, _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value)))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.ObjectSafePatch:
        return _fbthrift_safe_patch_types.ObjectSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.ObjectSafePatch) -> ObjectPatch:
        patch = ObjectPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch

    def merge(self, other: ObjectPatch) -> None:
        self._patch.merge(other._patch)


class ValuePatch(
    BaseUnionPatch[_fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value]
):
    pass
    @property
    def boolValue(self) -> OptionalFieldPatch[
            builtins.bool,
            BoolPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> BoolPatch:
            return patch.as_bool_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.typeinfo_bool)
    @property
    def byteValue(self) -> OptionalFieldPatch[
            builtins.int,
            BytePatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> BytePatch:
            return patch.as_byte_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.typeinfo_byte)
    @property
    def i16Value(self) -> OptionalFieldPatch[
            builtins.int,
            I16Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I16Patch:
            return patch.as_i16_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.typeinfo_i16)
    @property
    def i32Value(self) -> OptionalFieldPatch[
            builtins.int,
            I32Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I32Patch:
            return patch.as_i32_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            4,
            _fbthrift_python_types.typeinfo_i32)
    @property
    def i64Value(self) -> OptionalFieldPatch[
            builtins.int,
            I64Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I64Patch:
            return patch.as_i64_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            5,
            _fbthrift_python_types.typeinfo_i64)
    @property
    def floatValue(self) -> OptionalFieldPatch[
            builtins.float,
            FloatPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> FloatPatch:
            return patch.as_float_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            6,
            _fbthrift_python_types.typeinfo_float)
    @property
    def doubleValue(self) -> OptionalFieldPatch[
            builtins.float,
            DoublePatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> DoublePatch:
            return patch.as_double_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            7,
            _fbthrift_python_types.typeinfo_double)
    @property
    def stringValue(self) -> OptionalFieldPatch[
            builtins.bytes,
            BinaryPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> BinaryPatch:
            return patch.as_binary_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            8,
            _fbthrift_python_types.typeinfo_binary)
    @property
    def binaryValue(self) -> OptionalFieldPatch[
            _fbthrift_iobuf.IOBuf,
            BinaryPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> BinaryPatch:
            return patch.as_binary_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            9,
            _fbthrift_python_types.typeinfo_iobuf)
    @property
    def objectValue(self) -> OptionalFieldPatch[
            _fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Object,
            ObjectPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ObjectPatch:
            return ObjectPatch(patch)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            11,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Object))
    @property
    def listValue(self) -> OptionalFieldPatch[
            _typing.Sequence[_fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value],
            ListPatch[_fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[_fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value]:
            return ListPatch(patch.as_list_patch(), type_info)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            14,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value)))
    @property
    def setValue(self) -> OptionalFieldPatch[
            _typing.AbstractSet[_fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value],
            SetPatch[_fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> SetPatch[_fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value]:
            return SetPatch(patch.as_set_patch(), type_info)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            15,
            _fbthrift_python_types.SetTypeInfo(_fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value)))
    @property
    def mapValue(self) -> OptionalFieldPatch[
            _typing.Mapping[_fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value, _fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value],
            MapPatch[_fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value, _fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value, ValuePatch]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MapPatch[_fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value, _fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value, ValuePatch]:
            def cast_dynamic_patch_to_typed_map_value_patch(patch: DynamicPatch, type_info) -> ValuePatch:
                return ValuePatch(patch)
            return MapPatch(cast_dynamic_patch_to_typed_map_value_patch, patch.as_map_patch(), type_info)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            16,
            _fbthrift_python_types.MapTypeInfo(_fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value), _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value)))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.ValueSafePatch:
        return _fbthrift_safe_patch_types.ValueSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.ValueSafePatch) -> ValuePatch:
        patch = ValuePatch()
        DynamicPatch = DynamicUnionPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch

    def merge(self, other: ValuePatch) -> None:
        self._patch.merge(other._patch)

