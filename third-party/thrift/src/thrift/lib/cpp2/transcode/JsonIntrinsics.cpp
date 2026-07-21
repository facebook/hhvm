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

#include <thrift/lib/cpp2/transcode/JsonIntrinsics.h>

#include <thrift/lib/cpp2/transcode/IntrinsicsCommon.h>

#include <array>
#include <charconv>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <string>
#include <string_view>

#include <folly/Conv.h>
#include <folly/Range.h>
#include <folly/Unicode.h>
#include <folly/base64.h>
#include <folly/codec/hex.h>

using apache::thrift::transcode::cursorCanRead;
using apache::thrift::transcode::setCursorError;

// These helpers back the C ABI intrinsics below. Generated kernels still inline
// the trivial structural JSON operations.

namespace {

constexpr int64_t kJsonError = 1;
constexpr size_t kMaxJsonSkipDepth = 64;

void set_json_error(TranscodeCursor* cursor) {
  setCursorError(cursor, kJsonError);
}

void skip_whitespace(TranscodeCursor* cursor) {
  while (cursor->readPos < cursor->readEnd) {
    uint8_t c = *cursor->readPos;
    if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
      break;
    }
    ++cursor->readPos;
  }
}

// Decode exactly four hex digits at `p` into a 16-bit code unit. Returns false
// (leaving `out` untouched) if fewer than four bytes remain or any is non-hex.
bool parse_hex4(const uint8_t* p, const uint8_t* end, char16_t& out) {
  if (end - p < 4) {
    return false;
  }
  uint16_t value = 0;
  for (int i = 0; i < 4; ++i) {
    uint8_t digit = folly::hex_decode_digit(static_cast<char>(p[i]));
    if (digit > 0x0f) {
      return false;
    }

    value = static_cast<uint16_t>((value << 4) | digit);
  }
  out = static_cast<char16_t>(value);
  return true;
}

bool consume_json_literal(TranscodeCursor* cursor, std::string_view literal) {
  if (!cursorCanRead(cursor, literal.size())) {
    return false;
  }
  if (memcmp(cursor->readPos, literal.data(), literal.size()) != 0) {
    set_json_error(cursor);
    return false;
  }
  cursor->readPos += literal.size();
  return true;
}

bool is_json_value_terminator(TranscodeCursor* cursor) {
  if (cursor->readPos >= cursor->readEnd) {
    return true;
  }
  switch (*cursor->readPos) {
    case ' ':
    case '\t':
    case '\n':
    case '\r':
    case ',':
    case ']':
    case '}':
      return true;
    default:
      set_json_error(cursor);
      return false;
  }
}

bool read_json_string_token(
    TranscodeCursor* cursor, TranscodeJsonStringToken* token) {
  skip_whitespace(cursor);
  *token = {nullptr, nullptr, 0};
  if (cursor->readPos >= cursor->readEnd || *cursor->readPos != '"') {
    set_json_error(cursor);
    return false;
  }
  ++cursor->readPos;

  const uint8_t* begin = cursor->readPos;
  bool hasEscapes = false;
  while (cursor->readPos < cursor->readEnd) {
    uint8_t c = *cursor->readPos++;
    if (c == '"') {
      token->begin = begin;
      token->end = cursor->readPos - 1;
      token->hasEscapes = hasEscapes ? 1 : 0;
      return true;
    }
    if (c < 0x20) {
      set_json_error(cursor);
      return false;
    }
    if (c != '\\') {
      continue;
    }
    hasEscapes = true;
    if (cursor->readPos >= cursor->readEnd) {
      set_json_error(cursor);
      return false;
    }
    uint8_t escaped = *cursor->readPos++;
    switch (escaped) {
      case '"':
      case '\\':
      case '/':
      case 'b':
      case 'f':
      case 'n':
      case 'r':
      case 't':
        break;
      case 'u': {
        char16_t hi = 0;
        if (!parse_hex4(cursor->readPos, cursor->readEnd, hi)) {
          set_json_error(cursor);
          return false;
        }
        cursor->readPos += 4;
        if (folly::utf16_code_unit_is_high_surrogate(hi)) {
          char16_t lo = 0;
          if (cursor->readEnd - cursor->readPos < 6 ||
              cursor->readPos[0] != '\\' || cursor->readPos[1] != 'u' ||
              !parse_hex4(cursor->readPos + 2, cursor->readEnd, lo) ||
              !folly::utf16_code_unit_is_low_surrogate(lo)) {
            set_json_error(cursor);
            return false;
          }
          cursor->readPos += 6;
        } else if (folly::utf16_code_unit_is_low_surrogate(hi)) {
          set_json_error(cursor);
          return false;
        }
        break;
      }
      default:
        set_json_error(cursor);
        return false;
    }
  }

  set_json_error(cursor);
  return false;
}

