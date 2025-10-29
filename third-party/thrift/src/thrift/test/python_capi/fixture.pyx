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

from libcpp cimport bool as cbool
from libcpp.string cimport string
from libc.stdint cimport int64_t

cdef extern from "thrift/test/python_capi/gen-python-capi/module/thrift_types_capi.h":
    cppclass ModuleNs "thrift__test__python_capi__module::NamespaceTag" 


cdef extern from "thrift/test/python_capi/gen-cpp2/module_types.h" namespace "thrift::test::python_capi":
    cppclass AdaptedFields
    cppclass ComposeStruct
    cppclass DoubledPair
    cppclass ListStruct
    cppclass MapStruct
    cppclass MyDataItem
    cppclass MyEnum
    cppclass MyStruct
    cppclass Shallot
    cppclass PrimitiveStruct
    cppclass SetStruct
    cppclass StringPair
    cppclass VapidStruct
    cppclass SomeBinary
    cppclass Float32

cdef extern from "thrift/test/python_capi/gen-python-capi/containers/thrift_types_capi.h":
    cppclass TemplateNs "thrift__test__python_capi__containers::NamespaceTag" 

cdef extern from "thrift/test/python_capi/gen-cpp2/containers_types.h" namespace "thrift::test::python_capi":
    cppclass TemplateLists
    cppclass TemplateMaps
    cppclass TemplateSets

cdef extern from "thrift/test/python_capi/gen-python-capi/serialized_dep/thrift_types_capi.h":
    cppclass SerializedNs "thrift__test__python_capi__serialized_dep::NamespaceTag" 

cdef extern from "thrift/test/python_capi/gen-cpp2/serialized_dep_types.h" namespace "thrift::test::python_capi":
    cppclass SerializedStruct

cdef extern from "thrift/test/python_capi/fixture.h" namespace "apache::thrift::test":
    cdef object __shim__roundtrip[T, NsTag](object)
    cdef cbool __shim__typeCheck[T, NsTag](object)
    cdef object __shim__marshal_to_iobuf[T, NsTag](object)
    cdef object __shim__serialize_to_iobuf[T](object obj)
    cdef object __shim__gen_SerializedStruct(int64_t)
    cdef string serializeTemplateLists() noexcept
    cdef object constructTemplateLists()
    cdef string serializeTemplateMaps() noexcept
    cdef object constructTemplateMaps()
    cdef string serializeTemplateSets() noexcept
    cdef object constructTemplateSets()
    cdef string extractAndSerialize[T, NsTag](object obj)


def roundtrip_MyStruct(object x):
    return __shim__roundtrip[MyStruct, ModuleNs](x)

def roundtrip_MyDataItem(object x):
    return __shim__roundtrip[MyDataItem, ModuleNs](x)

def roundtrip_MyUnion(object x):
    return __shim__roundtrip[Shallot, ModuleNs](x)

def roundtrip_MyEnum(object x):
    return __shim__roundtrip[MyEnum, ModuleNs](x)

def roundtrip_EmptyStruct(object x):
    return __shim__roundtrip[VapidStruct, ModuleNs](x)

def roundtrip_StringPair(object x):
    return __shim__roundtrip[StringPair, ModuleNs](x)

def roundtrip_DoubledPair(object x):
    return __shim__roundtrip[DoubledPair, ModuleNs](x)

def roundtrip_PrimitiveStruct(object x):
    return __shim__roundtrip[PrimitiveStruct, ModuleNs](x)

def roundtrip_ListStruct(object x):
    return __shim__roundtrip[ListStruct, ModuleNs](x)

def roundtrip_SetStruct(object x):
    return __shim__roundtrip[SetStruct, ModuleNs](x)

def roundtrip_MapStruct(object x):
    return __shim__roundtrip[MapStruct, ModuleNs](x)

def roundtrip_ComposeStruct(object x):
    return __shim__roundtrip[ComposeStruct, ModuleNs](x)

def roundtrip_AdaptedFields(object x):
    return __shim__roundtrip[AdaptedFields, ModuleNs](x)

def roundtrip_SerializedStruct(object x):
    return __shim__roundtrip[SerializedStruct, SerializedNs](x)

