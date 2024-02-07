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

#include <thrift/lib/python/capi/constructor.h>
#include <thrift/test/python_capi/gen-cpp2/serialized_dep_types.h>

namespace apache::thrift::test {

using ::apache::thrift::python::capi::Constructor;

// DO NOT USE.
// IT IS VERY DANGER TO JUST YOLO ASSUME THE PYTHON TYPES ARE ALREADY IMPORTED
// DO NOT USE.

template <typename T>
PyObject* makeStruct(bool setOptional) noexcept {
  T s;
  s.s() = "Hello";
  s.i() = 42;
  if (setOptional) {
    s.os() = "Optional";
  }
  s.rs() = "World";
  return Constructor<T>{}(s);
}

template <typename T>
PyObject* makeError(bool setOptional) noexcept {
  T s;
  s.msg() = "oops";
  if (setOptional) {
    s.os() = "Optional";
  }
  s.rs() = "Required";
  return Constructor<T>{}(s);
}

template <typename T>
PyObject* makeUnion(bool setString) {
  T u;
  if (setString) {
    u.s_ref() = "Hello";
  } else {
    u.i_ref() = 42;
  }
  return Constructor<T>{}(u);
}

template <typename T>
PyObject* makeUnset() noexcept {
  return Constructor<T>{}(T{});
}

} // namespace apache::thrift::test