bool skip_json_string_token(TranscodeCursor* cursor) {
  TranscodeJsonStringToken token{};
  return read_json_string_token(cursor, &token);
}

bool parse_json_number_token(TranscodeCursor* cursor, bool allowFraction) {
  const uint8_t* p = cursor->readPos;
  const uint8_t* end = cursor->readEnd;
  if (p < end && *p == '-') {
    ++p;
  }
  if (p >= end) {
    return false;
  }

  if (*p == '0') {
    ++p;
    if (p < end && *p >= '0' && *p <= '9') {
      return false;
    }
  } else if (*p >= '1' && *p <= '9') {
    do {
      ++p;
    } while (p < end && *p >= '0' && *p <= '9');
  } else {
    return false;
  }

  if (!allowFraction) {
    if (p < end && (*p == '.' || *p == 'e' || *p == 'E')) {
      return false;
    }
    cursor->readPos = p;
    return true;
  }

  if (p < end && *p == '.') {
    ++p;
    const uint8_t* fracStart = p;
    while (p < end && *p >= '0' && *p <= '9') {
      ++p;
    }
    if (p == fracStart) {
      return false;
    }
  }

  if (p < end && (*p == 'e' || *p == 'E')) {
    ++p;
    if (p < end && (*p == '+' || *p == '-')) {
      ++p;
    }
    const uint8_t* expStart = p;
    while (p < end && *p >= '0' && *p <= '9') {
      ++p;
    }
    if (p == expStart) {
      return false;
    }
  }

  cursor->readPos = p;
  return true;
}

bool append_bytes(
    const uint8_t* data,
    size_t len,
    const uint8_t* expected,
    size_t expectedLen,
    size_t& matched) {
  if (matched > expectedLen || len > expectedLen - matched ||
      memcmp(expected + matched, data, len) != 0) {
    return false;
  }
  matched += len;
  return true;
}

uint8_t escaped_byte(uint8_t escaped) {
  switch (escaped) {
    case '"':
    case '\\':
    case '/':
      return escaped;
    case 'n':
      return '\n';
    case 't':
      return '\t';
    case 'r':
      return '\r';
    case 'b':
      return '\b';
    case 'f':
      return '\f';
    default:
      return 0;
  }
}

bool json_string_token_decoded_size(
    const TranscodeJsonStringToken* token, size_t* len) {
  if (token->hasEscapes == 0) {
    *len = static_cast<size_t>(token->end - token->begin);
    return true;
  }

  size_t out = 0;
  const uint8_t* p = token->begin;
  while (p < token->end) {
    uint8_t c = *p++;
    if (c != '\\') {
      ++out;
      continue;
    }
    if (p >= token->end) {
      return false;
    }
    uint8_t escaped = *p++;
    if (escaped != 'u') {
      if (escaped_byte(escaped) == 0) {
        return false;
      }
      ++out;
      continue;
    }

    char16_t hi = 0;
    if (!parse_hex4(p, token->end, hi)) {
      return false;
    }
    p += 4;
    char32_t cp = 0;
    if (folly::utf16_code_unit_is_high_surrogate(hi)) {
      char16_t lo = 0;
      if (token->end - p < 6 || p[0] != '\\' || p[1] != 'u' ||
          !parse_hex4(p + 2, token->end, lo) ||
          !folly::utf16_code_unit_is_low_surrogate(lo)) {
        return false;
      }
      cp = folly::unicode_code_point_from_utf16_surrogate_pair(hi, lo);
      p += 6;
    } else if (folly::utf16_code_unit_is_low_surrogate(hi)) {
      return false;
    } else {
      cp = static_cast<char32_t>(hi);
    }
    out += folly::unicode_code_point_to_utf8(cp).size;
  }
  *len = out;
  return true;
}

