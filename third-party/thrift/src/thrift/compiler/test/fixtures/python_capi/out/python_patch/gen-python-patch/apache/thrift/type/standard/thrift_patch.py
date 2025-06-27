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

import apache.thrift.type.standard.thrift_types as _fbthrift__apache__thrift__type__standard__thrift_types
import apache.thrift.type.gen_safe_patch_standard.thrift_types as _fbthrift_safe_patch_types



class TypeUriPatch(
    BaseUnionPatch[_fbthrift__apache__thrift__type__standard__thrift_types.TypeUri]
):
    pass
    @property
    def uri(self) -> OptionalFieldPatch[
            str,
            StringPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> StringPatch:
            return patch.as_string_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.typeinfo_string)
    @property
    def typeHashPrefixSha2_256(self) -> OptionalFieldPatch[
            bytes,
            BinaryPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> BinaryPatch:
            return patch.as_binary_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.typeinfo_binary)
    @property
    def scopedName(self) -> OptionalFieldPatch[
            str,
            StringPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> StringPatch:
            return patch.as_string_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.typeinfo_string)
    @property
    def definitionKey(self) -> OptionalFieldPatch[
            bytes,
            BinaryPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> BinaryPatch:
            return patch.as_binary_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            4,
            _fbthrift_python_types.typeinfo_binary)

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.TypeUriSafePatch:
        return _fbthrift_safe_patch_types.TypeUriSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.TypeUriSafePatch) -> TypeUriPatch:
        patch = TypeUriPatch()
        DynamicPatch = DynamicUnionPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class TypeNamePatch(
    BaseUnionPatch[_fbthrift__apache__thrift__type__standard__thrift_types.TypeName]
):
    pass
    @property
    def boolType(self) -> OptionalFieldPatch[
            _fbthrift__apache__thrift__type__standard__thrift_types.Void,
            EnumPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> EnumPatch:
            return patch.as_enum_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.EnumTypeInfo(_fbthrift__apache__thrift__type__standard__thrift_types.Void))
    @property
    def byteType(self) -> OptionalFieldPatch[
            _fbthrift__apache__thrift__type__standard__thrift_types.Void,
            EnumPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> EnumPatch:
            return patch.as_enum_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.EnumTypeInfo(_fbthrift__apache__thrift__type__standard__thrift_types.Void))
    @property
    def i16Type(self) -> OptionalFieldPatch[
            _fbthrift__apache__thrift__type__standard__thrift_types.Void,
            EnumPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> EnumPatch:
            return patch.as_enum_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.EnumTypeInfo(_fbthrift__apache__thrift__type__standard__thrift_types.Void))
    @property
    def i32Type(self) -> OptionalFieldPatch[
            _fbthrift__apache__thrift__type__standard__thrift_types.Void,
            EnumPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> EnumPatch:
            return patch.as_enum_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            4,
            _fbthrift_python_types.EnumTypeInfo(_fbthrift__apache__thrift__type__standard__thrift_types.Void))
    @property
    def i64Type(self) -> OptionalFieldPatch[
            _fbthrift__apache__thrift__type__standard__thrift_types.Void,
            EnumPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> EnumPatch:
            return patch.as_enum_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            5,
            _fbthrift_python_types.EnumTypeInfo(_fbthrift__apache__thrift__type__standard__thrift_types.Void))
    @property
    def floatType(self) -> OptionalFieldPatch[
            _fbthrift__apache__thrift__type__standard__thrift_types.Void,
            EnumPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> EnumPatch:
            return patch.as_enum_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            6,
            _fbthrift_python_types.EnumTypeInfo(_fbthrift__apache__thrift__type__standard__thrift_types.Void))
    @property
    def doubleType(self) -> OptionalFieldPatch[
            _fbthrift__apache__thrift__type__standard__thrift_types.Void,
            EnumPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> EnumPatch:
            return patch.as_enum_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            7,
            _fbthrift_python_types.EnumTypeInfo(_fbthrift__apache__thrift__type__standard__thrift_types.Void))
    @property
    def stringType(self) -> OptionalFieldPatch[
            _fbthrift__apache__thrift__type__standard__thrift_types.Void,
            EnumPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> EnumPatch:
            return patch.as_enum_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            8,
            _fbthrift_python_types.EnumTypeInfo(_fbthrift__apache__thrift__type__standard__thrift_types.Void))
    @property
    def binaryType(self) -> OptionalFieldPatch[
            _fbthrift__apache__thrift__type__standard__thrift_types.Void,
            EnumPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> EnumPatch:
            return patch.as_enum_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            9,
            _fbthrift_python_types.EnumTypeInfo(_fbthrift__apache__thrift__type__standard__thrift_types.Void))
    @property
    def enumType(self) -> OptionalFieldPatch[
            _fbthrift__apache__thrift__type__standard__thrift_types.TypeUri,
            TypeUriPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> TypeUriPatch:
            return TypeUriPatch(patch)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            10,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__standard__thrift_types.TypeUri))
    @property
    def structType(self) -> OptionalFieldPatch[
            _fbthrift__apache__thrift__type__standard__thrift_types.TypeUri,
            TypeUriPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> TypeUriPatch:
            return TypeUriPatch(patch)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            11,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__standard__thrift_types.TypeUri))
    @property
    def unionType(self) -> OptionalFieldPatch[
            _fbthrift__apache__thrift__type__standard__thrift_types.TypeUri,
            TypeUriPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> TypeUriPatch:
            return TypeUriPatch(patch)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            12,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__standard__thrift_types.TypeUri))
    @property
    def exceptionType(self) -> OptionalFieldPatch[
            _fbthrift__apache__thrift__type__standard__thrift_types.TypeUri,
            TypeUriPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> TypeUriPatch:
            return TypeUriPatch(patch)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            13,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__standard__thrift_types.TypeUri))
    @property
    def listType(self) -> OptionalFieldPatch[
            _fbthrift__apache__thrift__type__standard__thrift_types.Void,
            EnumPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> EnumPatch:
            return patch.as_enum_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            14,
            _fbthrift_python_types.EnumTypeInfo(_fbthrift__apache__thrift__type__standard__thrift_types.Void))
    @property
    def setType(self) -> OptionalFieldPatch[
            _fbthrift__apache__thrift__type__standard__thrift_types.Void,
            EnumPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> EnumPatch:
            return patch.as_enum_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            15,
            _fbthrift_python_types.EnumTypeInfo(_fbthrift__apache__thrift__type__standard__thrift_types.Void))
    @property
    def mapType(self) -> OptionalFieldPatch[
            _fbthrift__apache__thrift__type__standard__thrift_types.Void,
            EnumPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> EnumPatch:
            return patch.as_enum_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            16,
            _fbthrift_python_types.EnumTypeInfo(_fbthrift__apache__thrift__type__standard__thrift_types.Void))
    @property
    def typedefType(self) -> OptionalFieldPatch[
            _fbthrift__apache__thrift__type__standard__thrift_types.TypeUri,
            TypeUriPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> TypeUriPatch:
            return TypeUriPatch(patch)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            17,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__standard__thrift_types.TypeUri))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.TypeNameSafePatch:
        return _fbthrift_safe_patch_types.TypeNameSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.TypeNameSafePatch) -> TypeNamePatch:
        patch = TypeNamePatch()
        DynamicPatch = DynamicUnionPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch


