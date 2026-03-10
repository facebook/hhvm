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
#include <cstdint>
#include <string>
#include <variant>

#include <folly/io/Cursor.h>

namespace apache::thrift::json5::detail {

/**
 * @brief Low-level, JSON5 pull parser for streaming JSON5 deserialization.
 *
 * Json5Reader provides a cursor-based, pull-style API for parsing JSON5 input.
 * It reads from a folly::io::Cursor and offers methods to navigate through
 * JSON5 structures (objects, arrays) and extract primitive values.
 *
 * @section example Example Usage
 * @code
   auto buf = folly::IOBuf::copyBuffer(R"({"name": "Alice", "age": 30})");
 *   Json5Reader r;
 *   r.setCursor(folly::io::Cursor(buf.get()));
 *   r.readObjectBegin();
 *   auto key1 = r.readObjectName();   // "name"
 *   auto val1 = r.readPrimitive(...); // "Alice"
 *   auto key2 = r.readObjectName();   // "age"
 *   auto val2 = r.readPrimitive(...); // int64_t(30)
 *   r.readObjectEnd();
 * @endcode
 *
 * @section invariant Cursor Invariant (skip-after)
 *
 * After `setCursor()` or any public `read*` method returns, the cursor is
 * positioned at the next meaningful character — all whitespace and separating
 * commas have already been consumed. This means callers (and `peekToken()`)
 * can inspect the cursor immediately without skipping whitespace first.
 *
 * @section errors Error Handling
 *
 * Json5Reader uses a throw-on-error strategy. All `read*` methods throw
 * exceptions when they encounter unexpected JSON tokens or malformed input.
 * Examples of conditions that trigger exceptions:
 *
 * - Unexpected token type (e.g., expecting `{` but finding `[`)
 * - Missing or mismatched delimiters (e.g., unclosed brackets)
 * - Invalid syntax between elements (e.g., missing comma separator)
 * - Malformed primitives (e.g., invalid number format, unterminated strings)
 *
 * Callers should wrap parsing operations in try-catch blocks to handle
 * malformed JSON5 input gracefully.
 *
 * @section json5 JSON5 Feature Support
 *
 * | JSON5 feature                    | Supported |
 * |---------------------------------:|:---------:|
 * | Unquoted object names            | Yes       |
 * | Trailing commas (objects)        | Yes       |
 * | Trailing commas (arrays)         | Yes       |
 * | Single-quoted strings            | Yes       |
 * | Multi-line strings (escaped LF)  | Yes       |
 * | Infinity / NaN literals          | Yes       |
 * | Comments (//, block)             | Yes       |
 * | Hexadecimal integers             | No        |
 * | Leading/trailing decimal point   | Yes       |
 * | Explicit positive sign (+)       | Yes       |
 */
class Json5Reader final {
 public:
  /** Sets the input cursor and skips leading whitespace. */
  void setCursor(folly::io::Cursor cursor);

  /** Returns the current cursor position. */
  [[nodiscard]] const folly::io::Cursor& getCursor() const;

  /**
   * Peeks at the next token without consuming it.
   *
   * @throws std::runtime_error if the input has ended unexpectedly.
   */
  enum class Token { ListBegin, ListEnd, ObjectBegin, ObjectEnd, Primitive };
  [[nodiscard]] Token peekToken();

  /**
   * Variant type representing JSON primitive (non-compound) values.
   *
   * Maps JSON types to C++ types as follows:
   *   - JSON null    → std::monostate
   *   - JSON boolean → bool
   *   - JSON integer → std::int64_t
   *   - JSON float   → float (when FloatingPointPrecision::Single is requested)
   *   - JSON float   → double (when FloatingPointPrecision::Double is
   * requested)
   *   - JSON string  → std::string
   */
  using Primitive = std::
      variant<std::monostate, bool, std::int64_t, float, double, std::string>;

  /**
   * Reads and returns the next primitive value.
   *
   * @param precision Determines whether floating-point numbers are parsed as
   *                  float (Single) or double (Double). This parameter is only
   *                  used when the next value is a floating-point number.
   */
  enum class FloatingPointPrecision { Single, Double };
  Primitive readPrimitive(FloatingPointPrecision precision);

  /** Reads and returns the next object key name. */
  std::string readObjectName();

  /** Consumes the opening bracket of an array. */
  void readListBegin() { consume('['); }

  /** Consumes the closing bracket of an array. */
  void readListEnd() { consume(']'), expectCommaOrEnd(); }

  /** Consumes the opening brace of an object. */
  void readObjectBegin() { consume('{'); }

  /** Consumes the closing brace of an object. */
  void readObjectEnd() { consume('}'), expectCommaOrEnd(); }

 private:
  folly::io::Cursor& cursor();

  bool skipComment();
  void skipWhitespaceAndComments();
  char peekChar();
  char readChar();
  void consume(char c);

  std::string parseString(char quote);
  Primitive parseNumber(FloatingPointPrecision);

  // Consume a comma separator between elements. Throws if neither a comma
  // nor a closing delimiter follows the current position.
  void expectCommaOrEnd();

  std::optional<folly::io::Cursor> in_;
};

} // namespace apache::thrift::json5::detail
