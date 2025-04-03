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
    python_to_cpp_throws
)
from thrift.test.python_capi.thrift_dep.thrift_converter cimport (
    cDepEnum as cEnum,
    cDepStruct as cStruct,
    cDepUnion as cUnion,
    DepStruct_convert_to_cpp as Struct_convert_to_cpp,
    DepStruct_from_cpp as Struct_from_cpp,
    DepUnion_convert_to_cpp as Union_convert_to_cpp,
    DepUnion_from_cpp as Union_from_cpp,
)
from thrift.test.python_capi.thrift_dep.thrift_types import DepUnion, DepEnum


# cython bind some C++ functions
cdef extern from "thrift/test/python_capi/cpp_convert/lib.h" namespace "thrift::test::lib":
    cdef cEnum enumSource()
    cdef cStruct structSource()
    cdef cUnion unionSource()
    cdef cUnion overflowSource()
    int unpackEnum(cEnum)
    cstring unpackStruct(cStruct)
    cstring unpackUnion(cUnion)



# We could generate an enum converter, but not worth the build bloat
def enum_from_cpp():
    return DepEnum(<int>enumSource())

def struct_from_cpp():
    return Struct_from_cpp(structSource())

def union_from_cpp():
    return Union_from_cpp(unionSource())


# We could generate an enum converter, but not worth the build bloat
def unpack_python_enum(object x):
    return unpackEnum(<cEnum><int>(x))

def unpack_python_struct(object x):
    return unpackStruct(Struct_convert_to_cpp(x)).decode("utf8")

cdef object convert_return(cstring result, object type_):
    if type_ == DepUnion.Type.s:
        return result.decode("utf8")
    elif type_ == DepUnion.Type.i:
        return int(result)
    else:
        return None

def unpack_python_union(object x):
    return convert_return(
        unpackUnion(Union_convert_to_cpp(x)),
        x.type
    )

# include definitions of Extractor implementations used by python_to_cpp_throws
# if python_to_cpp_throws weren't used, we wouldn't need this manual include
cdef extern from "thrift/test/python_capi/gen-python-capi/thrift_dep/thrift_types_capi.h":
    cdef cppclass NamespaceTag "thrift__test__python_capi__thrift_dep::NamespaceTag"

def unpack_python_union_throws(object x):
    return convert_return(
        unpackUnion(python_to_cpp_throws[cUnion, NamespaceTag](x)),
        x.type
    )
