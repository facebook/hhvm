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
 * @file Json5ProtocolReader.h
 * @brief JSON/JSON5 deserialization for Thrift values.
 *
 * Json5ProtocolReader provides a Thrift protocol reader interface that parses
 * JSON5 or "basic" JSON input:
 * - "JSON5" refers to the "JSON5 Data Interchange Format (version 1.0.0/March
 *   2018)" as defined in https://spec.json5.org/. It is a subset of ECMAScript
 *   5.1 (https://262.ecma-international.org/5.1/).
 * - "Basic JSON" refers to the "JavaScript Object Notation (JSON) Data
 *   Interchange Format" as defined in RFC7159
 *   (https://datatracker.ietf.org/doc/html/rfc7159)
 *
 * The API of this class is modeled after SimpleJSONProtocolReader.
 *
 * ## Current status
 *
 * This is EXPERIMENTAL. The output is unstable and there are missing features.
 * APIs may break with no prior notice. USE AT YOUR OWN RISK (preferably NOT IN
 * PRODUCTION).
 *
 * In addition, `gen-cpp2/[module]_types_custom_protocol.h` needs to be
 * included. (TODO: implement structs/unions support in `op::decode` so that we
 * don't need to include custom protocol header to deserialize Json5 protocol).
 *
 * ## Compatibility Features
 *
 * To support backward compatibility with existing JSON formats, this reader
 * accepts multiple input formats for certain types:
 *
 * ### Bool Fields
 * - Accepts both true/false bare literals and quoted strings: `true`, `"true"`
 *
 * ### Integer Fields
 * - Accepts both JSON numbers and quoted strings: `42` and `"42"`
 *
 * ### Floating-Point Fields
 * - Accepts both JSON numbers and quoted strings: `3.14` and `"3.14"`
 * - Supports NaN and Infinity as bare literals or quoted: `NaN`, `"NaN"`,
 *   `Infinity`, `"Infinity"`, `-Infinity`, `"-Infinity"`
 *
 * ### Enum Fields
 * - Accepts: "NAME (value)", "NAME", "(value)", or bare integer
 * - Examples: `"ONE (1)"`, `"ONE"`, `"(1)"`, `1`
 *
 * ### Map Fields
 * - Object form: `{"key1": value1, "key2": value2}`
 * - Array form: `[{"key": k1, "value": v1}, {"key": k2, "value": v2}]`
 */

#pragma once

#include <concepts>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <fmt/core.h>
#include <folly/Conv.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp/protocol/TProtocol.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>
#include <thrift/lib/cpp2/protocol/detail/CompoundTypeTracker.h>
#include <thrift/lib/cpp2/protocol/detail/JsonReader.h>

namespace apache::thrift::json5::detail {

/**
 * Json5ProtocolReader provides a Thrift protocol reader interface that parses
 * JSON or JSON5 format. The implementation is based on JsonReader - it forwards
 * function calls to JsonReader, which reads directly from a Cursor.
 *
 * Key features:
 * - API similar to SimpleJSONProtocolReader
 * - Takes input from folly::io::Cursor or folly::IOBuf
 *
 * Unavailable metadata uses sentinel values (consistent with
 * SimpleJSONProtocolReader):
 *   - field id: std::numeric_limits<std::int16_t>::min()
 *   - field type: protocol::T_VOID
 *   - container size: std::numeric_limits<std::uint32_t>::max()
 */
class Json5ProtocolReader final {
 public:
  static constexpr bool kUsesFieldNames() { return true; }
  static constexpr bool kOmitsContainerSizes() { return true; }
  static constexpr bool kOmitsStringSizes() { return true; }
  static constexpr bool kOmitsContainerElemTypes() { return true; }
  static constexpr bool kHasDeferredRead() { return false; }
  static constexpr bool kCanReadStringView() { return false; }

  void setInput(const folly::io::Cursor& cursor);
  void setInput(const folly::IOBuf* buf);

  void readStructBegin(std::string& name);
  void readStructEnd();
  void readFieldBegin(
      std::string& name, protocol::TType& fieldType, std::int16_t& fieldId);
  void readFieldEnd();

  void readMapBegin(
      protocol::TType& keyType, protocol::TType& valType, std::uint32_t& size);
  void readMapEnd();
  void readListBegin(protocol::TType& elemType, std::uint32_t& size);
  void readListEnd();
  void readSetBegin(protocol::TType& elemType, std::uint32_t& size);
  void readSetEnd();

  void readBool(bool& value);
  void readBool(std::vector<bool>::reference value);
  void readByte(std::int8_t& value);
  void readI16(std::int16_t& value);
  void readI32(std::int32_t& value);
  void readI64(std::int64_t& value);
  void readFloat(float& value);
  void readDouble(double& value);

  template <typename StrType>
  void readString(StrType& str) {
    str = readStringValue();
  }
  template <typename StrType>
  void readBinary(StrType& str) {
    str = readBinaryValue();
  }
  void readBinary(std::unique_ptr<folly::IOBuf>& str);
  void readBinary(folly::IOBuf& str);

  // Enum support: parses "NAME (value)", "NAME", "(value)", or integer
  template <typename EnumType>
  void readEnum(EnumType& value);

  template <typename EnumType, typename Context>
  void readEnumWithContext(EnumType& value, Context& /*ctx*/) {
    readEnum(value);
  }

  bool peekMap();
  bool peekList();
  bool peekSet();

  void skip(protocol::TType type, int depth = 0);
  [[noreturn]] void skipBytes(size_t bytes);

  static constexpr std::size_t fixedSizeInContainer(protocol::TType) {
    return 0;
  }

  const folly::io::Cursor& getCursor() const;
  size_t getCursorPosition() const;

 private:
  // Invoked before reading any value. For maps with complex keys (KeyValueArray
  // form), reads `{"key":` when expecting a key, or `"value":` when expecting
  // a value.
  void beginReadValue();

  // Invoked after reading any value. For maps with complex keys, reads the
  // closing `}` after a value completes a key-value pair. Also toggles the map
  // state between expecting a key and expecting a value.
  void endReadValue();

  // Returns the object key name if we are inside an object-form map expecting
  // a key, otherwise returns nullopt. Toggles the map state when a key is read.
  std::optional<std::string> tryReadObjectMapKey();

  [[noreturn]] static void throwError(std::string_view message);

  std::int64_t readIntegralValue();

  template <std::floating_point T>
  T readFloatingPointValue();

  std::string readStringValue();
  std::string readBinaryValue();

  struct EnumReadResult {
    std::string name;
    std::optional<std::int32_t> value;
  };

  // Parse enum from JSON input, returning parsed name and optional integer
  // value. The caller is responsible for resolving the name to an enum value.
  EnumReadResult readEnumImpl();

  // Parse an enum string in "NAME (value)", "(value)", or "NAME" format.
  static EnumReadResult parseEnumString(std::string s);

  CompoundTypeTracker containerStack_;
  Json5Reader reader_;
};

template <typename EnumType>
void Json5ProtocolReader::readEnum(EnumType& value) {
  auto [name, intValue] = readEnumImpl();

  if (name.empty()) {
    value = folly::to<EnumType>(intValue.value());
    return;
  }

  if (!TEnumTraits<EnumType>::findValue(name, &value)) {
    throwError(
        fmt::format(
            "unknown enum name '{}' for type {}",
            name,
            TEnumTraits<EnumType>::typeName()));
  }

  if (intValue.has_value() && intValue != folly::to<std::int32_t>(value)) {
    throwError(
        fmt::format(
            "enum '{} ({})' does not match IDL value: {}",
            name,
            intValue.value(),
            folly::to<std::int32_t>(value)));
  }
}

} // namespace apache::thrift::json5::detail
