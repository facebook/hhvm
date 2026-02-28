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

import thrift.test.python_capi.serialized_dep.thrift_types as thrift_types

cdef extern from "thrift/test/python_capi/gen-cpp2/serialized_dep_types.h" namespace "thrift::test::python_capi":
    cppclass SerializedStruct
    cppclass MarshalStruct
    cppclass SerializedUnion
    cppclass MarshalUnion
    cppclass SerializedError
    cppclass MarshalError

cdef extern from "thrift/test/python_capi/gen-python-capi/serialized_dep/thrift_types_capi.h":
    pass

cdef extern from "thrift/test/python_capi/parity.h" namespace "apache::thrift::test":
    cdef object makeStruct[T](cbool)
    cdef object makeUnion[T](cbool)
    cdef object makeError[T](cbool)
    cdef object makeUnset[T]()

def make_serialized_struct(cbool is_set) -> thrift_types.SerializedStruct:
    return makeStruct[SerializedStruct](is_set)

def make_serialized_union(cbool set_string):
    return makeUnion[SerializedUnion](set_string)

def make_serialized_error(cbool is_set):
    return makeError[SerializedError](is_set)

def make_marshal_struct(cbool is_set):
    return makeStruct[MarshalStruct](is_set)

def make_marshal_union(cbool set_string):
    return makeUnion[MarshalUnion](set_string)

def make_marshal_error(cbool is_set):
    return makeError[MarshalError](is_set)

def make_unset_struct_serialized():
    return makeUnset[SerializedStruct]()

def make_unset_struct_marshal():
    return makeUnset[MarshalStruct]()

def make_unset_union_serialized():
    return makeUnset[SerializedUnion]()

def make_unset_union_marshal():
    return makeUnset[MarshalUnion]()

def make_unset_error_serialized():
    return makeUnset[SerializedError]()

def make_unset_error_marshal():
    return makeUnset[MarshalError]()
