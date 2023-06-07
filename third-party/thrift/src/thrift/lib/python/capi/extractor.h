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

#pragma once

#include <type_traits>

#include <Python.h>
#include <folly/Expected.h>
#include <folly/Preprocessor.h>
#include <thrift/lib/python/capi/types.h>

namespace apache {
namespace thrift {
namespace python {
namespace capi {

// Using an ExtractorResult lets us accommodate two different error handling
// strategies:
//   1. If an error has occurred, avoid making further capi calls
//      and throw C++ exception at C++ entrypoint
//   2. If an error has occurred, avoid making further capi calls and let
//      cython raise python error at cython-binding return to python.
//   The unexpected std::string_view arms lets us capture the C++ file and line
//   to augment the python error raised by capi.
template <typename T>
using ExtractorResult = folly::Expected<native_t<T>, std::string_view>;

template <typename T>
ExtractorResult<T> extractorError(std::string_view msg) {
  return folly::makeUnexpected(msg);
}

#define EXTRACTOR_ERROR(TYPE, MESSAGE) \
  extractorError<TYPE>(CAPI_LOCATED_ERROR(MESSAGE))

template <typename T>
ExtractorResult<T> extractorSuccess(T&& val) {
  static_assert(std::is_same_v<T, native_t<T>>);
  return folly::makeExpected<std::string_view>(std::forward<T>(val));
}

template <typename T>
struct Extractor;

/*
 * Base class provides a conditional extract(PyObject*, bool&). The bool
 * represents whether any python error has occurred. The base extractor calls
 * calls the derived Extractor operator (PyObject*) if no error has occurred;
 * otherwise it returns Unexpected variant to avoid cpython api calls
 * that are undefined behavior.
 */
template <typename T>
struct BaseExtractor {
  ExtractorResult<T> extract(PyObject* obj, bool& extractError) {
    if (extractError) {
      return EXTRACTOR_ERROR(T, "extractError already set");
    }
    auto res = (*static_cast<Extractor<T>*>(this))(obj);
    extractError = res.hasError();
    return res;
  }
};

/**
 * Series of functions to convert a Python object into a native type T.
 *
 * Some important notes:
 * 1. The Extractor borrows a reference to `obj`: It's intended to be used as a
 *    temporary.
 * 2: The extractor will leave the python indicator set if a python
 *    exception is encountered during the extraction. Be sure to check the error
 *    indicator before calling back into python!
 */
template <typename T>
struct Extractor : public BaseExtractor<T> {
  template <typename U>
  struct Unspecialized : std::false_type {};
  static_assert(Unspecialized<T>{}, "No extractor defined");
  ExtractorResult<T> operator()(/* Source */ PyObject* obj);
  int typeCheck(PyObject* obj); /* returns 0/1 bool or -1 on error */
};

// Numeric specializations
#define SPECIALIZE_SCALAR(type)                         \
  template <>                                           \
  struct Extractor<type> : public BaseExtractor<type> { \
    ExtractorResult<type> operator()(PyObject* obj);    \
    int typeCheck(PyObject* obj);                       \
  }

SPECIALIZE_SCALAR(int8_t);
SPECIALIZE_SCALAR(int16_t);
SPECIALIZE_SCALAR(int32_t);
SPECIALIZE_SCALAR(int64_t);
SPECIALIZE_SCALAR(uint32_t);
SPECIALIZE_SCALAR(uint64_t);
SPECIALIZE_SCALAR(bool);
SPECIALIZE_SCALAR(float);
SPECIALIZE_SCALAR(double);

#undef SPECIALIZE_SCALAR

// String specializations
#define SPECIALIZE_STR(type)                            \
  template <>                                           \
  struct Extractor<type> : public BaseExtractor<type> { \
    ExtractorResult<type> operator()(PyObject* obj);    \
    int typeCheck(PyObject* obj);                       \
  }
SPECIALIZE_STR(Bytes);
SPECIALIZE_STR(String);
SPECIALIZE_STR(BytesView);
SPECIALIZE_STR(StringView);

#undef SPECIALIZE_STR

} // namespace capi
} // namespace python
} // namespace thrift
} // namespace apache
