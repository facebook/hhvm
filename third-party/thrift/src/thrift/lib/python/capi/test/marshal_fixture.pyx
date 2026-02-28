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
from libcpp.memory cimport unique_ptr
from folly.iobuf cimport cIOBuf

INT8_MIN = stdint.INT8_MIN
INT8_MAX = stdint.INT8_MAX
INT16_MIN = stdint.INT16_MIN
INT16_MAX = stdint.INT16_MAX
INT32_MIN = stdint.INT32_MIN
INT32_MAX = stdint.INT32_MAX
INT64_MIN = stdint.INT64_MIN
INT64_MAX = stdint.INT64_MAX
UINT8_MAX = stdint.UINT8_MAX
UINT16_MAX = stdint.UINT16_MAX
UINT32_MAX = stdint.UINT32_MAX
UINT64_MAX = stdint.UINT64_MAX


cdef extern from "thrift/lib/python/capi/test/marshal_fixture.h" namespace "apache::thrift::python":
    cdef object __make_unicode(object)
    cdef object __make_unicode_list(object)
    cdef object __make_unicode_set(object)
    cdef object __roundtrip_pyobject[T](object)
    cdef object __roundtrip_unicode(object)
    cdef object __roundtrip_fallible_unicode(object)
    cdef object __roundtrip_bytes(object)
    cdef object __roundtrip_list[T](object)
    cdef object __roundtrip_unicode_list(object)
    cdef object __roundtrip_bytes_list(object)
    cdef object __roundtrip_set[T](object)
    cdef object __roundtrip_unicode_set(object)
    cdef object __roundtrip_bytes_set(object)
    cdef object __roundtrip_map[K, V](object)
    cdef object __roundtrip_bytes_key_map[V](object)
    cdef object __roundtrip_unicode_key_map[V](object)
    cdef object __roundtrip_bytes_val_map[K](object)
    cdef object __roundtrip_unicode_val_map[K](object)
    cdef object __make_unicode_val_map(object)

def roundtrip_int8(object x):
    return __roundtrip_pyobject[stdint.int8_t](x)

def roundtrip_int16(object x):
    return __roundtrip_pyobject[stdint.int16_t](x)

def roundtrip_int32(object x):
    return __roundtrip_pyobject[stdint.int32_t](x)

def roundtrip_int64(object x):
    return __roundtrip_pyobject[stdint.int64_t](x)

def roundtrip_uint8(object x):
    return __roundtrip_pyobject[stdint.uint8_t](x)

def roundtrip_uint16(object x):
    return __roundtrip_pyobject[stdint.uint16_t](x)

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

def roundtrip_fallible_unicode(object x):
    return __roundtrip_fallible_unicode(x)

def make_unicode(object x):
    return __make_unicode(x)

def roundtrip_iobuf_stack(object x):
    return __roundtrip_pyobject[cIOBuf](x)

def roundtrip_iobuf_heap(object x):
    return __roundtrip_pyobject[unique_ptr[cIOBuf]](x)

# Lists
def roundtrip_int32_list(object x):
    return __roundtrip_list[stdint.int32_t](x)

def roundtrip_bool_list(object x):
    return __roundtrip_list[cbool](x)

def roundtrip_double_list(object x):
    return __roundtrip_list[double](x)

def roundtrip_bytes_list(object x):
    return __roundtrip_bytes_list(x)

def roundtrip_unicode_list(object x):
    return __roundtrip_unicode_list(x)

def make_unicode_list(object x):
    return __make_unicode_list(x)

# Sets
def roundtrip_int32_set(object x):
    return __roundtrip_set[stdint.int32_t](x)

def roundtrip_bool_set(object x):
    return __roundtrip_set[cbool](x)

def roundtrip_double_set(object x):
    return __roundtrip_set[double](x)

def roundtrip_bytes_set(object x):
    return __roundtrip_bytes_set(x)

def roundtrip_unicode_set(object x):
    return __roundtrip_unicode_set(x)

def make_unicode_set(object x):
    return __make_unicode_set(x)

# Maps
def roundtrip_int32_bool_map(object x):
    return __roundtrip_map[stdint.int32_t, cbool](x)

def roundtrip_byte_float_map(object x):
    return __roundtrip_map[stdint.int8_t, double](x)

def roundtrip_bytes_key_map(object x):
    return __roundtrip_bytes_key_map[stdint.int64_t](x)

def roundtrip_unicode_key_map(object x):
    return __roundtrip_unicode_key_map[stdint.int64_t](x)

def roundtrip_bytes_val_map(object x):
    return __roundtrip_bytes_val_map[stdint.int64_t](x)

def roundtrip_unicode_val_map(object x):
    return __roundtrip_unicode_val_map[stdint.int64_t](x)

def make_unicode_val_map(object x):
    return __make_unicode_val_map(x)
