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

import apache.thrift.type.schema.thrift_types as _fbthrift__apache__thrift__type__schema__thrift_types
import apache.thrift.type.gen_safe_patch_schema.thrift_types as _fbthrift_safe_patch_types


import apache.thrift.protocol.detail.protocol_detail.thrift_types as _fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types
import apache.thrift.protocol.detail.protocol_detail.thrift_patch

import apache.thrift.protocol.protocol.thrift_types as _fbthrift__apache__thrift__protocol__protocol__thrift_types
import apache.thrift.protocol.protocol.thrift_patch

import apache.thrift.type.id.thrift_types as _fbthrift__apache__thrift__type__id__thrift_types
import apache.thrift.type.id.thrift_patch

import apache.thrift.type.standard.thrift_types as _fbthrift__apache__thrift__type__standard__thrift_types
import apache.thrift.type.standard.thrift_patch

import apache.thrift.type.type.thrift_types as _fbthrift__apache__thrift__type__type__thrift_types
import apache.thrift.type.type.thrift_patch

import apache.thrift.type.type_rep.thrift_types as _fbthrift__apache__thrift__type__type_rep__thrift_types
import apache.thrift.type.type_rep.thrift_patch


class AnnotationPatch(
    BaseStructPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Annotation]
):
    pass
    @property
    def fields(self) -> UnqualifiedFieldPatch[
            _typing.Mapping[str, _fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value],
            MapPatch[str, _fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value, apache.thrift.protocol.detail.protocol_detail.thrift_patch.ValuePatch]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MapPatch[str, _fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value, apache.thrift.protocol.detail.protocol_detail.thrift_patch.ValuePatch]:
            def cast_dynamic_patch_to_typed_map_value_patch(patch: DynamicPatch, type_info) -> apache.thrift.protocol.detail.protocol_detail.thrift_patch.ValuePatch:
                return apache.thrift.protocol.detail.protocol_detail.thrift_patch.ValuePatch(patch)
            return MapPatch(cast_dynamic_patch_to_typed_map_value_patch, patch.as_map_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.MapTypeInfo(_fbthrift_python_types.typeinfo_string, _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value)))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.AnnotationSafePatch:
        return _fbthrift_safe_patch_types.AnnotationSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.AnnotationSafePatch) -> AnnotationPatch:
        patch = AnnotationPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class StructuredAnnotationPatch(
    BaseStructPatch[_fbthrift__apache__thrift__type__schema__thrift_types.StructuredAnnotation]
):
    pass
    @property
    def type(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__standard__thrift_types.TypeUri,
            apache.thrift.type.standard.thrift_patch.TypeUriPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> apache.thrift.type.standard.thrift_patch.TypeUriPatch:
            return apache.thrift.type.standard.thrift_patch.TypeUriPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__standard__thrift_types.TypeUri))
    @property
    def fields(self) -> UnqualifiedFieldPatch[
            _typing.Mapping[str, _fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value],
            MapPatch[str, _fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value, apache.thrift.protocol.detail.protocol_detail.thrift_patch.ValuePatch]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MapPatch[str, _fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value, apache.thrift.protocol.detail.protocol_detail.thrift_patch.ValuePatch]:
            def cast_dynamic_patch_to_typed_map_value_patch(patch: DynamicPatch, type_info) -> apache.thrift.protocol.detail.protocol_detail.thrift_patch.ValuePatch:
                return apache.thrift.protocol.detail.protocol_detail.thrift_patch.ValuePatch(patch)
            return MapPatch(cast_dynamic_patch_to_typed_map_value_patch, patch.as_map_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.MapTypeInfo(_fbthrift_python_types.typeinfo_string, _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value)))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.StructuredAnnotationSafePatch:
        return _fbthrift_safe_patch_types.StructuredAnnotationSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.StructuredAnnotationSafePatch) -> StructuredAnnotationPatch:
        patch = StructuredAnnotationPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class DecodedUriPatch(
    BaseStructPatch[_fbthrift__apache__thrift__type__schema__thrift_types.DecodedUri]
):
    pass
    @property
    def scheme(self) -> UnqualifiedFieldPatch[
            str,
            StringPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> StringPatch:
            return patch.as_string_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.typeinfo_string)
    @property
    def domain(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[str],
            ListPatch[str]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[str]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.typeinfo_string))
    @property
    def path(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[str],
            ListPatch[str]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[str]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            4,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.typeinfo_string))
    @property
    def query(self) -> UnqualifiedFieldPatch[
            _typing.Mapping[str, str],
            MapPatch[str, str, StringPatch]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MapPatch[str, str, StringPatch]:
            def cast_dynamic_patch_to_typed_map_value_patch(patch: DynamicPatch, type_info) -> StringPatch:
                return patch.as_string_patch()
            return MapPatch(cast_dynamic_patch_to_typed_map_value_patch, patch.as_map_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            5,
            _fbthrift_python_types.MapTypeInfo(_fbthrift_python_types.typeinfo_string, _fbthrift_python_types.typeinfo_string))
    @property
    def fragment(self) -> UnqualifiedFieldPatch[
            str,
            StringPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> StringPatch:
            return patch.as_string_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            6,
            _fbthrift_python_types.typeinfo_string)

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.DecodedUriSafePatch:
        return _fbthrift_safe_patch_types.DecodedUriSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.DecodedUriSafePatch) -> DecodedUriPatch:
        patch = DecodedUriPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class DefinitionAttrsPatch(
    BaseStructPatch[_fbthrift__apache__thrift__type__schema__thrift_types.DefinitionAttrs]
):
    pass
    @property
    def name(self) -> UnqualifiedFieldPatch[
            str,
            StringPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> StringPatch:
            return patch.as_string_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.typeinfo_string)
    @property
    def uri(self) -> UnqualifiedFieldPatch[
            str,
            StringPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> StringPatch:
            return patch.as_string_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.typeinfo_string)
    @property
    def structuredAnnotations(self) -> UnqualifiedFieldPatch[
            _typing.AbstractSet[int],
            SetPatch[int]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> SetPatch[int]:
            return SetPatch(patch.as_set_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.SetTypeInfo(_fbthrift_python_types.typeinfo_i64))
    @property
    def unstructuredAnnotations(self) -> UnqualifiedFieldPatch[
            _typing.Mapping[str, str],
            MapPatch[str, str, StringPatch]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MapPatch[str, str, StringPatch]:
            def cast_dynamic_patch_to_typed_map_value_patch(patch: DynamicPatch, type_info) -> StringPatch:
                return patch.as_string_patch()
            return MapPatch(cast_dynamic_patch_to_typed_map_value_patch, patch.as_map_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            4,
            _fbthrift_python_types.MapTypeInfo(_fbthrift_python_types.typeinfo_string, _fbthrift_python_types.typeinfo_string))
    @property
    def docs(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.DocBlock,
            DocBlockPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> DocBlockPatch:
            return DocBlockPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            6,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.DocBlock))
    @property
    def sourceRange(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.SourceRange,
            SourceRangePatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> SourceRangePatch:
            return SourceRangePatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            7,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.SourceRange))
    @property
    def annotations(self) -> UnqualifiedFieldPatch[
            _typing.Mapping[str, _fbthrift__apache__thrift__type__schema__thrift_types.Annotation],
            MapPatch[str, _fbthrift__apache__thrift__type__schema__thrift_types.Annotation, AnnotationPatch]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MapPatch[str, _fbthrift__apache__thrift__type__schema__thrift_types.Annotation, AnnotationPatch]:
            def cast_dynamic_patch_to_typed_map_value_patch(patch: DynamicPatch, type_info) -> AnnotationPatch:
                return AnnotationPatch(patch)
            return MapPatch(cast_dynamic_patch_to_typed_map_value_patch, patch.as_map_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            8,
            _fbthrift_python_types.MapTypeInfo(_fbthrift_python_types.typeinfo_string, _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.Annotation)))
    @property
    def annotationsByKey(self) -> UnqualifiedFieldPatch[
            _typing.Mapping[bytes, _fbthrift__apache__thrift__type__schema__thrift_types.Annotation],
            MapPatch[bytes, _fbthrift__apache__thrift__type__schema__thrift_types.Annotation, AnnotationPatch]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MapPatch[bytes, _fbthrift__apache__thrift__type__schema__thrift_types.Annotation, AnnotationPatch]:
            def cast_dynamic_patch_to_typed_map_value_patch(patch: DynamicPatch, type_info) -> AnnotationPatch:
                return AnnotationPatch(patch)
            return MapPatch(cast_dynamic_patch_to_typed_map_value_patch, patch.as_map_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            9,
            _fbthrift_python_types.MapTypeInfo(_fbthrift_python_types.typeinfo_binary, _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.Annotation)))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.DefinitionAttrsSafePatch:
        return _fbthrift_safe_patch_types.DefinitionAttrsSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.DefinitionAttrsSafePatch) -> DefinitionAttrsPatch:
        patch = DefinitionAttrsPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class SourceRangePatch(
    BaseStructPatch[_fbthrift__apache__thrift__type__schema__thrift_types.SourceRange]
):
    pass
    @property
    def programId(self) -> UnqualifiedFieldPatch[
            int,
            I64Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I64Patch:
            return patch.as_i64_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.typeinfo_i64)
    @property
    def beginLine(self) -> UnqualifiedFieldPatch[
            int,
            I32Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I32Patch:
            return patch.as_i32_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.typeinfo_i32)
    @property
    def beginColumn(self) -> UnqualifiedFieldPatch[
            int,
            I32Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I32Patch:
            return patch.as_i32_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.typeinfo_i32)
    @property
    def endLine(self) -> UnqualifiedFieldPatch[
            int,
            I32Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I32Patch:
            return patch.as_i32_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            4,
            _fbthrift_python_types.typeinfo_i32)
    @property
    def endColumn(self) -> UnqualifiedFieldPatch[
            int,
            I32Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I32Patch:
            return patch.as_i32_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            5,
            _fbthrift_python_types.typeinfo_i32)

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.SourceRangeSafePatch:
        return _fbthrift_safe_patch_types.SourceRangeSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.SourceRangeSafePatch) -> SourceRangePatch:
        patch = SourceRangePatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class DocBlockPatch(
    BaseStructPatch[_fbthrift__apache__thrift__type__schema__thrift_types.DocBlock]
):
    pass
    @property
    def contents(self) -> UnqualifiedFieldPatch[
            str,
            StringPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> StringPatch:
            return patch.as_string_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.typeinfo_string)
    @property
    def sourceRange(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.SourceRange,
            SourceRangePatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> SourceRangePatch:
            return SourceRangePatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.SourceRange))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.DocBlockSafePatch:
        return _fbthrift_safe_patch_types.DocBlockSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.DocBlockSafePatch) -> DocBlockPatch:
        patch = DocBlockPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class InterfaceRefPatch(
    BaseStructPatch[_fbthrift__apache__thrift__type__schema__thrift_types.InterfaceRef]
):
    pass
    @property
    def uri(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__standard__thrift_types.TypeUri,
            apache.thrift.type.standard.thrift_patch.TypeUriPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> apache.thrift.type.standard.thrift_patch.TypeUriPatch:
            return apache.thrift.type.standard.thrift_patch.TypeUriPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__standard__thrift_types.TypeUri))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.InterfaceRefSafePatch:
        return _fbthrift_safe_patch_types.InterfaceRefSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.InterfaceRefSafePatch) -> InterfaceRefPatch:
        patch = InterfaceRefPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class EnumValuePatch(
    BaseStructPatch[_fbthrift__apache__thrift__type__schema__thrift_types.EnumValue]
):
    pass
    @property
    def attrs(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.DefinitionAttrs,
            DefinitionAttrsPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> DefinitionAttrsPatch:
            return DefinitionAttrsPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.DefinitionAttrs))
    @property
    def value(self) -> UnqualifiedFieldPatch[
            int,
            I32Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I32Patch:
            return patch.as_i32_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.typeinfo_i32)

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.EnumValueSafePatch:
        return _fbthrift_safe_patch_types.EnumValueSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.EnumValueSafePatch) -> EnumValuePatch:
        patch = EnumValuePatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class EnumPatch(
    BaseStructPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Enum]
):
    pass
    @property
    def attrs(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.DefinitionAttrs,
            DefinitionAttrsPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> DefinitionAttrsPatch:
            return DefinitionAttrsPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.DefinitionAttrs))
    @property
    def values(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[_fbthrift__apache__thrift__type__schema__thrift_types.EnumValue],
            ListPatch[_fbthrift__apache__thrift__type__schema__thrift_types.EnumValue]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[_fbthrift__apache__thrift__type__schema__thrift_types.EnumValue]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.EnumValue)))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.EnumSafePatch:
        return _fbthrift_safe_patch_types.EnumSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.EnumSafePatch) -> EnumPatch:
        patch = EnumPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class FieldPatch(
    BaseStructPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Field]
):
    pass
    @property
    def id(self) -> UnqualifiedFieldPatch[
            int,
            I16Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I16Patch:
            return patch.as_i16_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.typeinfo_i16)
    @property
    def qualifier(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.FieldQualifier,
            EnumPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> EnumPatch:
            return patch.as_enum_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.EnumTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.FieldQualifier))
    @property
    def type(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__type_rep__thrift_types.TypeStruct,
            apache.thrift.type.type_rep.thrift_patch.TypeStructPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> apache.thrift.type.type_rep.thrift_patch.TypeStructPatch:
            return apache.thrift.type.type_rep.thrift_patch.TypeStructPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__type_rep__thrift_types.TypeStruct))
    @property
    def attrs(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.DefinitionAttrs,
            DefinitionAttrsPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> DefinitionAttrsPatch:
            return DefinitionAttrsPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            4,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.DefinitionAttrs))
    @property
    def customDefault(self) -> UnqualifiedFieldPatch[
            int,
            I64Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I64Patch:
            return patch.as_i64_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            5,
            _fbthrift_python_types.typeinfo_i64)

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.FieldSafePatch:
        return _fbthrift_safe_patch_types.FieldSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.FieldSafePatch) -> FieldPatch:
        patch = FieldPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class StructPatch(
    BaseStructPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Struct]
):
    pass
    @property
    def attrs(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.DefinitionAttrs,
            DefinitionAttrsPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> DefinitionAttrsPatch:
            return DefinitionAttrsPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.DefinitionAttrs))
    @property
    def fields(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[_fbthrift__apache__thrift__type__schema__thrift_types.Field],
            ListPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Field]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Field]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.Field)))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.StructSafePatch:
        return _fbthrift_safe_patch_types.StructSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.StructSafePatch) -> StructPatch:
        patch = StructPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class UnionPatch(
    BaseStructPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Union]
):
    pass
    @property
    def attrs(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.DefinitionAttrs,
            DefinitionAttrsPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> DefinitionAttrsPatch:
            return DefinitionAttrsPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.DefinitionAttrs))
    @property
    def fields(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[_fbthrift__apache__thrift__type__schema__thrift_types.Field],
            ListPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Field]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Field]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.Field)))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.UnionSafePatch:
        return _fbthrift_safe_patch_types.UnionSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.UnionSafePatch) -> UnionPatch:
        patch = UnionPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class ExceptionPatch(
    BaseStructPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Exception]
):
    pass
    @property
    def attrs(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.DefinitionAttrs,
            DefinitionAttrsPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> DefinitionAttrsPatch:
            return DefinitionAttrsPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.DefinitionAttrs))
    @property
    def fields(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[_fbthrift__apache__thrift__type__schema__thrift_types.Field],
            ListPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Field]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Field]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.Field)))
    @property
    def safety(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.ErrorSafety,
            EnumPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> EnumPatch:
            return patch.as_enum_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.EnumTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.ErrorSafety))
    @property
    def kind(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.ErrorKind,
            EnumPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> EnumPatch:
            return patch.as_enum_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            4,
            _fbthrift_python_types.EnumTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.ErrorKind))
    @property
    def blame(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.ErrorBlame,
            EnumPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> EnumPatch:
            return patch.as_enum_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            5,
            _fbthrift_python_types.EnumTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.ErrorBlame))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.ExceptionSafePatch:
        return _fbthrift_safe_patch_types.ExceptionSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.ExceptionSafePatch) -> ExceptionPatch:
        patch = ExceptionPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class ParamlistPatch(
    BaseStructPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Paramlist]
):
    pass
    @property
    def fields(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[_fbthrift__apache__thrift__type__schema__thrift_types.Field],
            ListPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Field]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Field]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.Field)))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.ParamlistSafePatch:
        return _fbthrift_safe_patch_types.ParamlistSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.ParamlistSafePatch) -> ParamlistPatch:
        patch = ParamlistPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class StreamPatch(
    BaseStructPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Stream]
):
    pass
    @property
    def payload(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__type_rep__thrift_types.TypeStruct,
            apache.thrift.type.type_rep.thrift_patch.TypeStructPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> apache.thrift.type.type_rep.thrift_patch.TypeStructPatch:
            return apache.thrift.type.type_rep.thrift_patch.TypeStructPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__type_rep__thrift_types.TypeStruct))
    @property
    def exceptions(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[_fbthrift__apache__thrift__type__schema__thrift_types.Field],
            ListPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Field]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Field]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.Field)))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.StreamSafePatch:
        return _fbthrift_safe_patch_types.StreamSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.StreamSafePatch) -> StreamPatch:
        patch = StreamPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class SinkPatch(
    BaseStructPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Sink]
):
    pass
    @property
    def payload(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__type_rep__thrift_types.TypeStruct,
            apache.thrift.type.type_rep.thrift_patch.TypeStructPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> apache.thrift.type.type_rep.thrift_patch.TypeStructPatch:
            return apache.thrift.type.type_rep.thrift_patch.TypeStructPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__type_rep__thrift_types.TypeStruct))
    @property
    def clientExceptions(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[_fbthrift__apache__thrift__type__schema__thrift_types.Field],
            ListPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Field]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Field]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.Field)))
    @property
    def finalResponse(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__type_rep__thrift_types.TypeStruct,
            apache.thrift.type.type_rep.thrift_patch.TypeStructPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> apache.thrift.type.type_rep.thrift_patch.TypeStructPatch:
            return apache.thrift.type.type_rep.thrift_patch.TypeStructPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__type_rep__thrift_types.TypeStruct))
    @property
    def serverExceptions(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[_fbthrift__apache__thrift__type__schema__thrift_types.Field],
            ListPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Field]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Field]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            4,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.Field)))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.SinkSafePatch:
        return _fbthrift_safe_patch_types.SinkSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.SinkSafePatch) -> SinkPatch:
        patch = SinkPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class InteractionPatch(
    BaseStructPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Interaction]
):
    pass
    @property
    def attrs(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.DefinitionAttrs,
            DefinitionAttrsPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> DefinitionAttrsPatch:
            return DefinitionAttrsPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.DefinitionAttrs))
    @property
    def functions(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[_fbthrift__apache__thrift__type__schema__thrift_types.Function],
            ListPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Function]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Function]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.Function)))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.InteractionSafePatch:
        return _fbthrift_safe_patch_types.InteractionSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.InteractionSafePatch) -> InteractionPatch:
        patch = InteractionPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class ReturnTypePatch(
    BaseUnionPatch[_fbthrift__apache__thrift__type__schema__thrift_types.ReturnType]
):
    pass
    @property
    def thriftType(self) -> OptionalFieldPatch[
            _fbthrift__apache__thrift__type__type_rep__thrift_types.TypeStruct,
            apache.thrift.type.type_rep.thrift_patch.TypeStructPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> apache.thrift.type.type_rep.thrift_patch.TypeStructPatch:
            return apache.thrift.type.type_rep.thrift_patch.TypeStructPatch(patch)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__type_rep__thrift_types.TypeStruct))
    @property
    def streamType(self) -> OptionalFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.Stream,
            StreamPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> StreamPatch:
            return StreamPatch(patch)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.Stream))
    @property
    def sinkType(self) -> OptionalFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.Sink,
            SinkPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> SinkPatch:
            return SinkPatch(patch)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.Sink))
    @property
    def interactionType(self) -> OptionalFieldPatch[
            int,
            I64Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I64Patch:
            return patch.as_i64_patch()

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            4,
            _fbthrift_python_types.typeinfo_i64)

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.ReturnTypeSafePatch:
        return _fbthrift_safe_patch_types.ReturnTypeSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.ReturnTypeSafePatch) -> ReturnTypePatch:
        patch = ReturnTypePatch()
        DynamicPatch = DynamicUnionPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class FunctionPatch(
    BaseStructPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Function]
):
    pass
    @property
    def qualifier(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.FunctionQualifier,
            EnumPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> EnumPatch:
            return patch.as_enum_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.EnumTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.FunctionQualifier))
    @property
    def returnTypes(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[_fbthrift__apache__thrift__type__schema__thrift_types.ReturnType],
            ListPatch[_fbthrift__apache__thrift__type__schema__thrift_types.ReturnType]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[_fbthrift__apache__thrift__type__schema__thrift_types.ReturnType]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.ReturnType)))
    @property
    def attrs(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.DefinitionAttrs,
            DefinitionAttrsPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> DefinitionAttrsPatch:
            return DefinitionAttrsPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.DefinitionAttrs))
    @property
    def paramlist(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.Paramlist,
            ParamlistPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ParamlistPatch:
            return ParamlistPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            4,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.Paramlist))
    @property
    def exceptions(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[_fbthrift__apache__thrift__type__schema__thrift_types.Field],
            ListPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Field]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Field]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            5,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.Field)))
    @property
    def returnType(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__type_rep__thrift_types.TypeStruct,
            apache.thrift.type.type_rep.thrift_patch.TypeStructPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> apache.thrift.type.type_rep.thrift_patch.TypeStructPatch:
            return apache.thrift.type.type_rep.thrift_patch.TypeStructPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            6,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__type_rep__thrift_types.TypeStruct))
    @property
    def streamOrSink(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.ReturnType,
            ReturnTypePatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ReturnTypePatch:
            return ReturnTypePatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            7,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.ReturnType))
    @property
    def interactionType(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.InterfaceRef,
            InterfaceRefPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> InterfaceRefPatch:
            return InterfaceRefPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            8,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.InterfaceRef))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.FunctionSafePatch:
        return _fbthrift_safe_patch_types.FunctionSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.FunctionSafePatch) -> FunctionPatch:
        patch = FunctionPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class ServicePatch(
    BaseStructPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Service]
):
    pass
    @property
    def attrs(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.DefinitionAttrs,
            DefinitionAttrsPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> DefinitionAttrsPatch:
            return DefinitionAttrsPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.DefinitionAttrs))
    @property
    def functions(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[_fbthrift__apache__thrift__type__schema__thrift_types.Function],
            ListPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Function]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Function]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.Function)))
    @property
    def baseService(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.InterfaceRef,
            InterfaceRefPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> InterfaceRefPatch:
            return InterfaceRefPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.InterfaceRef))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.ServiceSafePatch:
        return _fbthrift_safe_patch_types.ServiceSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.ServiceSafePatch) -> ServicePatch:
        patch = ServicePatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class ConstPatch(
    BaseStructPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Const]
):
    pass
    @property
    def attrs(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.DefinitionAttrs,
            DefinitionAttrsPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> DefinitionAttrsPatch:
            return DefinitionAttrsPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.DefinitionAttrs))
    @property
    def type(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__type_rep__thrift_types.TypeStruct,
            apache.thrift.type.type_rep.thrift_patch.TypeStructPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> apache.thrift.type.type_rep.thrift_patch.TypeStructPatch:
            return apache.thrift.type.type_rep.thrift_patch.TypeStructPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__type_rep__thrift_types.TypeStruct))
    @property
    def value(self) -> UnqualifiedFieldPatch[
            int,
            I64Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I64Patch:
            return patch.as_i64_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.typeinfo_i64)

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.ConstSafePatch:
        return _fbthrift_safe_patch_types.ConstSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.ConstSafePatch) -> ConstPatch:
        patch = ConstPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class TypedefPatch(
    BaseStructPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Typedef]
):
    pass
    @property
    def attrs(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.DefinitionAttrs,
            DefinitionAttrsPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> DefinitionAttrsPatch:
            return DefinitionAttrsPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.DefinitionAttrs))
    @property
    def type(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__type_rep__thrift_types.TypeStruct,
            apache.thrift.type.type_rep.thrift_patch.TypeStructPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> apache.thrift.type.type_rep.thrift_patch.TypeStructPatch:
            return apache.thrift.type.type_rep.thrift_patch.TypeStructPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__type_rep__thrift_types.TypeStruct))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.TypedefSafePatch:
        return _fbthrift_safe_patch_types.TypedefSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.TypedefSafePatch) -> TypedefPatch:
        patch = TypedefPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class DefinitionPatch(
    BaseUnionPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Definition]
):
    pass
    @property
    def structDef(self) -> OptionalFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.Struct,
            StructPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> StructPatch:
            return StructPatch(patch)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.Struct))
    @property
    def unionDef(self) -> OptionalFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.Union,
            UnionPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> UnionPatch:
            return UnionPatch(patch)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.Union))
    @property
    def exceptionDef(self) -> OptionalFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.Exception,
            ExceptionPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ExceptionPatch:
            return ExceptionPatch(patch)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.Exception))
    @property
    def enumDef(self) -> OptionalFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.Enum,
            EnumPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> EnumPatch:
            return EnumPatch(patch)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            4,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.Enum))
    @property
    def typedefDef(self) -> OptionalFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.Typedef,
            TypedefPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> TypedefPatch:
            return TypedefPatch(patch)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            5,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.Typedef))
    @property
    def constDef(self) -> OptionalFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.Const,
            ConstPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ConstPatch:
            return ConstPatch(patch)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            6,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.Const))
    @property
    def serviceDef(self) -> OptionalFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.Service,
            ServicePatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ServicePatch:
            return ServicePatch(patch)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            7,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.Service))
    @property
    def interactionDef(self) -> OptionalFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.Interaction,
            InteractionPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> InteractionPatch:
            return InteractionPatch(patch)

        return OptionalFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            8,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.Interaction))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.DefinitionSafePatch:
        return _fbthrift_safe_patch_types.DefinitionSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.DefinitionSafePatch) -> DefinitionPatch:
        patch = DefinitionPatch()
        DynamicPatch = DynamicUnionPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class ProgramPatch(
    BaseStructPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Program]
):
    pass
    @property
    def attrs(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.DefinitionAttrs,
            DefinitionAttrsPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> DefinitionAttrsPatch:
            return DefinitionAttrsPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.DefinitionAttrs))
    @property
    def id(self) -> UnqualifiedFieldPatch[
            int,
            I64Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I64Patch:
            return patch.as_i64_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.typeinfo_i64)
    @property
    def includes(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[int],
            ListPatch[int]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[int]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.typeinfo_i64))
    @property
    def definitions(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[int],
            ListPatch[int]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[int]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            4,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.typeinfo_i64))
    @property
    def path(self) -> UnqualifiedFieldPatch[
            str,
            StringPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> StringPatch:
            return patch.as_string_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            5,
            _fbthrift_python_types.typeinfo_string)
    @property
    def languageIncludes(self) -> UnqualifiedFieldPatch[
            _typing.Mapping[str, _typing.Sequence[str]],
            MapPatch[str, _typing.Sequence[str], ListPatch[str]]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MapPatch[str, _typing.Sequence[str], ListPatch[str]]:
            def cast_dynamic_patch_to_typed_map_value_patch(patch: DynamicPatch, type_info) -> ListPatch[str]:
                return ListPatch(patch.as_list_patch(), type_info)
            return MapPatch(cast_dynamic_patch_to_typed_map_value_patch, patch.as_map_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            6,
            _fbthrift_python_types.MapTypeInfo(_fbthrift_python_types.typeinfo_string, _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.typeinfo_string)))
    @property
    def namespaces(self) -> UnqualifiedFieldPatch[
            _typing.Mapping[str, str],
            MapPatch[str, str, StringPatch]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MapPatch[str, str, StringPatch]:
            def cast_dynamic_patch_to_typed_map_value_patch(patch: DynamicPatch, type_info) -> StringPatch:
                return patch.as_string_patch()
            return MapPatch(cast_dynamic_patch_to_typed_map_value_patch, patch.as_map_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            7,
            _fbthrift_python_types.MapTypeInfo(_fbthrift_python_types.typeinfo_string, _fbthrift_python_types.typeinfo_string))
    @property
    def definitionKeys(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[bytes],
            ListPatch[bytes]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[bytes]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            8,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.typeinfo_binary))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.ProgramSafePatch:
        return _fbthrift_safe_patch_types.ProgramSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.ProgramSafePatch) -> ProgramPatch:
        patch = ProgramPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class SchemaPatch(
    BaseStructPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Schema]
):
    pass
    @property
    def programs(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[_fbthrift__apache__thrift__type__schema__thrift_types.Program],
            ListPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Program]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Program]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.Program)))
    @property
    def values(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[_fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value],
            ListPatch[_fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[_fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value)))
    @property
    def valuesMap(self) -> UnqualifiedFieldPatch[
            _typing.Mapping[int, _fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value],
            MapPatch[int, _fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value, apache.thrift.protocol.detail.protocol_detail.thrift_patch.ValuePatch]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MapPatch[int, _fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value, apache.thrift.protocol.detail.protocol_detail.thrift_patch.ValuePatch]:
            def cast_dynamic_patch_to_typed_map_value_patch(patch: DynamicPatch, type_info) -> apache.thrift.protocol.detail.protocol_detail.thrift_patch.ValuePatch:
                return apache.thrift.protocol.detail.protocol_detail.thrift_patch.ValuePatch(patch)
            return MapPatch(cast_dynamic_patch_to_typed_map_value_patch, patch.as_map_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.MapTypeInfo(_fbthrift_python_types.typeinfo_i64, _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__protocol__detail__protocol_detail__thrift_types.Value)))
    @property
    def definitions(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[_fbthrift__apache__thrift__type__schema__thrift_types.Definition],
            ListPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Definition]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[_fbthrift__apache__thrift__type__schema__thrift_types.Definition]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            4,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.Definition)))
    @property
    def sources(self) -> UnqualifiedFieldPatch[
            _typing.Mapping[int, _fbthrift__apache__thrift__type__schema__thrift_types.SourceInfo],
            MapPatch[int, _fbthrift__apache__thrift__type__schema__thrift_types.SourceInfo, SourceInfoPatch]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MapPatch[int, _fbthrift__apache__thrift__type__schema__thrift_types.SourceInfo, SourceInfoPatch]:
            def cast_dynamic_patch_to_typed_map_value_patch(patch: DynamicPatch, type_info) -> SourceInfoPatch:
                return SourceInfoPatch(patch)
            return MapPatch(cast_dynamic_patch_to_typed_map_value_patch, patch.as_map_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            5,
            _fbthrift_python_types.MapTypeInfo(_fbthrift_python_types.typeinfo_i64, _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.SourceInfo)))
    @property
    def identifierSourceRanges(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[_fbthrift__apache__thrift__type__schema__thrift_types.IdentifierRef],
            ListPatch[_fbthrift__apache__thrift__type__schema__thrift_types.IdentifierRef]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[_fbthrift__apache__thrift__type__schema__thrift_types.IdentifierRef]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            6,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.IdentifierRef)))
    @property
    def includeSourceRanges(self) -> UnqualifiedFieldPatch[
            _typing.Sequence[_fbthrift__apache__thrift__type__schema__thrift_types.IncludeRef],
            ListPatch[_fbthrift__apache__thrift__type__schema__thrift_types.IncludeRef]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> ListPatch[_fbthrift__apache__thrift__type__schema__thrift_types.IncludeRef]:
            return ListPatch(patch.as_list_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            7,
            _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.IncludeRef)))
    @property
    def definitionsMap(self) -> UnqualifiedFieldPatch[
            _typing.Mapping[bytes, _fbthrift__apache__thrift__type__schema__thrift_types.Definition],
            MapPatch[bytes, _fbthrift__apache__thrift__type__schema__thrift_types.Definition, DefinitionPatch]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MapPatch[bytes, _fbthrift__apache__thrift__type__schema__thrift_types.Definition, DefinitionPatch]:
            def cast_dynamic_patch_to_typed_map_value_patch(patch: DynamicPatch, type_info) -> DefinitionPatch:
                return DefinitionPatch(patch)
            return MapPatch(cast_dynamic_patch_to_typed_map_value_patch, patch.as_map_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            8,
            _fbthrift_python_types.MapTypeInfo(_fbthrift_python_types.typeinfo_binary, _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.Definition)))

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.SchemaSafePatch:
        return _fbthrift_safe_patch_types.SchemaSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.SchemaSafePatch) -> SchemaPatch:
        patch = SchemaPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class SourceInfoPatch(
    BaseStructPatch[_fbthrift__apache__thrift__type__schema__thrift_types.SourceInfo]
):
    pass
    @property
    def fileName(self) -> UnqualifiedFieldPatch[
            str,
            StringPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> StringPatch:
            return patch.as_string_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.typeinfo_string)
    @property
    def languageIncludes(self) -> UnqualifiedFieldPatch[
            _typing.Mapping[str, _typing.Sequence[int]],
            MapPatch[str, _typing.Sequence[int], ListPatch[int]]]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> MapPatch[str, _typing.Sequence[int], ListPatch[int]]:
            def cast_dynamic_patch_to_typed_map_value_patch(patch: DynamicPatch, type_info) -> ListPatch[int]:
                return ListPatch(patch.as_list_patch(), type_info)
            return MapPatch(cast_dynamic_patch_to_typed_map_value_patch, patch.as_map_patch(), type_info)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.MapTypeInfo(_fbthrift_python_types.typeinfo_string, _fbthrift_python_types.ListTypeInfo(_fbthrift_python_types.typeinfo_i64)))
    @property
    def namespaces(self) -> UnqualifiedFieldPatch[
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

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.SourceInfoSafePatch:
        return _fbthrift_safe_patch_types.SourceInfoSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.SourceInfoSafePatch) -> SourceInfoPatch:
        patch = SourceInfoPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class IdentifierRefPatch(
    BaseStructPatch[_fbthrift__apache__thrift__type__schema__thrift_types.IdentifierRef]
):
    pass
    @property
    def range(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.SourceRange,
            SourceRangePatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> SourceRangePatch:
            return SourceRangePatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.SourceRange))
    @property
    def uri(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__standard__thrift_types.TypeUri,
            apache.thrift.type.standard.thrift_patch.TypeUriPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> apache.thrift.type.standard.thrift_patch.TypeUriPatch:
            return apache.thrift.type.standard.thrift_patch.TypeUriPatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__standard__thrift_types.TypeUri))
    @property
    def enumValue(self) -> UnqualifiedFieldPatch[
            str,
            StringPatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> StringPatch:
            return patch.as_string_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            3,
            _fbthrift_python_types.typeinfo_string)

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.IdentifierRefSafePatch:
        return _fbthrift_safe_patch_types.IdentifierRefSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.IdentifierRefSafePatch) -> IdentifierRefPatch:
        patch = IdentifierRefPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch



class IncludeRefPatch(
    BaseStructPatch[_fbthrift__apache__thrift__type__schema__thrift_types.IncludeRef]
):
    pass
    @property
    def range(self) -> UnqualifiedFieldPatch[
            _fbthrift__apache__thrift__type__schema__thrift_types.SourceRange,
            SourceRangePatch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> SourceRangePatch:
            return SourceRangePatch(patch)

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            1,
            _fbthrift_python_types.StructTypeInfo(_fbthrift__apache__thrift__type__schema__thrift_types.SourceRange))
    @property
    def target(self) -> UnqualifiedFieldPatch[
            int,
            I64Patch]:

        def cast_dynamic_patch_to_typed_field_patch(patch: DynamicPatch, type_info) -> I64Patch:
            return patch.as_i64_patch()

        return UnqualifiedFieldPatch(
            cast_dynamic_patch_to_typed_field_patch,
            self._patch,
            2,
            _fbthrift_python_types.typeinfo_i64)

    def to_safe_patch(self) -> _fbthrift_safe_patch_types.IncludeRefSafePatch:
        return _fbthrift_safe_patch_types.IncludeRefSafePatch(
            version=1, data=self._patch.serialize_to_compact_protocol()
        )

    @staticmethod
    def from_safe_patch(safe_patch: _fbthrift_safe_patch_types.IncludeRefSafePatch) -> IncludeRefPatch:
        patch = IncludeRefPatch()
        DynamicPatch = DynamicStructPatch
        patch._patch = DynamicPatch.deserialize_from_compact_protocol(safe_patch.data)
        return patch