bool write_decoded_json_string_token(
    TranscodeCursor* cursor,
    const TranscodeJsonStringToken* token,
    size_t* len) {
  *len = 0;
  const size_t maxLen = static_cast<size_t>(token->end - token->begin);
  thrift_transcode_cursor_ensure_write(cursor, maxLen);
  if (cursor->error != 0) {
    return false;
  }

  if (token->hasEscapes == 0) {
    memcpy(cursor->writePos, token->begin, maxLen);
    cursor->writePos += maxLen;
    *len = maxLen;
    return true;
  }

  uint8_t* out = cursor->writePos;
  const uint8_t* p = token->begin;
  while (p < token->end) {
    uint8_t c = *p++;
    if (c != '\\') {
      *out++ = c;
      continue;
    }

    if (p >= token->end) {
      return false;
    }
    uint8_t escaped = *p++;
    if (escaped != 'u') {
      uint8_t decoded = escaped_byte(escaped);
      if (decoded == 0) {
        return false;
      }
      *out++ = decoded;
      continue;
    }

    char16_t hi = 0;
    if (!parse_hex4(p, token->end, hi)) {
      return false;
    }
    p += 4;
    char32_t cp = 0;
    if (folly::utf16_code_unit_is_high_surrogate(hi)) {
      char16_t lo = 0;
      if (token->end - p < 6 || p[0] != '\\' || p[1] != 'u' ||
          !parse_hex4(p + 2, token->end, lo) ||
          !folly::utf16_code_unit_is_low_surrogate(lo)) {
        return false;
      }
      cp = folly::unicode_code_point_from_utf16_surrogate_pair(hi, lo);
      p += 6;
    } else if (folly::utf16_code_unit_is_low_surrogate(hi)) {
      return false;
    } else {
      cp = static_cast<char32_t>(hi);
    }

    folly::unicode_code_point_utf8 utf8 = folly::unicode_code_point_to_utf8(cp);
    memcpy(out, utf8.data, utf8.size);
    out += utf8.size;
  }

  *len = static_cast<size_t>(out - cursor->writePos);
  cursor->writePos = out;
  return true;
}

bool append_decoded_json_string_token(
    const TranscodeJsonStringToken* token, std::string& output) {
  if (token->hasEscapes == 0) {
    output.append(
        reinterpret_cast<const char*>(token->begin),
        static_cast<size_t>(token->end - token->begin));
    return true;
  }

  const uint8_t* p = token->begin;
  while (p < token->end) {
    uint8_t c = *p++;
    if (c != '\\') {
      output.push_back(static_cast<char>(c));
      continue;
    }

    if (p >= token->end) {
      return false;
    }
    uint8_t escaped = *p++;
    if (escaped != 'u') {
      uint8_t decoded = escaped_byte(escaped);
      if (decoded == 0) {
        return false;
      }
      output.push_back(static_cast<char>(decoded));
      continue;
    }

    char16_t hi = 0;
    if (!parse_hex4(p, token->end, hi)) {
      return false;
    }
    p += 4;
    char32_t cp = 0;
    if (folly::utf16_code_unit_is_high_surrogate(hi)) {
      char16_t lo = 0;
      if (token->end - p < 6 || p[0] != '\\' || p[1] != 'u' ||
          !parse_hex4(p + 2, token->end, lo) ||
          !folly::utf16_code_unit_is_low_surrogate(lo)) {
        return false;
      }
      cp = folly::unicode_code_point_from_utf16_surrogate_pair(hi, lo);
      p += 6;
    } else if (folly::utf16_code_unit_is_low_surrogate(hi)) {
      return false;
    } else {
      cp = static_cast<char32_t>(hi);
    }

    folly::unicode_code_point_utf8 utf8 = folly::unicode_code_point_to_utf8(cp);
    output.append(reinterpret_cast<const char*>(utf8.data), utf8.size);
  }
  return true;
}

bool json_string_token_equals_decoded_bytes(
    const TranscodeJsonStringToken* token, const uint8_t* data, size_t len) {
  if (token->hasEscapes == 0) {
    size_t tokenLen = static_cast<size_t>(token->end - token->begin);
    return tokenLen == len && memcmp(token->begin, data, len) == 0;
  }

  size_t matched = 0;
  const uint8_t* p = token->begin;
  while (p < token->end) {
    const uint8_t* plainStart = p;
    while (p < token->end && *p != '\\') {
      ++p;
    }
    if (p != plainStart &&
        !append_bytes(
            plainStart,
            static_cast<size_t>(p - plainStart),
            data,
            len,
            matched)) {
      return false;
    }
    if (p == token->end) {
      break;
    }

    ++p;
    if (p >= token->end) {
      return false;
    }
    uint8_t escaped = *p++;
    if (escaped != 'u') {
      uint8_t decoded = escaped_byte(escaped);
      if (decoded == 0 || !append_bytes(&decoded, 1, data, len, matched)) {
        return false;
      }
      continue;
    }

    char16_t hi = 0;
    if (!parse_hex4(p, token->end, hi)) {
      return false;
    }
    p += 4;
    char32_t cp = 0;
    if (folly::utf16_code_unit_is_high_surrogate(hi)) {
      char16_t lo = 0;
      if (token->end - p < 6 || p[0] != '\\' || p[1] != 'u' ||
          !parse_hex4(p + 2, token->end, lo) ||
          !folly::utf16_code_unit_is_low_surrogate(lo)) {
        return false;
      }
      cp = folly::unicode_code_point_from_utf16_surrogate_pair(hi, lo);
      p += 6;
    } else if (folly::utf16_code_unit_is_low_surrogate(hi)) {
      return false;
    } else {
      cp = static_cast<char32_t>(hi);
    }

    folly::unicode_code_point_utf8 utf8 = folly::unicode_code_point_to_utf8(cp);
    if (!append_bytes(utf8.data, utf8.size, data, len, matched)) {
      return false;
    }
  }
  return matched == len;
}

