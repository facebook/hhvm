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

#include <array>
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

using apache::thrift::transcode::cursorCanRead;
using apache::thrift::transcode::setCursorError;

// ─────────────────────────────────────────────────────────────────────────
// JSON intrinsics — complex operations only
// ─────────────────────────────────────────────────────────────────────────
// Simple JSON operations (whitespace skip, structural chars, field-name
// matching) are generated inline by KernelCodegen. The JIT also inlines a
// permissive bool-keyword check; the strict validating reader below is used by
// the interpreter for untrusted wire→wire JSON.

namespace {

constexpr int64_t kJsonError = 1;

void setJsonError(TranscodeCursor* cursor) {
  setCursorError(cursor, kJsonError);
}

void skipWhitespace(TranscodeCursor* cursor) {
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
bool parseHex4(const uint8_t* p, const uint8_t* end, char16_t& out) {
  if (end - p < 4) {
    return false;
  }
  uint16_t value = 0;
  for (int i = 0; i < 4; ++i) {
    uint8_t c = p[i];
    uint8_t digit;
    if (c >= '0' && c <= '9') {
      digit = c - '0';
    } else if (c >= 'a' && c <= 'f') {
      digit = c - 'a' + 10;
    } else if (c >= 'A' && c <= 'F') {
      digit = c - 'A' + 10;
    } else {
      return false;
    }
    value = static_cast<uint16_t>((value << 4) | digit);
  }
  out = static_cast<char16_t>(value);
  return true;
}

bool consumeJsonLiteral(TranscodeCursor* cursor, std::string_view literal) {
  if (!cursorCanRead(cursor, literal.size())) {
    return false;
  }
  if (memcmp(cursor->readPos, literal.data(), literal.size()) != 0) {
    setJsonError(cursor);
    return false;
  }
  cursor->readPos += literal.size();
  return true;
}

bool isJsonValueTerminator(TranscodeCursor* cursor) {
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
      setJsonError(cursor);
      return false;
  }
}

bool skipJsonStringToken(TranscodeCursor* cursor) {
  if (cursor->readPos >= cursor->readEnd || *cursor->readPos != '"') {
    setJsonError(cursor);
    return false;
  }
  ++cursor->readPos;

  while (cursor->readPos < cursor->readEnd) {
    uint8_t c = *cursor->readPos++;
    if (c == '"') {
      return true;
    }
    if (c < 0x20) {
      setJsonError(cursor);
      return false;
    }
    if (c != '\\') {
      continue;
    }
    if (cursor->readPos >= cursor->readEnd) {
      setJsonError(cursor);
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
        if (!parseHex4(cursor->readPos, cursor->readEnd, hi)) {
          setJsonError(cursor);
          return false;
        }
        cursor->readPos += 4;
        if (folly::utf16_code_unit_is_high_surrogate(hi)) {
          char16_t lo = 0;
          if (cursor->readEnd - cursor->readPos < 6 ||
              cursor->readPos[0] != '\\' || cursor->readPos[1] != 'u' ||
              !parseHex4(cursor->readPos + 2, cursor->readEnd, lo) ||
              !folly::utf16_code_unit_is_low_surrogate(lo)) {
            setJsonError(cursor);
            return false;
          }
          cursor->readPos += 6;
        } else if (folly::utf16_code_unit_is_low_surrogate(hi)) {
          setJsonError(cursor);
          return false;
        }
        break;
      }
      default:
        setJsonError(cursor);
        return false;
    }
  }

  setJsonError(cursor);
  return false;
}

