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

from libcpp.string cimport string as cstring
from thrift.python.capi.cpp_converter cimport (
    cpp_to_python,
    python_to_cpp,
    python_to_cpp_throws
)
from thrift.test.python_capi.thrift_dep.thrift_types import DepUnion

# bind C++ thrift types to instantiate templates in cpp_to_python, python_to_cpp
cdef extern from "thrift/test/python_capi/gen-cpp2/thrift_dep_types.h":
    cdef cppclass cEnum "thrift::test::python_capi::DepEnum"
    cdef cppclass cStruct "thrift::test::python_capi::DepStruct"
    cdef cppclass cUnion "thrift::test::python_capi::DepUnion"

# include definitions of Constructor and Extractor implementations used by
# cpp_to_python, python_to_cpp
cdef extern from "thrift/test/python_capi/gen-python-capi/thrift_dep/thrift_types_capi.h":
    pass


# cython bind some C++ functions
cdef extern from "thrift/test/python_capi/cpp_convert/lib.h" namespace "thrift::test::lib":
    cdef cEnum enumSource()
    cdef cStruct structSource()
    cdef cUnion unionSource()
    cdef cUnion overflowSource()
    int unpackEnum(cEnum)
    cstring unpackStruct(cStruct)
    cstring unpackUnion(cUnion)



# instead of Py3Struct._fbthrift_create, use python_from_cpp
def enum_from_cpp():
    return cpp_to_python[cEnum](enumSource())

def struct_from_cpp():
    return cpp_to_python[cStruct](structSource())

def union_from_cpp():
    return cpp_to_python[cUnion](unionSource())


# instead of py3struct._cpp_obj, use python_to_cpp
def unpack_python_enum(object x):
    return unpackEnum(python_to_cpp[cEnum](x))

def unpack_python_struct(object x):
    return unpackStruct(python_to_cpp[cStruct](x)).decode("utf8")

cdef object convert_return(cstring result, object type_):
    if type_ == DepUnion.Type.s:
        return result.decode("utf8")
    elif type_ == DepUnion.Type.i:
        return int(result)
    else:
        return None

def unpack_python_union(object x):
    return convert_return(
        unpackUnion(python_to_cpp[cUnion](x)),
        x.type
    )

def unpack_python_union_throws(object x):
    return convert_return(
        unpackUnion(python_to_cpp_throws[cUnion](x)),
        x.type
    )