bool decode_json_string_token_into(
    const TranscodeJsonStringToken* token, uint8_t* out, size_t len) {
  size_t decodedLen = 0;
  if (!json_string_token_decoded_size(token, &decodedLen) ||
      decodedLen != len) {
    return false;
  }

  if (token->hasEscapes == 0) {
    memcpy(out, token->begin, len);
    return true;
  }

  const uint8_t* p = token->begin;
  uint8_t* dst = out;
  while (p < token->end) {
    uint8_t c = *p++;
    if (c != '\\') {
      *dst++ = c;
      continue;
    }

    if (p >= token->end) {
      return false;
    }
    uint8_t escaped = *p++;
    if (escaped != 'u') {
      uint8_t decoded = escaped_byte(escaped);
      if (decoded == 0) {
        return false;
      }
      *dst++ = decoded;
      continue;
    }

    char16_t hi = 0;
    if (!parse_hex4(p, token->end, hi)) {
      return false;
    }
    p += 4;
    char32_t cp = 0;
    if (folly::utf16_code_unit_is_high_surrogate(hi)) {
      char16_t lo = 0;
      if (token->end - p < 6 || p[0] != '\\' || p[1] != 'u' ||
          !parse_hex4(p + 2, token->end, lo) ||
          !folly::utf16_code_unit_is_low_surrogate(lo)) {
        return false;
      }
      cp = folly::unicode_code_point_from_utf16_surrogate_pair(hi, lo);
      p += 6;
    } else if (folly::utf16_code_unit_is_low_surrogate(hi)) {
      return false;
    } else {
      cp = static_cast<char32_t>(hi);
    }

    folly::unicode_code_point_utf8 utf8 = folly::unicode_code_point_to_utf8(cp);
    memcpy(dst, utf8.data, utf8.size);
    dst += utf8.size;
  }
  return static_cast<size_t>(dst - out) == len;
}

struct Base64Input {
  std::string scratch;
  std::string_view text;
};

bool prepare_json_base64_token_input(
    const TranscodeJsonStringToken* token, Base64Input& input) {
  input = {};

  if (token->hasEscapes == 0) {
    input.text = std::string_view(
        reinterpret_cast<const char*>(token->begin),
        static_cast<size_t>(token->end - token->begin));
  } else if (append_decoded_json_string_token(token, input.scratch)) {
    input.text = input.scratch;
  } else {
    return false;
  }

  switch (input.text.size() % 4) {
    case 0:
      return true;
    case 2:
      if (input.text.data() != input.scratch.data()) {
        input.scratch.assign(input.text);
      }
      input.scratch += "==";
      input.text = input.scratch;
      return true;
    case 3:
      if (input.text.data() != input.scratch.data()) {
        input.scratch.assign(input.text);
      }
      input.scratch += "=";
      input.text = input.scratch;
      return true;
    default:
      return false;
  }
}

