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

#include <thrift/lib/cpp2/type/Protocol.h>

// Helper so gtest prints out the line number when running the given check.
#define FBTHRIFT_SCOPED_CHECK(check) \
  {                                  \
    SCOPED_TRACE(#check);            \
    check;                           \
  }

namespace apache::thrift::test {

template <typename T>
struct TestValue {
  T value;
};

struct TestAdapter {
  template <typename T>
  static TestValue<T> fromThrift(T&& value) {
    return {std::forward<T>(value)};
  }

  template <typename T>
  static const T& toThrift(const TestValue<T>& value) {
    return value.value;
  }
};

template <typename T, typename Struct, int16_t FieldId>
struct FieldValue {
  T value;
};

struct FieldAdapter {
  template <typename T, typename Struct, int16_t FieldId>
  static FieldValue<T, Struct, FieldId> fromThriftField(
      T&& value, FieldContext<Struct, FieldId>&&) {
    return {std::forward<T>(value)};
  }

  template <typename T, typename Struct, int16_t FieldId>
  static const T& toThrift(const FieldValue<T, Struct, FieldId>& value) {
    return value.value;
  }
};

struct TestStruct {
  void __fbthrift_clear() {}
};

template <typename T, typename Struct, int16_t FieldId>
struct FieldValueWithContext {
  T value;
  std::string* meta = nullptr;
};

struct FieldAdapterWithContext {
  template <typename T, typename Struct, int16_t FieldId>
  static FieldValueWithContext<T, Struct, FieldId> fromThriftField(
      T&& value, FieldContext<Struct, FieldId>&& context) {
    return {std::forward<T>(value), &context.object.meta};
  }

  template <typename T, typename Struct, int16_t FieldId>
  static const T& toThrift(
      const FieldValueWithContext<T, Struct, FieldId>& value) {
    return value.value;
  }
};

struct TestStructWithContext {
  void __fbthrift_clear() { meta.clear(); }
  // custom default
  std::string meta = "meta";
};

// Creates a custom protocol, skipping validation.
inline type::Protocol makeProtocol(std::string name) {
  type::ProtocolUnion data;
  data.custom() = std::move(name);
  return type::Protocol(std::move(data));
}

constexpr auto kUnknownStdProtocol = static_cast<type::StandardProtocol>(1000);
inline const type::Protocol& UnknownProtocol() {
  return type::Protocol::get<kUnknownStdProtocol>();
}

// Returns the full thrift type name, for the given type.
inline std::string thriftType(std::string_view type) {
  if (type.empty()) {
    return {};
  }
  return "facebook.com/thrift/" + std::string(type);
}

} // namespace apache::thrift::test
