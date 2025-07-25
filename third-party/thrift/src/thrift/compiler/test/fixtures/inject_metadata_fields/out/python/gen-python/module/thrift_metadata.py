#
# Autogenerated by Thrift
#
# DO NOT EDIT
#  @generated
#

from __future__ import annotations

import apache.thrift.metadata.thrift_types as _fbthrift_metadata

import module.thrift_enums as _fbthrift_current_module_enums
import module.thrift_enums


import foo.thrift_enums
import foo.thrift_metadata as _fbthrift__foo__thrift_metadata

# TODO (ffrancet): This general pattern can be optimized by using tuples and dicts
# instead of re-generating thrift structs
def _fbthrift_gen_metadata_struct_Fields(metadata_struct: _fbthrift_metadata.ThriftMetadata) -> _fbthrift_metadata.ThriftMetadata:
    qualified_name = "module.Fields"

    if qualified_name in metadata_struct.structs:
        return metadata_struct
    fields = [
        _fbthrift_metadata.ThriftField(id=100, type=_fbthrift_metadata.ThriftType(t_primitive=_fbthrift_metadata.ThriftPrimitiveType.THRIFT_STRING_TYPE), name="injected_field", is_optional=False, structured_annotations=[
        ]),
    ]
    struct_dict = dict(metadata_struct.structs)
    struct_dict[qualified_name] = _fbthrift_metadata.ThriftStruct(name=qualified_name, fields=fields,
        is_union=False,
        structured_annotations=[
        ])
    new_struct = metadata_struct(structs=struct_dict)

    # injected_field
    
    return new_struct
def gen_metadata_struct_Fields() -> _fbthrift_metadata.ThriftMetadata:
    return _fbthrift_gen_metadata_struct_Fields(_fbthrift_metadata.ThriftMetadata(structs={}, enums={}, exceptions={}, services={}))

# TODO (ffrancet): This general pattern can be optimized by using tuples and dicts
# instead of re-generating thrift structs
def _fbthrift_gen_metadata_struct_FieldsInjectedToEmptyStruct(metadata_struct: _fbthrift_metadata.ThriftMetadata) -> _fbthrift_metadata.ThriftMetadata:
    qualified_name = "module.FieldsInjectedToEmptyStruct"

    if qualified_name in metadata_struct.structs:
        return metadata_struct
    fields = [
        _fbthrift_metadata.ThriftField(id=-1100, type=_fbthrift_metadata.ThriftType(t_primitive=_fbthrift_metadata.ThriftPrimitiveType.THRIFT_STRING_TYPE), name="injected_field", is_optional=False, structured_annotations=[
        ]),
    ]
    struct_dict = dict(metadata_struct.structs)
    struct_dict[qualified_name] = _fbthrift_metadata.ThriftStruct(name=qualified_name, fields=fields,
        is_union=False,
        structured_annotations=[
            _fbthrift_metadata.ThriftConstStruct(type=_fbthrift_metadata.ThriftStructType(name="internal.InjectMetadataFields"), fields= { "type": _fbthrift_metadata.ThriftConstValue(cv_string="Fields"),  }),
        ])
    new_struct = metadata_struct(structs=struct_dict)

    # injected_field
    
    return new_struct
def gen_metadata_struct_FieldsInjectedToEmptyStruct() -> _fbthrift_metadata.ThriftMetadata:
    return _fbthrift_gen_metadata_struct_FieldsInjectedToEmptyStruct(_fbthrift_metadata.ThriftMetadata(structs={}, enums={}, exceptions={}, services={}))

# TODO (ffrancet): This general pattern can be optimized by using tuples and dicts
# instead of re-generating thrift structs
def _fbthrift_gen_metadata_struct_FieldsInjectedToStruct(metadata_struct: _fbthrift_metadata.ThriftMetadata) -> _fbthrift_metadata.ThriftMetadata:
    qualified_name = "module.FieldsInjectedToStruct"

    if qualified_name in metadata_struct.structs:
        return metadata_struct
    fields = [
        _fbthrift_metadata.ThriftField(id=-1100, type=_fbthrift_metadata.ThriftType(t_primitive=_fbthrift_metadata.ThriftPrimitiveType.THRIFT_STRING_TYPE), name="injected_field", is_optional=False, structured_annotations=[
        ]),
        _fbthrift_metadata.ThriftField(id=1, type=_fbthrift_metadata.ThriftType(t_primitive=_fbthrift_metadata.ThriftPrimitiveType.THRIFT_STRING_TYPE), name="string_field", is_optional=False, structured_annotations=[
        ]),
    ]
    struct_dict = dict(metadata_struct.structs)
    struct_dict[qualified_name] = _fbthrift_metadata.ThriftStruct(name=qualified_name, fields=fields,
        is_union=False,
        structured_annotations=[
            _fbthrift_metadata.ThriftConstStruct(type=_fbthrift_metadata.ThriftStructType(name="internal.InjectMetadataFields"), fields= { "type": _fbthrift_metadata.ThriftConstValue(cv_string="Fields"),  }),
        ])
    new_struct = metadata_struct(structs=struct_dict)

    # string_field
        # injected_field
    
    return new_struct