void skip_json_value(TranscodeCursor* cursor, size_t depth) {
  skip_whitespace(cursor);
  if (cursor->readPos >= cursor->readEnd) {
    set_json_error(cursor);
    return;
  }

  uint8_t c = *cursor->readPos;

  if (c == '"') {
    if (!skip_json_string_token(cursor)) {
      return;
    }
    is_json_value_terminator(cursor);
  } else if (c == '{') {
    if (depth >= kMaxJsonSkipDepth) {
      set_json_error(cursor);
      return;
    }
    ++cursor->readPos;
    skip_whitespace(cursor);
    if (cursor->readPos < cursor->readEnd && *cursor->readPos == '}') {
      ++cursor->readPos;
      is_json_value_terminator(cursor);
      return;
    }
    while (cursor->readPos < cursor->readEnd) {
      if (!skip_json_string_token(cursor)) {
        return;
      }
      skip_whitespace(cursor);
      if (cursor->readPos >= cursor->readEnd || *cursor->readPos != ':') {
        set_json_error(cursor);
        return;
      }
      ++cursor->readPos;
      skip_json_value(cursor, depth + 1);
      if (cursor->error) {
        return;
      }
      skip_whitespace(cursor);
      if (cursor->readPos < cursor->readEnd && *cursor->readPos == ',') {
        ++cursor->readPos;
        skip_whitespace(cursor);
      } else {
        break;
      }
    }
    skip_whitespace(cursor);
    if (cursor->readPos < cursor->readEnd && *cursor->readPos == '}') {
      ++cursor->readPos;
      is_json_value_terminator(cursor);
    } else {
      set_json_error(cursor);
    }
  } else if (c == '[') {
    if (depth >= kMaxJsonSkipDepth) {
      set_json_error(cursor);
      return;
    }
    ++cursor->readPos;
    skip_whitespace(cursor);
    if (cursor->readPos < cursor->readEnd && *cursor->readPos == ']') {
      ++cursor->readPos;
      is_json_value_terminator(cursor);
      return;
    }
    while (cursor->readPos < cursor->readEnd) {
      skip_json_value(cursor, depth + 1);
      if (cursor->error) {
        return;
      }
      skip_whitespace(cursor);
      if (cursor->readPos < cursor->readEnd && *cursor->readPos == ',') {
        ++cursor->readPos;
        skip_whitespace(cursor);
      } else {
        break;
      }
    }
    skip_whitespace(cursor);
    if (cursor->readPos < cursor->readEnd && *cursor->readPos == ']') {
      ++cursor->readPos;
      is_json_value_terminator(cursor);
    } else {
      set_json_error(cursor);
    }
  } else if (c == 't') {
    if (consume_json_literal(cursor, "true")) {
      is_json_value_terminator(cursor);
    }
  } else if (c == 'f') {
    if (consume_json_literal(cursor, "false")) {
      is_json_value_terminator(cursor);
    }
  } else if (c == 'n') {
    if (consume_json_literal(cursor, "null")) {
      is_json_value_terminator(cursor);
    }
  } else if (c == '-' || (c >= '0' && c <= '9')) {
    if (!parse_json_number_token(cursor, true)) {
      set_json_error(cursor);
      return;
    }
    is_json_value_terminator(cursor);
  } else {
    set_json_error(cursor);
  }
}
} // namespace

namespace apache::thrift::transcode {

bool thrift_transcode_parse_strict_i64(std::string_view text, int64_t& value) {
  auto [ptr, ec] =
      std::from_chars(text.data(), text.data() + text.size(), value);
  return ec == std::errc{} && ptr == text.data() + text.size();
}

bool thrift_transcode_parse_json_object_enum_key(
    std::string_view text,
    const std::vector<std::pair<int32_t, std::string>>* enumNames,
    int64_t& value) {
  if (thrift_transcode_parse_strict_i64(text, value)) {
    return true;
  }

  if (enumNames != nullptr) {
    for (const auto& entry : *enumNames) {
      if (std::string_view(entry.second) == text) {
        value = entry.first;
        return true;
      }
    }
  }

  return false;
}

int64_t thrift_transcode_parse_json_enum_value(
    TranscodeCursor* cursor,
    const std::vector<std::pair<int32_t, std::string>>* enumNames) {
  skip_whitespace(cursor);
  if (cursor->readPos < cursor->readEnd && *cursor->readPos == '"') {
    TranscodeJsonStringToken token{};
    if (!thrift_transcode_read_json_string_token(cursor, &token)) {
      return 0;
    }
    if (enumNames != nullptr) {
      // TODO(json-enum-lookup): Build a name-to-value index if this shows up in
      // profiles.
      for (const auto& entry : *enumNames) {
        if (thrift_transcode_json_string_token_equals(
                &token,
                reinterpret_cast<const uint8_t*>(entry.second.data()),
                entry.second.size())) {
          return entry.first;
        }
      }
    }
    set_json_error(cursor);
    return 0;
  }
  return thrift_transcode_parse_decimal_int(cursor);
}

void thrift_transcode_json_skip_whitespace(TranscodeCursor* cursor) {
  skip_whitespace(cursor);
}

uint8_t thrift_transcode_json_peek(TranscodeCursor* cursor) {
  if (cursor->readPos >= cursor->readEnd) {
    return 0;
  }
  return *cursor->readPos;
}

bool thrift_transcode_json_consume_null(TranscodeCursor* cursor) {
  skip_whitespace(cursor);
  if (cursor->readEnd - cursor->readPos >= 4 &&
      std::memcmp(cursor->readPos, "null", 4) == 0) {
    const uint8_t* afterNull = cursor->readPos + 4;
    const uint8_t* savedReadPos = cursor->readPos;
    cursor->readPos = afterNull;
    if (!is_json_value_terminator(cursor)) {
      cursor->readPos = savedReadPos;
      return false;
    }
    return true;
  }
  return false;
}

bool thrift_transcode_json_expect_byte(
    TranscodeCursor* cursor, uint8_t expected) {
  if (cursor->readPos >= cursor->readEnd || *cursor->readPos != expected) {
    set_json_error(cursor);
    return false;
  }
  ++cursor->readPos;
  return true;
}

bool thrift_transcode_read_json_object_key(
    TranscodeCursor* cursor, std::string& key) {
  key.clear();
  if (cursor->error != 0) {
    return false;
  }
  TranscodeJsonStringToken token{};
  if (!thrift_transcode_read_json_string_token(cursor, &token)) {
    return false;
  }
  key = thrift_transcode_decode_json_string_token_to_string(cursor, token);
  if (cursor->error != 0) {
    key.clear();
    return false;
  }
  return true;
}

std::string thrift_transcode_decode_json_string_token_to_string(
    TranscodeCursor* cursor, const TranscodeJsonStringToken& token) {
  size_t len = 0;
  std::string out;
  if (thrift_transcode_json_string_token_decoded_size(&token, &len)) {
    out.resize(len);
    if (!thrift_transcode_decode_json_string_token(
            &token, reinterpret_cast<uint8_t*>(out.data()), out.size())) {
      set_json_error(cursor);
      out.clear();
    }
  } else {
    set_json_error(cursor);
  }
  return out;
}

} // namespace apache::thrift::transcode

