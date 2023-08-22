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

cdef extern from "thrift/test/python_capi/fixture.h" namespace "apache::thrift::test":
    cdef object __shim__roundtrip[T](object)
    cdef cbool __shim__typeCheck[T](object)


def roundtrip_MyStruct(object x):
    return __shim__roundtrip[MyStruct](x)

def roundtrip_MyDataItem(object x):
    return __shim__roundtrip[MyDataItem](x)

def roundtrip_MyUnion(object x):
    return __shim__roundtrip[Shallot](x)

def roundtrip_MyEnum(object x):
    return __shim__roundtrip[MyEnum](x)

def roundtrip_EmptyStruct(object x):
    return __shim__roundtrip[VapidStruct](x)

def roundtrip_StringPair(object x):
    return __shim__roundtrip[StringPair](x)

def roundtrip_DoubledPair(object x):
    return __shim__roundtrip[DoubledPair](x)

def roundtrip_PrimitiveStruct(object x):
    return __shim__roundtrip[PrimitiveStruct](x)

def roundtrip_ListStruct(object x):
    return __shim__roundtrip[ListStruct](x)

def roundtrip_SetStruct(object x):
    return __shim__roundtrip[SetStruct](x)

def roundtrip_MapStruct(object x):
    return __shim__roundtrip[MapStruct](x)

def roundtrip_ComposeStruct(object x):
    return __shim__roundtrip[ComposeStruct](x)

def roundtrip_AdaptedFields(object x):
    return __shim__roundtrip[AdaptedFields](x)

def check_MyStruct(object x):
    return bool(__shim__typeCheck[MyStruct](x))

def check_MyDataItem(object x):
    return bool(__shim__typeCheck[MyDataItem](x))

def check_MyUnion(object x):
    return bool(__shim__typeCheck[Shallot](x))

def check_MyEnum(object x):
    return bool(__shim__typeCheck[MyEnum](x))

def check_StringPair(object x):
    return __shim__typeCheck[StringPair](x)

def check_DoubledPair(object x):
    return __shim__typeCheck[DoubledPair](x)

def check_PrimitiveStruct(object x):
    return bool(__shim__typeCheck[PrimitiveStruct](x))

def check_ListStruct(object x):
    return bool(__shim__typeCheck[ListStruct](x))

def check_SetStruct(object x):
    return bool(__shim__typeCheck[SetStruct](x))

def check_MapStruct(object x):
    return bool(__shim__typeCheck[MapStruct](x))

def check_ComposeStruct(object x):
    return bool(__shim__typeCheck[ComposeStruct](x))
