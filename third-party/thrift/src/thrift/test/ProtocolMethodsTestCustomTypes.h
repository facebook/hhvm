/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <string>

#include <thrift/lib/cpp2/protocol/detail/protocol_methods.h>

namespace apache {
namespace thrift {

namespace test {

struct MyInt {
  int value{};
  constexpr MyInt() = default;
  MyInt(int value) : value(value) {}
  bool operator==(const MyInt& other) const { return value == other.value; }
};

struct MyString {
  std::string value;
  MyString() = default;
  MyString(const char* value) : value(value) {}
  bool operator==(const MyString& other) const { return value == other.value; }
};

} // namespace test

namespace detail {
namespace pm {

template <>
struct protocol_methods<type_class::integral, test::MyInt> {
  template <typename Protocol>
  static void read(Protocol& protocol, test::MyInt& out) {
    protocol.readI32(out.value);
  }

  template <typename Protocol, typename Context>
  static void readWithContext(Protocol& protocol, test::MyInt& out, Context&) {
    read(protocol, out);
  }

  template <typename Protocol>
  static std::size_t write(Protocol& protocol, const test::MyInt& in) {
    return protocol.writeI32(in.value);
  }

  template <bool, typename Protocol>
  static std::size_t serializedSize(Protocol& protocol, const test::MyInt& in) {
    return protocol.serializedSizeI32(in.value);
  }
};

template <>
struct protocol_methods<type_class::string, test::MyString> {
  template <typename Protocol>
  static void read(Protocol& protocol, test::MyString& out) {
    protocol.readString(out.value);
  }

  template <typename Protocol, typename Context>
  static void readWithContext(
      Protocol& protocol, test::MyString& out, Context&) {
    read(protocol, out);
  }

  template <typename Protocol>
  static std::size_t write(Protocol& protocol, const test::MyString& in) {
    return protocol.writeString(in.value);
  }

  template <bool, typename Protocol>
  static std::size_t serializedSize(
      Protocol& protocol, const test::MyString& in) {
    return protocol.serializedSizeString(in.value);
  }
};

} // namespace pm
} // namespace detail
} // namespace thrift
} // namespace apache
