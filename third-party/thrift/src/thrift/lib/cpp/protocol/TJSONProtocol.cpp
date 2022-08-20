/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <thrift/lib/cpp/protocol/TJSONProtocol.h>

#include <math.h>
#include <folly/Conv.h>
#include <folly/dynamic.h>
#include <folly/json.h>
#include <thrift/lib/cpp/protocol/TBase64Utils.h>
#include <thrift/lib/cpp/transport/TTransportException.h>

using namespace apache::thrift::transport;

namespace apache {
namespace thrift {
namespace protocol {

// Static data
const uint8_t TJSONProtocol::kJSONObjectStart = '{';
const uint8_t TJSONProtocol::kJSONObjectEnd = '}';
const uint8_t TJSONProtocol::kJSONArrayStart = '[';
const uint8_t TJSONProtocol::kJSONArrayEnd = ']';
const uint8_t TJSONProtocol::kJSONPairSeparator = ':';
const uint8_t TJSONProtocol::kJSONElemSeparator = ',';
const uint8_t TJSONProtocol::kJSONBackslash = '\\';
const uint8_t TJSONProtocol::kJSONStringDelimiter = '"';
const uint8_t TJSONProtocol::kJSONZeroChar = '0';
const uint8_t TJSONProtocol::kJSONEscapeChar = 'u';
const uint8_t TJSONProtocol::kJSONSpace = ' ';
const uint8_t TJSONProtocol::kJSONNewline = '\n';
const uint8_t TJSONProtocol::kJSONTab = '\t';
const uint8_t TJSONProtocol::kJSONCarriageReturn = '\r';

const std::string TJSONProtocol::kJSONEscapePrefix("\\u00");

const std::string TJSONProtocol::kJSONTrue("true");
const std::string TJSONProtocol::kJSONFalse("false");

const uint32_t TJSONProtocol::kThriftVersion1 = 1;

const std::string TJSONProtocol::kThriftNan("NaN");
const std::string TJSONProtocol::kThriftNegativeNan("-NaN");
const std::string TJSONProtocol::kThriftInfinity("Infinity");
const std::string TJSONProtocol::kThriftNegativeInfinity("-Infinity");

const std::string TJSONProtocol::kTypeNameBool("tf");
const std::string TJSONProtocol::kTypeNameByte("i8");
const std::string TJSONProtocol::kTypeNameI16("i16");
const std::string TJSONProtocol::kTypeNameI32("i32");
const std::string TJSONProtocol::kTypeNameI64("i64");
const std::string TJSONProtocol::kTypeNameDouble("dbl");
const std::string TJSONProtocol::kTypeNameFloat("flt");
const std::string TJSONProtocol::kTypeNameStruct("rec");
const std::string TJSONProtocol::kTypeNameString("str");
const std::string TJSONProtocol::kTypeNameMap("map");
const std::string TJSONProtocol::kTypeNameList("lst");
const std::string TJSONProtocol::kTypeNameSet("set");

const std::string& TJSONProtocol::getTypeNameForTypeID(TType typeID) {
  switch (typeID) {
    case T_BOOL:
      return kTypeNameBool;
    case T_BYTE:
      return kTypeNameByte;
    case T_I16:
      return kTypeNameI16;
    case T_I32:
      return kTypeNameI32;
    case T_I64:
      return kTypeNameI64;
    case T_DOUBLE:
      return kTypeNameDouble;
    case T_FLOAT:
      return kTypeNameFloat;
    case T_STRING:
      return kTypeNameString;
    case T_STRUCT:
      return kTypeNameStruct;
    case T_MAP:
      return kTypeNameMap;
    case T_SET:
      return kTypeNameSet;
    case T_LIST:
      return kTypeNameList;
    default:
      throw TProtocolException(
          TProtocolException::NOT_IMPLEMENTED, "Unrecognized type");
  }
}

TType TJSONProtocol::getTypeIDForTypeName(const std::string& name) {
  TType result = T_STOP; // Sentinel value
  if (name.length() > 1) {
    switch (name[0]) {
      case 'd':
        result = T_DOUBLE;
        break;
      case 'f':
        result = T_FLOAT;
        break;
      case 'i':
        switch (name[1]) {
          case '8':
            result = T_BYTE;
            break;
          case '1':
            result = T_I16;
            break;
          case '3':
            result = T_I32;
            break;
          case '6':
            result = T_I64;
            break;
        }
        break;
      case 'l':
        result = T_LIST;
        break;
      case 'm':
        result = T_MAP;
        break;
      case 'r':
        result = T_STRUCT;
        break;
      case 's':
        if (name[1] == 't') {
          result = T_STRING;
        } else if (name[1] == 'e') {
          result = T_SET;
        }
        break;
      case 't':
        result = T_BOOL;
        break;
    }
  }
  if (result == T_STOP) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED, "Unrecognized type");
  }
  return result;
}

// This table describes the handling for the first 0x30 characters
//  0 : escape using "\u00xx" notation
//  1 : just output index
// <other> : escape using "\<other>" notation
static const uint8_t kJSONCharTable[0x30] = {
    //  0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
    0, 0, 0,   0, 0, 0, 0, 0, 'b', 't', 'n', 0, 'f', 'r', 0, 0, // 0
    0, 0, 0,   0, 0, 0, 0, 0, 0,   0,   0,   0, 0,   0,   0, 0, // 1
    1, 1, '"', 1, 1, 1, 1, 1, 1,   1,   1,   1, 1,   1,   1, 1, // 2
};

// This string's characters must match up with the elements in kEscapeCharVals.
// I don't have '/' on this list even though it appears on www.json.org --
// it is not in the RFC
const static std::string kEscapeChars("\"\\/bfnrt");

// The elements of this array must match up with the sequence of characters in
// kEscapeChars
const static uint8_t kEscapeCharVals[8] = {
    '"',
    '\\',
    '/',
    '\b',
    '\f',
    '\n',
    '\r',
    '\t',
};

// Static helper functions

// Reads any whitespace characters
static uint32_t skipWhitespace(TJSONProtocol::LookaheadReader& reader) {
  uint32_t result = 0;

  while (reader.canPeek()) {
    uint8_t ch = reader.peek();
    if (ch != TJSONProtocol::kJSONSpace && ch != TJSONProtocol::kJSONNewline &&
        ch != TJSONProtocol::kJSONTab &&
        ch != TJSONProtocol::kJSONCarriageReturn) {
      break;
    }
    reader.read();
    ++result;
  }

  return result;
}

// Read 1 character from the transport trans and verify that it is the
// expected character ch.
// Throw a protocol exception if it is not.
static uint32_t readSyntaxChar(
    TJSONProtocol::LookaheadReader& reader, uint8_t ch) {
  uint8_t ch2 = reader.read();
  if (ch2 != ch) {
    throw TProtocolException(
        TProtocolException::INVALID_DATA,
        "Expected \'" + std::string((char*)&ch, 1) + "\'; got \'" +
            std::string((char*)&ch2, 1) + "\'.");
  }
  return 1;
}

// Reads a structural character (any of '[', ']', '{', '}', ',' and ':'),
// and any whitespace characters around it
static uint32_t readStructuralChar(
    TJSONProtocol::LookaheadReader& reader, uint8_t ch) {
  assert(
      ch == TJSONProtocol::kJSONObjectStart ||
      ch == TJSONProtocol::kJSONObjectEnd ||
      ch == TJSONProtocol::kJSONArrayStart ||
      ch == TJSONProtocol::kJSONArrayEnd ||
      ch == TJSONProtocol::kJSONPairSeparator ||
      ch == TJSONProtocol::kJSONElemSeparator);

  uint32_t result = 0;
  result += skipWhitespace(reader);
  result += readSyntaxChar(reader, ch);
  result += skipWhitespace(reader);

  return result;
}

// Return the integer value of a hex character ch.
// Throw a protocol exception if the character is not [0-9a-f].
static uint8_t hexVal(uint8_t ch) {
  if ((ch >= '0') && (ch <= '9')) {
    return ch - '0';
  } else if ((ch >= 'a') && (ch <= 'f')) {
    return ch - 'a' + 10;
  } else {
    throw TProtocolException(
        TProtocolException::INVALID_DATA,
        "Expected hex val ([0-9a-f]); got \'" + std::string((char*)&ch, 1) +
            "\'.");
  }
}

// Return the hex character representing the integer val. The value is masked
// to make sure it is in the correct range.
static uint8_t hexChar(uint8_t val) {
  val &= 0x0F;
  if (val < 10) {
    return val + '0';
  } else {
    return val - 10 + 'a';
  }
}

// Return true if the character ch is in [-+0-9.Ee]; false otherwise
static bool isJSONNumeric(uint8_t ch) {
  switch (ch) {
    case '+':
    case '-':
    case '.':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case 'E':
    case 'e':
      return true;
  }
  return false;
}

/**
 * Class to serve as base JSON context and as base class for other context
 * implementations
 */
class TJSONContext {
 public:
  TJSONContext() {}

  virtual ~TJSONContext() {}

  /**
   * Write context data to the transport. Default is to do nothing.
   */
  virtual uint32_t write(TTransport& /*trans*/) { return 0; }

  /**
   * Read context data from the transport. Default is to do nothing.
   */
  virtual uint32_t read(TJSONProtocol::LookaheadReader& /*reader*/) {
    return 0;
  }

  /**
   * Return true if numbers need to be escaped as strings in this context.
   * Default behavior is to return false.
   */
  virtual bool escapeNum() { return false; }
};

// Context class for object member key-value pairs
class JSONPairContext : public TJSONContext {
 public:
  JSONPairContext() : first_(true), colon_(true) {}

  uint32_t write(TTransport& trans) override {
    if (first_) {
      first_ = false;
      colon_ = true;
      return 0;
    } else {
      trans.write(
          colon_ ? &TJSONProtocol::kJSONPairSeparator
                 : &TJSONProtocol::kJSONElemSeparator,
          1);
      colon_ = !colon_;
      return 1;
    }
  }

  uint32_t read(TJSONProtocol::LookaheadReader& reader) override {
    if (first_) {
      first_ = false;
      colon_ = true;
      return 0;
    } else {
      uint8_t ch =
          (colon_ ? TJSONProtocol::kJSONPairSeparator
                  : TJSONProtocol::kJSONElemSeparator);
      colon_ = !colon_;
      return readStructuralChar(reader, ch);
    }
  }

  // Numbers must be turned into strings if they are the key part of a pair
  bool escapeNum() override { return colon_; }

 private:
  bool first_;
  bool colon_;
};

// Context class for lists
class JSONListContext : public TJSONContext {
 public:
  JSONListContext() : first_(true) {}

  uint32_t write(TTransport& trans) override {
    if (first_) {
      first_ = false;
      return 0;
    } else {
      trans.write(&TJSONProtocol::kJSONElemSeparator, 1);
      return 1;
    }
  }

  uint32_t read(TJSONProtocol::LookaheadReader& reader) override {
    if (first_) {
      first_ = false;
      return 0;
    } else {
      return readStructuralChar(reader, TJSONProtocol::kJSONElemSeparator);
    }
  }

 private:
  bool first_;
};

TJSONProtocol::TJSONProtocol(std::shared_ptr<TTransport> ptrans)
    : TVirtualProtocol<TJSONProtocol>(ptrans),
      trans_(ptrans.get()),
      context_(new TJSONContext()),
      allowDecodeUTF8_(false),
      reader_(*ptrans) {}

TJSONProtocol::TJSONProtocol(TTransport* ptrans)
    : TVirtualProtocol<TJSONProtocol>(ptrans),
      trans_(ptrans),
      context_(new TJSONContext()),
      allowDecodeUTF8_(false),
      reader_(*ptrans) {}

TJSONProtocol::~TJSONProtocol() {}

void TJSONProtocol::pushContext(std::shared_ptr<TJSONContext> c) {
  contexts_.push(context_);
  context_ = c;
}

void TJSONProtocol::popContext() {
  context_ = contexts_.top();
  contexts_.pop();
}

// Write the character ch as a JSON escape sequence ("\u00xx")
uint32_t TJSONProtocol::writeJSONEscapeChar(uint8_t ch) {
  trans_->write(
      (const uint8_t*)kJSONEscapePrefix.c_str(), kJSONEscapePrefix.length());
  uint8_t outCh = hexChar(ch >> 4);
  trans_->write(&outCh, 1);
  outCh = hexChar(ch);
  trans_->write(&outCh, 1);
  return 6;
}

// Write the character ch as part of a JSON string, escaping as appropriate.
uint32_t TJSONProtocol::writeJSONChar(uint8_t ch) {
  if (ch >= 0x30) {
    if (ch == kJSONBackslash) { // Only special character >= 0x30 is '\'
      trans_->write(&kJSONBackslash, 1);
      trans_->write(&kJSONBackslash, 1);
      return 2;
    } else {
      trans_->write(&ch, 1);
      return 1;
    }
  } else {
    uint8_t outCh = kJSONCharTable[ch];
    // Check if regular character, backslash escaped, or JSON escaped
    if (outCh == 1) {
      trans_->write(&ch, 1);
      return 1;
    } else if (outCh > 1) {
      trans_->write(&kJSONBackslash, 1);
      trans_->write(&outCh, 1);
      return 2;
    } else {
      return writeJSONEscapeChar(ch);
    }
  }
}

// Write out the contents of the string str as a JSON string, escaping
// characters as appropriate.
uint32_t TJSONProtocol::writeJSONString(const std::string& str) {
  uint32_t result = context_->write(*trans_);
  result += 2; // For quotes
  trans_->write(&kJSONStringDelimiter, 1);
  std::string::const_iterator iter(str.begin());
  std::string::const_iterator end(str.end());
  while (iter != end) {
    result += writeJSONChar(*iter++);
  }
  trans_->write(&kJSONStringDelimiter, 1);
  return result;
}

// Write out the contents of the string as JSON string, base64-encoding
// the string's contents, and escaping as appropriate
uint32_t TJSONProtocol::writeJSONBase64(const std::string& str) {
  uint32_t result = context_->write(*trans_);
  result += 2; // For quotes
  trans_->write(&kJSONStringDelimiter, 1);
  uint8_t b[4];
  const uint8_t* bytes = (const uint8_t*)str.c_str();
  uint32_t len = str.length();
  while (len >= 3) {
    // Encode 3 bytes at a time
    base64_encode(bytes, 3, b);
    trans_->write(b, 4);
    result += 4;
    bytes += 3;
    len -= 3;
  }
  if (len) { // Handle remainder
    base64_encode(bytes, len, b);
    trans_->write(b, len + 1);
    result += len + 1;
  }
  trans_->write(&kJSONStringDelimiter, 1);
  return result;
}

// Convert the given integer type to a JSON number, or a string
// if the context requires it (eg: key in a map pair).
template <typename NumberType>
uint32_t TJSONProtocol::writeJSONInteger(NumberType num) {
  uint32_t result = context_->write(*trans_);
  auto val = folly::to<std::string>(num);
  bool escapeNum = context_->escapeNum();
  if (escapeNum) {
    trans_->write(&kJSONStringDelimiter, 1);
    result += 1;
  }
  trans_->write((const uint8_t*)val.c_str(), val.length());
  result += val.length();
  if (escapeNum) {
    trans_->write(&kJSONStringDelimiter, 1);
    result += 1;
  }
  return result;
}

// Convert the given boolean type to a true/false token
uint32_t TJSONProtocol::writeJSONBool(const bool value) {
  uint32_t result = context_->write(*trans_);
  result += 1;

  bool escapeNum = context_->escapeNum();
  if (escapeNum) {
    trans_->write(&kJSONStringDelimiter, 1);
    result += 1;
  }

  std::string boolVal = value ? "true" : "false";

  std::string::const_iterator iter(boolVal.begin());
  std::string::const_iterator end(boolVal.end());
  while (iter != end) {
    result += writeJSONChar(*iter++);
  }

  if (escapeNum) {
    trans_->write(&kJSONStringDelimiter, 1);
    result += 1;
  }

  return result;
}

// Convert the given double to a JSON string, which is either the number,
// "NaN" or "Infinity" or "-Infinity".
template <typename NumberType>
uint32_t TJSONProtocol::writeJSONDouble(NumberType num) {
  uint32_t result = context_->write(*trans_);

  std::string val;
  bool special = true;
  if (num == std::numeric_limits<NumberType>::infinity()) {
    val = kThriftInfinity;
  } else if (num == -std::numeric_limits<NumberType>::infinity()) {
    val = kThriftNegativeInfinity;
  } else if (std::isnan(num)) {
    val = kThriftNan;
  } else {
    special = false;
    val = folly::to<std::string>(num);
  }

  bool escapeNum = special || context_->escapeNum();
  if (escapeNum) {
    trans_->write(&kJSONStringDelimiter, 1);
    result += 1;
  }
  trans_->write((const uint8_t*)val.c_str(), val.length());
  result += val.length();
  if (escapeNum) {
    trans_->write(&kJSONStringDelimiter, 1);
    result += 1;
  }
  return result;
}

uint32_t TJSONProtocol::writeJSONObjectStart() {
  uint32_t result = context_->write(*trans_);
  trans_->write(&kJSONObjectStart, 1);
  pushContext(std::shared_ptr<TJSONContext>(new JSONPairContext()));
  return result + 1;
}

uint32_t TJSONProtocol::writeJSONObjectEnd() {
  popContext();
  trans_->write(&kJSONObjectEnd, 1);
  return 1;
}

uint32_t TJSONProtocol::writeJSONArrayStart() {
  uint32_t result = context_->write(*trans_);
  trans_->write(&kJSONArrayStart, 1);
  pushContext(std::shared_ptr<TJSONContext>(new JSONListContext()));
  return result + 1;
}

uint32_t TJSONProtocol::writeJSONArrayEnd() {
  popContext();
  trans_->write(&kJSONArrayEnd, 1);
  return 1;
}

uint32_t TJSONProtocol::writeMessageBegin(
    const std::string& name,
    const TMessageType messageType,
    const int32_t seqid) {
  uint32_t result = writeJSONArrayStart();
  result += writeJSONInteger(kThriftVersion1);
  result += writeJSONString(name);
  result += writeJSONInteger(messageType);
  result += writeJSONInteger(seqid);
  return result;
}

uint32_t TJSONProtocol::writeMessageEnd() {
  return writeJSONArrayEnd();
}

uint32_t TJSONProtocol::writeStructBegin(const char* /*name*/) {
  return writeJSONObjectStart();
}

uint32_t TJSONProtocol::writeStructEnd() {
  return writeJSONObjectEnd();
}

uint32_t TJSONProtocol::writeFieldBegin(
    const char* /*name*/, const TType fieldType, const int16_t fieldId) {
  uint32_t result = writeJSONInteger(fieldId);
  result += writeJSONObjectStart();
  result += writeJSONString(getTypeNameForTypeID(fieldType));
  return result;
}

uint32_t TJSONProtocol::writeFieldEnd() {
  return writeJSONObjectEnd();
}

uint32_t TJSONProtocol::writeFieldStop() {
  return 0;
}

uint32_t TJSONProtocol::writeMapBegin(
    const TType keyType, const TType valType, const uint32_t size) {
  uint32_t result = writeJSONArrayStart();
  result += writeJSONString(getTypeNameForTypeID(keyType));
  result += writeJSONString(getTypeNameForTypeID(valType));
  result += writeJSONInteger((int64_t)size);
  result += writeJSONObjectStart();
  return result;
}

uint32_t TJSONProtocol::writeMapEnd() {
  return writeJSONObjectEnd() + writeJSONArrayEnd();
}

uint32_t TJSONProtocol::writeListBegin(
    const TType elemType, const uint32_t size) {
  uint32_t result = writeJSONArrayStart();
  result += writeJSONString(getTypeNameForTypeID(elemType));
  result += writeJSONInteger((int64_t)size);
  return result;
}

uint32_t TJSONProtocol::writeListEnd() {
  return writeJSONArrayEnd();
}

uint32_t TJSONProtocol::writeSetBegin(
    const TType elemType, const uint32_t size) {
  uint32_t result = writeJSONArrayStart();
  result += writeJSONString(getTypeNameForTypeID(elemType));
  result += writeJSONInteger((int64_t)size);
  return result;
}

uint32_t TJSONProtocol::writeSetEnd() {
  return writeJSONArrayEnd();
}

uint32_t TJSONProtocol::writeBool(const bool value) {
  return writeJSONInteger(value);
}

uint32_t TJSONProtocol::writeByte(const int8_t byte) {
  // writeByte() must be handled specially because folly::to sees
  // int8_t as a text type instead of an integer type
  return writeJSONInteger((int16_t)byte);
}

uint32_t TJSONProtocol::writeI16(const int16_t i16) {
  return writeJSONInteger(i16);
}

uint32_t TJSONProtocol::writeI32(const int32_t i32) {
  return writeJSONInteger(i32);
}

uint32_t TJSONProtocol::writeI64(const int64_t i64) {
  return writeJSONInteger(i64);
}

uint32_t TJSONProtocol::writeDouble(const double dub) {
  return writeJSONDouble(dub);
}

uint32_t TJSONProtocol::writeFloat(const float flt) {
  return writeJSONDouble(flt);
}

uint32_t TJSONProtocol::writeString(const std::string& str) {
  return writeJSONString(str);
}

uint32_t TJSONProtocol::writeBinary(const std::string& str) {
  return writeJSONBase64(str);
}

/**
 * Reading functions
 */

// Skips any whitespace characters
uint32_t TJSONProtocol::skipJSONWhitespace() {
  return skipWhitespace(reader_);
}

// Whitespace characters is allowed before or after any
// structural characters
uint32_t TJSONProtocol::readJSONStructuralChar(uint8_t ch) {
  return readStructuralChar(reader_, ch);
}

// Reads 1 byte and verifies that it matches ch.
uint32_t TJSONProtocol::readJSONSyntaxChar(uint8_t ch) {
  return readSyntaxChar(reader_, ch);
}

// Decodes the four hex parts of a JSON escaped string character and returns
// the character via out. The first two characters must be "00".
uint32_t TJSONProtocol::readJSONEscapeChar(uint8_t* out) {
  uint8_t b[2];
  readJSONSyntaxChar(kJSONZeroChar);
  readJSONSyntaxChar(kJSONZeroChar);
  b[0] = reader_.read();
  b[1] = reader_.read();
  *out = (hexVal(b[0]) << 4) + hexVal(b[1]);
  return 4;
}

// Decodes a JSON string, including unescaping, and returns the string via str
uint32_t TJSONProtocol::readJSONString(std::string& str, bool skipContext) {
  uint32_t result = (skipContext ? 0 : context_->read(reader_));
  result += readJSONSyntaxChar(kJSONStringDelimiter);

  std::string json("\"");
  uint8_t ch;
  str.clear();
  while (true) {
    ch = reader_.read();
    ++result;
    if (ch == kJSONStringDelimiter) {
      break;
    }
    if (ch == kJSONBackslash) {
      ch = reader_.read();
      ++result;
      if (ch == kJSONEscapeChar) {
        if (allowDecodeUTF8_) {
          json += "\\u";
          continue;
        } else {
          result += readJSONEscapeChar(&ch);
        }
      } else {
        size_t pos = kEscapeChars.find(ch);
        if (pos == std::string::npos) {
          throw TProtocolException(
              TProtocolException::INVALID_DATA,
              "Expected control char, got '" +
                  std::string((const char*)&ch, 1) + "'.");
        }
        if (allowDecodeUTF8_) {
          json += "\\";
          json += kEscapeChars[pos];
          continue;
        } else {
          ch = kEscapeCharVals[pos];
        }
      }
    }

    if (allowDecodeUTF8_) {
      json += ch;
    } else {
      str += ch;
    }
  }

  if (allowDecodeUTF8_) {
    json += "\"";
    folly::dynamic parsed = folly::parseJson(json);
    str += parsed.getString();
  }

  return result;
}

// Reads a block of base64 characters, decoding it, and returns via str
uint32_t TJSONProtocol::readJSONBase64(std::string& str) {
  std::string tmp;
  uint32_t result = readJSONString(tmp);
  uint8_t* b = (uint8_t*)tmp.c_str();
  uint32_t len = tmp.length();
  str.clear();
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
  return result;
}

// Reads a sequence of characters, stopping at the first one that is not
// a valid JSON numeric character.
uint32_t TJSONProtocol::readJSONNumericChars(std::string& str) {
  uint32_t result = 0;
  str.clear();
  while (reader_.canPeek()) {
    uint8_t ch = reader_.peek();
    if (!isJSONNumeric(ch)) {
      break;
    }
    reader_.read();
    str += ch;
    ++result;
  }
  return result;
}

uint32_t TJSONProtocol::readJSONBool(bool& value) {
  uint32_t result = context_->read(reader_);

  if (context_->escapeNum()) {
    result += readJSONSyntaxChar(kJSONStringDelimiter);
  }

  uint8_t ch = reader_.read();
  ++result;

  if (ch == kJSONTrue.at(0)) {
    for (size_t i = 1; i < kJSONTrue.length(); ++i) {
      result += readJSONSyntaxChar(kJSONTrue.at(i));
    }
    value = true;

  } else if (ch == kJSONFalse.at(0)) {
    for (size_t i = 1; i < kJSONFalse.length(); ++i) {
      result += readJSONSyntaxChar(kJSONFalse.at(i));
    }
    value = false;

  } else {
    throw TProtocolException(
        TProtocolException::INVALID_DATA,
        "Expected 't' or 'f'; got '" + std::string((char*)&ch, 1) + "'.");
  }

  if (context_->escapeNum()) {
    result += readJSONSyntaxChar(kJSONStringDelimiter);
  }

  return result;
}

// Reads a sequence of characters and assembles them into a number,
// returning them via num
template <typename NumberType>
uint32_t TJSONProtocol::readJSONInteger(NumberType& num) {
  uint32_t result = context_->read(reader_);
  if (context_->escapeNum()) {
    result += readJSONSyntaxChar(kJSONStringDelimiter);
  }
  std::string str;
  result += readJSONNumericChars(str);
  try {
    num = folly::to<NumberType>(str);
  } catch (const std::exception&) {
    throw TProtocolException(
        TProtocolException::INVALID_DATA,
        "Expected numeric value; got \"" + str + "\"");
  }
  if (context_->escapeNum()) {
    result += readJSONSyntaxChar(kJSONStringDelimiter);
  }
  return result;
}

// Reads a JSON number or string and interprets it as a double.
template <typename NumberType>
uint32_t TJSONProtocol::readJSONDouble(NumberType& num) {
  uint32_t result = context_->read(reader_);
  std::string str;
  if (reader_.peek() == kJSONStringDelimiter) {
    result += readJSONString(str, true);
    // Check for NaN, Infinity and -Infinity
    if (str == kThriftNan) {
      num = HUGE_VAL / HUGE_VAL; // generates NaN
    } else if (str == kThriftNegativeNan) {
      num = -NAN;
    } else if (str == kThriftInfinity) {
      num = HUGE_VAL;
    } else if (str == kThriftNegativeInfinity) {
      num = -HUGE_VAL;
    } else {
      if (!context_->escapeNum()) {
        // Throw exception -- we should not be in a string in this case
        throw TProtocolException(
            TProtocolException::INVALID_DATA,
            "Numeric data unexpectedly quoted");
      }
      try {
        num = folly::to<NumberType>(str);
      } catch (const std::exception&) {
        throw TProtocolException(
            TProtocolException::INVALID_DATA,
            "Expected numeric value; got \"" + str + "\"");
      }
    }
  } else {
    if (context_->escapeNum()) {
      // This will throw - we should have had a quote if escapeNum == true
      readJSONSyntaxChar(kJSONStringDelimiter);
    }
    result += readJSONNumericChars(str);
    try {
      num = folly::to<NumberType>(str);
    } catch (const std::exception&) {
      throw TProtocolException(
          TProtocolException::INVALID_DATA,
          "Expected numeric value; got \"" + str + "\"");
    }
  }
  return result;
}

uint32_t TJSONProtocol::readJSONObjectStart() {
  uint32_t result = context_->read(reader_);
  result += readJSONStructuralChar(kJSONObjectStart);
  pushContext(std::shared_ptr<TJSONContext>(new JSONPairContext()));
  return result;
}

uint32_t TJSONProtocol::readJSONObjectEnd() {
  uint32_t result = readJSONStructuralChar(kJSONObjectEnd);
  popContext();
  return result;
}

uint32_t TJSONProtocol::readJSONArrayStart() {
  uint32_t result = context_->read(reader_);
  result += readJSONStructuralChar(kJSONArrayStart);
  pushContext(std::shared_ptr<TJSONContext>(new JSONListContext()));
  return result;
}

uint32_t TJSONProtocol::readJSONArrayEnd() {
  uint32_t result = readJSONStructuralChar(kJSONArrayEnd);
  popContext();
  return result;
}

uint32_t TJSONProtocol::readMessageBegin(
    std::string& name, TMessageType& messageType, int32_t& seqid) {
  uint32_t result = readJSONArrayStart();
  uint64_t tmpVal = 0;
  result += readJSONInteger(tmpVal);
  if (tmpVal != kThriftVersion1) {
    throw TProtocolException(
        TProtocolException::BAD_VERSION, "Message contained bad version.");
  }
  result += readJSONString(name);
  result += readJSONInteger(tmpVal);
  messageType = (TMessageType)tmpVal;
  result += readJSONInteger(tmpVal);
  seqid = tmpVal;
  return result;
}

uint32_t TJSONProtocol::readMessageEnd() {
  return readJSONArrayEnd();
}

uint32_t TJSONProtocol::readStructBegin(std::string& /*name*/) {
  return readJSONObjectStart();
}

uint32_t TJSONProtocol::readStructEnd() {
  return readJSONObjectEnd();
}

uint32_t TJSONProtocol::readFieldBegin(
    std::string& /*name*/, TType& fieldType, int16_t& fieldId) {
  uint32_t result = 0;
  result += skipJSONWhitespace();
  // Check if we hit the end of the list
  uint8_t ch = reader_.peek();
  if (ch == kJSONObjectEnd) {
    fieldType = apache::thrift::protocol::T_STOP;
  } else {
    uint64_t tmpVal = 0;
    std::string tmpStr;
    result += readJSONInteger(tmpVal);
    fieldId = tmpVal;
    result += readJSONObjectStart();
    result += readJSONString(tmpStr);
    fieldType = getTypeIDForTypeName(tmpStr);
  }
  return result;
}

uint32_t TJSONProtocol::readFieldEnd() {
  return readJSONObjectEnd();
}

uint32_t TJSONProtocol::readMapBegin(
    TType& keyType, TType& valType, uint32_t& size, bool& sizeUnknown) {
  uint64_t tmpVal = 0;
  std::string tmpStr;
  uint32_t result = readJSONArrayStart();
  result += readJSONString(tmpStr);
  keyType = getTypeIDForTypeName(tmpStr);
  result += readJSONString(tmpStr);
  valType = getTypeIDForTypeName(tmpStr);
  result += readJSONInteger(tmpVal);
  size = tmpVal;
  result += readJSONObjectStart();
  sizeUnknown = false;
  return result;
}

uint32_t TJSONProtocol::readMapEnd() {
  return readJSONObjectEnd() + readJSONArrayEnd();
}

uint32_t TJSONProtocol::readListBegin(
    TType& elemType, uint32_t& size, bool& sizeUnknown) {
  uint64_t tmpVal = 0;
  std::string tmpStr;
  uint32_t result = readJSONArrayStart();
  result += readJSONString(tmpStr);
  elemType = getTypeIDForTypeName(tmpStr);
  result += readJSONInteger(tmpVal);
  size = tmpVal;
  sizeUnknown = false;
  return result;
}

uint32_t TJSONProtocol::readListEnd() {
  return readJSONArrayEnd();
}

uint32_t TJSONProtocol::readSetBegin(
    TType& elemType, uint32_t& size, bool& sizeUnknown) {
  uint64_t tmpVal = 0;
  std::string tmpStr;
  uint32_t result = readJSONArrayStart();
  result += readJSONString(tmpStr);
  elemType = getTypeIDForTypeName(tmpStr);
  result += readJSONInteger(tmpVal);
  size = tmpVal;
  sizeUnknown = false;
  return result;
}

uint32_t TJSONProtocol::readSetEnd() {
  return readJSONArrayEnd();
}

uint32_t TJSONProtocol::readBool(bool& value) {
  return readJSONInteger(value);
}

// readByte() must be handled properly because folly::to sees int8_t
// as a text type instead of an integer type
uint32_t TJSONProtocol::readByte(int8_t& byte) {
  int16_t tmp = (int16_t)byte;
  uint32_t result = readJSONInteger(tmp);

  if (tmp < -128 || tmp > 127) {
    throw TProtocolException(
        TProtocolException::INVALID_DATA,
        folly::to<std::string>("Expected numeric value; got \"", tmp, "\""));
  }

  byte = (int8_t)tmp;
  return result;
}

uint32_t TJSONProtocol::readI16(int16_t& i16) {
  return readJSONInteger(i16);
}

uint32_t TJSONProtocol::readI32(int32_t& i32) {
  return readJSONInteger(i32);
}

uint32_t TJSONProtocol::readI64(int64_t& i64) {
  return readJSONInteger(i64);
}

uint32_t TJSONProtocol::readDouble(double& dub) {
  return readJSONDouble(dub);
}

uint32_t TJSONProtocol::readFloat(float& flt) {
  return readJSONDouble(flt);
}

uint32_t TJSONProtocol::readString(std::string& str) {
  return readJSONString(str);
}

uint32_t TJSONProtocol::readBinary(std::string& str) {
  return readJSONBase64(str);
}

} // namespace protocol
} // namespace thrift
} // namespace apache