def gen_metadata_struct_FieldsInjectedToStruct() -> _fbthrift_metadata.ThriftMetadata:
    return _fbthrift_gen_metadata_struct_FieldsInjectedToStruct(_fbthrift_metadata.ThriftMetadata(structs={}, enums={}, exceptions={}, services={}))

# TODO (ffrancet): This general pattern can be optimized by using tuples and dicts
# instead of re-generating thrift structs
def _fbthrift_gen_metadata_struct_FieldsInjectedWithIncludedStruct(metadata_struct: _fbthrift_metadata.ThriftMetadata) -> _fbthrift_metadata.ThriftMetadata:
    qualified_name = "module.FieldsInjectedWithIncludedStruct"

    if qualified_name in metadata_struct.structs:
        return metadata_struct
    fields = [
        _fbthrift_metadata.ThriftField(id=-1102, type=_fbthrift_metadata.ThriftType(t_primitive=_fbthrift_metadata.ThriftPrimitiveType.THRIFT_STRING_TYPE), name="injected_unstructured_annotation_field", is_optional=True, structured_annotations=[
            _fbthrift_metadata.ThriftConstStruct(type=_fbthrift_metadata.ThriftStructType(name="thrift.Box"), fields= {  }),
        ]),
        _fbthrift_metadata.ThriftField(id=-1101, type=_fbthrift_metadata.ThriftType(t_primitive=_fbthrift_metadata.ThriftPrimitiveType.THRIFT_STRING_TYPE), name="injected_structured_annotation_field", is_optional=True, structured_annotations=[
            _fbthrift_metadata.ThriftConstStruct(type=_fbthrift_metadata.ThriftStructType(name="thrift.Box"), fields= {  }),
        ]),
        _fbthrift_metadata.ThriftField(id=-1100, type=_fbthrift_metadata.ThriftType(t_primitive=_fbthrift_metadata.ThriftPrimitiveType.THRIFT_STRING_TYPE), name="injected_field", is_optional=False, structured_annotations=[
        ]),
        _fbthrift_metadata.ThriftField(id=1, type=_fbthrift_metadata.ThriftType(t_primitive=_fbthrift_metadata.ThriftPrimitiveType.THRIFT_STRING_TYPE), name="string_field", is_optional=False, structured_annotations=[
        ]),
    ]
    struct_dict = dict(metadata_struct.structs)
    struct_dict[qualified_name] = _fbthrift_metadata.ThriftStruct(name=qualified_name, fields=fields,
        is_union=False,
        structured_annotations=[
            _fbthrift_metadata.ThriftConstStruct(type=_fbthrift_metadata.ThriftStructType(name="internal.InjectMetadataFields"), fields= { "type": _fbthrift_metadata.ThriftConstValue(cv_string="foo.Fields"),  }),
        ])
    new_struct = metadata_struct(structs=struct_dict)

    # string_field
        # injected_field
        # injected_structured_annotation_field
        # injected_unstructured_annotation_field
    
    return new_struct
def gen_metadata_struct_FieldsInjectedWithIncludedStruct() -> _fbthrift_metadata.ThriftMetadata:
    return _fbthrift_gen_metadata_struct_FieldsInjectedWithIncludedStruct(_fbthrift_metadata.ThriftMetadata(structs={}, enums={}, exceptions={}, services={}))



def getThriftModuleMetadata() -> _fbthrift_metadata.ThriftMetadata:
    meta = _fbthrift_metadata.ThriftMetadata(structs={}, enums={}, exceptions={}, services={})
    meta = _fbthrift_gen_metadata_struct_Fields(meta)
    meta = _fbthrift_gen_metadata_struct_FieldsInjectedToEmptyStruct(meta)
    meta = _fbthrift_gen_metadata_struct_FieldsInjectedToStruct(meta)
    meta = _fbthrift_gen_metadata_struct_FieldsInjectedWithIncludedStruct(meta)
    return meta
