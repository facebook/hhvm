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

from libc cimport stdint
from libcpp cimport bool as cbool
from libcpp.string cimport string

INT32_MIN = stdint.INT32_MIN
INT32_MAX = stdint.INT32_MAX
UINT32_MAX = stdint.UINT32_MAX
INT64_MIN = stdint.INT64_MIN
INT64_MAX = stdint.INT64_MAX
UINT64_MAX = stdint.UINT64_MAX


cdef extern from "thrift/lib/python/capi/test/marshal_fixture.h" namespace "apache::thrift::python":
    cdef object __make_unicode(object)
    cdef object __roundtrip_pyobject[T](object)
    cdef object __roundtrip_unicode(object)
    cdef object __roundtrip_bytes(object)


def roundtrip_int32(object x):
    return __roundtrip_pyobject[stdint.int32_t](x)

def roundtrip_int64(object x):
    return __roundtrip_pyobject[stdint.int64_t](x)

def roundtrip_uint32(object x):
    return __roundtrip_pyobject[stdint.uint32_t](x)

def roundtrip_uint64(object x):
    return __roundtrip_pyobject[stdint.uint64_t](x)

def roundtrip_float(object x):
    return __roundtrip_pyobject[float](x)

def roundtrip_double(object x):
    return __roundtrip_pyobject[double](x)

def roundtrip_bool(object x):
    return __roundtrip_pyobject[cbool](x)

def roundtrip_bytes(object x):
    return __roundtrip_bytes(x)

def roundtrip_unicode(object x):
    return __roundtrip_unicode(x)

def make_unicode(object x):
    return __make_unicode(x)
