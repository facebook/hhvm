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

/**
 * @file Json5Protocol.h
 * @brief Utility functions for JSON/JSON5 serialization of Thrift values.
 *
 * This file provides public APIs to serialize/deserialize Thrift values
 * to/from JSON and JSON5 formats:
 *
 *   // Convert Thrift value to basic JSON string (with explicit type Tag)
 *   template<class Tag>
 *   std::string Json5ProtocolUtils::toBasicJson(const native_type<Tag>& value);
 *
 *   // Convert Thrift value to JSON5 string (with explicit type Tag)
 *   template<class Tag>
 *   std::string Json5ProtocolUtils::toJson5(const native_type<Tag>& value);
 *
 *   // Deserialize JSON5 string to Thrift value (with explicit type Tag)
 *   template<class Tag>
 *   native_type<Tag> Json5ProtocolUtils::fromJson5(std::string_view json);
 *
 * ## Examples
 *
 *   // With explicit type tag
 *   std::string json = Json5ProtocolUtils::toBasicJson<type::int32_t>(42);
 *   assert(json == "42");
 *
 *   // With type inference
 *   MyStruct s;
 *   s.floatVal() = std::numeric_limits<float>::quiet_NaN();
 *   std::string json = Json5ProtocolUtils::toBasicJson(s);
 *   assert(json == R"({
 *     "floatVal": "NaN"
 *   })");
 *   std::string json5 = Json5ProtocolUtils::toJson5(s);
 *   assert(json5 == R"({
 *     floatVal: NaN,
 *   })");
 *
 *   // Deserialization with type inference
 *   auto s2 = Json5ProtocolUtils::fromJson5<MyStruct>("{floatVal: NaN}");
 */

#pragma once

#include <thrift/lib/cpp2/protocol/detail/Json5ProtocolReader.h>
#include <thrift/lib/cpp2/protocol/detail/Json5ProtocolWriter.h>

namespace apache::thrift {
class Json5ProtocolUtils final {
 public:
  Json5ProtocolUtils() = delete;

  /**
   * Converts a Thrift value to a standard JSON string.
   *
   * This function serializes Thrift values to JSON format with 2-space
   * indentation. Special floating-point values (NaN, Infinity) are encoded as
   * quoted strings for compatibility with standard JSON parsers.
   *
   * @tparam Tag  The Thrift type tag (e.g., type::i32_t, type::string_t).
   *              The Tag can be omitted for thrift structs/unions/exceptions.
   * @param value The Thrift value to serialize.
   * @return      A JSON-formatted string representation of the value.
   *
   * Example:
   *   std::string json = toBasicJson<type::i32_t>(42);
   *   // Returns: "42"
   *
   *   MyStruct s;
   *   s.floatVal() = std::numeric_limits<float>::quiet_NaN();
   *   std::string json = toBasicJson(s);
   *   // Equivalent to `toBasicJson<type::struct_t<MyStruct>>(s)`
   *   // Returns:
   *   // {
   *   //   "floatVal": "NaN"
   *   // }
   */
  template <class Tag>
  [[nodiscard]] static std::string toBasicJson(
      const type::native_type<Tag>& value) {
    return json5::detail::toJsonImpl<Tag>(value, {.indentWidth = 2});
  }
  template <class T>
    requires(is_thrift_class_v<T>)
  [[nodiscard]] static std::string toBasicJson(const T& value) {
    return toBasicJson<type::infer_tag<T>>(value);
  }

  /**
   * Converts a Thrift value to a JSON5 string.
   *
   * This function serializes Thrift values to JSON5 format with 2-space
   * indentation. JSON5 is a superset of JSON that supports:
   * - Unquoted object keys (identifiers)
   * - Trailing commas in objects and arrays
   * - NaN and Infinity as bare literals (not quoted)
   * - Single-line and multi-line comments
   *
   * @tparam Tag  The Thrift type tag (e.g., type::i32_t, type::string_t).
   *              The Tag can be omitted for thrift structs/unions/exceptions.
   * @param value The Thrift value to serialize.
   * @return      A JSON5-formatted string representation of the value.
   *
   * Example:
   *   std::string json5 = toJson5<type::i32_t>(42);
   *   // Returns: "42"
   *
   *   MyStruct s;
   *   s.floatVal() = std::numeric_limits<float>::quiet_NaN();
   *   std::string json5 = toJson5(s);
   *   // Equivalent to `toJson5<type::struct_t<MyStruct>>(s)`
   *   // Returns:
   *   // {
   *   //   floatVal: NaN,
   *   // }
   */
  template <class Tag>
  [[nodiscard]] static std::string toJson5(
      const type::native_type<Tag>& value) {
    auto opts = json5::detail::kJson5Options;
    opts.indentWidth = 2;
    return json5::detail::toJsonImpl<Tag>(value, opts);
  }
  template <class T>
    requires(is_thrift_class_v<T>)
  [[nodiscard]] static std::string toJson5(const T& value) {
    return toJson5<type::infer_tag<T>>(value);
  }

  /**
   * Deserializes a JSON5 string into a Thrift value.
   *
   * @tparam T    The Thrift type tag (e.g., type::i32_t, type::string_t), or
   *              thrift structs/unions/exceptions.
   * @param json  The JSON5 string to deserialize.
   * @return      The deserialized Thrift value.
   *
   * Example:
   *   auto s = fromJson5<type::struct_t<MyStruct>>("{floatVal: NaN}");
   *
   *   // Thrift struct overload.
   *   auto s = fromJson5<MyStruct>("{floatVal: NaN}");
   */
  template <class Tag>
  [[nodiscard]] static type::native_type<Tag> fromJson5(std::string_view json) {
    auto buf = folly::IOBuf::copyBuffer(json);
    json5::detail::Json5ProtocolReader reader;
    reader.setInput(buf.get());
    type::native_type<Tag> value;
    op::decode<Tag>(reader, value);
    if (!reader.getCursor().isAtEnd()) {
      throw std::runtime_error(
          "Json5ProtocolReader: unexpected trailing content after JSON value");
    }
    return value;
  }
  template <ThriftClass T>
  [[nodiscard]] static T fromJson5(std::string_view json) {
    return fromJson5<type::infer_tag<T>>(json);
  }
};
} // namespace apache::thrift
