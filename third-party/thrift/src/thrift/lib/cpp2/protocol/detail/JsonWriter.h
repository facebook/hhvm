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
 * JsonWriter is a low-level helper that handles formatting concerns such as
 * indentation, trailing commas, and string escaping, so that callers can focus
 * on constructing the data without worrying about syntax details.
 *
 * Options only affect formatting, never the data itself. For example,
 * JsonWriter would never convert a NaN float to a "NaN" string — callers must
 * handle such conversions if desired.
 *
 * Incorrect call sequences (e.g., writing object name in list) are rejected to
 * guarantee the output is always valid JSON/JSON5.
 *
 * The API mirrors `CompactProtocolWriter`. Callers must call `setOutput`
 * before writing any data. Example usage:
 *
 *   folly::IOBufQueue queue;
 *   folly::io::QueueAppender appender(&queue, 1 << 14);
 *   JsonWriter w;
 *   w.setOutput(std::move(appender));
 *   w.writeObjectBegin();
 *   w.writeObjectName("name");
 *   w.writeString("Alice");
 *   w.writeObjectName("age");
 *   w.writeI32(30);
 *   w.writeObjectEnd();
 *   assert(queue.move()->toString() == "{\"name\":\"Alice\",\"age\":30}");
 *
 * ## JSON5 Feature Support
 *
 * JSON5 feature                    | Supported | Notes
 * ---------------------------------+-----------+------------------------------
 * Unquoted object names            | Yes       | Via unquoteObjectName option
 * Trailing commas (objects)        | Yes       | Via objectTrailingComma option
 * Trailing commas (arrays)         | Yes       | Via listTrailingComma option
 * Infinity / NaN literals          | Yes       | Via allowNanInf option
 * Single-quoted strings            | No        |
 * Multi-line strings               | No        |
 * Hexadecimal integers             | No        |
 * Leading/trailing decimal point   | No        |
 * Explicit positive sign (+)       | No        |
 * Additional escapes (\x, \v, \0)  | No        |
 * Single-line comments (//)        | N/A       |
 * Multi-line comments              | N/A       |
 */

#pragma once

#include <cstdint>
#include <string_view>
#include <folly/io/Cursor.h>
#include <folly/small_vector.h>

namespace apache::thrift::json5::detail {

struct JsonWriterOptions {
  // Append a trailing comma after the last element in a list.
  bool listTrailingComma = false;
  // Append a trailing comma after the last entry in an object.
  bool objectTrailingComma = false;
  // Omit quotes around object names that are valid ECMAScript 5.1 identifiers.
  bool unquoteObjectName = false;
  // Allow Infinity and NaN as bare literals; rejected when false.
  bool allowNanInf = false;
  // Number of spaces per indentation level. indentWidth=0 produces the compact
  // output with no extra whitespace or newlines.
  std::size_t indentWidth = 0;
};

inline constexpr JsonWriterOptions kJson5Options = {
    .listTrailingComma = true,
    .objectTrailingComma = true,
    .unquoteObjectName = true,
    .allowNanInf = true,
};

class JsonWriter {
 public:
  explicit JsonWriter(JsonWriterOptions options = {}) : options_(options) {}

  void setOutput(folly::io::QueueAppender&& output) {
    out_ = std::move(output);
  }

  std::uint32_t writeListBegin() { return openContainer(ContainerType::List); }
  std::uint32_t writeListEnd() { return closeContainer(ContainerType::List); }
  std::uint32_t writeObjectBegin() {
    return openContainer(ContainerType::Object);
  }
  std::uint32_t writeObjectEnd() {
    return closeContainer(ContainerType::Object);
  }

  std::uint32_t writeFloat(float f) { return writeFloatingPoint(f); }
  std::uint32_t writeDouble(double d) { return writeFloatingPoint(d); }
  std::uint32_t writeBool(bool b) { return writeIntegral(b); }
  std::uint32_t writeByte(std::int8_t i) { return writeIntegral(i); }
  std::uint32_t writeI16(std::int16_t i) { return writeIntegral(i); }
  std::uint32_t writeI32(std::int32_t i) { return writeIntegral(i); }
  std::uint32_t writeI64(std::int64_t i) { return writeIntegral(i); }

  std::uint32_t writeObjectName(std::string_view s);
  std::uint32_t writeString(std::string_view s);

 private:
  enum class ContainerType : uint8_t { List, Object };

  std::uint32_t openContainer(ContainerType type);
  std::uint32_t closeContainer(ContainerType type);

  std::uint32_t writeFloatingPoint(std::floating_point auto t);
  std::uint32_t writeIntegral(std::integral auto t);

  bool inContainer(ContainerType type) const;
  std::uint32_t beginValue();
  std::uint32_t appendComma();
  char back();
  bool trailingComma(ContainerType) const;
  static char getOpenBracket(ContainerType);
  static char getCloseBracket(ContainerType);
  std::uint32_t appendNewlineAndIndent();

  JsonWriterOptions options_;
  std::optional<folly::io::QueueAppender> out_;
  folly::small_vector<ContainerType, 4> containerStack_;

  // Only used for error detection, e.g., rejecting consecutive object name
  // writes.
  bool lastWrittenValueWasObjectName_ = false;
};

} // namespace apache::thrift::json5::detail
