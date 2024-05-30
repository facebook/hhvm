# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from types import MappingProxyType

from libcpp.memory cimport shared_ptr
from thrift.py3.types cimport CompiledEnum, Struct
from thrift.py3.exceptions cimport GeneratedError
from thrift.py3.server cimport ServiceInterface
from thrift.py3.client cimport Client
from thrift.python.common cimport MetadataBox
from apache.thrift.metadata.types cimport (
    cThriftMetadata,
    cThriftStruct,
    cThriftException,
    cThriftService,
    cThriftEnum,
)
from apache.thrift.metadata.types import (
    ThriftMetadata,
    ThriftStruct,
    ThriftException,
    ThriftService,
    ThriftEnum,
    ThriftType,
    ThriftField,
    ThriftStructType,
    ThriftUnionType,
    ThriftEnumType,
    ThriftPrimitiveType,
    ThriftListType,
    ThriftSetType,
    ThriftMapType,
    ThriftFunction,
    ThriftTypedefType,
    ThriftSinkType,
    ThriftStreamType,
    ThriftConstStruct,
    ThriftConstValue,
)
from apache.thrift.metadata.converter cimport ThriftMetadata_from_cpp


cpdef enum ThriftKind:
    PRIMITIVE = 0
    LIST = 1
    SET = 2
    MAP = 3
    ENUM = 4
    STRUCT = 5
    UNION = 6
    TYPEDEF = 7
    STREAM = 8
    SINK = 9


cpdef enum ThriftConstKind:
    CV_BOOL = 0
    CV_INT = 1
    CV_FLOAT = 2
    CV_STRING = 3
    CV_MAP = 4
    CV_LIST = 5
    CV_STRUCT = 6


cdef class ThriftTypeProxy:
    # A union of a bunch of thrift metadata types
    cdef readonly object thriftType
    cdef readonly object thriftMeta #: ThriftMetadata
    cdef readonly ThriftKind kind

    def __init__(self, object thriftType not None, thriftMeta not None: ThriftMetadata):
        if not isinstance(thriftType, (
            ThriftStruct,
            ThriftEnum,
            ThriftPrimitiveType,
            ThriftListType,
            ThriftSetType,
            ThriftMapType,
            ThriftTypedefType,
            ThriftSinkType,
            ThriftStreamType,
        )):
            raise TypeError(f"{thriftType!r} is not a known thrift type.")
        self.thriftType = thriftType
        self.thriftMeta = thriftMeta

    @staticmethod
    cdef _fbthrift_create(thriftType: ThriftType, thriftMeta: ThriftMetadata):
        # Determine value and kind
        if thriftType.type is ThriftType.Type.t_list:
            return ThriftListProxy(thriftType.value, thriftMeta)
        elif thriftType.type is ThriftType.Type.t_set:
            return ThriftSetProxy(thriftType.value, thriftMeta)
        elif thriftType.type is ThriftType.Type.t_map:
            return ThriftMapProxy(thriftType.value, thriftMeta)
        elif thriftType.type is ThriftType.Type.t_enum:
            specialType = ThriftTypeProxy(thriftMeta.enums[thriftType.value.name], thriftMeta)
            specialType.kind = ThriftKind.ENUM
            return specialType
        elif thriftType.type is ThriftType.Type.t_struct:
            return ThriftStructProxy(thriftType.value.name, thriftMeta)
        elif thriftType.type is ThriftType.Type.t_union:
            return ThriftStructProxy(thriftType.value.name, thriftMeta)
        elif thriftType.type is ThriftType.Type.t_typedef:
            return ThriftTypedefProxy(thriftType.value, thriftMeta)
        elif thriftType.type is ThriftType.Type.t_stream:
            return ThriftStreamProxy(thriftType.value, thriftMeta)
        elif thriftType.type is ThriftType.Type.t_sink:
            return ThriftSinkProxy(thriftType.value, thriftMeta)
        specialType = ThriftTypeProxy(thriftType.value, thriftMeta)
        specialType.kind = ThriftKind.PRIMITIVE
        return specialType

    def as_primitive(self):
        if self.kind == ThriftKind.PRIMITIVE:
            return self.thriftType
        raise TypeError('Type is not primitive')

    def as_struct(self):
        if self.kind == ThriftKind.STRUCT or self.kind == ThriftKind.UNION:
            return self
        raise TypeError('Type is not a struct')

    def as_union(self):
        if self.kind == ThriftKind.UNION:
            return self
        raise TypeError('Type is not a union')

    def as_enum(self):
        if self.kind == ThriftKind.ENUM:
            return self.thriftType
        raise TypeError('Type is not an enum')

    def as_list(self):
        if self.kind == ThriftKind.LIST:
            return self
        raise TypeError('Type is not a list')

    def as_set(self):
        if self.kind == ThriftKind.SET:
            return self
        raise TypeError('Type is not a set')

    def as_map(self):
        if self.kind == ThriftKind.MAP:
            return self
        raise TypeError('Type is not a map')

    def as_typedef(self):
        if self.kind == ThriftKind.TYPEDEF:
            return self
        raise TypeError('Type is not a typedef')

    def as_stream(self):
        if self.kind == ThriftKind.STREAM:
            return self
        raise TypeError('Type is not a stream')

    def as_sink(self):
        if self.kind == ThriftKind.SINK:
            return self
        raise TypeError('Type is not a sink')


