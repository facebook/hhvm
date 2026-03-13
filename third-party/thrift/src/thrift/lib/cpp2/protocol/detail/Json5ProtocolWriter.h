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
 * @file Json5ProtocolWriter.h
 * @brief JSON/JSON5 serialization for Thrift values.
 *
 * Json5ProtocolWriter provides a Thrift protocol writer interface that outputs
 * valid JSON5 or "basic" JSON output:
 * - "JSON5" refers to the "JSON5 Data Interchange Format (version 1.0.0/March
 *   2018)" as defined in https://spec.json5.org/. It is a subset of ECMAScript
 *   5.1 (https://262.ecma-international.org/5.1/).
 * - "Basic JSON" refers to the "JavaScript Object Notation (JSON) Data
 *   Interchange Format" as defined in RFC7159
 *   (https://datatracker.ietf.org/doc/html/rfc7159)
 *
 * The API of this class is modeled after CompactProtocolWriter.
 *
 * ## Current status
 *
 * This is EXPERIMENTAL. The output is unstable and there are missing features.
 * APIs may break with no prior notice. USE AT YOUR OWN RISK (preferably NOT IN
 * PRODUCTION).
 *
 * Here is the current status as of Feb 2026:
 *
 * - All thrift values: supported.
 * - UTF8 string validation: supported via JsonWriter.
 * - Binary field encoding: JSON object with "utf-8" or "base64url" key.
 *   The value is either utf-8 encoded or base64url encoded (RFC 4648 §5) string
 * - AnyStruct: outputted as raw thrift struct.
 * - Deterministic output: Not supported.
 *
 * ## Enum encoding
 *
 * Enums are encoded as "enum-name (enum-value)"
 *
 * ## Special Floating-Point Handling
 *
 * `toBasicJson` serializes special floating-point values as quoted
 * strings for compatibility with standard JSON parsers:
 * - NaN → "NaN"
 * - Infinity → "Infinity"
 * - -Infinity → "-Infinity"
 *
 * `toJson5` serializes them as bare literals, since JSON5 natively
 * supports NaN and Infinity:
 * - NaN → NaN
 * - Infinity → Infinity
 * - -Infinity → -Infinity
 *
 * In both cases:
 * - -0.0 is preserved as a numeric value.
 * - Floating-point values use the shortest decimal representation that
 *   roundtrips correctly.
 *
 * ## Map Key Encoding
 *
 * Maps are serialized based on their key type:
 *
 * ### String and Enum Keys
 * Serialized as JSON objects with keys converted to strings. Example:
 *   map<string, i32> {"a": 1, "b": 2}
 *   → {"a": 1, "b": 2}
 *
 *   map<MyEnum, i32> {MyEnum.FOO: 1, MyEnum.BAR: 2}
 *   → {"FOO (1)": 1, "BAR (2)": 2}
 *
 * ### All Other Keys (scalars, binaries, structs, lists, sets, maps)
 * Serialized as arrays of {key, value} objects. Example:
 *   map<i32, string> {1: "a", 2: "b"}
 *   → [{"key": 1, "value": "a"}, {"key": 2, "value": "b"}]
 *
 *   map<list<i32>, i32> {[1, 2]: 3, [4, 5]: 6}
 *   → [{"key": [1, 2], "value": 3}, {"key": [4, 5], "value": 6}]
 */

#pragma once

#include <variant>

#include <glog/logging.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp2/op/Encode.h>
#include <thrift/lib/cpp2/protocol/detail/CompoundTypeTracker.h>
#include <thrift/lib/cpp2/protocol/detail/JsonWriter.h>

namespace apache::thrift::json5::detail {

/**
 * Json5ProtocolWriter provides a Thrift protocol writer interface that outputs
 * JSON or JSON5 format. The implementation is based on JsonWriter - it forwards
 * function calls to JsonWriter, which writes directly to a QueueAppender.
 *
 * Key features:
 * - API similar to CompactProtocolWriter
 * - Always sorts object keys
 * - Takes JsonWriterOptions in constructor
 * - Outputs to folly::io::QueueAppender
 */
class Json5ProtocolWriter final {
 public:
  explicit Json5ProtocolWriter(JsonWriterOptions options = {})
      : options_(options), writer_(options) {}

  static constexpr KeyOrder keyOrder() { return KeyOrder::NativeAscending; }
  FieldOrder fieldOrder() const { return FieldOrder::IdAscending; }

  static constexpr size_t kDefaultGrowth = 1 << 14; // 16KB

  void setOutput(folly::IOBufQueue* queue, size_t growth = kDefaultGrowth);
  void setOutput(folly::io::QueueAppender&& output);

  std::uint32_t writeStructBegin(const char* name);
  std::uint32_t writeStructEnd();

  std::uint32_t writeFieldBegin(const char* name, protocol::TType, int16_t);
  std::uint32_t writeFieldEnd();
  std::uint32_t writeFieldStop();

  std::uint32_t writeListBegin(protocol::TType, uint32_t);
  std::uint32_t writeListEnd();
  std::uint32_t writeSetBegin(protocol::TType, uint32_t);
  std::uint32_t writeSetEnd();

  template <typename KeyTag, typename ValueTag>
  std::uint32_t writeMapBegin(uint32_t /* size */) {
    return writeMapBegin(
        std::is_same_v<KeyTag, type::string_t> ||
        type::is_a_v<KeyTag, type::enum_c>);
  }

  std::uint32_t writeMapEnd();
  std::uint32_t writeMapValueBegin() { return 0; }
  std::uint32_t writeMapValueEnd() { return 0; }

  std::uint32_t writeBool(bool value);
  std::uint32_t writeByte(int8_t value);
  std::uint32_t writeI16(int16_t value);
  std::uint32_t writeI32(int32_t value);
  std::uint32_t writeEnum(std::string_view name, int32_t value);
  std::uint32_t writeI64(int64_t value);
  std::uint32_t writeFloat(float value);
  std::uint32_t writeDouble(double value);

  std::uint32_t writeString(folly::StringPiece str);
  std::uint32_t writeBinary(folly::StringPiece str);
  std::uint32_t writeBinary(folly::ByteRange str);
  std::uint32_t writeBinary(const std::unique_ptr<folly::IOBuf>& buf);
  std::uint32_t writeBinary(const folly::IOBuf& buf);

 private:
  std::uint32_t writeMapBegin(bool objectForm);

  std::optional<std::string_view> encodeNanInfAsString(
      std::floating_point auto f) const;

  // Invoked before writing any value. For maps with complex keys (KeyValueArray
  // form), writes the opening `{"key":` when expecting a key, or `"value":`
  // when expecting a value.
  std::uint32_t beginWriteValue();

  // Invoked after writing any value. For maps with complex keys, writes the
  // closing `}` after a value completes a key-value pair. Also toggles the map
  // state between expecting a key and expecting a value.
  std::uint32_t endWriteValue();

  template <class Tag, class T>
  std::optional<std::uint32_t> maybeWriteSimpleMapKey(const T& value);

  CompoundTypeTracker containerStack_;
  JsonWriterOptions options_;
  JsonWriter writer_;
};

template <class Tag>
[[nodiscard]] std::string toJsonImpl(
    const type::native_type<Tag>& value, const JsonWriterOptions& options) {
  folly::IOBufQueue queue;
  Json5ProtocolWriter writer(options);
  writer.setOutput(&queue);
  [[maybe_unused]] auto size = op::encode<Tag>(writer, value);
  auto output = queue.moveAsValue().toString();
  DCHECK_EQ(size, output.size());
  return output;
}

} // namespace apache::thrift::json5::detail
