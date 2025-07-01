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

import test.fixtures.python_capi.serialized_dep.thrift_types as _fbthrift__test__fixtures__python_capi__serialized_dep__thrift_types
import test.fixtures.python_capi.gen_safe_patch_serialized_dep.thrift_types as _fbthrift_safe_patch_types



class SerializedStructPatch(
    BaseStructPatch[_fbthrift__test__fixtures__python_capi__serialized_dep__thrift_types.SerializedStruct]
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
    def i(self) -> UnqualifiedFieldPatch[
            builtins.int,
            I32Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I32Patch:
            return patch.as_i32_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.typeinfo_i32)
    @property
    def os(self) -> OptionalFieldPatch[
            builtins.str,
            StringPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> StringPatch:
            return patch.as_string_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.typeinfo_string)
    @property
    def rs(self) -> UnqualifiedFieldPatch[
            builtins.str,
            StringPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> StringPatch:
            return patch.as_string_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            4,
            _fbthrift_python_types.typeinfo_string)

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.SerializedStructSafePatch:
        return _fbthrift_safe_patch_types.SerializedStructSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.SerializedStructSafePatch) -> SerializedStructPatch:
        patch = SerializedStructPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class SerializedUnionPatch(
    BaseUnionPatch[_fbthrift__test__fixtures__python_capi__serialized_dep__thrift_types.SerializedUnion]
):
    pass
    @property
    def s(self) -> OptionalFieldPatch[
            builtins.str,
            StringPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> StringPatch:
            return patch.as_string_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.typeinfo_string)
    @property
    def i(self) -> OptionalFieldPatch[
            builtins.int,
            I32Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I32Patch:
            return patch.as_i32_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.typeinfo_i32)

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.SerializedUnionSafePatch:
        return _fbthrift_safe_patch_types.SerializedUnionSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.SerializedUnionSafePatch) -> SerializedUnionPatch:
        patch = SerializedUnionPatch()
        DynamicPatch = DynamicUnionPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class SerializedErrorPatch(
    BaseStructPatch[_fbthrift__test__fixtures__python_capi__serialized_dep__thrift_types.SerializedError]
):
    pass
    @property
    def msg(self) -> UnqualifiedFieldPatch[
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
    def os(self) -> OptionalFieldPatch[
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
    def rs(self) -> UnqualifiedFieldPatch[
            builtins.str,
            StringPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> StringPatch:
            return patch.as_string_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.typeinfo_string)

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.SerializedErrorSafePatch:
        return _fbthrift_safe_patch_types.SerializedErrorSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.SerializedErrorSafePatch) -> SerializedErrorPatch:
        patch = SerializedErrorPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class MarshalStructPatch(
    BaseStructPatch[_fbthrift__test__fixtures__python_capi__serialized_dep__thrift_types.MarshalStruct]
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
    def i(self) -> UnqualifiedFieldPatch[
            builtins.int,
            I32Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I32Patch:
            return patch.as_i32_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.typeinfo_i32)
    @property
    def os(self) -> OptionalFieldPatch[
            builtins.str,
            StringPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> StringPatch:
            return patch.as_string_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.typeinfo_string)
    @property
    def rs(self) -> UnqualifiedFieldPatch[
            builtins.str,
            StringPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> StringPatch:
            return patch.as_string_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            4,
            _fbthrift_python_types.typeinfo_string)

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.MarshalStructSafePatch:
        return _fbthrift_safe_patch_types.MarshalStructSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.MarshalStructSafePatch) -> MarshalStructPatch:
        patch = MarshalStructPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class MarshalUnionPatch(
    BaseUnionPatch[_fbthrift__test__fixtures__python_capi__serialized_dep__thrift_types.MarshalUnion]
):
    pass
    @property
    def s(self) -> OptionalFieldPatch[
            builtins.str,
            StringPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> StringPatch:
            return patch.as_string_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.typeinfo_string)
    @property
    def i(self) -> OptionalFieldPatch[
            builtins.int,
            I32Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I32Patch:
            return patch.as_i32_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.typeinfo_i32)

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.MarshalUnionSafePatch:
        return _fbthrift_safe_patch_types.MarshalUnionSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.MarshalUnionSafePatch) -> MarshalUnionPatch:
        patch = MarshalUnionPatch()
        DynamicPatch = DynamicUnionPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class MarshalErrorPatch(
    BaseStructPatch[_fbthrift__test__fixtures__python_capi__serialized_dep__thrift_types.MarshalError]
):
    pass
    @property
    def msg(self) -> UnqualifiedFieldPatch[
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
    def os(self) -> OptionalFieldPatch[
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
    def rs(self) -> UnqualifiedFieldPatch[
            builtins.str,
            StringPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> StringPatch:
            return patch.as_string_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.typeinfo_string)

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.MarshalErrorSafePatch:
        return _fbthrift_safe_patch_types.MarshalErrorSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.MarshalErrorSafePatch) -> MarshalErrorPatch:
        patch = MarshalErrorPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch


