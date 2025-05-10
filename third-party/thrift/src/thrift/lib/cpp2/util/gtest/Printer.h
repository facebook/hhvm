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

#include <ostream>

#include <gtest/gtest.h>

// portability/GTest must be imported before any other gtest header
#include <gtest/gtest-spi.h>

#include <thrift/lib/cpp2/FieldRef.h>
#include <thrift/lib/cpp2/protocol/DebugProtocol.h>

#include <thrift/lib/cpp2/gen/module_types_tcc.h>

namespace apache::thrift {

template <typename T>
void PrintTo(field_ref<T> obj, std::ostream* os) {
  *os << "field_ref holding " << testing::PrintToString(*obj);
}

template <typename T>
void PrintTo(optional_field_ref<T> obj, std::ostream* os) {
  if (!obj) {
    *os << "empty optional_field_ref";
    return;
  }
  *os << "optional_field_ref holding " << testing::PrintToString(*obj);
}

#if __cpp_concepts

namespace detail {

template <typename T>
constexpr bool is_type_complete() {
  return requires { sizeof(T); };
}

template <typename T>
constexpr bool supports_custom_protocol() {
  // TccStructTraits is defied in the _custom_protocol.h header, so
  // we can check if that header has been included by checking if
  // the type is complete
  return is_type_complete<TccStructTraits<T>>();
}

} // namespace detail

// WARNING: The behavior of this function depends on which headers
// were included by **the first time it is invoked**.
// The compiler caches the first template instantiation, so the detection
// on whether the _types_custom_protocol.h was included or not
// happens at that point.
// If you include the header, but still see the fallback message,
// make sure to include the header **above** whatever else you are
// including that might be calling this function (normally some other testing
// code).
template <typename T>
  requires(
      is_thrift_class_v<std::remove_cvref_t<T>> ||
      is_thrift_exception_v<std::remove_cvref_t<T>>)
void PrintTo(const T& obj, std::ostream* os) {
  if constexpr (detail::supports_custom_protocol<std::remove_cvref_t<T>>()) {
    *os << debugString(obj);
  } else {
    *os << "a thrift object of type " << folly::pretty_name<T>()
        << " (include the corresponding _types_custom_protocol.h header before thrift/lib/cpp2/test/Printer.h to see the contents)";
  }
}

} // namespace apache::thrift

#endif
