/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thrift/lib/python/capi/constructor.h>

namespace apache {
namespace thrift {
namespace python {
namespace capi {
namespace {

// use *FromStringAndSize in case not null-terminated
PyObject* PyBytes_FromCppString(const std::string& str) {
  return PyBytes_FromStringAndSize(str.data(), str.size());
}

PyObject* PyUnicode_FromCppString(const std::string& str) {
  return PyUnicode_FromStringAndSize(str.c_str(), str.size());
}

} // namespace

// In simplest case, returning nullptr via cython signals an error, so no
// need to check capi for error in cpp.
// If this is called from another Constructor, that caller is responsible
// for checking error before calling other capi functions.
#define SPECIALIZE_CAPI(type, capi_fn)                                 \
  PyObject* FOLLY_NULLABLE Constructor<type>::operator()(type&& val) { \
    return capi_fn(std::move(val));                                    \
  }

SPECIALIZE_CAPI(int8_t, PyLong_FromLong)
SPECIALIZE_CAPI(int16_t, PyLong_FromLong)
SPECIALIZE_CAPI(int32_t, PyLong_FromLong)
SPECIALIZE_CAPI(int64_t, PyLong_FromLongLong)
SPECIALIZE_CAPI(uint32_t, PyLong_FromUnsignedLong)
SPECIALIZE_CAPI(uint64_t, PyLong_FromUnsignedLongLong)
SPECIALIZE_CAPI(bool, PyBool_FromLong)
SPECIALIZE_CAPI(float, PyFloat_FromDouble)
SPECIALIZE_CAPI(double, PyFloat_FromDouble)
#undef SPECIALIZE_CAPI

#define SPECIALIZE_CAPI_STR(cpp_type, py_type, capi_fn)                       \
  PyObject* FOLLY_NULLABLE Constructor<py_type>::operator()(cpp_type&& val) { \
    return capi_fn(std::move(val));                                           \
  }
SPECIALIZE_CAPI_STR(std::string, Bytes, PyBytes_FromCppString)
SPECIALIZE_CAPI_STR(std::string, String, PyUnicode_FromCppString)
#undef SPECIALIZE_CAPI_STR

} // namespace capi
} // namespace python
} // namespace thrift
} // namespace apache