cdef class ThriftSetProxy(ThriftTypeProxy):
    cdef readonly ThriftTypeProxy valueType

    def __init__(self, thriftType not None: ThriftSetType, thriftMeta not None: ThriftMetadata):
        super().__init__(thriftType, thriftMeta)
        self.kind = ThriftKind.SET
        self.valueType = ThriftTypeProxy._fbthrift_create(self.thriftType.valueType, self.thriftMeta)


cdef class ThriftListProxy(ThriftTypeProxy):
    cdef readonly ThriftTypeProxy valueType

    def __init__(self, thriftType not None: ThriftListType, thriftMeta not None: ThriftMetadata):
        super().__init__(thriftType, thriftMeta)
        self.kind = ThriftKind.LIST
        self.valueType = ThriftTypeProxy._fbthrift_create(self.thriftType.valueType, self.thriftMeta)


cdef class ThriftMapProxy(ThriftTypeProxy):
    cdef readonly ThriftTypeProxy valueType
    cdef readonly ThriftTypeProxy keyType

    def __init__(self, thriftType not None: ThriftMapType, thriftMeta not None: ThriftMetadata):
        super().__init__(thriftType, thriftMeta)
        self.kind = ThriftKind.MAP
        self.valueType = ThriftTypeProxy._fbthrift_create(self.thriftType.valueType, self.thriftMeta)
        self.keyType = ThriftTypeProxy._fbthrift_create(self.thriftType.keyType, self.thriftMeta)


cdef class ThriftTypedefProxy(ThriftTypeProxy):
    cdef readonly ThriftTypeProxy underlyingType
    cdef readonly str name

    def __init__(self, thriftType not None: ThriftTypedefType, thriftMeta not None: ThriftMetadata):
        super().__init__(thriftType, thriftMeta)
        self.kind = ThriftKind.TYPEDEF
        self.name = self.thriftType.name
        self.underlyingType = ThriftTypeProxy._fbthrift_create(self.thriftType.underlyingType, self.thriftMeta)


cdef class ThriftSinkProxy(ThriftTypeProxy):
    cdef readonly ThriftTypeProxy elemType
    cdef readonly ThriftTypeProxy initialResponseType
    cdef readonly ThriftTypeProxy finalResponseType

    def __init__(self, thriftType not None: ThriftSinkType, thriftMeta not None: ThriftMetadata):
        super().__init__(thriftType, thriftMeta)
        self.kind = ThriftKind.SINK
        self.elemType = ThriftTypeProxy._fbthrift_create(self.thriftType.elemType, self.thriftMeta)
        if self.thriftType.initialResponseType is not None:
            self.initialResponseType = ThriftTypeProxy._fbthrift_create(self.thriftType.initialResponseType, self.thriftMeta)
        if self.thriftType.finalResponseType is not None:
            self.finalResponseType = ThriftTypeProxy._fbthrift_create(self.thriftType.finalResponseType, self.thriftMeta)


cdef class ThriftStreamProxy(ThriftTypeProxy):
    cdef readonly ThriftTypeProxy elemType
    cdef readonly ThriftTypeProxy initialResponseType

    def __init__(self, thriftType not None: ThriftStreamType, thriftMeta not None: ThriftMetadata):
        super().__init__(thriftType, thriftMeta)
        self.kind = ThriftKind.STREAM
        self.elemType = ThriftTypeProxy._fbthrift_create(self.thriftType.elemType, self.thriftMeta)
        if self.thriftType.initialResponseType is not None:
            self.initialResponseType = ThriftTypeProxy._fbthrift_create(self.thriftType.initialResponseType, self.thriftMeta)


