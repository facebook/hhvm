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
#include <folly/CppAttributes.h>
#include <thrift/lib/python/capi/types.h>

namespace apache {
namespace thrift {
namespace python {
namespace capi {
/**
 * Series of functions to convert a native type T into a Python object
 *
 * Some important notes:
 * 1. The construct function returns a 'new reference'. i.e. you own the object
 *    it returns.
 * 2: The constructor will leave the python indicator set if a python
 *    exception is encountered during the extraction. Be sure to check the error
 *    indicator before calling back into python!
 */
template <typename T>
struct Constructor {
  template <typename U>
  struct Unspecialized : std::false_type {};
  static_assert(Unspecialized<T>{}, "No constructor defined");
  /* Target */ PyObject* operator()(T&&);
};

#define SPECIALIZE_SCALAR(type)                  \
  template <>                                    \
  struct Constructor<type> {                     \
    PyObject* FOLLY_NULLABLE operator()(type&&); \
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
#define SPECIALIZE_STR(cpp_type, py_type)            \
  template <>                                        \
  struct Constructor<py_type> {                      \
    PyObject* FOLLY_NULLABLE operator()(cpp_type&&); \
  }

SPECIALIZE_STR(std::string, Bytes);
SPECIALIZE_STR(std::string, String);

#undef SPECIALIZE_STR

template <typename ElemT, typename CppT>
struct Constructor<list<ElemT, CppT>> {
  PyObject* FOLLY_NULLABLE operator()(CppT&& val) {
    const size_t size = val.size();
    StrongRef list(PyTuple_New(size));
    if (!list) {
      return nullptr;
    }
    Constructor<ElemT> ctor{};
    for (size_t i = 0; i < size; ++i) {
      // PyTuple_SET_ITEM steals, so don't use StrongRef
      PyObject* elem(ctor(std::move(val[i])));
      if (elem == nullptr) {
        // StrongRef DECREFs the list tuple on scope exit
        return nullptr;
      }
      PyTuple_SET_ITEM(*list, i, elem);
    }
    return std::move(list).release();
  }
};

} // namespace capi
} // namespace python
} // namespace thrift
} // namespace apache
