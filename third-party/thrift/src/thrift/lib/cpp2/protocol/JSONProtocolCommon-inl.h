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

#include <folly/Utility.h>
#include <folly/algorithm/simd/find_first_of.h>
#include <folly/codec/hex.h>

namespace apache {
namespace thrift {

namespace detail::json {

inline constexpr auto json_ws_alphabet = std::array{
    char(kJSONSpace),
    char(kJSONNewline),
    char(kJSONTab),
    char(kJSONCarriageReturn),
};
using json_ws_scalar_needle_t = folly::conditional_t<
    folly::kIsMobile,
    folly::simd::default_scalar_finder_first_not_of,
    folly::simd::ltindex_scalar_finder_first_not_of>;
using json_ws_vector_needle_t = folly::conditional_t<
    folly::kIsMobile || !folly::kIsArchAmd64,
    folly::simd::default_vector_finder_first_not_of,
    folly::simd::shuffle_vector_finder_first_not_of>;
inline constexpr auto json_ws_vector_needle =
    json_ws_vector_needle_t{json_ws_alphabet};
inline constexpr auto json_ws_scalar_needle =
    json_ws_scalar_needle_t{json_ws_alphabet};

} // namespace detail::json

// Return the hex character representing the integer val. The value is masked
// to make sure it is in the correct range.
inline uint8_t JSONProtocolWriterCommon::hexChar(uint8_t val) {
  val &= 0x0F;
  if (val < 10) {
    return val + '0';
  } else {
    return val - 10 + 'a';
  }
}

/*
 * Public writing methods
 */

inline uint32_t JSONProtocolWriterCommon::writeMessageBegin(
    const std::string& name, MessageType messageType, int32_t seqid) {
  auto ret = beginContext(ContextType::ARRAY);
  ret += writeI32(apache::thrift::detail::json::kThriftVersion1);
  ret += writeString(name);
  ret += writeI32(static_cast<int32_t>(messageType));
  ret += writeI32(seqid);
  return ret;
}

inline uint32_t JSONProtocolWriterCommon::writeMessageEnd() {
  return endContext();
}

inline uint32_t JSONProtocolWriterCommon::writeByte(int8_t byte) {
  auto ret = writeContext();
  return ret + writeJSONInt(byte);
}

inline uint32_t JSONProtocolWriterCommon::writeI16(int16_t i16) {
  auto ret = writeContext();
  return ret + writeJSONInt(i16);
}

inline uint32_t JSONProtocolWriterCommon::writeI32(int32_t i32) {
  auto ret = writeContext();
  return ret + writeJSONInt(i32);
}

inline uint32_t JSONProtocolWriterCommon::writeI64(int64_t i64) {
  auto ret = writeContext();
  return ret + writeJSONInt(i64);
}

inline uint32_t JSONProtocolWriterCommon::writeDouble(double dub) {
  auto ret = writeContext();
  return ret + writeJSONDouble(dub);
}

inline uint32_t JSONProtocolWriterCommon::writeFloat(float flt) {
  auto ret = writeContext();
  return ret + writeJSONDouble(flt);
}

inline uint32_t JSONProtocolWriterCommon::writeString(folly::StringPiece str) {
  auto ret = writeContext();
  return ret + writeJSONString(str);
}

inline uint32_t JSONProtocolWriterCommon::writeBinary(folly::StringPiece str) {
  return writeBinary(folly::ByteRange(str));
}

inline uint32_t JSONProtocolWriterCommon::writeBinary(folly::ByteRange v) {
  auto ret = writeContext();
  return ret + writeJSONBase64(v);
}

inline uint32_t JSONProtocolWriterCommon::writeBinary(
    const std::unique_ptr<folly::IOBuf>& str) {
  DCHECK(str);
  if (!str) {
    auto ret = writeContext();
    return ret + writeJSONString(folly::StringPiece());
  }
  return writeBinary(*str);
}

inline uint32_t JSONProtocolWriterCommon::writeBinary(const folly::IOBuf& str) {
  auto ret = writeContext();
  return ret + writeJSONBase64(str.clone()->coalesce());
}

inline uint32_t JSONProtocolWriterCommon::serializedSizeByte(
    int8_t /*val*/) const {
  // 3 bytes for serialized, plus it might be a key, plus context
  return 6;
}

inline uint32_t JSONProtocolWriterCommon::serializedSizeI16(
    int16_t /*val*/) const {
  return 8;
}

inline uint32_t JSONProtocolWriterCommon::serializedSizeI32(
    int32_t /*val*/) const {
  return 13;
}

inline uint32_t JSONProtocolWriterCommon::serializedSizeI64(
    int64_t /*val*/) const {
  return 25;
}

inline uint32_t JSONProtocolWriterCommon::serializedSizeDouble(
    double /*val*/) const {
  return 25;
}

inline uint32_t JSONProtocolWriterCommon::serializedSizeFloat(
    float /*val*/) const {
  return 25;
}

inline uint32_t JSONProtocolWriterCommon::serializedSizeString(
    folly::StringPiece str) const {
  return static_cast<uint32_t>(str.size()) * 6 + 3;
}

inline uint32_t JSONProtocolWriterCommon::serializedSizeBinary(
    folly::StringPiece str) const {
  return serializedSizeBinary(folly::ByteRange(str));
}

inline uint32_t JSONProtocolWriterCommon::serializedSizeBinary(
    folly::ByteRange v) const {
  return static_cast<uint32_t>(v.size()) * 6 + 3;
}

inline uint32_t JSONProtocolWriterCommon::serializedSizeBinary(
    const std::unique_ptr<folly::IOBuf>& v) const {
  return (v ? serializedSizeBinary(*v) * 6 : 0) + 3;
}

inline uint32_t JSONProtocolWriterCommon::serializedSizeBinary(
    const folly::IOBuf& v) const {
  size_t size = v.computeChainDataLength();
  uint32_t limit = std::numeric_limits<uint32_t>::max() - serializedSizeI32();
  if (size > limit) {
    TProtocolException::throwExceededSizeLimit(size, limit);
  }
  return static_cast<uint32_t>(size) * 6 + 3;
}

inline uint32_t JSONProtocolWriterCommon::serializedSizeZCBinary(
    folly::StringPiece str) const {
  return serializedSizeZCBinary(folly::ByteRange(str));
}

inline uint32_t JSONProtocolWriterCommon::serializedSizeZCBinary(
    folly::ByteRange v) const {
  return serializedSizeBinary(v);
}

inline uint32_t JSONProtocolWriterCommon::serializedSizeZCBinary(
    const std::unique_ptr<folly::IOBuf>&) const {
  // size only
  return serializedSizeI32();
}

inline uint32_t JSONProtocolWriterCommon::serializedSizeZCBinary(
    const folly::IOBuf&) const {
  // size only
  return serializedSizeI32();
}

/**
 * Protected writing methods
 */

inline uint32_t JSONProtocolWriterCommon::beginContext(ContextType type) {
  context.push_back({type, 0});
  switch (type) {
    case ContextType::MAP:
      out_.write(apache::thrift::detail::json::kJSONObjectStart);
      return 1;
    case ContextType::ARRAY:
      out_.write(apache::thrift::detail::json::kJSONArrayStart);
      return 1;
    default:
      CHECK(false);
  }
  CHECK(false);
  return 0;
}

inline uint32_t JSONProtocolWriterCommon::endContext() {
  DCHECK(!context.empty());
  switch (context.back().type) {
    case ContextType::MAP:
      out_.write(apache::thrift::detail::json::kJSONObjectEnd);
      break;
    case ContextType::ARRAY:
      out_.write(apache::thrift::detail::json::kJSONArrayEnd);
      break;
    default:
      break;
  }
  context.pop_back();
  return 1;
}

inline uint32_t JSONProtocolWriterCommon::writeContext() {
  if (context.empty()) {
    return 0;
  }
  auto& ctx = context.back();
  auto meta = ctx.meta++;
  switch (ctx.type) {
    case ContextType::MAP:
      if (meta == 0) {
        return 0;
      } else if (meta % 2 == 0) {
        out_.write(apache::thrift::detail::json::kJSONElemSeparator);
      } else {
        out_.write(apache::thrift::detail::json::kJSONPairSeparator);
      }
      return 1;
    case ContextType::ARRAY:
      if (meta != 0) {
        out_.write(apache::thrift::detail::json::kJSONElemSeparator);
        return 1;
      }
      return 0;
    default:
      break;
  }
  CHECK(false);
  return 0;
}

inline void JSONProtocolWriterCommon::writeJSONEscapeChar(
    uint8_t* p, uint8_t ch) {
  DCHECK(apache::thrift::detail::json::kJSONEscapePrefix.size() == 4);
  memcpy(p, apache::thrift::detail::json::kJSONEscapePrefix.data(), 4);
  p[4] = hexChar(ch >> 4);
  p[5] = hexChar(ch);
}

inline uint32_t JSONProtocolWriterCommon::writeJSONEscapeChar(uint8_t ch) {
  out_.ensure(6);
  uint8_t* p = out_.writableData();
  writeJSONEscapeChar(p, ch);
  out_.append(6);
  return 6;
}

inline void JSONProtocolWriterCommon::writeJSONStringChar(
    uint8_t*& p, uint8_t ch) {
  // Only special characters >= 32 are '\' and '"'
  if (ch == apache::thrift::detail::json::kJSONBackslash ||
      ch == apache::thrift::detail::json::kJSONStringDelimiter) {
    *p++ = apache::thrift::detail::json::kJSONBackslash;
  }
  if (ch >= 32) {
    *p++ = ch;
  } else {
    uint8_t outCh = kJSONCharTable[ch];
    // Check if regular character, backslash escaped, or JSON escaped
    if (outCh != 0) {
      p[0] = apache::thrift::detail::json::kJSONBackslash;
      p[1] = outCh;
      p += 2;
    } else {
      writeJSONEscapeChar(p, ch);
      p += 6;
    }
  }
}

// Writes a string that fits the queue's initial growth strategy
// Having this specialization improves throughput of small strings by 25% to 33%
inline uint32_t JSONProtocolWriterCommon::writeJSONStringSmall(
    folly::StringPiece str) {
  out_.ensure(str.size() * 6 + 2);
  uint8_t* p = out_.writableData();
  uint8_t* start = p;
  *p++ = apache::thrift::detail::json::kJSONStringDelimiter;
  for (uint8_t ch : str) {
    writeJSONStringChar(p, ch);
  }
  *p++ = apache::thrift::detail::json::kJSONStringDelimiter;
  size_t len = p - start;
  out_.append(len);
  return static_cast<uint32_t>(len);
}

// Writes a string requiring an allocation bigger than the growth strategy
inline uint32_t JSONProtocolWriterCommon::writeJSONStringLarge(
    folly::StringPiece str) {
  out_.write(apache::thrift::detail::json::kJSONStringDelimiter);
  size_t totalBytesWritten = 2; // Two delimiters
  size_t i = 0;
  size_t bytesDesired = str.size() * 2;
  do {
    out_.ensureWithinMaxGrowth(bytesDesired);
    uint8_t* p = out_.writableData();
    uint8_t* start = p;
    // do-while is correct, empty strings are processed by writeJSONStringSmall
    assert(str.size() > 0);
    size_t outstandingBufferLen = out_.length();
    size_t bytesWrittenOnAlloc = 0;
    do {
      size_t charsGuaranteed = outstandingBufferLen / 6;
      size_t indexLimit = std::min<size_t>(i + charsGuaranteed, str.size());
      do {
        uint8_t ch = str[i];
        writeJSONStringChar(p, ch);
        i += 1;
      } while (i < indexLimit);
      size_t bytesWrittenInIteration = p - start;
      bytesWrittenOnAlloc += bytesWrittenInIteration;
      outstandingBufferLen -= bytesWrittenInIteration;
      start = p;
    } while (i < str.size() && outstandingBufferLen > 5);
    out_.append(bytesWrittenOnAlloc);
    totalBytesWritten += bytesWrittenOnAlloc;
  } while (i < str.size());
  out_.write(apache::thrift::detail::json::kJSONStringDelimiter);
  return static_cast<uint32_t>(totalBytesWritten);
}

inline uint32_t JSONProtocolWriterCommon::writeJSONString(
    folly::StringPiece str) {
  // We need 2 bytes for the delimiters, plus at most 6 bytes per char
  constexpr size_t kMaxStringLengthFittingGrowthStrategy =
      (kDesiredQueueGrowth - 2) / 6;
  if (LIKELY(str.size() <= kMaxStringLengthFittingGrowthStrategy)) {
    return writeJSONStringSmall(str);
  } else {
    return writeJSONStringLarge(str);
  }
}

inline uint32_t JSONProtocolWriterCommon::writeJSONBase64(folly::ByteRange v) {
  uint32_t ret = 2;

  out_.write(apache::thrift::detail::json::kJSONStringDelimiter);
  auto bytes = v.data();
  uint32_t len = folly::to_narrow(v.size());
  uint8_t b[4];
  while (len >= 3) {
    // Encode 3 bytes at a time
    base64_encode(bytes, 3, b);
    for (int i = 0; i < 4; i++) {
      out_.write(b[i]);
    }
    ret += 4;
    bytes += 3;
    len -= 3;
  }
  if (len) { // Handle remainder
    DCHECK_LE(len, folly::to_unsigned(std::numeric_limits<int>::max()));
    base64_encode(bytes, folly::to_narrow(len), b);
    for (uint32_t i = 0; i < len + 1; i++) {
      out_.write(b[i]);
    }
    ret += len + 1;
  }
  out_.write(apache::thrift::detail::json::kJSONStringDelimiter);

  return ret;
}

inline uint32_t JSONProtocolWriterCommon::writeJSONBool(bool val) {
  return writeJSONBoolInternal(val);
}

inline uint32_t JSONProtocolWriterCommon::writeJSONInt(int64_t num) {
  return writeJSONIntInternal(num);
}

template <typename T>
inline uint32_t JSONProtocolWriterCommon::writeJSONDouble(T dbl) {
  if (dbl == std::numeric_limits<T>::infinity()) {
    return writeJSONString(apache::thrift::detail::json::kThriftInfinity);
  } else if (dbl == -std::numeric_limits<T>::infinity()) {
    return writeJSONString(
        apache::thrift::detail::json::kThriftNegativeInfinity);
  } else if (std::isnan(dbl)) {
    return writeJSONString(apache::thrift::detail::json::kThriftNan);
  } else {
    return writeJSONDoubleInternal(dbl);
  }
}

/**
 * Public reading functions
 */

inline void JSONProtocolReaderCommon::readMessageBegin(
    std::string& name, MessageType& messageType, int32_t& seqid) {
  ensureAndBeginContext(ContextType::ARRAY);
  int64_t tmpVal;
  readI64(tmpVal);
  if (tmpVal != apache::thrift::detail::json::kThriftVersion1) {
    throwBadVersion();
  }
  readString(name);
  readI64(tmpVal);
  messageType = (MessageType)tmpVal;
  readI32(seqid);
}

inline void JSONProtocolReaderCommon::readMessageEnd() {
  endContext();
}

inline void JSONProtocolReaderCommon::readByte(int8_t& byte) {
  readInContext<int8_t>(byte);
}

inline void JSONProtocolReaderCommon::readI16(int16_t& i16) {
  readInContext<int16_t>(i16);
}

inline void JSONProtocolReaderCommon::readI32(int32_t& i32) {
  readInContext<int32_t>(i32);
}

inline void JSONProtocolReaderCommon::readI64(int64_t& i64) {
  readInContext<int64_t>(i64);
}

inline void JSONProtocolReaderCommon::readDouble(double& dub) {
  readInContext<double>(dub);
}

inline void JSONProtocolReaderCommon::readFloat(float& flt) {
  readInContext<float>(flt);
}

template <typename StrType>
inline void JSONProtocolReaderCommon::readString(StrType& str) {
  bool keyish;
  ensureAndReadContext(keyish);
  readJSONString(str);
}

template <typename StrType>
inline void JSONProtocolReaderCommon::readBinary(StrType& str) {
  bool keyish;
  ensureAndReadContext(keyish);
  readJSONBase64(str);
}

inline void JSONProtocolReaderCommon::readBinary(
    std::unique_ptr<folly::IOBuf>& str) {
  std::string tmp;
  bool keyish;
  ensureAndReadContext(keyish);
  readJSONBase64(tmp);
  str = folly::IOBuf::copyBuffer(tmp);
}

inline void JSONProtocolReaderCommon::readBinary(folly::IOBuf& str) {
  std::string tmp;
  bool keyish;
  ensureAndReadContext(keyish);
  readJSONBase64(tmp);
  str.appendChain(folly::IOBuf::copyBuffer(tmp));
}

/**
 * Protected reading functions
 */

inline void JSONProtocolReaderCommon::skipWhitespace() {
  constexpr auto& vector_needle = detail::json::json_ws_vector_needle;
  constexpr auto& scalar_needle = detail::json::json_ws_scalar_needle;
  while (true) { // for loop generates larger code with 2 calls to peekBytesSlow
    auto const peek = folly::reinterpret_span_cast<char const>(in_.peek());
    if (peek.empty()) {
      return;
    }
    auto const newl = apache::thrift::detail::json::kJSONNewline;
    // if 0th char is newline then it is likely that indentation follows; vector
    // algorithm is better for indentation but worse for single-char whitespace
    auto const usevec = !folly::kIsMobile /* has vector acceleration */ && //
        folly::kIsArchAmd64 && peek.size() > 16 && peek[0] == newl;
    auto const vecskip = size_t(usevec);
    auto const size =
        vecskip + vector_needle(scalar_needle, usevec, peek.subspan(vecskip));
    skippedWhitespace_ += size;
    in_.skip(size);
    if (size < peek.size()) {
      return;
    }
  }
}

inline uint32_t JSONProtocolReaderCommon::readWhitespace() {
  skipWhitespace();
  auto ret = skippedWhitespace_;
  skippedWhitespace_ = 0;
  return ret;
}

inline uint32_t JSONProtocolReaderCommon::ensureCharNoWhitespace(
    char expected) {
  auto actual = in_.read<int8_t>();
  if (actual != expected) {
    throwUnexpectedChar(actual, expected);
  }
  return 1;
}

inline uint32_t JSONProtocolReaderCommon::ensureChar(char expected) {
  auto ret = readWhitespace();
  return ret + ensureCharNoWhitespace(expected);
}

inline void JSONProtocolReaderCommon::ensureAndSkipContext() {
  if (skippedIsUnread_) {
    return;
  }
  skippedIsUnread_ = true;
  keyish_ = false;
  if (!context.empty()) {
    auto meta = context.back().meta++;
    switch (context.back().type) {
      case ContextType::MAP:
        if (meta % 2 == 0) {
          if (meta != 0) {
            skippedChars_ +=
                ensureChar(apache::thrift::detail::json::kJSONElemSeparator);
          }
          keyish_ = true;
        } else {
          skippedChars_ +=
              ensureChar(apache::thrift::detail::json::kJSONPairSeparator);
        }
        break;
      case ContextType::ARRAY:
        if (meta != 0) {
          skippedChars_ +=
              ensureChar(apache::thrift::detail::json::kJSONElemSeparator);
        }
        break;
      default:
        break;
    }
  }
}

inline void JSONProtocolReaderCommon::ensureAndReadContext(bool& keyish) {
  ensureAndSkipContext();
  keyish = keyish_;
  skippedChars_ = 0;
  skippedIsUnread_ = false;
}

inline void JSONProtocolReaderCommon::ensureAndBeginContext(ContextType type) {
  bool keyish;
  ensureAndReadContext(keyish);
  // perhaps handle keyish == true?  I think for backwards compat we want to
  // be able to handle non-string keys, even if it isn't valid JSON
  beginContext(type);
}

inline void JSONProtocolReaderCommon::beginContext(ContextType type) {
  context.push_back({type, 0});
  switch (type) {
    case ContextType::MAP:
      ensureChar(apache::thrift::detail::json::kJSONObjectStart);
      return;
    case ContextType::ARRAY:
      ensureChar(apache::thrift::detail::json::kJSONArrayStart);
      return;
    default:
      break;
  }
  CHECK(false);
}

inline void JSONProtocolReaderCommon::endContext() {
  DCHECK(!context.empty());

  auto type = context.back().type;
  context.pop_back();
  switch (type) {
    case ContextType::MAP:
      ensureChar(apache::thrift::detail::json::kJSONObjectEnd);
      return;
    case ContextType::ARRAY:
      ensureChar(apache::thrift::detail::json::kJSONArrayEnd);
      return;
    default:
      break;
  }
  CHECK(false);
}

template <typename T>
T JSONProtocolReaderCommon::castIntegral(folly::StringPiece val) {
  return folly::tryTo<T>(val).thenOrThrow(
      [](auto x) { return x; },
      [&](folly::ConversionCode) {
        throwUnrecognizableAsIntegral(val, folly::pretty_name<T>());
      });
}

template <typename T>
void JSONProtocolReaderCommon::readInContext(T& val) {
  static_assert(
      !apache::thrift::detail::is_string<T>::value,
      "Strings are strings in any context, use readJSONString");

  bool keyish;
  ensureAndReadContext(keyish);
  if (keyish) {
    readJSONKey(val);
  } else {
    readJSONVal(val);
  }
}

inline void JSONProtocolReaderCommon::readJSONKey(bool& key) {
  std::string s;
  readJSONString(s);
  key = JSONtoBool(s);
}

template <typename T>
void JSONProtocolReaderCommon::readJSONKey(T& key) {
  std::string s;
  readJSONString(s);
  key = castIntegral<T>(s);
}

template <typename T>
void JSONProtocolReaderCommon::readJSONIntegral(T& val) {
  std::string serialized;
  readNumericalChars(serialized);
  val = castIntegral<T>(serialized);
}

inline void JSONProtocolReaderCommon::readNumericalChars(std::string& val) {
  readWhitespace();
  readWhile(
      [](uint8_t ch) {
        return (ch >= '0' && ch <= '9') || ch == '+' || ch == '-' ||
            ch == '.' || ch == 'E' || ch == 'e';
      },
      val);
}

inline void JSONProtocolReaderCommon::readJSONVal(int8_t& val) {
  readJSONIntegral<int8_t>(val);
}

inline void JSONProtocolReaderCommon::readJSONVal(int16_t& val) {
  readJSONIntegral<int16_t>(val);
}

inline void JSONProtocolReaderCommon::readJSONVal(int32_t& val) {
  readJSONIntegral<int32_t>(val);
}

inline void JSONProtocolReaderCommon::readJSONVal(int64_t& val) {
  readJSONIntegral<int64_t>(val);
}

template <typename Floating>
inline typename std::enable_if_t<std::is_floating_point_v<Floating>>
JSONProtocolReaderCommon::readJSONVal(Floating& val) {
  static_assert(
      std::numeric_limits<Floating>::is_iec559,
      "Parameter type must fulfill IEEE 754 floating-point standard");

  readWhitespace();
  if (peekCharSafe() == apache::thrift::detail::json::kJSONStringDelimiter) {
    std::string str;
    readJSONString(str);
    if (str == apache::thrift::detail::json::kThriftNan) {
      val = std::numeric_limits<Floating>::quiet_NaN();
    } else if (str == apache::thrift::detail::json::kThriftNegativeNan) {
      val = -std::numeric_limits<Floating>::quiet_NaN();
    } else if (str == apache::thrift::detail::json::kThriftInfinity) {
      val = std::numeric_limits<Floating>::infinity();
    } else if (str == apache::thrift::detail::json::kThriftNegativeInfinity) {
      val = -std::numeric_limits<Floating>::infinity();
    } else {
      throwUnrecognizableAsFloatingPoint(str);
    }
    return;
  }
  std::string s;
  readNumericalChars(s);
  try {
    val = folly::to<Floating>(s);
  } catch (const std::exception&) {
    throwUnrecognizableAsFloatingPoint(s);
  }
}

template <typename Str>
inline typename std::enable_if_t<apache::thrift::detail::is_string<Str>::value>
JSONProtocolReaderCommon::readJSONVal(Str& val) {
  readJSONString(val);
}

inline bool JSONProtocolReaderCommon::JSONtoBool(const std::string& s) {
  if (s == "true") {
    return true;
  } else if (s == "false") {
    return false;
  } else {
    throwUnrecognizableAsBoolean(s);
  }
  return false;
}

inline void JSONProtocolReaderCommon::readJSONVal(bool& val) {
  std::string s;
  readJSONKeyword(s);
  val = JSONtoBool(s);
}

inline void JSONProtocolReaderCommon::readJSONNull() {
  std::string s;
  readJSONKeyword(s);
  if (s != "null") {
    throwUnrecognizableAsAny(s);
  }
}

inline void JSONProtocolReaderCommon::readJSONKeyword(std::string& kw) {
  readWhitespace();
  readWhile([](int8_t ch) { return ch >= 'a' && ch <= 'z'; }, kw);
}

inline void JSONProtocolReaderCommon::readJSONEscapeChar(uint8_t& out) {
  uint8_t b1, b2;
  ensureCharNoWhitespace(apache::thrift::detail::json::kJSONZeroChar);
  ensureCharNoWhitespace(apache::thrift::detail::json::kJSONZeroChar);
  b1 = in_.read<uint8_t>();
  b2 = in_.read<uint8_t>();
  out = static_cast<uint8_t>((hexVal(b1) << 4) + hexVal(b2));
}

template <typename StrType>
inline void JSONProtocolReaderCommon::readJSONString(StrType& val) {
  ensureChar(apache::thrift::detail::json::kJSONStringDelimiter);
  val.clear();

  while (true) { // for loop generates larger code with 2 calls to peekBytesSlow
  next_peek:
    auto peek = in_.peekBytes();
    if (FOLLY_UNLIKELY(peek.empty())) {
      std::ignore = in_.read<uint8_t>(); // this should throw
      folly::assume_unreachable();
    }
    uint32_t size = 0;
    for (auto const ch : peek) {
      if (ch == apache::thrift::detail::json::kJSONStringDelimiter) {
        val += std::string_view(folly::StringPiece(peek)).substr(0, size);
        in_.skip(size + 1);
        return;
      }
      if (ch == apache::thrift::detail::json::kJSONBackslash) {
        val += std::string_view(folly::StringPiece(peek)).substr(0, size);
        in_.skip(size + 1);
        auto const seq = readJSONEscapeSequence();
        auto const utf8 = folly::span{seq.data, seq.size};
        auto const utf8c = folly::reinterpret_span_cast<char const>(utf8);
        val += std::string_view(utf8c.data(), utf8c.size());
        goto next_peek; // out of for-ch-in-peek; continue to while-true
      }
      ++size;
    }
    val += std::string_view(folly::StringPiece(peek)).substr(0, size);
    in_.skip(size);
  }
}

template <typename StrType>
inline void JSONProtocolReaderCommon::readJSONBase64(StrType& str) {
  std::string tmp;
  readJSONString(tmp);

  uint8_t* b = (uint8_t*)tmp.c_str();
  uint32_t len = folly::to_narrow(tmp.length());
  str.clear();

  // Allow optional trailing '=' as padding
  while (len > 0 && b[len - 1] == '=') {
    --len;
  }

  while (len >= 4) {
    base64_decode(b, 4);
    str.append((const char*)b, 3);
    b += 4;
    len -= 4;
  }
  // Don't decode if we hit the end or got a single leftover byte (invalid
  // base64 but legal for skip of regular string type)
  if (len > 1) {
    base64_decode(b, len);
    str.append((const char*)b, len - 1);
  }
}

// Return the integer value of a hex character ch.
// Throw a protocol exception if the character is not [0-9a-f].
inline uint8_t JSONProtocolReaderCommon::hexVal(uint8_t ch) {
  auto const v = folly::hex_decode_digit_table(char(ch));
  if (!folly::hex_decoded_digit_is_valid(v)) {
    throwInvalidHexChar(ch);
  }
  return v;
}

template <class Predicate>
uint32_t JSONProtocolReaderCommon::readWhile(
    const Predicate& pred, std::string& out) {
  uint32_t ret = 0;
  for (auto peek = in_.peekBytes(); !peek.empty(); peek = in_.peekBytes()) {
    uint32_t size = 0;
    for (uint8_t ch : peek) {
      if (!pred(ch)) {
        out.append(peek.begin(), peek.begin() + size);
        in_.skip(size);
        return ret + size;
      }
      ++size;
    }
    out.append(peek.begin(), peek.end());
    ret += size;
    in_.skip(size);
  }
  return ret;
}

inline int8_t JSONProtocolReaderCommon::peekCharSafe() {
  auto peek = in_.peekBytes();
  return peek.empty() ? 0 : *peek.data();
}

} // namespace thrift
} // namespace apache
