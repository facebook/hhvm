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

#include <thrift/lib/cpp2/protocol/detail/Json5ProtocolReader.h>

#include <limits>
#include <optional>
#include <stdexcept>

#include <re2/re2.h>
#include <folly/Exception.h>
#include <folly/base64.h>

namespace apache::thrift::json5::detail {

[[noreturn]] void Json5ProtocolReader::throwError(std::string_view message) {
  throw std::runtime_error(fmt::format("Json5ProtocolReader: {}", message));
}

// ============================================================================
// Cursor Methods
// ============================================================================

void Json5ProtocolReader::setInput(const folly::io::Cursor& cursor) {
  reader_.setCursor(cursor);
}

void Json5ProtocolReader::setInput(const folly::IOBuf* buf) {
  reader_.setCursor(folly::io::Cursor(buf));
}

const folly::io::Cursor& Json5ProtocolReader::getCursor() const {
  return reader_.getCursor();
}

size_t Json5ProtocolReader::getCursorPosition() const {
  return reader_.getCursor().getCurrentPosition();
}

// ============================================================================
// Value Lifecycle Helpers
// ============================================================================

void Json5ProtocolReader::beginReadValue() {
  auto* map = containerStack_.mapState();
  if (map == nullptr ||
      map->encodedAs != CompoundTypeTracker::MapForm::KeyValueArray) {
    return;
  }

  switch (map->expectingToken) {
    case CompoundTypeTracker::MapToken::Key: {
      // We are reading a complex key, we need to read `{"key":`
      //
      //   [{"key":              [{"key":
      //    ^            =>              ^
      // before                       after
      reader_.readObjectBegin();
      auto name = reader_.readObjectName();
      if (name != "key") {
        throwError("expected 'key' in map array form, got '" + name + "'");
      }
      break;
    }
    case CompoundTypeTracker::MapToken::Value: {
      // We are reading a value after a complex key, we need to read `"value":`
      //
      //   [{"key": ..., "value":              [{"key": ..., "value":
      //                 ^             =>                            ^
      //              before                                      after
      auto name = reader_.readObjectName();
      if (name != "value") {
        throwError("expected 'value' in map array form, got '" + name + "'");
      }
      break;
    }
  }
}

void Json5ProtocolReader::endReadValue() {
  auto* map = containerStack_.mapState();
  if (map == nullptr) {
    return;
  }

  if (map->encodedAs == CompoundTypeTracker::MapForm::KeyValueArray &&
      map->expectingToken == CompoundTypeTracker::MapToken::Value) {
    // We read a value after a complex key, we need to read `}`:
    //
    //   [{"key": ..., "value": ...}         [{"key": ..., "value": ...},
    //                             ^    =>                               ^
    //                          before                                 after
    reader_.readObjectEnd();
  }

  containerStack_.toggleExpectingKeyValue();
}

std::optional<std::string> Json5ProtocolReader::tryReadObjectMapKey() {
  if (!containerStack_.inObjectMapExpectingKey()) {
    return std::nullopt;
  }
  auto name = reader_.readObjectName();
  containerStack_.toggleExpectingKeyValue();
  return name;
}

// ============================================================================
// Struct Methods
// ============================================================================

void Json5ProtocolReader::readStructBegin(std::string& /*name*/) {
  beginReadValue();
  containerStack_.beginStruct();
  reader_.readObjectBegin();
}

void Json5ProtocolReader::readStructEnd() {
  containerStack_.endStruct();
  reader_.readObjectEnd();
  endReadValue();
}

void Json5ProtocolReader::readFieldBegin(
    std::string& name, protocol::TType& fieldType, std::int16_t& fieldId) {
  if (reader_.peekToken() == Json5Reader::Token::ObjectEnd) {
    fieldType = protocol::T_STOP;
    fieldId = 0;
    return;
  }

  // Similar to SimpleJSONProtocol: assign dummy fieldId and fieldType since
  // they are not available from JSON field names.
  fieldId = std::numeric_limits<std::int16_t>::min();
  fieldType = protocol::T_VOID;
  name = reader_.readObjectName();
}

void Json5ProtocolReader::readFieldEnd() {}

// ============================================================================
// Map Methods
// ============================================================================

void Json5ProtocolReader::readMapBegin(
    protocol::TType& /*keyType*/,
    protocol::TType& /*valType*/,
    std::uint32_t& size) {
  beginReadValue();
  size = std::numeric_limits<std::uint32_t>::max();

  auto token = reader_.peekToken();
  if (token == Json5Reader::Token::ObjectBegin) {
    reader_.readObjectBegin();
    containerStack_.beginMap(CompoundTypeTracker::MapForm::Object);
  } else if (token == Json5Reader::Token::ListBegin) {
    reader_.readListBegin();
    containerStack_.beginMap(CompoundTypeTracker::MapForm::KeyValueArray);
  } else {
    throwError("expected map (object or array)");
  }
}

void Json5ProtocolReader::readMapEnd() {
  auto* map = containerStack_.mapState();
  CHECK_THROW(map != nullptr, std::logic_error);

  // Map must end after a complete key-value pair (i.e., expecting next key).
  CHECK_THROW(
      map->expectingToken == CompoundTypeTracker::MapToken::Key,
      std::logic_error);

  auto encodedAs = map->encodedAs;
  containerStack_.endMap();
  if (encodedAs == CompoundTypeTracker::MapForm::KeyValueArray) {
    reader_.readListEnd();
  } else {
    reader_.readObjectEnd();
  }
  endReadValue();
}

// ============================================================================
// List Methods
// ============================================================================

void Json5ProtocolReader::readListBegin(
    protocol::TType& /*elemType*/, std::uint32_t& size) {
  beginReadValue();
  size = std::numeric_limits<std::uint32_t>::max();
  containerStack_.beginList();
  reader_.readListBegin();
}

void Json5ProtocolReader::readListEnd() {
  containerStack_.endList();
  reader_.readListEnd();
  endReadValue();
}

// ============================================================================
// Set Methods
// ============================================================================

void Json5ProtocolReader::readSetBegin(
    protocol::TType& /*elemType*/, std::uint32_t& size) {
  beginReadValue();
  size = std::numeric_limits<std::uint32_t>::max();
  containerStack_.beginSet();
  reader_.readListBegin();
}

void Json5ProtocolReader::readSetEnd() {
  containerStack_.endSet();
  reader_.readListEnd();
  endReadValue();
}

// ============================================================================
// Peek Methods
// ============================================================================

bool Json5ProtocolReader::peekMap() {
  auto* map = containerStack_.mapState();
  if (map == nullptr) {
    return false;
  }
  auto endToken = map->encodedAs == CompoundTypeTracker::MapForm::Object
      ? Json5Reader::Token::ObjectEnd
      : Json5Reader::Token::ListEnd;
  return reader_.peekToken() != endToken;
}

bool Json5ProtocolReader::peekList() {
  return reader_.peekToken() != Json5Reader::Token::ListEnd;
}

bool Json5ProtocolReader::peekSet() {
  return peekList();
}

// ============================================================================
// Skip Methods
// ============================================================================

void Json5ProtocolReader::skip(protocol::TType type, int depth) {
  apache::thrift::skip(*this, type, depth);
}

[[noreturn]] void Json5ProtocolReader::skipBytes(size_t /*bytes*/) {
  throwError("not supported");
}

// ============================================================================
// Enum Parsing Helpers
// ============================================================================

Json5ProtocolReader::EnumReadResult Json5ProtocolReader::readEnumImpl() {
  // When reading enum as a map key in Object form, the enum value is the
  // JSON object property name (a string), not a JSON value.
  if (auto key = tryReadObjectMapKey()) {
    return parseEnumString(std::move(*key));
  }

  beginReadValue();
  auto primitive =
      reader_.readPrimitive(Json5Reader::FloatingPointPrecision::Double);
  endReadValue();

  if (auto* i = std::get_if<std::int64_t>(&primitive)) {
    return {.name = {}, .value = folly::to<std::int32_t>(*i)};
  }

  return parseEnumString(std::get<std::string>(primitive));
}

Json5ProtocolReader::EnumReadResult Json5ProtocolReader::parseEnumString(
    std::string s) {
  EnumReadResult ret;

  // matches "NAME (value)" or "(value)" format
  static const re2::RE2 kNameValuePattern = R"(^(\w*)\s*\((-?\d+)\)$)";
  if (re2::RE2::FullMatch(
          s, kNameValuePattern, &ret.name, &ret.value.emplace())) {
    return ret;
  }

  // matches bare identifier "NAME"
  static const re2::RE2 kNameOnlyPattern = R"(^\w+$)";
  if (re2::RE2::FullMatch(s, kNameOnlyPattern)) {
    return {.name = std::move(s)};
  }

  throwError(fmt::format("invalid enum value: '{}'", s));
}

// ============================================================================
// Internal Value Reading Helpers
// ============================================================================

std::string Json5ProtocolReader::readStringValue() {
  if (auto key = tryReadObjectMapKey()) {
    return std::move(*key);
  }

  beginReadValue();
  auto primitive =
      reader_.readPrimitive(Json5Reader::FloatingPointPrecision::Double);
  endReadValue();

  return std::get<std::string>(std::move(primitive));
}

std::string Json5ProtocolReader::readBinaryValue() {
  beginReadValue();
  std::string name, value;
  if (reader_.peekToken() == Json5Reader::Token::ObjectBegin) {
    reader_.readObjectBegin();
    name = reader_.readObjectName();
    auto primitive =
        reader_.readPrimitive(Json5Reader::FloatingPointPrecision::Double);
    value = std::get<std::string>(std::move(primitive));
    reader_.readObjectEnd();
  } else {
    // folly::base64URLDecode accepts both standard and url-safe base64 string.
    name = "base64url";
    auto primitive =
        reader_.readPrimitive(Json5Reader::FloatingPointPrecision::Double);
    value = std::get<std::string>(std::move(primitive));
  }
  endReadValue();

  if (name == "utf-8") {
    return value;
  } else if (name == "base64url") {
    return folly::base64URLDecode(value);
  } else if (name == "base64") {
    while (value.size() % 4 != 0) {
      // We want to support unpadded base64 string. Unfortunately there is no
      // folly function to support base64 decode without padding. We have to add
      // padding manually.
      value += '=';
    }
    return folly::base64Decode(value);
  }
  throwError("Unsupported encoding type for binary object: " + name);
}

namespace {

std::optional<std::int64_t> tryParseI64(
    const Json5Reader::Primitive& primitive) {
  if (auto* i = std::get_if<std::int64_t>(&primitive)) {
    return *i;
  }
  if (auto* s = std::get_if<std::string>(&primitive)) {
    if (auto i = folly::tryTo<std::int64_t>(*s)) {
      return *i;
    }
  }
  return std::nullopt;
}

} // namespace

std::int64_t Json5ProtocolReader::readIntegralValue() {
  if (auto key = tryReadObjectMapKey()) {
    return folly::to<std::int64_t>(*key);
  }

  beginReadValue();
  auto primitive =
      reader_.readPrimitive(Json5Reader::FloatingPointPrecision::Double);
  endReadValue();

  if (auto i = tryParseI64(primitive)) {
    return *i;
  }
  throwError("expected integer value");
}

template <std::floating_point T>
T Json5ProtocolReader::readFloatingPointValue() {
  constexpr auto precision = std::is_same_v<T, float>
      ? Json5Reader::FloatingPointPrecision::Single
      : Json5Reader::FloatingPointPrecision::Double;

  if (auto key = tryReadObjectMapKey()) {
    return folly::to<T>(*key);
  }

  beginReadValue();
  auto primitive = reader_.readPrimitive(precision);
  endReadValue();

  if (auto* f = std::get_if<T>(&primitive)) {
    return *f;
  }
  if (auto i = tryParseI64(primitive)) {
    T result = static_cast<T>(*i);
    if (static_cast<std::int64_t>(result) != *i) {
      throwError("precision loss converting integer to floating point");
    }
    return result;
  }
  return folly::to<T>(std::get<std::string>(primitive));
}

// ============================================================================
// Primitive Type Read Methods
// ============================================================================

void Json5ProtocolReader::readBool(bool& value) {
  auto parseBool = [](const std::string& s) {
    if (s == "true") {
      return true;
    }
    if (s == "false") {
      return false;
    }
    throwError("expected boolean, got '" + s + "'");
  };

  if (auto key = tryReadObjectMapKey()) {
    value = parseBool(*key);
    return;
  }

  beginReadValue();
  auto primitive =
      reader_.readPrimitive(Json5Reader::FloatingPointPrecision::Double);
  endReadValue();

  if (auto* b = std::get_if<bool>(&primitive)) {
    value = *b;
    return;
  }
  value = parseBool(std::get<std::string>(primitive));
}

void Json5ProtocolReader::readBool(std::vector<bool>::reference value) {
  bool tmp = false;
  readBool(tmp);
  value = tmp;
}

void Json5ProtocolReader::readByte(std::int8_t& value) {
  value = folly::to<std::int8_t>(readIntegralValue());
}

void Json5ProtocolReader::readI16(std::int16_t& value) {
  value = folly::to<std::int16_t>(readIntegralValue());
}

void Json5ProtocolReader::readI32(std::int32_t& value) {
  value = folly::to<std::int32_t>(readIntegralValue());
}

void Json5ProtocolReader::readI64(std::int64_t& value) {
  value = readIntegralValue();
}

void Json5ProtocolReader::readFloat(float& value) {
  value = readFloatingPointValue<float>();
}

void Json5ProtocolReader::readDouble(double& value) {
  value = readFloatingPointValue<double>();
}

// ============================================================================
// Binary/String Read Methods
// ============================================================================

void Json5ProtocolReader::readBinary(std::unique_ptr<folly::IOBuf>& str) {
  str = folly::IOBuf::copyBuffer(readBinaryValue());
}

void Json5ProtocolReader::readBinary(folly::IOBuf& str) {
  str = folly::IOBuf(folly::IOBuf::CopyBufferOp{}, readBinaryValue());
}

} // namespace apache::thrift::json5::detail
