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

#include <thrift/lib/cpp2/protocol/JSONProtocolCommon.h>

#include <type_traits>

#include <fmt/core.h>

#include <folly/String.h>
#include <folly/json.h>

namespace {

class WrappedIOBufQueueAppender {
 public:
  explicit WrappedIOBufQueueAppender(folly::io::QueueAppender& out)
      : out_(out) {}

  void append(const char* s, const size_t n) {
    if (n == 0) {
      return;
    }
    out_.push(reinterpret_cast<const uint8_t*>(CHECK_NOTNULL(s)), n);
    length_ += n;
  }

  void push_back(const char c) { append(&c, 1); }

  WrappedIOBufQueueAppender& operator+=(const char c) {
    push_back(c);
    return *this;
  }

  size_t size() const { return length_; }

 private:
  folly::io::QueueAppender& out_;
  size_t length_ = 0;
};

} // namespace

namespace folly {

template <>
struct IsSomeString<WrappedIOBufQueueAppender> : std::true_type {};

} // namespace folly

namespace apache::thrift {

// This table describes the handling for the first 0x30 characters
//  0 : escape using "\u00xx" notation
//  1 : just output index
// <other> : escape using "\<other>" notation
const uint8_t JSONProtocolWriterCommon::kJSONCharTable[0x30] = {
    //  0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
    0, 0, 0,   0, 0, 0, 0, 0, 'b', 't', 'n', 0, 'f', 'r', 0, 0, // 0
    0, 0, 0,   0, 0, 0, 0, 0, 0,   0,   0,   0, 0,   0,   0, 0, // 1
    1, 1, '"', 1, 1, 1, 1, 1, 1,   1,   1,   1, 1,   1,   1, 1, // 2
};

// The elements of this array must match up with the sequence of characters in
// kEscapeChars
const uint8_t JSONProtocolReaderCommon::kEscapeCharVals[8] = {
    '"',
    '\\',
    '/',
    '\b',
    '\f',
    '\n',
    '\r',
    '\t',
};

uint32_t JSONProtocolWriterCommon::writeJSONDoubleInternal(double dbl) {
  WrappedIOBufQueueAppender appender(out_);
  if (isMapKey()) {
    folly::toAppend('"', dbl, '"', &appender);
  } else {
    folly::toAppend(dbl, &appender);
  }
  return appender.size();
}

uint32_t JSONProtocolWriterCommon::writeJSONDoubleInternal(float flt) {
  WrappedIOBufQueueAppender appender(out_);
  if (isMapKey()) {
    folly::toAppend('"', &appender);
  }
  folly::toAppend(
      flt,
      &appender,
      folly::DtoaMode::SHORTEST_SINGLE,
      0 /* numDigits is unused in shortest */);
  if (isMapKey()) {
    folly::toAppend('"', &appender);
  }
  return appender.size();
}

uint32_t JSONProtocolWriterCommon::writeJSONIntInternal(int64_t num) {
  WrappedIOBufQueueAppender appender(out_);
  if (isMapKey()) {
    folly::toAppend('"', num, '"', &appender);
  } else {
    folly::toAppend(num, &appender);
  }
  return appender.size();
}

uint32_t JSONProtocolWriterCommon::writeJSONBoolInternal(bool val) {
  const auto& out = val ? apache::thrift::detail::json::kJSONTrue
                        : apache::thrift::detail::json::kJSONFalse;
  WrappedIOBufQueueAppender appender(out_);
  if (isMapKey()) {
    folly::toAppend('"', out, '"', &appender);
  } else {
    folly::toAppend(out, &appender);
  }
  return appender.size();
}

char16_t JSONProtocolReaderCommon::readJSONEscapeCodeUnit16Suffix() {
  char16_t c = 0;
  for (size_t i = 0; i < 4; ++i) {
    c = c | (hexVal(in_.read<uint8_t>()) << (12 - (i * 4)));
  }
  return c;
}

JSONProtocolReaderCommon::DecodedEscapeSequence
JSONProtocolReaderCommon::readJSONEscapeCodePoint16Suffix() {
  char32_t cp = 0;
  char16_t c0 = readJSONEscapeCodeUnit16Suffix();
  if (folly::utf16_code_unit_is_bmp(c0)) {
    cp = c0;
  } else if (folly::utf16_code_unit_is_high_surrogate(c0)) {
    ensureCharNoWhitespace(detail::json::kJSONBackslash);
    ensureCharNoWhitespace(detail::json::kJSONEscapeChar);
    char16_t c1 = readJSONEscapeCodeUnit16Suffix();
    if (folly::utf16_code_unit_is_low_surrogate(c1)) {
      cp = folly::unicode_code_point_from_utf16_surrogate_pair(c0, c1);
    } else {
      throwInvalidTrailingSurrogate(c1);
    }
  } else {
    throwInvalidUtf16CodeUnit(c0);
  }
  return folly::unicode_code_point_to_utf8(cp);
}

JSONProtocolReaderCommon::DecodedEscapeSequence
JSONProtocolReaderCommon::readJSONEscapeSequence() {
  auto ch = in_.read<uint8_t>();
  if (ch == apache::thrift::detail::json::kJSONEscapeChar) {
    if (allowDecodeUTF8_) {
      return readJSONEscapeCodePoint16Suffix();
    } else {
      readJSONEscapeChar(ch);
      return {1, {ch}};
    }
  } else {
    size_t pos = kEscapeChars().find_first_of(ch);
    if (pos == std::string::npos) {
      throwInvalidEscapeChar(ch);
    }
    ch = kEscapeCharVals[pos];
    return {1, {ch}};
  }
}

static folly::StringPiece sp(const char& ch) {
  return {&ch, 1};
}

static std::string escape(folly::StringPiece str) {
  return folly::cEscape<std::string>(str);
}

static std::string quote(folly::StringPiece str) {
  return fmt::format("\"{}\"", escape(str));
}

std::string JSONProtocolReaderCommon::readJSONStringViaDynamic(
    std::string const& json) {
  return folly::parseJson(json).getString();
}

[[noreturn]] void JSONProtocolReaderCommon::throwBadVersion() {
  throw TProtocolException(
      TProtocolException::BAD_VERSION, "Message contained bad version.");
}

[[noreturn]] void JSONProtocolReaderCommon::throwUnrecognizableAsBoolean(
    const std::string& s) {
  throw TProtocolException(
      TProtocolException::INVALID_DATA, quote(s) + " is not a valid bool");
}

[[noreturn]] void JSONProtocolReaderCommon::throwUnrecognizableAsIntegral(
    folly::StringPiece s, folly::StringPiece typeName) {
  throw TProtocolException(
      TProtocolException::INVALID_DATA,
      folly::to<std::string>(quote(s), " is not a valid ", typeName));
}

[[noreturn]] void JSONProtocolReaderCommon::throwUnrecognizableAsFloatingPoint(
    const std::string& s) {
  throw TProtocolException(
      TProtocolException::INVALID_DATA,
      quote(s) + " is not a valid float/double");
}

[[noreturn]] void JSONProtocolReaderCommon::throwUnrecognizableAsAny(
    const std::string& s) {
  throw TProtocolException(
      TProtocolException::INVALID_DATA, quote(s) + " is not valid JSON");
}

[[noreturn]] void JSONProtocolReaderCommon::throwInvalidFieldStart(
    const char ch) {
  throw TProtocolException(
      TProtocolException::INVALID_DATA,
      quote(sp(ch)) + " is not a valid start to a JSON field");
}

[[noreturn]] void JSONProtocolReaderCommon::throwUnexpectedChar(
    const char ch, const char expected) {
  const auto msg = fmt::format(
      "expected '{0}' (hex {0:#02x}), read '{1}' (hex {1:#02x})", expected, ch);
  throw TProtocolException(TProtocolException::INVALID_DATA, msg);
}

[[noreturn]] void JSONProtocolReaderCommon::throwInvalidEscapeChar(
    const char ch) {
  throw TProtocolException(
      TProtocolException::INVALID_DATA,
      folly::to<std::string>(
          "Expected control char, got '", escape(sp(ch)), "'."));
}

[[noreturn]] void JSONProtocolReaderCommon::throwInvalidHexChar(const char ch) {
  throw TProtocolException(
      TProtocolException::INVALID_DATA,
      folly::to<std::string>(
          "Expected hex val ([0-9a-f]); got \'", escape(sp(ch)), "\'."));
}

[[noreturn]] void JSONProtocolReaderCommon::throwInvalidTrailingSurrogate(
    char16_t const c) {
  throw TProtocolException(
      TProtocolException::INVALID_DATA,
      fmt::format(
          "expected utf16 trailing surrogate, got '{:04x}'.", uint16_t(c)));
}

[[noreturn]] void JSONProtocolReaderCommon::throwInvalidUtf16CodeUnit(
    char16_t const c) {
  throw TProtocolException(
      TProtocolException::INVALID_DATA,
      fmt::format("expected utf16 code unit, got '{:04x}'.", uint16_t(c)));
}

} // namespace apache::thrift
