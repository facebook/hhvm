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

import apache.thrift.protocol.protocol.thrift_types as _fbthrift__apache__thrift__protocol__protocol__thrift_types
import apache.thrift.protocol.gen_safe_patch_protocol.thrift_types as _fbthrift_safe_patch_types


import apache.thrift.protocol.detail.protocol_detail.thrift_types as _fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types
import apache.thrift.protocol.detail.protocol_detail.thrift_patch

import apache.thrift.type.id.thrift_types as _fbthrift__apache__thrift__type__id__thrift_types
import apache.thrift.type.id.thrift_patch

import apache.thrift.type.standard.thrift_types as _fbthrift__apache__thrift__type__standard__thrift_types
import apache.thrift.type.standard.thrift_patch

import apache.thrift.type.type.thrift_types as _fbthrift__apache__thrift__type__type__thrift_types
import apache.thrift.type.type.thrift_patch

import apache.thrift.type.type_rep.thrift_types as _fbthrift__apache__thrift__type__type_rep__thrift_types
import apache.thrift.type.type_rep.thrift_patch


class MaskedDataPatch(
    BaseUnionPatch[_fbthrift__apache__thrift__protocol__protocol__thrift_types.MaskedData]
):
    pass
    @property
    def full(self) -> OptionalFieldPatch[
            int,
            I64Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I64Patch:
            return patch.as_i64_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.typeinfo_i64)
    @property
    def fields(self) -> OptionalFieldPatch[
            _typing.Mapping[int, _fbthrift__apache__thrift__protocol__protocol__thrift_types.MaskedData],
            MapPatch[int, _fbthrift__apache__thrift__protocol__protocol__thrift_types.MaskedData, MaskedDataPatch]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MapPatch[int, _fbthrift__apache__thrift__protocol__protocol__thrift_types.MaskedData, MaskedDataPatch]:
            def cast_dynamic_patch_to_typed_map_value_patch(patch: DynamicPatch, type_info) -> MaskedDataPatch:
                return MaskedDataPatch(patch)
            return MapPatch(cast_dynamic_patch_to_typed_map_value_patch, patch.as_map_patch(), type_info)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.MapTypeInfo(_fbthrift_python_types.typeinfo_i16, _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__protocol__protocol__thrift_types.MaskedData)))
    @property
    def values(self) -> OptionalFieldPatch[
            _typing.Mapping[int, _fbthrift__apache__thrift__protocol__protocol__thrift_types.MaskedData],
            MapPatch[int, _fbthrift__apache__thrift__protocol__protocol__thrift_types.MaskedData, MaskedDataPatch]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MapPatch[int, _fbthrift__apache__thrift__protocol__protocol__thrift_types.MaskedData, MaskedDataPatch]:
            def cast_dynamic_patch_to_typed_map_value_patch(patch: DynamicPatch, type_info) -> MaskedDataPatch:
                return MaskedDataPatch(patch)
            return MapPatch(cast_dynamic_patch_to_typed_map_value_patch, patch.as_map_patch(), type_info)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.MapTypeInfo(_fbthrift_python_types.typeinfo_i64, _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__protocol__protocol__thrift_types.MaskedData)))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.MaskedDataSafePatch:
        return _fbthrift_safe_patch_types.MaskedDataSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.MaskedDataSafePatch) -> MaskedDataPatch:
        patch = MaskedDataPatch()
        DynamicPatch = DynamicUnionPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class EncodedValuePatch(
    BaseStructPatch[_fbthrift__apache__thrift__protocol__protocol__thrift_types.EncodedValue]
):
    pass
    @property
    def wireType(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__type__thrift_types.BaseTypeEnum,
            EnumPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> EnumPatch:
            return patch.as_enum_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.EnumTypeInfo(_fbthrift__apache__thrift__type__type__thrift_types.BaseTypeEnum))
    @property
    def data(self) -> UnqualifiedFieldPatch[
            _fbthrift_iobuf.IOBuf,
            BinaryPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> BinaryPatch:
            return patch.as_binary_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.typeinfo_iobuf)

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.EncodedValueSafePatch:
        return _fbthrift_safe_patch_types.EncodedValueSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.EncodedValueSafePatch) -> EncodedValuePatch:
        patch = EncodedValuePatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class MaskedProtocolDataPatch(
    BaseStructPatch[_fbthrift__apache__thrift__protocol__protocol__thrift_types.MaskedProtocolData]
):
    pass
    @property
    def protocol(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__type_rep__thrift_types.ProtocolUnion,
            apache.thrift.type.type_rep.thrift_patch.ProtocolUnionPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> apache.thrift.type.type_rep.thrift_patch.ProtocolUnionPatch:
            return apache.thrift.type.type_rep.thrift_patch.ProtocolUnionPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__type_rep__thrift_types.ProtocolUnion))
    @property
    def data(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__protocol__protocol__thrift_types.MaskedData,
            MaskedDataPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MaskedDataPatch:
            return MaskedDataPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__protocol__protocol__thrift_types.MaskedData))
    @property
    def values(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[_fbthrift__apache__thrift__protocol__protocol__thrift_types.EncodedValue],
            ListPatch[_fbthrift__apache__thrift__protocol__protocol__thrift_types.EncodedValue]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[_fbthrift__apache__thrift__protocol__protocol__thrift_types.EncodedValue]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__protocol__protocol__thrift_types.EncodedValue)))
    @property
    def keys(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[_fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value],
            ListPatch[_fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[_fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            4,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value)))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.MaskedProtocolDataSafePatch:
        return _fbthrift_safe_patch_types.MaskedProtocolDataSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.MaskedProtocolDataSafePatch) -> MaskedProtocolDataPatch:
        patch = MaskedProtocolDataPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch


