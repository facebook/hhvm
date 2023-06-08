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
    cppclass MyStruct
    cppclass MyDataItem
    cppclass OurUnion

cdef extern from "thrift/test/python_capi/fixture.h" namespace "apache::thrift::test":
    cdef object __shim__roundtrip[T](object)
    cdef cbool __shim__typeCheck[T](object)



def roundtrip_MyStruct(object x):
    return __shim__roundtrip[MyStruct](x)

def roundtrip_MyDataItem(object x):
    return __shim__roundtrip[MyDataItem](x)

def roundtrip_MyUnion(object x):
    return __shim__roundtrip[OurUnion](x)

def check_MyStruct(object x):
    return bool(__shim__typeCheck[MyStruct](x))

def check_MyDataItem(object x):
    return bool(__shim__typeCheck[MyDataItem](x))

def check_MyUnion(object x):
    return bool(__shim__typeCheck[OurUnion](x))