def roundtrip_SomeBinary(object x):
    return __shim__roundtrip[SomeBinary, ModuleNs](x)

def roundtrip_Float32(object x):
    return __shim__roundtrip[Float32, ModuleNs](x)

def gen_SerializedStruct(int64_t len_):
    return __shim__gen_SerializedStruct(len_)

def check_MyStruct(object x):
    return bool(__shim__typeCheck[MyStruct, ModuleNs](x))

def check_MyDataItem(object x):
    return bool(__shim__typeCheck[MyDataItem, ModuleNs](x))

def check_MyUnion(object x):
    return bool(__shim__typeCheck[Shallot, ModuleNs](x))

def check_MyEnum(object x):
    return bool(__shim__typeCheck[MyEnum, ModuleNs](x))

def check_StringPair(object x):
    return __shim__typeCheck[StringPair, ModuleNs](x)

def check_DoubledPair(object x):
    return __shim__typeCheck[DoubledPair, ModuleNs](x)

def check_PrimitiveStruct(object x):
    return bool(__shim__typeCheck[PrimitiveStruct, ModuleNs](x))

def check_ListStruct(object x):
    return bool(__shim__typeCheck[ListStruct, ModuleNs](x))

def check_SetStruct(object x):
    return bool(__shim__typeCheck[SetStruct, ModuleNs](x))

def check_MapStruct(object x):
    return bool(__shim__typeCheck[MapStruct, ModuleNs](x))

def check_ComposeStruct(object x):
    return bool(__shim__typeCheck[ComposeStruct, ModuleNs](x))

# Test marshal parity with serialize
def extract_and_serialize_PrimitiveStruct(object x):
    return __shim__marshal_to_iobuf[PrimitiveStruct, ModuleNs](x)

def deserialize_and_serialize_PrimitiveStruct(object x):
    return __shim__serialize_to_iobuf[PrimitiveStruct](x)

def extract_and_serialize_MyStruct(object x):
    return __shim__marshal_to_iobuf[MyStruct, ModuleNs](x)

def deserialize_and_serialize_MyStruct(object x):
    return __shim__serialize_to_iobuf[MyStruct](x)

def extract_and_serialize_AdaptedFields(object x):
    return __shim__marshal_to_iobuf[AdaptedFields, ModuleNs](x)

def deserialize_and_serialize_AdaptedFields(object x):
    return __shim__serialize_to_iobuf[AdaptedFields](x)

def extract_and_serialize_ListStruct(object x):
    return __shim__marshal_to_iobuf[ListStruct, ModuleNs](x)

def deserialize_and_serialize_ListStruct(object x):
    return __shim__serialize_to_iobuf[ListStruct](x)

def extract_and_serialize_SetStruct(object x):
    return __shim__marshal_to_iobuf[SetStruct, ModuleNs](x)

def deserialize_and_serialize_SetStruct(object x):
    return __shim__serialize_to_iobuf[SetStruct](x)

def extract_and_serialize_MapStruct(object x):
    return __shim__marshal_to_iobuf[MapStruct, ModuleNs](x)

def deserialize_and_serialize_MapStruct(object x):
    return __shim__serialize_to_iobuf[MapStruct](x)

def extract_and_serialize_ComposeStruct(object x):
    return __shim__marshal_to_iobuf[ComposeStruct, ModuleNs](x)

def deserialize_and_serialize_ComposeStruct(object x):
    return __shim__serialize_to_iobuf[ComposeStruct](x)

# for testing template overrides

def serialize_template_lists():
    return serializeTemplateLists()

def construct_template_lists():
    return constructTemplateLists()

def extract_template_lists(obj):
    return extractAndSerialize[TemplateLists, TemplateNs](obj)

def serialize_template_sets():
    return serializeTemplateSets()

def construct_template_sets():
    return constructTemplateSets()

def extract_template_sets(obj):
    return extractAndSerialize[TemplateSets, TemplateNs](obj)

def serialize_template_maps():
    return serializeTemplateMaps()

def construct_template_maps():
    return constructTemplateMaps()

def extract_template_maps(obj):
    return extractAndSerialize[TemplateMaps, TemplateNs](obj)
