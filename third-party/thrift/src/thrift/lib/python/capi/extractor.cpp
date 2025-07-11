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
#include <thrift/lib/python/capi/extractor.h>

namespace apache::thrift::python::capi {
namespace {

template <typename T, typename V>
constexpr std::enable_if_t<std::is_integral_v<T>, bool> numericOverflow(V x) {
  return x < std::numeric_limits<T>::min() || x > std::numeric_limits<T>::max();
}

template <typename T, typename V>
constexpr std::enable_if_t<std::is_floating_point_v<T>, bool> numericOverflow(
    V x) {
  // python 'float' is C `double`, so no chance of overflow
  if constexpr (std::is_same_v<T, double>) {
    return false;
  } else {
    if (std::isnan(x) || std::isinf(x)) {
      return false;
    }
    return x < -std::numeric_limits<T>::max() ||
        x > std::numeric_limits<T>::max();
  }
}

template <typename R>
ExtractorResult<R> extractFromBytes(PyObject* obj) {
  Py_ssize_t size = 0;
  char* buffer = nullptr;
  int ret = PyBytes_AsStringAndSize(obj, &buffer, &size);
  if (ret == -1) {
    return EXTRACTOR_ERROR(R, "PyBytes_AsStringAndSize error");
  }
  return folly::makeExpected<std::string_view>(R(buffer, size));
}

template <typename R>
ExtractorResult<R> extractFromString(PyObject* obj) {
  Py_ssize_t size = 0;
  const char* buffer = PyUnicode_AsUTF8AndSize(obj, &size);
  if (buffer == nullptr) {
    return EXTRACTOR_ERROR(R, "PyUnicode_AsUTF8AndSize error");
  }
  return folly::makeExpected<std::string_view>(R(buffer, size));
}
} // namespace

/*
 * int extraction is tricky because pycapi returns (unsigned) long, which can be
 * 64-bit, so have to PyErr_Set(PyExc_OverflowError, ...) manually for 32-bit;
 * Also, -1 could signify success (int == -1) or error, so have to use
 * PyErr_Occurred to disambiguate. Float extraction has the same -1.0
 * disambiguation issue.
 * A future diff could introduce `Prechecked<T>` variants that always skip the
 * numericOverflow checks.
 */
#define SPECIALIZE_NUM(type, capi_extract, capi_check)                         \
  ExtractorResult<type> Extractor<type>::operator()(                           \
      /* source */ PyObject* obj) {                                            \
    using PyCapiReturnT =                                                      \
        std::invoke_result_t<decltype(&capi_extract), PyObject*>;              \
    PyCapiReturnT long_res = capi_extract(obj);                                \
    if (long_res == static_cast<PyCapiReturnT>(-1) && PyErr_Occurred()) {      \
      return EXTRACTOR_ERROR(type, "C-api error");                             \
    } else if (numericOverflow<type>(long_res)) {                              \
      PyErr_SetString(PyExc_OverflowError, "type overflow");                   \
      return EXTRACTOR_ERROR(type, "Overflow");                                \
    }                                                                          \
    return static_cast<type>(long_res);                                        \
  }                                                                            \
  int Extractor<type>::typeCheck(PyObject* obj) {                              \
    if (!capi_check(obj)) { /* false for implicitly convertible */             \
      return 0;                                                                \
    }                                                                          \
    if (!std::is_same_v<type, double> && !Extractor<type>()(obj).hasValue()) { \
      PyErr_Clear(); /*clear OverflowError */                                  \
      return 0;                                                                \
    }                                                                          \
    return 1;                                                                  \
  }

SPECIALIZE_NUM(signed char, PyLong_AsLong, PyLong_CheckExact)
SPECIALIZE_NUM(signed short, PyLong_AsLong, PyLong_CheckExact)
SPECIALIZE_NUM(signed int, PyLong_AsLong, PyLong_CheckExact)
SPECIALIZE_NUM(signed long, PyLong_AsLongLong, PyLong_CheckExact)
SPECIALIZE_NUM(signed long long, PyLong_AsLongLong, PyLong_CheckExact)
SPECIALIZE_NUM(unsigned char, PyLong_AsUnsignedLong, PyLong_CheckExact)
SPECIALIZE_NUM(unsigned short, PyLong_AsUnsignedLong, PyLong_CheckExact)
SPECIALIZE_NUM(unsigned int, PyLong_AsUnsignedLong, PyLong_CheckExact)
SPECIALIZE_NUM(unsigned long, PyLong_AsUnsignedLongLong, PyLong_CheckExact)
SPECIALIZE_NUM(unsigned long long, PyLong_AsUnsignedLongLong, PyLong_CheckExact)
SPECIALIZE_NUM(float, PyFloat_AsDouble, PyFloat_CheckExact)
SPECIALIZE_NUM(double, PyFloat_AsDouble, PyFloat_CheckExact)

#undef SPECIALIZE_NUM

ExtractorResult<bool> Extractor<bool>::operator()(PyObject* obj) {
  if (obj == Py_True) {
    return true;
  } else if (obj == Py_False) {
    return false;
  } else {
    PyErr_SetString(PyExc_TypeError, CAPI_LOCATED_ERROR("Not a bool"));
    return EXTRACTOR_ERROR(bool, "Not a bool");
  }
}

int Extractor<bool>::typeCheck(PyObject* obj) {
  return obj == Py_True || obj == Py_False;
}

#define SPECIALIZE_STR(py_type, cpp_type, extract_fn, check_capi)          \
  ExtractorResult<py_type> Extractor<py_type>::operator()(PyObject* obj) { \
    return extract_fn<cpp_type>(obj);                                      \
  }                                                                        \
  int Extractor<py_type>::typeCheck(PyObject* obj) {                       \
    return check_capi(obj);                                                \
  }

SPECIALIZE_STR(Bytes, std::string, extractFromBytes, PyBytes_CheckExact)
SPECIALIZE_STR(
    BytesView, std::string_view, extractFromBytes, PyBytes_CheckExact)
SPECIALIZE_STR(String, std::string, extractFromString, PyUnicode_CheckExact)
SPECIALIZE_STR(
    StringView, std::string_view, extractFromString, PyUnicode_CheckExact)

#undef SPECIALIZE_STR

ExtractorResult<FallibleString> Extractor<FallibleString>::operator()(
    PyObject* obj) {
  auto result = extractFromString<std::string>(obj);
  if (result.hasValue()) {
    return result;
  }
  CHECK(PyErr_Occurred());
  PyErr_Clear();
  result = extractFromBytes<std::string>(obj);
  if (result.hasValue()) {
    return result;
  }
  result = EXTRACTOR_ERROR(FallibleString, "Expected unicode `str`");
  return result;
}

int Extractor<FallibleString>::typeCheck(PyObject* obj) {
  return PyUnicode_CheckExact(obj) || PyBytes_CheckExact(obj);
}

ExtractorResult<folly::IOBuf> Extractor<folly::IOBuf>::operator()(
    PyObject* obj) {
  auto buf = folly::python::iobuf_from_python_iobuf(obj);
  if (PyErr_Occurred()) {
    return EXTRACTOR_ERROR(folly::IOBuf, "IOBuf extract error");
  }
  return buf;
}

#define SPECIALIZE_IOBUF_TYPECHECK(type)          \
  int Extractor<type>::typeCheck(PyObject* obj) { \
    auto result = (*this)(obj);                   \
    if (result.hasError()) {                      \
      PyErr_Clear();                              \
      return 0;                                   \
    }                                             \
    return 1;                                     \
  }

SPECIALIZE_IOBUF_TYPECHECK(folly::IOBuf)

ExtractorResult<std::unique_ptr<folly::IOBuf>>
Extractor<std::unique_ptr<folly::IOBuf>>::operator()(PyObject* obj) {
  auto buf = folly::python::iobuf_ptr_from_python_iobuf(obj);
  if (buf == nullptr) {
    CHECK(PyErr_Occurred());
    return EXTRACTOR_ERROR(
        std::unique_ptr<folly::IOBuf>, "IOBuf extract error");
  }
  return buf;
}
SPECIALIZE_IOBUF_TYPECHECK(std::unique_ptr<folly::IOBuf>)
#undef SPECIALIZE_IOBUF_TYPECHECK

} // namespace apache::thrift::python::capi
