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

#include <folly/python/iobuf.h>
#include <thrift/lib/python/capi/constructor.h>

namespace apache {
namespace thrift {
namespace python {
namespace capi {

// In simplest case, returning nullptr via cython signals an error, so no
// need to check capi for error in cpp.
// If this is called from another Constructor, that caller is responsible
// for checking error before calling other capi functions.
#define SPECIALIZE_CAPI(type, capi_fn)                               \
  PyObject* FOLLY_NULLABLE Constructor<type>::operator()(type val) { \
    return capi_fn(val);                                             \
  }

SPECIALIZE_CAPI(signed char, PyLong_FromLong)
SPECIALIZE_CAPI(signed short, PyLong_FromLong)
SPECIALIZE_CAPI(signed int, PyLong_FromLong)
SPECIALIZE_CAPI(signed long, PyLong_FromLongLong)
SPECIALIZE_CAPI(signed long long, PyLong_FromLongLong)
SPECIALIZE_CAPI(unsigned char, PyLong_FromUnsignedLong)
SPECIALIZE_CAPI(unsigned short, PyLong_FromUnsignedLong)
SPECIALIZE_CAPI(unsigned int, PyLong_FromUnsignedLong)
SPECIALIZE_CAPI(unsigned long, PyLong_FromUnsignedLongLong)
SPECIALIZE_CAPI(unsigned long long, PyLong_FromUnsignedLongLong)
SPECIALIZE_CAPI(bool, PyBool_FromLong)
SPECIALIZE_CAPI(float, PyFloat_FromDouble)
SPECIALIZE_CAPI(double, PyFloat_FromDouble)
#undef SPECIALIZE_CAPI

PyObject* Constructor<folly::IOBuf>::operator()(const folly::IOBuf& val) {
  /* No need to check return value here because */
  /* there are no further c-api calls in this function */
  return folly::python::make_python_iobuf(std::make_unique<folly::IOBuf>(val));
}

PyObject* Constructor<folly::IOBuf>::operator()(folly::IOBuf&& val) {
  return folly::python::make_python_iobuf(
      std::make_unique<folly::IOBuf>(std::move(val)));
}

PyObject* Constructor<std::unique_ptr<folly::IOBuf>>::operator()(
    const std::unique_ptr<folly::IOBuf>& val) {
  return folly::python::make_python_iobuf(val->clone());
}

PyObject* Constructor<std::unique_ptr<folly::IOBuf>>::operator()(
    std::unique_ptr<folly::IOBuf>&& val) {
  return folly::python::make_python_iobuf(std::move(val));
}

} // namespace capi
} // namespace python
} // namespace thrift
} // namespace apache
