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

#include <thrift/lib/cpp/protocol/TSimpleJSONProtocol.h>

using namespace apache::thrift::transport;
using namespace apache::thrift::reflection;

namespace apache::thrift::protocol {

TSimpleJSONProtocol::TSimpleJSONProtocol(std::shared_ptr<TTransport> ptrans)
    : TVirtualProtocol<TSimpleJSONProtocol, TJSONProtocol>(ptrans),
      nextType_(nullptr),
      numSkipped_(0) {}

TSimpleJSONProtocol::TSimpleJSONProtocol(TTransport* ptrans)
    : TVirtualProtocol<TSimpleJSONProtocol, TJSONProtocol>(ptrans),
      nextType_(nullptr),
      numSkipped_(0) {}

TSimpleJSONProtocol::~TSimpleJSONProtocol() {}

/**
 * Writing functions.
 */

Schema* TSimpleJSONProtocol::getSchema() {
  return &schema_;
}

uint32_t TSimpleJSONProtocol::writeFieldBegin(
    const char* name, const TType /*fieldType*/, const int16_t /*fieldId*/) {
  return writeJSONString(name);
}

uint32_t TSimpleJSONProtocol::writeFieldEnd() {
  return 0;
}

uint32_t TSimpleJSONProtocol::writeMapBegin(
    const TType /*keyType*/, const TType /*valType*/, const uint32_t /*size*/) {
  return writeJSONObjectStart();
}

uint32_t TSimpleJSONProtocol::writeMapEnd() {
  return writeJSONObjectEnd();
}

uint32_t TSimpleJSONProtocol::writeListBegin(
    const TType /*elemType*/, const uint32_t /*size*/) {
  return writeJSONArrayStart();
}

uint32_t TSimpleJSONProtocol::writeSetBegin(
    const TType /*elemType*/, const uint32_t /*size*/) {
  return writeJSONArrayStart();
}

uint32_t TSimpleJSONProtocol::writeBool(const bool value) {
  return writeJSONBool(value);
}

/**
 * Reading functions
 */

void TSimpleJSONProtocol::setNextStructType(uint64_t reflection_id) {
  nextType_ = getDataTypeFromTypeNum(reflection_id);
}

uint32_t TSimpleJSONProtocol::readStructBegin(std::string& name) {
  uint32_t result =
      TVirtualProtocol<TSimpleJSONProtocol, TJSONProtocol>::readStructBegin(
          name);
  enterType();
  return result;
}

uint32_t TSimpleJSONProtocol::readStructEnd() {
  uint32_t result =
      TVirtualProtocol<TSimpleJSONProtocol, TJSONProtocol>::readStructEnd();
  exitType();
  return result;
}

uint32_t TSimpleJSONProtocol::readFieldBegin(
    std::string& /*name*/, TType& fieldType, int16_t& fieldId) {
  uint32_t result = 0;

  auto currentType = getCurrentDataType();

  skipWhitespace();
  result += getNumSkippedChars();

  // Check if we hit the end of the list
  uint8_t ch = reader_.peek();
  if (ch == kJSONObjectEnd) {
    fieldType = T_STOP;
    return result;
  }

  std::string tmpStr;
  result += readJSONString(tmpStr);

  if (currentType != nullptr) {
    auto fields = currentType->fields();
    if (!fields) {
      throw TProtocolException(
          TProtocolException::INVALID_DATA,
          "Expected a struct type, but actually not a struct");
    }

    // find the corresponding StructField object and field id of the field
    for (auto ite = fields->begin(); ite != fields->end(); ++ite) {
      if (*ite->second.name() == tmpStr) {
        fieldId = ite->first;
        fieldType = getTypeIdFromTypeNum(*ite->second.type());

        // set the nextType_ if the field type is a compound type
        // e.g. list<i64>, mySimpleStruct
        //
        // this is not really necessary, because before calling
        // readStructBegin(), setNextStructType() should be called
        // but this allows only calling setNextStructType() on the
        // base type
        auto& field = ite->second;
        if (isCompoundType(*field.type())) {
          nextType_ = getDataTypeFromTypeNum(*field.type());
        }

        return result;
      }
    }
  }

  // if the field is not found or
  // the entire struct is being skipped

  fieldId = 0;

  skipWhitespace();
  uint8_t delimiter = reader_.read(); // delimiter should be ':'

  fieldType = guessTypeIdFromFirstByte();

  bool wasPutBack = reader_.put(delimiter);
  (void)wasPutBack;

  assert(wasPutBack);

  return result + getNumSkippedChars();
}

uint32_t TSimpleJSONProtocol::readFieldEnd() {
  return 0;
}

uint32_t TSimpleJSONProtocol::readMapBegin(
    TType& keyType, TType& valType, uint32_t& size, bool& sizeUnknown) {
  enterType();

  auto currentType = getCurrentDataType();
  bool beingSkipped = (currentType == nullptr);
  (void)beingSkipped;

  // since we never guess an unknown field to have a map type
  // we should never arrive here
  assert(!beingSkipped);

  auto keyTypeNum = currentType->mapKeyType().value_or(0);
  auto valTypeNum = currentType->valueType().value_or(0);
  keyType = getTypeIdFromTypeNum(keyTypeNum);
  valType = getTypeIdFromTypeNum(valTypeNum);
  size = 0;
  sizeUnknown = true;

  if (isCompoundType(keyTypeNum)) {
    nextType_ = getDataTypeFromTypeNum(keyTypeNum);
  } else if (isCompoundType(valTypeNum)) {
    nextType_ = getDataTypeFromTypeNum(valTypeNum);
  }

  return readJSONObjectStart();
}

bool TSimpleJSONProtocol::peekMap() {
  skipWhitespace();
  return reader_.peek() != kJSONObjectEnd;
}

uint32_t TSimpleJSONProtocol::readMapEnd() {
  uint32_t result = getNumSkippedChars() + readJSONObjectEnd();
  exitType();
  return result;
}

uint32_t TSimpleJSONProtocol::readListBegin(
    TType& elemType, uint32_t& size, bool& sizeUnknown) {
  enterType();

  auto currentType = getCurrentDataType();
  bool beingSkipped = (currentType == nullptr);

  if (beingSkipped) {
    uint32_t result = readJSONArrayStart();
    elemType = guessTypeIdFromFirstByte();
    size = 0;
    sizeUnknown = true;
    return result + getNumSkippedChars();

  } else {
    auto elemTypeNum = currentType->valueType().value_or(0);
    elemType = getTypeIdFromTypeNum(elemTypeNum);
    size = 0;
    sizeUnknown = true;

    if (isCompoundType(elemTypeNum)) {
      nextType_ = getDataTypeFromTypeNum(elemTypeNum);
    }

    return readJSONArrayStart();
  }
}

bool TSimpleJSONProtocol::peekList() {
  skipWhitespace();
  return reader_.peek() != kJSONArrayEnd;
}

uint32_t TSimpleJSONProtocol::readListEnd() {
  uint32_t result = getNumSkippedChars();
  result += TVirtualProtocol<TSimpleJSONProtocol, TJSONProtocol>::readListEnd();
  exitType();
  return result;
}

uint32_t TSimpleJSONProtocol::readSetBegin(
    TType& elemType, uint32_t& size, bool& sizeUnknown) {
  enterType();

  auto currentType = getCurrentDataType();
  bool beingSkipped = (currentType == nullptr);
  (void)beingSkipped;

  // since we never guess an unknown field to have a set type
  // we should never arrive here
  assert(!beingSkipped);

  auto elemTypeNum = currentType->valueType().value_or(0);
  elemType = getTypeIdFromTypeNum(elemTypeNum);
  size = 0;
  sizeUnknown = true;

  if (isCompoundType(elemTypeNum)) {
    nextType_ = getDataTypeFromTypeNum(elemTypeNum);
  }

  return readJSONArrayStart();
}

bool TSimpleJSONProtocol::peekSet() {
  skipWhitespace();
  return reader_.peek() != kJSONArrayEnd;
}

uint32_t TSimpleJSONProtocol::readSetEnd() {
  uint32_t result = getNumSkippedChars();
  result += TVirtualProtocol<TSimpleJSONProtocol, TJSONProtocol>::readSetEnd();
  exitType();
  return result;
}

uint32_t TSimpleJSONProtocol::readBool(bool& value) {
  return readJSONBool(value);
}

TType TSimpleJSONProtocol::getTypeIdFromTypeNum(int64_t fieldType) {
  Type type = getType(fieldType);
  switch (type) {
    case Type::TYPE_VOID:
      return T_VOID;
    case Type::TYPE_STRING:
      return T_STRING;
    case Type::TYPE_BOOL:
      return T_BOOL;
    case Type::TYPE_BYTE:
      return T_BYTE;
    case Type::TYPE_I16:
      return T_I16;
    case Type::TYPE_I32:
      return T_I32;
    case Type::TYPE_I64:
      return T_I64;
    case Type::TYPE_DOUBLE:
      return T_DOUBLE;
    case Type::TYPE_FLOAT:
      return T_FLOAT;
    case Type::TYPE_LIST:
      return T_LIST;
    case Type::TYPE_SET:
      return T_SET;
    case Type::TYPE_MAP:
      return T_MAP;
    case Type::TYPE_STRUCT:
      return T_STRUCT;
    case Type::TYPE_ENUM:
      return T_I32;
    case Type::TYPE_SERVICE:
    case Type::TYPE_PROGRAM:
    default:
      throw TProtocolException(
          TProtocolException::NOT_IMPLEMENTED, "Unrecognized type");
  }
}

/**
 * Given a byte, peeked from the beginning of some JSON value, determine a type
 * of that value. Result type is used to decide how to skip that value, so
 * differences between compatible types don't matter and the more general type
 * is assumed.
 * STOP is returned for } and ] to indicate end of collection.
 */
TType TSimpleJSONProtocol::guessTypeIdFromFirstByte() {
  skipWhitespace();
  uint8_t byte = reader_.peek();

  if (byte == kJSONObjectEnd || byte == kJSONArrayEnd) {
    return T_STOP;
  } else if (byte == kJSONStringDelimiter) {
    return T_STRING;
  } else if (byte == kJSONObjectStart) {
    return T_STRUCT;
  } else if (byte == kJSONArrayStart) {
    return T_LIST;
  } else if (byte == kJSONTrue[0] || byte == kJSONFalse[0]) {
    return T_BOOL;
  } else if (
      byte == '+' || byte == '-' || byte == '.' || byte == '0' || byte == '1' ||
      byte == '2' || byte == '3' || byte == '4' || byte == '5' || byte == '6' ||
      byte == '7' || byte == '8' || byte == '9') {
    return T_DOUBLE;
  } else {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "Unrecognized byte: " + std::string((char*)&byte, 1));
  }
}

DataType* TSimpleJSONProtocol::getDataTypeFromTypeNum(int64_t typeNum) {
  auto ite = schema_.dataTypes()->find(typeNum);

  if (ite == schema_.dataTypes()->cend()) {
    throw TProtocolException(
        TProtocolException::INVALID_DATA,
        "Type id not found, schema is corrupted");
  }

  return &(ite->second);
}

void TSimpleJSONProtocol::enterType() {
  typeStack_.push(nextType_);
  nextType_ = nullptr;
}

void TSimpleJSONProtocol::exitType() {
  auto lastType = getCurrentDataType();
  typeStack_.pop();

  auto currentType = getCurrentDataType();
  if (!currentType) {
    nextType_ = nullptr;
    return;
  }

  auto mapKeyType = currentType->mapKeyType();
  auto valueType = currentType->valueType();
  if (mapKeyType && isCompoundType(*mapKeyType) &&
      (!valueType || !isCompoundType(*valueType) ||
       lastType == getDataTypeFromTypeNum(*valueType))) {
    nextType_ = getDataTypeFromTypeNum(*mapKeyType);
  } else if (valueType && isCompoundType(*valueType)) {
    nextType_ = getDataTypeFromTypeNum(*valueType);
  } else {
    nextType_ = nullptr;
  }
}

// returns nullptr when the struct is to be skipped
const DataType* TSimpleJSONProtocol::getCurrentDataType() {
  if (!typeStack_.empty()) {
    return typeStack_.top();
  } else {
    return nullptr;
  }
}

void TSimpleJSONProtocol::skipWhitespace() {
  numSkipped_ += this->skipJSONWhitespace();
}

uint32_t TSimpleJSONProtocol::getNumSkippedChars() {
  auto temp = numSkipped_;
  numSkipped_ = 0;
  return temp;
}

bool TSimpleJSONProtocol::isCompoundType(int64_t fieldType) {
  return !isBaseType(getType(fieldType));
}

} // namespace apache::thrift::protocol
