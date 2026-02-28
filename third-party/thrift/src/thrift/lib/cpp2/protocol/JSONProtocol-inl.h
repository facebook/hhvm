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
#include <folly/portability/Constexpr.h>

namespace apache {
namespace thrift {

namespace detail {
namespace json {

static constexpr folly::StringPiece kTypeNameBool("tf");
static constexpr folly::StringPiece kTypeNameByte("i8");
static constexpr folly::StringPiece kTypeNameI16("i16");
static constexpr folly::StringPiece kTypeNameI32("i32");
static constexpr folly::StringPiece kTypeNameI64("i64");
static constexpr folly::StringPiece kTypeNameDouble("dbl");
static constexpr folly::StringPiece kTypeNameFloat("flt");
static constexpr folly::StringPiece kTypeNameStruct("rec");
static constexpr folly::StringPiece kTypeNameString("str");
static constexpr folly::StringPiece kTypeNameMap("map");
static constexpr folly::StringPiece kTypeNameList("lst");
static constexpr folly::StringPiece kTypeNameSet("set");

[[noreturn]] void throwNegativeSize(int64_t size);
[[noreturn]] void throwExceededSizeLimit(int64_t size, int64_t sizeMax);
[[noreturn]] void throwUnrecognizedType();

static folly::StringPiece getTypeNameForTypeID(TType typeID) {
  using namespace apache::thrift::protocol;
  switch (typeID) {
    case TType::T_BOOL:
      return kTypeNameBool;
    case TType::T_BYTE:
      return kTypeNameByte;
    case TType::T_I16:
      return kTypeNameI16;
    case TType::T_I32:
      return kTypeNameI32;
    case TType::T_I64:
      return kTypeNameI64;
    case TType::T_DOUBLE:
      return kTypeNameDouble;
    case TType::T_FLOAT:
      return kTypeNameFloat;
    case TType::T_STRING:
      return kTypeNameString;
    case TType::T_STRUCT:
      return kTypeNameStruct;
    case TType::T_MAP:
      return kTypeNameMap;
    case TType::T_SET:
      return kTypeNameSet;
    case TType::T_LIST:
      return kTypeNameList;
    case TType::T_STOP:
    case TType::T_VOID:
    case TType::T_U64:
    case TType::T_UTF8:
    case TType::T_UTF16:
    case TType::T_STREAM:
    default:
      throwUnrecognizedType();
  }
}

static protocol::TType getTypeIDForTypeName(folly::StringPiece name) {
  using namespace apache::thrift::protocol;
  auto fail = [] {
    throwUnrecognizedType();
    return TType::T_STOP;
  };
  if (name.size() <= 1) {
    return fail();
  }
  switch (name[0]) {
    case 'd':
      return TType::T_DOUBLE;
    case 'f':
      return TType::T_FLOAT;
    case 'i':
      switch (name[1]) {
        case '8':
          return TType::T_BYTE;
        case '1':
          return TType::T_I16;
        case '3':
          return TType::T_I32;
        case '6':
          return TType::T_I64;
        default:
          return fail();
      }
    case 'l':
      return TType::T_LIST;
    case 'm':
      return TType::T_MAP;
    case 'r':
      return TType::T_STRUCT;
    case 's':
      switch (name[1]) {
        case 't':
          return TType::T_STRING;
        case 'e':
          return TType::T_SET;
        default:
          return fail();
      }
    case 't':
      return TType::T_BOOL;
    default:
      return fail();
  }
}

static uint32_t clampSize(int64_t size) {
  if (size < 0) {
    throwNegativeSize(size);
  }
  constexpr auto sizeMax = std::numeric_limits<uint32_t>::max();
  if (size > sizeMax) {
    throwExceededSizeLimit(size, sizeMax);
  }
  return uint32_t(size);
}

} // namespace json
} // namespace detail

/*
 * Public writing methods
 */

uint32_t JSONProtocolWriter::writeStructBegin(const char* /*name*/) {
  descend();

  auto ret = writeContext();
  ret += beginContext(ContextType::MAP);
  return ret;
}

uint32_t JSONProtocolWriter::writeStructEnd() {
  auto ret = endContext();

  ascend();

  return ret;
}

uint32_t JSONProtocolWriter::writeFieldBegin(
    const char* /*name*/, TType fieldType, int16_t fieldId) {
  uint32_t ret = 0;
  ret += writeString(folly::to<std::string>(fieldId));
  ret += writeContext();
  ret += beginContext(ContextType::MAP);
  ret += writeString(
      apache::thrift::detail::json::getTypeNameForTypeID(fieldType).str());
  return ret;
}

uint32_t JSONProtocolWriter::writeFieldEnd() {
  return endContext();
}

uint32_t JSONProtocolWriter::writeFieldStop() {
  return 0;
}

uint32_t JSONProtocolWriter::writeMapBegin(
    TType keyType, TType valType, uint32_t size) {
  descend();

  auto ret = writeContext();
  ret += beginContext(ContextType::ARRAY);
  ret += writeString(
      apache::thrift::detail::json::getTypeNameForTypeID(keyType).str());
  ret += writeString(
      apache::thrift::detail::json::getTypeNameForTypeID(valType).str());
  ret += writeI32(size);
  ret += writeContext();
  ret += beginContext(ContextType::MAP);
  return ret;
}

uint32_t JSONProtocolWriter::writeMapEnd() {
  auto ret = endContext();
  ret += endContext();

  ascend();

  return ret;
}

uint32_t JSONProtocolWriter::writeListBegin(TType elemType, uint32_t size) {
  descend();

  auto ret = writeContext();
  ret += beginContext(ContextType::ARRAY);
  ret += writeString(
      apache::thrift::detail::json::getTypeNameForTypeID(elemType).str());
  ret += writeI32(size);
  return ret;
}

uint32_t JSONProtocolWriter::writeListEnd() {
  auto ret = endContext();

  ascend();

  return ret;
}

uint32_t JSONProtocolWriter::writeSetBegin(TType elemType, uint32_t size) {
  descend();

  auto ret = writeContext();
  ret += beginContext(ContextType::ARRAY);
  ret += writeString(
      apache::thrift::detail::json::getTypeNameForTypeID(elemType).str());
  ret += writeI32(size);
  return ret;
}

uint32_t JSONProtocolWriter::writeSetEnd() {
  auto ret = endContext();

  ascend();

  return ret;
}

uint32_t JSONProtocolWriter::writeBool(bool value) {
  auto ret = writeContext();
  return ret + writeJSONInt(value);
}

/**
 * Functions that return the serialized size
 */

uint32_t JSONProtocolWriter::serializedMessageSize(
    const std::string& name) const {
  return 2 // list begin and end
      + serializedSizeI32() * 3 + serializedSizeString(name);
}

uint32_t JSONProtocolWriter::serializedFieldSize(
    const char* /*name*/, TType /*fieldType*/, int16_t /*fieldId*/) const {
  // string plus ":"
  return folly::to_narrow(folly::constexpr_strlen(R"(,"32767":{"typ":})"));
}

uint32_t JSONProtocolWriter::serializedStructSize(const char* /*name*/) const {
  return folly::to_narrow(folly::constexpr_strlen(R"({})"));
}

uint32_t JSONProtocolWriter::serializedSizeMapBegin(
    TType /*keyType*/, TType /*valType*/, uint32_t /*size*/) const {
  return folly::to_narrow(
      folly::constexpr_strlen(R"(["typ","typ",4294967295,{)"));
}

uint32_t JSONProtocolWriter::serializedSizeMapEnd() const {
  return folly::to_narrow(folly::constexpr_strlen(R"(}])"));
}

uint32_t JSONProtocolWriter::serializedSizeListBegin(
    TType /*elemType*/, uint32_t /*size*/) const {
  return folly::to_narrow(folly::constexpr_strlen(R"(["typ",4294967295)"));
}

uint32_t JSONProtocolWriter::serializedSizeListEnd() const {
  return folly::to_narrow(folly::constexpr_strlen(R"(])"));
}

uint32_t JSONProtocolWriter::serializedSizeSetBegin(
    TType /*elemType*/, uint32_t /*size*/) const {
  return folly::to_narrow(folly::constexpr_strlen(R"(["typ",4294967295)"));
}

uint32_t JSONProtocolWriter::serializedSizeSetEnd() const {
  return folly::to_narrow(folly::constexpr_strlen(R"(])"));
}

uint32_t JSONProtocolWriter::serializedSizeStop() const {
  return 0;
}

uint32_t JSONProtocolWriter::serializedSizeBool(bool /*val*/) const {
  return 2;
}

/**
 * Protected reading functions
 */

/**
 * Public reading functions
 */

void JSONProtocolReader::readStructBegin(std::string& /*name*/) {
  descend();

  ensureAndBeginContext(ContextType::MAP);
}

void JSONProtocolReader::readStructEnd() {
  endContext();

  ascend();
}

void JSONProtocolReader::readFieldBegin(
    std::string& /*name*/, TType& fieldType, int16_t& fieldId) {
  skipWhitespace();

  auto peek = peekCharSafe();
  if (peek == apache::thrift::detail::json::kJSONObjectEnd) {
    fieldType = TType::T_STOP;
    fieldId = 0;
    return;
  }
  readI16(fieldId);
  ensureAndBeginContext(ContextType::MAP);
  std::string fieldTypeS;
  readString(fieldTypeS);
  fieldType = apache::thrift::detail::json::getTypeIDForTypeName(fieldTypeS);
}

void JSONProtocolReader::readFieldEnd() {
  endContext();
}

void JSONProtocolReader::readMapBegin(
    TType& keyType, TType& valType, uint32_t& size) {
  descend();

  ensureAndBeginContext(ContextType::ARRAY);
  std::string keyTypeS;
  readString(keyTypeS);
  keyType = apache::thrift::detail::json::getTypeIDForTypeName(keyTypeS);
  std::string valTypeS;
  readString(valTypeS);
  valType = apache::thrift::detail::json::getTypeIDForTypeName(valTypeS);
  int64_t sizeRead = 0;
  readI64(sizeRead);
  size = apache::thrift::detail::json::clampSize(sizeRead);
  ensureAndBeginContext(ContextType::MAP);
}

void JSONProtocolReader::readMapEnd() {
  endContext();
  endContext();

  ascend();
}

void JSONProtocolReader::readListBegin(TType& elemType, uint32_t& size) {
  descend();

  ensureAndBeginContext(ContextType::ARRAY);
  std::string elemTypeS;
  readString(elemTypeS);
  elemType = apache::thrift::detail::json::getTypeIDForTypeName(elemTypeS);
  int64_t sizeRead = 0;
  readI64(sizeRead);
  size = apache::thrift::detail::json::clampSize(sizeRead);
}

void JSONProtocolReader::readListEnd() {
  endContext();

  ascend();
}

void JSONProtocolReader::readSetBegin(TType& elemType, uint32_t& size) {
  readListBegin(elemType, size);
}

void JSONProtocolReader::readSetEnd() {
  readListEnd();
}

void JSONProtocolReader::readBool(bool& value) {
  int8_t tmp = false;
  readInContext<int8_t>(tmp);
  if (tmp < 0 || tmp > 1) {
    throwUnrecognizableAsBoolean(tmp);
  }
  value = bool(tmp);
}

void JSONProtocolReader::readBool(std::vector<bool>::reference value) {
  bool tmp = false;
  readBool(tmp);
  value = tmp;
}

bool JSONProtocolReader::peekMap() {
  return false;
}

bool JSONProtocolReader::peekSet() {
  return false;
}

bool JSONProtocolReader::peekList() {
  return false;
}

void JSONProtocolReader::skip(TType type, int depth) {
  apache::thrift::skip(*this, type, depth);
}

} // namespace thrift
} // namespace apache
