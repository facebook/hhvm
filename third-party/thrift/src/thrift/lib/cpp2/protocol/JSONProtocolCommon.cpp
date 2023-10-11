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
#include <folly/portability/GFlags.h>

FOLLY_GFLAGS_DEFINE_bool(
    thrift_cpp2_simple_json_base64_allow_padding,
    true,
    "Allow '=' padding when decoding base64 encoded binary fields in "
    "SimpleJsonProtocol");

namespace {

class WrappedIOBufQueueAppender {
 public:
  explicit WrappedIOBufQueueAppender(folly::io::QueueAppender& out)
      : out_(out) {}

  void append(const char* s, const size_t n) {
    if (n == 0)
      return;
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

namespace apache {
namespace thrift {

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
      double_conversion::DoubleToStringConverter::SHORTEST_SINGLE,
      0);
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

static inline folly::StringPiece sp(const char& ch) {
  return {&ch, 1};
}

[[noreturn]] void JSONProtocolReaderCommon::throwBadVersion() {
  throw TProtocolException(
      TProtocolException::BAD_VERSION, "Message contained bad version.");
}

[[noreturn]] void JSONProtocolReaderCommon::throwUnrecognizableAsBoolean(
    const std::string& s) {
  throw TProtocolException(
      TProtocolException::INVALID_DATA, s + " is not a valid bool");
}

[[noreturn]] void JSONProtocolReaderCommon::throwUnrecognizableAsIntegral(
    folly::StringPiece s, folly::StringPiece typeName) {
  throw TProtocolException(
      TProtocolException::INVALID_DATA,
      folly::to<std::string>(s, " is not a valid ", typeName));
}

[[noreturn]] void JSONProtocolReaderCommon::throwUnrecognizableAsFloatingPoint(
    const std::string& s) {
  throw TProtocolException(
      TProtocolException::INVALID_DATA, s + " is not a valid float/double");
}

[[noreturn]] void JSONProtocolReaderCommon::throwUnrecognizableAsString(
    const std::string& s, const std::exception& e) {
  throw TProtocolException(
      TProtocolException::INVALID_DATA,
      s + " is not a valid JSON string: " + e.what());
}

[[noreturn]] void JSONProtocolReaderCommon::throwUnrecognizableAsAny(
    const std::string& s) {
  throw TProtocolException(
      TProtocolException::INVALID_DATA, s + " is not valid JSON");
}

[[noreturn]] void JSONProtocolReaderCommon::throwInvalidFieldStart(
    const char ch) {
  throw TProtocolException(
      TProtocolException::INVALID_DATA,
      std::string(1, ch) + " is not a valid start to a JSON field");
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
      folly::to<std::string>("Expected control char, got '", sp(ch), "'."));
}

[[noreturn]] void JSONProtocolReaderCommon::throwInvalidHexChar(const char ch) {
  throw TProtocolException(
      TProtocolException::INVALID_DATA,
      folly::to<std::string>(
          "Expected hex val ([0-9a-f]); got \'", sp(ch), "\'."));
}
} // namespace thrift
} // namespace apache
