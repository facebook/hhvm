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

import apache.thrift.type.type_rep.thrift_types as _fbthrift__apache__thrift__type__type_rep__thrift_types
import apache.thrift.type.gen_safe_patch_type_rep.thrift_types as _fbthrift_safe_patch_types


import apache.thrift.type.id.thrift_types as _fbthrift__apache__thrift__type__id__thrift_types
import apache.thrift.type.id.thrift_patch

import apache.thrift.type.standard.thrift_types as _fbthrift__apache__thrift__type__standard__thrift_types
import apache.thrift.type.standard.thrift_patch


class ProtocolUnionPatch(
    BaseUnionPatch[_fbthrift__apache__thrift__type__type_rep__thrift_types.ProtocolUnion]
):
    pass
    @property
    def standard(self) -> OptionalFieldPatch[
            _fbthrift__apache__thrift__type__standard__thrift_types.StandardProtocol,
            EnumPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> EnumPatch:
            return patch.as_enum_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.EnumTypeInfo(_fbthrift__apache__thrift__type__standard__thrift_types.StandardProtocol))
    @property
    def custom(self) -> OptionalFieldPatch[
            builtins.str,
            StringPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> StringPatch:
            return patch.as_string_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.typeinfo_string)
    @property
    def id(self) -> OptionalFieldPatch[
            builtins.int,
            I64Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I64Patch:
            return patch.as_i64_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.typeinfo_i64)
    @property
    def compressed(self) -> OptionalFieldPatch[
            _fbthrift__apache__thrift__type__type_rep__thrift_types.CompressedProtocolStruct,
            CompressedProtocolStructPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> CompressedProtocolStructPatch:
            return CompressedProtocolStructPatch(patch)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            4,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__type_rep__thrift_types.CompressedProtocolStruct))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.ProtocolUnionSafePatch:
        return _fbthrift_safe_patch_types.ProtocolUnionSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.ProtocolUnionSafePatch) -> ProtocolUnionPatch:
        patch = ProtocolUnionPatch()
        DynamicPatch = DynamicUnionPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class TypeStructPatch(
    BaseStructPatch[_fbthrift__apache__thrift__type__type_rep__thrift_types.TypeStruct]
):
    pass
    @property
    def name(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__standard__thrift_types.TypeName,
            apache.thrift.type.standard.thrift_patch.TypeNamePatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> apache.thrift.type.standard.thrift_patch.TypeNamePatch:
            return apache.thrift.type.standard.thrift_patch.TypeNamePatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__standard__thrift_types.TypeName))
    @property
    def params(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[_fbthrift__apache__thrift__type__type_rep__thrift_types.TypeStruct],
            ListPatch[_fbthrift__apache__thrift__type__type_rep__thrift_types.TypeStruct]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[_fbthrift__apache__thrift__type__type_rep__thrift_types.TypeStruct]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__type_rep__thrift_types.TypeStruct)))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.TypeStructSafePatch:
        return _fbthrift_safe_patch_types.TypeStructSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.TypeStructSafePatch) -> TypeStructPatch:
        patch = TypeStructPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class CompressedProtocolStructPatch(
    BaseStructPatch[_fbthrift__apache__thrift__type__type_rep__thrift_types.CompressedProtocolStruct]
):
    pass
    @property
    def protocol(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__type_rep__thrift_types.ProtocolUnion,
            ProtocolUnionPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ProtocolUnionPatch:
            return ProtocolUnionPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__type_rep__thrift_types.ProtocolUnion))
    @property
    def compression(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__type_rep__thrift_types.CompressionSpecStruct,
            CompressionSpecStructPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> CompressionSpecStructPatch:
            return CompressionSpecStructPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__type_rep__thrift_types.CompressionSpecStruct))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.CompressedProtocolStructSafePatch:
        return _fbthrift_safe_patch_types.CompressedProtocolStructSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.CompressedProtocolStructSafePatch) -> CompressedProtocolStructPatch:
        patch = CompressedProtocolStructPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class CompressionSpecStructPatch(
    BaseStructPatch[_fbthrift__apache__thrift__type__type_rep__thrift_types.CompressionSpecStruct]
):
    pass
    @property
    def id(self) -> UnqualifiedFieldPatch[
            builtins.int,
            BytePatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> BytePatch:
            return patch.as_byte_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.typeinfo_byte)
    @property
    def uncompressed_data_size_bytes(self) -> OptionalFieldPatch[
            builtins.int,
            I64Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I64Patch:
            return patch.as_i64_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.typeinfo_i64)
    @property
    def compression_parameters(self) -> OptionalFieldPatch[
            builtins.bytes,
            BinaryPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> BinaryPatch:
            return patch.as_binary_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.typeinfo_binary)

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.CompressionSpecStructSafePatch:
        return _fbthrift_safe_patch_types.CompressionSpecStructSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.CompressionSpecStructSafePatch) -> CompressionSpecStructPatch:
        patch = CompressionSpecStructPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch


