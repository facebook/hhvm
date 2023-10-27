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

namespace apache {
namespace thrift {

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
  }
  CHECK(false);
  return 0;
}

inline uint32_t JSONProtocolWriterCommon::writeJSONEscapeChar(uint8_t ch) {
  DCHECK(apache::thrift::detail::json::kJSONEscapePrefix.size() == 4);
  out_.push(
      (const uint8_t*)apache::thrift::detail::json::kJSONEscapePrefix.data(),
      4);
  out_.write(hexChar(ch >> 4));
  out_.write(hexChar(ch));
  return 6;
}

inline uint32_t JSONProtocolWriterCommon::writeJSONChar(uint8_t ch) {
  if (ch >= 0x30) {
    // Only special character >= 0x30 is '\'
    if (ch == apache::thrift::detail::json::kJSONBackslash) {
      out_.write(apache::thrift::detail::json::kJSONBackslash);
      out_.write(apache::thrift::detail::json::kJSONBackslash);
      return 2;
    } else {
      out_.write(ch);
      return 1;
    }
  } else {
    uint8_t outCh = kJSONCharTable[ch];
    // Check if regular character, backslash escaped, or JSON escaped
    if (outCh == 1) {
      out_.write(ch);
      return 1;
    } else if (outCh > 1) {
      out_.write(apache::thrift::detail::json::kJSONBackslash);
      out_.write(outCh);
      return 2;
    } else {
      return writeJSONEscapeChar(ch);
    }
  }
}

inline uint32_t JSONProtocolWriterCommon::writeJSONString(
    folly::StringPiece str) {
  uint32_t ret = 2;

  out_.write(apache::thrift::detail::json::kJSONStringDelimiter);
  for (auto c : str) {
    ret += writeJSONChar(c);
  }
  out_.write(apache::thrift::detail::json::kJSONStringDelimiter);

  return ret;
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
  for (auto peek = in_.peekBytes(); !peek.empty(); peek = in_.peekBytes()) {
    uint32_t size = 0;
    for (char ch : peek) {
      if (ch != apache::thrift::detail::json::kJSONSpace &&
          ch != apache::thrift::detail::json::kJSONNewline &&
          ch != apache::thrift::detail::json::kJSONTab &&
          ch != apache::thrift::detail::json::kJSONCarriageReturn) {
        in_.skip(size);
        return;
      }
      ++skippedWhitespace_;
      ++size;
    }
    in_.skip(size);
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
inline typename std::enable_if<std::is_floating_point<Floating>::value>::type
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
inline
    typename std::enable_if<apache::thrift::detail::is_string<Str>::value>::type
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

  std::string json = "\"";
  bool fullDecodeRequired = false;
  val.clear();
  while (true) {
    auto ch = in_.read<uint8_t>();
    if (ch == apache::thrift::detail::json::kJSONStringDelimiter) {
      break;
    }
    if (ch == apache::thrift::detail::json::kJSONBackslash) {
      ch = in_.read<uint8_t>();
      if (ch == apache::thrift::detail::json::kJSONEscapeChar) {
        if (allowDecodeUTF8_) {
          json += "\\u";
          fullDecodeRequired = true;
          continue;
        } else {
          readJSONEscapeChar(ch);
        }
      } else {
        size_t pos = kEscapeChars().find_first_of(ch);
        if (pos == std::string::npos) {
          throwInvalidEscapeChar(ch);
        }
        if (fullDecodeRequired) {
          json += "\\";
          json += ch;
          continue;
        } else {
          ch = kEscapeCharVals[pos];
        }
      }
    }

    if (fullDecodeRequired) {
      json += ch;
    } else {
      val += ch;
    }
  }

  if (fullDecodeRequired) {
    json += "\"";
    try {
      folly::dynamic parsed = folly::parseJson(json);
      val += parsed.getString();
    } catch (const std::exception& e) {
      throwUnrecognizableAsString(json, e);
    }
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
  if ((ch >= '0') && (ch <= '9')) {
    return ch - '0';
  } else if ((ch >= 'a') && (ch <= 'f')) {
    return ch - 'a' + 10;
  } else {
    throwInvalidHexChar(ch);
  }
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