cdef class ThriftFieldProxy:
    cdef readonly ThriftTypeProxy type
    cdef readonly object thriftType # :ThriftField
    cdef readonly object thriftMeta # :ThriftMetadata
    cdef readonly int id
    cdef readonly str name
    cdef readonly int is_optional
    cdef readonly tuple structuredAnnotations

    def __init__(self, thriftType not None: ThriftField, thriftMeta not None: ThriftMetadata):
        self.type = ThriftTypeProxy._fbthrift_create(thriftType.type, thriftMeta)
        self.thriftType = thriftType
        self.thriftMeta = thriftMeta
        self.id = self.thriftType.id
        self.name = self.thriftType.name
        self.is_optional = self.thriftType.is_optional
        self.structuredAnnotations = tuple(ThriftConstStructProxy(annotation) for annotation in self.thriftType.structured_annotations)

    @property
    def pyname(self):
        if self.thriftType.structured_annotations is not None:
            for annotation in self.thriftType.structured_annotations:
                if annotation.type.name == "python.Name":
                    return annotation.fields["name"].cv_string

        if self.thriftType.unstructured_annotations is not None:
            return self.thriftType.unstructured_annotations.get("py3.name", self.name)

        return self.name



cdef class ThriftStructProxy(ThriftTypeProxy):
    cdef readonly str name
    cdef readonly int is_union
    cdef readonly tuple structuredAnnotations

    def __init__(self, str name not None, thriftMeta not None: ThriftMetadata):
        super().__init__(thriftMeta.structs[name], thriftMeta)
        self.name = self.thriftType.name
        self.is_union = self.thriftType.is_union
        self.structuredAnnotations = tuple(ThriftConstStructProxy(annotation) for annotation in self.thriftType.structured_annotations)

        if self.is_union:
            self.kind = ThriftKind.UNION
        else:
            self.kind = ThriftKind.STRUCT

    @property
    def fields(self):
        for field in self.thriftType.fields:
            yield ThriftFieldProxy(field, self.thriftMeta)


cdef class ThriftConstValueProxy:
    cdef readonly object thriftType # :ThriftConstValue
    cdef readonly ThriftConstKind kind
    cdef readonly object type
    def __init__(self, value not None: ThriftConstValue):
        self.thriftType = value
        if self.thriftType.type in (ThriftConstValue.Type.cv_bool, ThriftConstValue.Type.cv_integer, ThriftConstValue.Type.cv_double, ThriftConstValue.Type.cv_string):
            self.type = self.thriftType.value
            if self.thriftType.type is ThriftConstValue.Type.cv_bool:
                self.kind = CV_BOOL
            elif self.thriftType.type is ThriftConstValue.Type.cv_integer:
                self.kind = CV_INT
            elif self.thriftType.type is ThriftConstValue.Type.cv_double:
                self.kind = CV_FLOAT
            else:
                self.kind = CV_STRING
        if self.thriftType.type is ThriftConstValue.Type.cv_struct:
            self.type = ThriftConstStructProxy(self.thriftType.value)
            self.kind = CV_STRUCT
        if self.thriftType.type is ThriftConstValue.Type.cv_list:
            self.type = tuple(ThriftConstValueProxy(ele) for ele in self.thriftType.value)
            self.kind = CV_LIST
        if self.thriftType.type is ThriftConstValue.Type.cv_map:
            self.type = MappingProxyType({ThriftConstValueProxy(ele.key).type: ThriftConstValueProxy(ele.value) for ele in self.thriftType.value})
            self.kind = CV_MAP

    def as_bool(self):
        if self.kind == ThriftConstKind.CV_BOOL:
            return self.type
        raise TypeError('Type is not a boolean')

    def as_int(self):
        if self.kind == ThriftConstKind.CV_INT:
            return self.type
        raise TypeError('Type is not an integer')

    def as_float(self):
        if self.kind == ThriftConstKind.CV_FLOAT:
            return self.type
        raise TypeError('Type is not a float')

    def as_string(self):
        if self.kind == ThriftConstKind.CV_STRING:
            return self.type
        raise TypeError('Type is not a string')

    def as_struct(self):
        if self.kind == ThriftConstKind.CV_STRUCT:
            return self.type
        raise TypeError('Type is not a struct')

    def as_list(self):
        if self.kind == ThriftConstKind.CV_LIST:
            return self.type
        raise TypeError('Type is not a list')
    def as_map(self):
        if self.kind == ThriftConstKind.CV_MAP:
            return self.type
        raise TypeError('Type is not a map')

