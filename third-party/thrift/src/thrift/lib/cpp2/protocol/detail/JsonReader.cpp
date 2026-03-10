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

#include <thrift/lib/cpp2/protocol/detail/JsonReader.h>

#include <cctype>
#include <charconv>
#include <cstdint>
#include <limits>
#include <stdexcept>

#include <fmt/core.h>
#include <folly/Conv.h>
#include <folly/Unicode.h>

namespace apache::thrift::json5::detail {

namespace {

bool isIdentifierStart(char c) {
  return std::isalpha(c) || c == '_' || c == '$';
}

bool isIdentifierPart(char c) {
  return isIdentifierStart(c) || std::isdigit(c);
}

[[noreturn]] void throwParseError(const std::string& msg) {
  throw std::runtime_error("Json5Reader: " + msg);
}

std::int64_t parseHexInteger(std::string_view str, bool isNegative) {
  uint64_t uval = 0;
  auto [ptr, ec] =
      std::from_chars(str.data(), str.data() + str.size(), uval, 16);

  if (ec != std::errc{} || ptr != str.data() + str.size()) {
    throwParseError(fmt::format("invalid hex number {}", str));
  }

  constexpr auto kMinAsUnsigned =
      static_cast<uint64_t>(std::numeric_limits<int64_t>::max()) + 1;
  if (uval == kMinAsUnsigned && isNegative) {
    return std::numeric_limits<int64_t>::min();
  }
  if (uval <= std::numeric_limits<int64_t>::max()) {
    return isNegative ? -static_cast<int64_t>(uval)
                      : static_cast<int64_t>(uval);
  }

  throwParseError(fmt::format("hex number {} out of range", str));
}

} // namespace

// -- cursor operations ----------------------------------------------------

void Json5Reader::setCursor(folly::io::Cursor c) {
  in_ = std::move(c);
  skipWhitespaceAndComments();
}

const folly::io::Cursor& Json5Reader::getCursor() const {
  return in_.value();
}

folly::io::Cursor& Json5Reader::cursor() {
  return in_.value();
}

char Json5Reader::peekChar() {
  return cursor().isAtEnd() ? '\0' : static_cast<char>(cursor().peek().front());
}

char Json5Reader::readChar() {
  if (cursor().isAtEnd()) {
    throwParseError("unexpected end of input");
  }
  return static_cast<char>(cursor().read<char>());
}

void Json5Reader::consume(char expected) {
  char got = readChar();
  if (got != expected) {
    throwParseError(fmt::format("expected '{}', got '{}'", expected, got));
  }
  skipWhitespaceAndComments();
}

// -- whitespace and comments ----------------------------------------------

bool Json5Reader::skipComment() {
  if (peekChar() != '/') {
    return false;
  }
  cursor().skip(1);
  switch (readChar()) {
    case '/':
      // Skip "// ..." comment
      cursor().skipWhile([](char c) { return c != '\n' && c != '\r'; });
      if (!cursor().isAtEnd()) {
        cursor().skip(1); // Skip "\n"
      }
      return true;
    case '*': {
      // Skip "/* ... */" comment
      char prev = 0;
      cursor().skipWhile([&prev](char curr) {
        if (prev == '*' && curr == '/') {
          return false;
        }
        prev = curr;
        return true;
      });
      if (cursor().isAtEnd()) {
        throwParseError("unterminated block comment");
      } else {
        cursor().skip(1); // skip "/"
      }
      return true;
    }
    default:
      throwParseError("expected '//' or '/*' comment");
  }
}

void Json5Reader::skipWhitespaceAndComments() {
  do {
    cursor().skipWhile([](char c) { return std::isspace(c); });
    // Loop to handle adjacent comments, e.g. `/*a*//*b*/`.
  } while (skipComment());
}

// -- comma handling -------------------------------------------------------

void Json5Reader::expectCommaOrEnd() {
  skipWhitespaceAndComments();
  char c = peekChar();
  if (c == ',') {
    cursor().skip(1);
    skipWhitespaceAndComments();
    if (peekChar() == ',') {
      throwParseError("unexpected consecutive commas"); // e.g., "[1,,2]"
    }
  } else if (c != ']' && c != '}' && c != '\0') {
    throwParseError(
        fmt::format("expected ',' or closing delimiter, got '{}'", c));
  }
}

// -- token peeking --------------------------------------------------------

Json5Reader::Token Json5Reader::peekToken() {
  switch (peekChar()) {
    case '[':
      return Token::ListBegin;
    case ']':
      return Token::ListEnd;
    case '{':
      return Token::ObjectBegin;
    case '}':
      return Token::ObjectEnd;
    case '\0':
      throwParseError("unexpected end of input");
    default:
      return Token::Primitive;
  }
}

// -- strings and identifiers ----------------------------------------------

std::string Json5Reader::parseString(char quote) {
  std::string result;
  while (true) {
    char c = readChar();
    if (c == quote) {
      return result;
    }
    if (c == '\n' || c == '\r') {
      throwParseError("unescaped newline in string");
    }
    if (c != '\\') {
      result.push_back(c);
      continue;
    }
    char esc = readChar();
    switch (esc) {
      case '"':
      case '/':
      case '\'':
      case '\\':
      case '\n':
      case '\r':
        result.push_back(esc);
        if (esc == '\r' && peekChar() == '\n') {
          // handles "\r\n" (CRLF)
          result.push_back(readChar());
        }
        break;
      case 'b':
        result.push_back('\b');
        break;
      case 'f':
        result.push_back('\f');
        break;
      case 'n':
        result.push_back('\n');
        break;
      case 'r':
        result.push_back('\r');
        break;
      case 't':
        result.push_back('\t');
        break;
      case 'u': {
        auto cp = folly::to<std::int32_t>(
            parseHexInteger(cursor().readFixedString(4), false));
        folly::appendCodePointToUtf8(cp, result);
        break;
      }
      default:
        throwParseError(fmt::format("unknown escape '\\{}' in string", esc));
    }
  }
}

std::string Json5Reader::readObjectName() {
  std::string name;
  char c = peekChar();
  if (c == '"' || c == '\'') {
    cursor().skip(1);
    name = parseString(c);
  } else if (isIdentifierStart(c)) {
    name = cursor().readWhile(isIdentifierPart);
  } else {
    throwParseError(fmt::format("expected object name, got '{}'", c));
  }
  skipWhitespaceAndComments();
  consume(':');
  return name;
}

// -- numbers --------------------------------------------------------------

Json5Reader::Primitive Json5Reader::parseNumber(
    FloatingPointPrecision precision) {
  std::string numStr;
  char c = peekChar();

  const int8_t sign = (c == '-' ? -1 : 1);
  if (c == '+' || c == '-') {
    numStr.push_back(readChar());
    c = peekChar();
  }

  // Infinity or NaN
  if (c == 'I' || c == 'N') {
    std::string word = cursor().readWhile(isIdentifierPart);
    if (word == "Infinity") {
      double d = std::copysign(std::numeric_limits<double>::infinity(), sign);
      if (precision == FloatingPointPrecision::Single) {
        return float(d);
      }
      return d;
    }
    if (word == "NaN") {
      double d = std::copysign(std::numeric_limits<double>::quiet_NaN(), sign);
      if (precision == FloatingPointPrecision::Single) {
        return float(d);
      }
      return d;
    }
    throwParseError(
        fmt::format("expected 'Infinity' or 'NaN', got '{}'", word));
  }
  // Hex literal: 0x...
  if (c == '0') {
    numStr.push_back(readChar());
    c = peekChar();
    if (c == 'x' || c == 'X') {
      cursor().skip(1);
      return parseHexInteger(
          cursor().readWhile([](char c) { return std::isxdigit(c); }),
          sign < 0);
    }
  } else if (c != '.') {
    numStr += cursor().readWhile([](char c) { return std::isdigit(c); });
    c = peekChar();
  }

  bool isFloating = false;

  if (c == '.') {
    isFloating = true;
    numStr.push_back(readChar());
    numStr += cursor().readWhile([](char c) { return std::isdigit(c); });
    c = peekChar();
  }

  if (c == 'e' || c == 'E') {
    isFloating = true;
    numStr.push_back(readChar());
    if (peekChar() == '+' || peekChar() == '-') {
      numStr.push_back(readChar());
    }
    numStr += cursor().readWhile([](char c) { return std::isdigit(c); });
  }

  if (numStr.empty() || numStr == "+" || numStr == "-") {
    throwParseError("expected number");
  }

  if (!isFloating) {
    return folly::to<std::int64_t>(numStr);
  }
  if (precision == FloatingPointPrecision::Single) {
    return folly::to<float>(numStr);
  }
  return folly::to<double>(numStr);
}

// -- values ---------------------------------------------------------------

Json5Reader::Primitive Json5Reader::readPrimitive(
    FloatingPointPrecision precision) {
  char c = peekChar();
  Primitive result;

  if (c == '"' || c == '\'') {
    cursor().skip(1);
    result = parseString(c);
  } else if (
      std::isdigit(c) || c == '-' || c == '+' || c == '.' || c == 'N' ||
      c == 'I') {
    result = parseNumber(precision);
  } else if (isIdentifierStart(c)) {
    std::string word = cursor().readWhile(isIdentifierPart);
    if (word == "null") {
      result = std::monostate{};
    } else if (word == "true") {
      result = true;
    } else if (word == "false") {
      result = false;
    } else {
      throwParseError(fmt::format("unexpected identifier '{}'", word));
    }
  } else {
    throwParseError(fmt::format("expected value, got '{}'", c));
  }

  skipWhitespaceAndComments();
  expectCommaOrEnd();
  return result;
}

} // namespace apache::thrift::json5::detail