bool parseJsonNumberToken(TranscodeCursor* cursor, bool allowFraction) {
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

} // namespace

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
  skipWhitespace(cursor);

  const uint8_t* tokenStart = cursor->readPos;
  if (!parseJsonNumberToken(cursor, false)) {
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
  skipWhitespace(cursor);

  if (cursor->readPos < cursor->readEnd && *cursor->readPos == '"') {
    size_t len = 0;
    const uint8_t* s = thrift_transcode_parse_escaped_string(cursor, &len);
    if (cursor->error != 0) {
      return 0.0;
    }
    std::string_view token(reinterpret_cast<const char*>(s), len);
    if (token == "NaN") {
      return std::numeric_limits<double>::quiet_NaN();
    }
    if (token == "Infinity") {
      return std::numeric_limits<double>::infinity();
    }
    if (token == "-Infinity") {
      return -std::numeric_limits<double>::infinity();
    }
    cursor->error = 1;
    return 0.0;
  }

  const uint8_t* start = cursor->readPos;
  if (!parseJsonNumberToken(cursor, true)) {
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
  skipWhitespace(cursor);
  if (cursor->readPos < cursor->readEnd) {
    if (*cursor->readPos == 't') {
      if (cursor->readEnd - cursor->readPos >= 4 &&
          memcmp(cursor->readPos, "true", 4) == 0) {
        cursor->readPos += 4;
        return 1;
      }
    } else if (*cursor->readPos == 'f') {
      if (cursor->readEnd - cursor->readPos >= 5 &&
          memcmp(cursor->readPos, "false", 5) == 0) {
        cursor->readPos += 5;
        return 0;
      }
    }
  }
  // TODO(D4a): use canonical TranscodeErrc codes here once the intrinsic codes
  // are renumbered.
  cursor->error = 1;
  return 0;
}

const uint8_t* FOLLY_NULLABLE
thrift_transcode_parse_escaped_string(TranscodeCursor* cursor, size_t* len) {
  skipWhitespace(cursor);
  if (cursor->readPos >= cursor->readEnd || *cursor->readPos != '"') {
    cursor->error = 1;
    *len = 0;
    return nullptr;
  }
  ++cursor->readPos;

  const uint8_t* start = cursor->readPos;
  bool hasEscapes = false;
  while (cursor->readPos < cursor->readEnd) {
    uint8_t c = *cursor->readPos;
    if (c == '"') {
      break;
    }
    if (c < 0x20) {
      cursor->error = 1;
      *len = 0;
      return nullptr;
    }
    if (c == '\\') {
      hasEscapes = true;
      ++cursor->readPos;
      if (cursor->readPos >= cursor->readEnd) {
        cursor->error = 1;
        *len = 0;
        return nullptr;
      }
    }
    ++cursor->readPos;
  }

  if (cursor->readPos >= cursor->readEnd) {
    cursor->error = 1;
    *len = 0;
    return nullptr;
  }

  if (!hasEscapes) {
    *len = cursor->readPos - start;
    ++cursor->readPos;
    return start;
  }

  // Slow path: unescape into scratch space.
  // TODO(D7/D8): this aliases the cursor's write buffer as scratch; the
  // hardening pass should give the unescaper a bounded dedicated scratch region
  // instead of borrowing writePos.
  cursor->readPos = start;
  size_t maxLen = cursor->readEnd - cursor->readPos;
  thrift_transcode_cursor_ensure_write(cursor, maxLen);
  uint8_t* out = cursor->writePos;
  const uint8_t* outStart = out;

  while (cursor->readPos < cursor->readEnd) {
    uint8_t c = *cursor->readPos;
    if (c == '"') {
      break;
    }
    if (c == '\\') {
      ++cursor->readPos;
      if (cursor->readPos >= cursor->readEnd) {
        cursor->error = 1;
        *len = 0;
        return nullptr;
      }
      c = *cursor->readPos;
      switch (c) {
        case '"':
        case '\\':
        case '/':
          *out++ = c;
          break;
        case 'n':
          *out++ = '\n';
          break;
        case 't':
          *out++ = '\t';
          break;
        case 'r':
          *out++ = '\r';
          break;
        case 'b':
          *out++ = '\b';
          break;
        case 'f':
          *out++ = '\f';
          break;
        case 'u': {
          char16_t hi = 0;
          if (!parseHex4(cursor->readPos + 1, cursor->readEnd, hi)) {
            // TODO(D4a): use canonical TranscodeErrc codes here once the
            // intrinsic codes are renumbered.
            cursor->error = 1;
            break;
          }
          cursor->readPos += 4;
          char32_t cp = 0;
          if (folly::utf16_code_unit_is_high_surrogate(hi)) {
            char16_t lo = 0;
            const uint8_t* pair = cursor->readPos + 1;
            if (pair + 1 >= cursor->readEnd || pair[0] != '\\' ||
                pair[1] != 'u' || !parseHex4(pair + 2, cursor->readEnd, lo) ||
                !folly::utf16_code_unit_is_low_surrogate(lo)) {
              // TODO(D4a): use canonical TranscodeErrc codes here once the
              // intrinsic codes are renumbered.
              cursor->error = 1;
              break;
            }
            cp = folly::unicode_code_point_from_utf16_surrogate_pair(hi, lo);
            cursor->readPos += 6;
          } else if (folly::utf16_code_unit_is_low_surrogate(hi)) {
            // TODO(D4a): use canonical TranscodeErrc codes here once the
            // intrinsic codes are renumbered.
            cursor->error = 1;
            break;
          } else {
            cp = static_cast<char32_t>(hi);
          }
          folly::unicode_code_point_utf8 utf8 =
              folly::unicode_code_point_to_utf8(cp);
          memcpy(out, utf8.data, utf8.size);
          out += utf8.size;
          break;
        }
        default:
          cursor->error = 1;
          break;
      }
      if (cursor->error != 0) {
        *len = 0;
        return nullptr;
      }
    } else {
      if (c < 0x20) {
        cursor->error = 1;
        *len = 0;
        return nullptr;
      }
      *out++ = c;
    }
    ++cursor->readPos;
  }

  if (cursor->readPos >= cursor->readEnd || *cursor->readPos != '"') {
    cursor->error = 1;
    *len = 0;
    return nullptr;
  }

  *len = out - outStart;
  ++cursor->readPos;
  return outStart;
}

const uint8_t* FOLLY_NULLABLE
thrift_transcode_parse_base64_string(TranscodeCursor* cursor, size_t* len) {
  size_t encodedLen = 0;
  const uint8_t* encoded =
      thrift_transcode_parse_escaped_string(cursor, &encodedLen);
  if (cursor->error != 0) {
    *len = 0;
    return nullptr;
  }

  std::string encodedScratch(
      reinterpret_cast<const char*>(encoded), encodedLen);
  switch (encodedScratch.size() % 4) {
    case 0:
      break;
    case 2:
      encodedScratch += "==";
      break;
    case 3:
      encodedScratch += "=";
      break;
    default:
      cursor->error = 1;
      *len = 0;
      return nullptr;
  }
  size_t decodedCap = folly::base64DecodedSize(encodedScratch);
  thrift_transcode_cursor_ensure_write(cursor, decodedCap);
  if (cursor->error != 0) {
    *len = 0;
    return nullptr;
  }

  char* out = reinterpret_cast<char*>(cursor->writePos);
  auto decoded = folly::base64DecodeRuntime(
      std::string_view(encodedScratch.data(), encodedScratch.size()), out);
  if (!decoded.is_success) {
    cursor->error = 1;
    *len = 0;
    return nullptr;
  }
  *len = static_cast<size_t>(decoded.o - out);
  return reinterpret_cast<const uint8_t*>(out);
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
    setJsonError(cursor);
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
    setJsonError(cursor);
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

void thrift_transcode_skip_json_value(TranscodeCursor* cursor) {
  skipWhitespace(cursor);
  if (cursor->readPos >= cursor->readEnd) {
    setJsonError(cursor);
    return;
  }

  uint8_t c = *cursor->readPos;

  if (c == '"') {
    if (!skipJsonStringToken(cursor)) {
      return;
    }
    isJsonValueTerminator(cursor);
  } else if (c == '{') {
    ++cursor->readPos;
    skipWhitespace(cursor);
    if (cursor->readPos < cursor->readEnd && *cursor->readPos == '}') {
      ++cursor->readPos;
      isJsonValueTerminator(cursor);
      return;
    }
    while (cursor->readPos < cursor->readEnd) {
      if (!skipJsonStringToken(cursor)) {
        return;
      }
      skipWhitespace(cursor);
      if (cursor->readPos >= cursor->readEnd || *cursor->readPos != ':') {
        setJsonError(cursor);
        return;
      }
      ++cursor->readPos;
      thrift_transcode_skip_json_value(cursor);
      if (cursor->error) {
        return;
      }
      skipWhitespace(cursor);
      if (cursor->readPos < cursor->readEnd && *cursor->readPos == ',') {
        ++cursor->readPos;
        skipWhitespace(cursor);
      } else {
        break;
      }
    }
    skipWhitespace(cursor);
    if (cursor->readPos < cursor->readEnd && *cursor->readPos == '}') {
      ++cursor->readPos;
      isJsonValueTerminator(cursor);
    } else {
      setJsonError(cursor);
    }
  } else if (c == '[') {
    ++cursor->readPos;
    skipWhitespace(cursor);
    if (cursor->readPos < cursor->readEnd && *cursor->readPos == ']') {
      ++cursor->readPos;
      isJsonValueTerminator(cursor);
      return;
    }
    while (cursor->readPos < cursor->readEnd) {
      thrift_transcode_skip_json_value(cursor);
      if (cursor->error) {
        return;
      }
      skipWhitespace(cursor);
      if (cursor->readPos < cursor->readEnd && *cursor->readPos == ',') {
        ++cursor->readPos;
        skipWhitespace(cursor);
      } else {
        break;
      }
    }
    skipWhitespace(cursor);
    if (cursor->readPos < cursor->readEnd && *cursor->readPos == ']') {
      ++cursor->readPos;
      isJsonValueTerminator(cursor);
    } else {
      setJsonError(cursor);
    }
  } else if (c == 't') {
    if (consumeJsonLiteral(cursor, "true")) {
      isJsonValueTerminator(cursor);
    }
  } else if (c == 'f') {
    if (consumeJsonLiteral(cursor, "false")) {
      isJsonValueTerminator(cursor);
    }
  } else if (c == 'n') {
    if (consumeJsonLiteral(cursor, "null")) {
      isJsonValueTerminator(cursor);
    }
  } else if (c == '-' || (c >= '0' && c <= '9')) {
    if (!parseJsonNumberToken(cursor, true)) {
      setJsonError(cursor);
      return;
    }
    isJsonValueTerminator(cursor);
  } else {
    setJsonError(cursor);
  }
}

} // extern "C"
