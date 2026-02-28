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

import test.fixtures.python_capi.thrift_dep.thrift_types as _fbthrift__test__fixtures__python_capi__thrift_dep__thrift_types
import test.fixtures.python_capi.gen_safe_patch_thrift_dep.thrift_types as _fbthrift_safe_patch_types



class DepStructPatch(
    BaseStructPatch[_fbthrift__test__fixtures__python_capi__thrift_dep__thrift_types.DepStruct]
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

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.DepStructSafePatch:
        return _fbthrift_safe_patch_types.DepStructSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.DepStructSafePatch) -> DepStructPatch:
        patch = DepStructPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch

    def merge(self, other: DepStructPatch) -> None:
        self._patch.merge(other._patch)


class SomeErrorPatch(
    BaseStructPatch[_fbthrift__test__fixtures__python_capi__thrift_dep__thrift_types.SomeError]
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

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.SomeErrorSafePatch:
        return _fbthrift_safe_patch_types.SomeErrorSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.SomeErrorSafePatch) -> SomeErrorPatch:
        patch = SomeErrorPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch

    def merge(self, other: SomeErrorPatch) -> None:
        self._patch.merge(other._patch)