extern "C" {

// TODO(json-primitive-unification / "B"): these read primitives reuse folly's
// canonical Unicode + number leaves, but the string-scan and escape framing is
// still local to this file. A follow-up should extract a single strict, tested
// C-ABI JSON primitive layer under thrift/lib/cpp2/protocol/detail/ that both
// Json5Reader and these intrinsics call, so there is exactly one
// implementation. (These are called out-of-line by the JIT, not
// bitcode-inlined, because the bitcode twin is compiled without the folly
// header tree.)

int64_t thrift_transcode_parse_decimal_int(TranscodeCursor* cursor) {
  skip_whitespace(cursor);

  const uint8_t* tokenStart = cursor->readPos;
  if (!parse_json_number_token(cursor, false)) {
    // TODO(D4a): use canonical TranscodeErrc codes here once the intrinsic
    // codes are renumbered.
    cursor->error = 1;
    return 0;
  }

  auto parsed = folly::tryTo<int64_t>(folly::StringPiece(
      reinterpret_cast<const char*>(tokenStart),
      static_cast<size_t>(cursor->readPos - tokenStart)));
  if (parsed.hasError()) {
    // TODO(D4a): use canonical TranscodeErrc codes here once the intrinsic
    // codes are renumbered.
    cursor->error = 1;
    return 0;
  }
  return *parsed;
}

double thrift_transcode_parse_decimal_float(TranscodeCursor* cursor) {
  skip_whitespace(cursor);

  if (cursor->readPos < cursor->readEnd && *cursor->readPos == '"') {
    TranscodeJsonStringToken token{};
    if (!read_json_string_token(cursor, &token)) {
      return 0.0;
    }
    if (json_string_token_equals_decoded_bytes(
            &token, reinterpret_cast<const uint8_t*>("NaN"), 3)) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    if (json_string_token_equals_decoded_bytes(
            &token, reinterpret_cast<const uint8_t*>("Infinity"), 8)) {
      return std::numeric_limits<double>::infinity();
    }
    if (json_string_token_equals_decoded_bytes(
            &token, reinterpret_cast<const uint8_t*>("-Infinity"), 9)) {
      return -std::numeric_limits<double>::infinity();
    }
    cursor->error = 1;
    return 0.0;
  }

  const uint8_t* start = cursor->readPos;
  if (!parse_json_number_token(cursor, true)) {
    // TODO(D4a): use canonical TranscodeErrc codes here once the intrinsic
    // codes are renumbered.
    cursor->error = 1;
    return 0.0;
  }

  auto parsed = folly::tryTo<double>(folly::StringPiece(
      reinterpret_cast<const char*>(start),
      static_cast<size_t>(cursor->readPos - start)));
  if (parsed.hasError()) {
    // TODO(D4a): use canonical TranscodeErrc codes here once the intrinsic
    // codes are renumbered.
    cursor->error = 1;
    return 0.0;
  }
  return *parsed;
}

int64_t thrift_transcode_parse_bool_keyword(TranscodeCursor* cursor) {
  skip_whitespace(cursor);
  if (cursor->readPos < cursor->readEnd) {
    if (*cursor->readPos == 't') {
      if (consume_json_literal(cursor, "true")) {
        if (is_json_value_terminator(cursor)) {
          return 1;
        }
        return 0;
      }
    } else if (*cursor->readPos == 'f') {
      if (consume_json_literal(cursor, "false")) {
        is_json_value_terminator(cursor);
        return 0;
      }
    }
  }
  // TODO(D4a): use canonical TranscodeErrc codes here once the intrinsic codes
  // are renumbered.
  cursor->error = 1;
  return 0;
}

uint8_t thrift_transcode_read_json_string_token(
    TranscodeCursor* cursor, TranscodeJsonStringToken* token) {
  return read_json_string_token(cursor, token) ? 1 : 0;
}

uint8_t thrift_transcode_json_string_token_equals(
    const TranscodeJsonStringToken* token, const uint8_t* data, size_t len) {
  return json_string_token_equals_decoded_bytes(token, data, len) ? 1 : 0;
}

uint8_t thrift_transcode_json_string_token_decoded_size(
    const TranscodeJsonStringToken* token, size_t* len) {
  return json_string_token_decoded_size(token, len) ? 1 : 0;
}

uint8_t thrift_transcode_decode_json_string_token(
    const TranscodeJsonStringToken* token, uint8_t* out, size_t len) {
  return decode_json_string_token_into(token, out, len) ? 1 : 0;
}

size_t thrift_transcode_write_json_string_token(
    TranscodeCursor* cursor, const TranscodeJsonStringToken* token) {
  size_t len = 0;
  if (!write_decoded_json_string_token(cursor, token, &len)) {
    set_json_error(cursor);
    return 0;
  }
  return len;
}

void thrift_transcode_write_json_string_token_i32_prefixed(
    TranscodeCursor* cursor, const TranscodeJsonStringToken* token) {
  size_t decodedLen = 0;
  if (!json_string_token_decoded_size(token, &decodedLen) ||
      decodedLen > static_cast<size_t>(std::numeric_limits<int32_t>::max())) {
    set_json_error(cursor);
    return;
  }
  thrift_transcode_write_fixed32_be_checked(
      cursor, static_cast<uint32_t>(decodedLen));
  if (cursor->error != 0) {
    return;
  }
  size_t written = thrift_transcode_write_json_string_token(cursor, token);
  if (cursor->error == 0 && written != decodedLen) {
    set_json_error(cursor);
  }
}

void thrift_transcode_write_json_string_token_varint_prefixed(
    TranscodeCursor* cursor, const TranscodeJsonStringToken* token) {
  size_t decodedLen = 0;
  if (!json_string_token_decoded_size(token, &decodedLen)) {
    set_json_error(cursor);
    return;
  }
  thrift_transcode_write_unsigned_varint(
      cursor, static_cast<uint64_t>(decodedLen));
  if (cursor->error != 0) {
    return;
  }
  size_t written = thrift_transcode_write_json_string_token(cursor, token);
  if (cursor->error == 0 && written != decodedLen) {
    set_json_error(cursor);
  }
}

void thrift_transcode_write_json_base64_token_i32_prefixed(
    TranscodeCursor* cursor, const TranscodeJsonStringToken* token) {
  Base64Input input;
  if (!prepare_json_base64_token_input(token, input)) {
    set_json_error(cursor);
    return;
  }
  size_t decodedLen = folly::base64DecodedSize(input.text);
  if (decodedLen > static_cast<size_t>(std::numeric_limits<int32_t>::max())) {
    set_json_error(cursor);
    return;
  }
  thrift_transcode_write_fixed32_be_checked(
      cursor, static_cast<uint32_t>(decodedLen));
  if (cursor->error != 0) {
    return;
  }
  thrift_transcode_cursor_ensure_write(cursor, decodedLen);
  if (cursor->error != 0) {
    return;
  }

  char* out = reinterpret_cast<char*>(cursor->writePos);
  auto decoded = folly::base64DecodeRuntime(input.text, out);
  if (!decoded.is_success) {
    set_json_error(cursor);
    return;
  }
  cursor->writePos = reinterpret_cast<uint8_t*>(decoded.o);
}

void thrift_transcode_write_json_base64_token_varint_prefixed(
    TranscodeCursor* cursor, const TranscodeJsonStringToken* token) {
  Base64Input input;
  if (!prepare_json_base64_token_input(token, input)) {
    set_json_error(cursor);
    return;
  }
  size_t decodedLen = folly::base64DecodedSize(input.text);
  thrift_transcode_write_unsigned_varint(
      cursor, static_cast<uint64_t>(decodedLen));
  if (cursor->error != 0) {
    return;
  }
  thrift_transcode_cursor_ensure_write(cursor, decodedLen);
  if (cursor->error != 0) {
    return;
  }

  char* out = reinterpret_cast<char*>(cursor->writePos);
  auto decoded = folly::base64DecodeRuntime(input.text, out);
  if (!decoded.is_success) {
    set_json_error(cursor);
    return;
  }
  cursor->writePos = reinterpret_cast<uint8_t*>(decoded.o);
}

void thrift_transcode_write_json_string_token_quoted(
    TranscodeCursor* cursor, const TranscodeJsonStringToken* token) {
  thrift_transcode_write_byte_checked(cursor, '"');
  thrift_transcode_write_raw_bytes_checked(
      cursor, token->begin, static_cast<size_t>(token->end - token->begin));
  thrift_transcode_write_byte_checked(cursor, '"');
}

void thrift_transcode_format_decimal_int(
    TranscodeCursor* cursor, int64_t value) {
  thrift_transcode_cursor_ensure_write(cursor, 21);
  if (cursor->error != 0) {
    return;
  }
  std::array<char, 21> buf{};
  int len =
      snprintf(buf.data(), buf.size(), "%lld", static_cast<long long>(value));
  if (len <= 0) {
    set_json_error(cursor);
    return;
  }
  memcpy(cursor->writePos, buf.data(), static_cast<size_t>(len));
  cursor->writePos += len;
}

void thrift_transcode_format_decimal_float(
    TranscodeCursor* cursor, double value) {
  thrift_transcode_cursor_ensure_write(cursor, 32);
  if (cursor->error != 0) {
    return;
  }
  if (std::isnan(value) || std::isinf(value)) {
    if (std::isnan(value)) {
      memcpy(cursor->writePos, "\"NaN\"", 5);
      cursor->writePos += 5;
    } else {
      const char* s = value > 0 ? "\"Infinity\"" : "\"-Infinity\"";
      size_t slen = strlen(s);
      memcpy(cursor->writePos, s, slen);
      cursor->writePos += slen;
    }
    return;
  }
  if (value == 0.0 && std::signbit(value)) {
    memcpy(cursor->writePos, "-0.0", 4);
    cursor->writePos += 4;
    return;
  }
  std::array<char, 32> buf{};
  int len = snprintf(buf.data(), buf.size(), "%.17g", value);
  if (len <= 0) {
    set_json_error(cursor);
    return;
  }
  memcpy(cursor->writePos, buf.data(), static_cast<size_t>(len));
  cursor->writePos += len;
}

void thrift_transcode_format_escaped_string(
    TranscodeCursor* cursor, const uint8_t* data, size_t len) {
  thrift_transcode_cursor_ensure_write(cursor, len * 6 + 2);
  if (cursor->error != 0) {
    return;
  }
  *cursor->writePos++ = '"';
  for (size_t i = 0; i < len; ++i) {
    uint8_t c = data[i];
    switch (c) {
      case '"':
        *cursor->writePos++ = '\\';
        *cursor->writePos++ = '"';
        break;
      case '\\':
        *cursor->writePos++ = '\\';
        *cursor->writePos++ = '\\';
        break;
      case '\n':
        *cursor->writePos++ = '\\';
        *cursor->writePos++ = 'n';
        break;
      case '\t':
        *cursor->writePos++ = '\\';
        *cursor->writePos++ = 't';
        break;
      case '\r':
        *cursor->writePos++ = '\\';
        *cursor->writePos++ = 'r';
        break;
      case '\b':
        *cursor->writePos++ = '\\';
        *cursor->writePos++ = 'b';
        break;
      case '\f':
        *cursor->writePos++ = '\\';
        *cursor->writePos++ = 'f';
        break;
      default:
        if (c < 0x20) {
          std::array<char, 7> hex{};
          snprintf(hex.data(), hex.size(), "\\u%04x", c);
          memcpy(cursor->writePos, hex.data(), 6);
          cursor->writePos += 6;
        } else {
          *cursor->writePos++ = c;
        }
        break;
    }
  }
  *cursor->writePos++ = '"';
}

void thrift_transcode_format_base64_string(
    TranscodeCursor* cursor, const uint8_t* data, size_t len) {
  size_t encodedLen = folly::base64EncodedSize(len);
  thrift_transcode_cursor_ensure_write(cursor, encodedLen + 2);
  if (cursor->error != 0) {
    return;
  }
  *cursor->writePos++ = '"';
  const char* begin = len == 0 ? "" : reinterpret_cast<const char*>(data);
  auto* out = reinterpret_cast<char*>(cursor->writePos);
  auto* end = folly::base64EncodeRuntime(begin, begin + len, out);
  cursor->writePos = reinterpret_cast<uint8_t*>(end);
  *cursor->writePos++ = '"';
}

uint8_t thrift_transcode_skip_json_value(TranscodeCursor* cursor) {
  skip_json_value(cursor, 0);
  return cursor->error == 0;
}

} // extern "C"