cdef class ThriftConstStructProxy:
    cdef readonly object thriftType # :ThriftConstStruct
    cdef readonly str name
    cdef readonly ThriftKind kind
    def __init__(self, struct not None: ThriftConstStruct):
        self.name = struct.type.name
        self.kind = ThriftKind.STRUCT
        self.thriftType = struct

    @property
    def fields(self):
        return MappingProxyType({key: ThriftConstValueProxy(self.thriftType.fields[key]) for key in self.thriftType.fields})


cdef class ThriftExceptionProxy:
    cdef readonly object thriftType # :ThriftException
    cdef readonly object thriftMeta # :ThriftMetadata
    cdef readonly str name
    cdef readonly tuple structuredAnnotations

    def __init__(self, str name not None, thriftMeta not None: ThriftMetadata):
        self.thriftType = thriftMeta.exceptions[name]
        self.thriftMeta = thriftMeta
        self.name = self.thriftType.name
        self.structuredAnnotations = tuple(ThriftConstStructProxy(annotation) for annotation in self.thriftType.structured_annotations)

    @property
    def fields(self):
        for field in self.thriftType.fields:
            yield ThriftFieldProxy(field, self.thriftMeta)


cdef class ThriftFunctionProxy:
    cdef readonly str name
    cdef readonly object thriftType # :ThriftFunction
    cdef readonly object thriftMeta # :ThriftMetadata
    cdef readonly ThriftTypeProxy return_type
    cdef readonly int is_oneway
    cdef readonly tuple structuredAnnotations

    def __init__(self, thriftType not None: ThriftFunction, thriftMeta not None: ThriftMetadata):
        self.name = thriftType.name
        self.thriftType = thriftType
        self.thriftMeta = thriftMeta
        self.return_type = ThriftTypeProxy._fbthrift_create(self.thriftType.return_type, self.thriftMeta)
        self.is_oneway = self.thriftType.is_oneway
        self.structuredAnnotations = tuple(ThriftConstStructProxy(annotation) for annotation in self.thriftType.structured_annotations)

    @property
    def arguments(self):
        for argument in self.thriftType.arguments:
            yield ThriftFieldProxy(argument, self.thriftMeta)

    @property
    def exceptions(self):
        for exception in self.thriftType.exceptions:
            yield ThriftFieldProxy(exception, self.thriftMeta)


cdef class ThriftServiceProxy:
    cdef readonly object thriftType # :ThriftService
    cdef readonly str name
    cdef readonly object thriftMeta # :ThriftMetadata
    cdef readonly ThriftServiceProxy parent
    cdef readonly tuple structuredAnnotations

    def __init__(self, str name not None, thriftMeta not None: ThriftMetadata):
        self.thriftType = thriftMeta.services[name]
        self.name = self.thriftType.name
        self.thriftMeta = thriftMeta
        self.parent = None if self.thriftType.parent is None else ThriftServiceProxy(
            self.thriftType.parent,
            self.thriftMeta
        )
        self.structuredAnnotations = tuple(ThriftConstStructProxy(annotation) for annotation in self.thriftType.structured_annotations)

    @property
    def functions(self):
        for function in self.thriftType.functions:
            yield ThriftFunctionProxy(function, self.thriftMeta)


def gen_metadata(obj_or_cls):
    if hasattr(obj_or_cls, "getThriftModuleMetadata"):
        return obj_or_cls.getThriftModuleMetadata()

    cls = obj_or_cls if isinstance(obj_or_cls, type) else type(obj_or_cls)

    if not issubclass(cls, (Struct, GeneratedError, ServiceInterface, Client, CompiledEnum)):
        raise TypeError(f'{cls!r} is not a thrift-py3 type.')

    # get the box
    cdef MetadataBox box = cls.__get_metadata__()
    # unbox the box
    cdef shared_ptr[cThriftMetadata] box_obj = box._cpp_obj
    meta = ThriftMetadata_from_cpp(box_obj)
    cdef str name = cls.__get_thrift_name__()

    if issubclass(cls, Struct):
        return ThriftStructProxy(name, meta)
    elif issubclass(cls, GeneratedError):
        return ThriftExceptionProxy(name, meta)
    elif issubclass(cls, ServiceInterface):
        return ThriftServiceProxy(name, meta)
    elif issubclass(cls, Client):
        return ThriftServiceProxy(name, meta)
    elif issubclass(cls, CompiledEnum):
        return meta.enums[name]
    else:
        raise TypeError(f'unsupported thrift-py3 type: {cls!r}.')
