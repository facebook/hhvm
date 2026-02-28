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

#ifndef THRIFT2_PROTOCOL_TSIMPLEJSONPROTOCOL_TCC_
#define THRIFT2_PROTOCOL_TSIMPLEJSONPROTOCOL_TCC_ 1

#include <limits>

#include <thrift/lib/cpp/protocol/TBase64Utils.h>

namespace apache {
namespace thrift {

/*
 * Public writing methods
 */
uint32_t SimpleJSONProtocolWriter::writeStructBegin(const char* /*name*/) {
  descend();

  auto ret = writeContext();
  return ret + beginContext(ContextType::MAP);
}

uint32_t SimpleJSONProtocolWriter::writeStructEnd() {
  auto ret = endContext();

  ascend();

  return ret;
}

uint32_t SimpleJSONProtocolWriter::writeFieldBegin(
    const char* name, TType /*fieldType*/, int16_t /*fieldId*/) {
  auto ret = writeContext();
  return ret + writeJSONString(name);
}

uint32_t SimpleJSONProtocolWriter::writeFieldEnd() {
  return 0;
}

uint32_t SimpleJSONProtocolWriter::writeFieldStop() {
  return 0;
}

uint32_t SimpleJSONProtocolWriter::writeMapBegin(
    const TType /*keyType*/, TType /*valType*/, uint32_t /*size*/) {
  descend();

  auto ret = writeContext();
  return ret + beginContext(ContextType::MAP);
}

uint32_t SimpleJSONProtocolWriter::writeMapEnd() {
  auto ret = endContext();

  ascend();

  return ret;
}

uint32_t SimpleJSONProtocolWriter::writeListBegin(
    TType /*elemType*/, uint32_t /*size*/) {
  descend();

  auto ret = writeContext();
  return ret + beginContext(ContextType::ARRAY);
}

uint32_t SimpleJSONProtocolWriter::writeListEnd() {
  auto ret = endContext();

  ascend();

  return ret;
}

uint32_t SimpleJSONProtocolWriter::writeSetBegin(
    TType /*elemType*/, uint32_t /*size*/) {
  descend();

  auto ret = writeContext();
  return ret + beginContext(ContextType::ARRAY);
}

uint32_t SimpleJSONProtocolWriter::writeSetEnd() {
  auto ret = endContext();

  ascend();

  return ret;
}

uint32_t SimpleJSONProtocolWriter::writeBool(bool value) {
  auto ret = writeContext();
  return ret + writeJSONBool(value);
}

/**
 * Functions that return the serialized size
 */

uint32_t SimpleJSONProtocolWriter::serializedMessageSize(
    const std::string& name) const {
  return 2 // list begin and end
      + serializedSizeI32() * 3 + serializedSizeString(name);
}

uint32_t SimpleJSONProtocolWriter::serializedFieldSize(
    const char* name, TType /*fieldType*/, int16_t /*fieldId*/) const {
  // string plus ":"
  return static_cast<uint32_t>(strlen(name)) * 6 + 3;
}

uint32_t SimpleJSONProtocolWriter::serializedStructSize(
    const char* /*name*/) const {
  return 2; // braces
}

uint32_t SimpleJSONProtocolWriter::serializedSizeMapBegin(
    TType /*keyType*/, TType /*valType*/, uint32_t /*size*/) const {
  return 1;
}

uint32_t SimpleJSONProtocolWriter::serializedSizeMapEnd() const {
  return 1;
}

uint32_t SimpleJSONProtocolWriter::serializedSizeListBegin(
    TType /*elemType*/, uint32_t /*size*/) const {
  return 1;
}

uint32_t SimpleJSONProtocolWriter::serializedSizeListEnd() const {
  return 1;
}

uint32_t SimpleJSONProtocolWriter::serializedSizeSetBegin(
    TType /*elemType*/, uint32_t /*size*/) const {
  return 1;
}

uint32_t SimpleJSONProtocolWriter::serializedSizeSetEnd() const {
  return 1;
}

uint32_t SimpleJSONProtocolWriter::serializedSizeStop() const {
  return 0;
}

uint32_t SimpleJSONProtocolWriter::serializedSizeBool(bool /*val*/) const {
  return 6;
}

/**
 * Protected reading functions
 */

/**
 * Public reading functions
 */

void SimpleJSONProtocolReader::readStructBegin(std::string& /*name*/) {
  descend();

  ensureAndBeginContext(ContextType::MAP);
}

void SimpleJSONProtocolReader::readStructEnd() {
  endContext();

  ascend();
}

void SimpleJSONProtocolReader::readFieldBegin(
    std::string& name, TType& fieldType, int16_t& fieldId) {
  if (!peekMap()) {
    fieldType = TType::T_STOP;
    fieldId = 0;
    return;
  }
  fieldId = std::numeric_limits<int16_t>::min();
  fieldType = TType::T_VOID;
  readString(name);
  ensureAndSkipContext();
  skipWhitespace();
  auto peek = peekCharSafe();
  if (peek == 'n') {
    bool tmp;
    ensureAndReadContext(tmp);
    readWhitespace();
    readJSONNull();
    readFieldBegin(name, fieldType, fieldId);
  }
}

inline void SimpleJSONProtocolReader::readFieldBeginWithState(
    StructReadState& state) {
  readFieldBegin(state.fieldName_, state.fieldType, state.fieldId);
}

void SimpleJSONProtocolReader::readFieldEnd() {}

void SimpleJSONProtocolReader::readMapBegin(
    TType& /*keyType*/, TType& /*valType*/, uint32_t& size) {
  descend();

  size = std::numeric_limits<uint32_t>::max();
  ensureAndBeginContext(ContextType::MAP);
}

void SimpleJSONProtocolReader::readMapEnd() {
  endContext();

  ascend();
}

void SimpleJSONProtocolReader::readListBegin(
    TType& /*elemType*/, uint32_t& size) {
  descend();

  size = std::numeric_limits<uint32_t>::max();
  ensureAndBeginContext(ContextType::ARRAY);
}

void SimpleJSONProtocolReader::readListEnd() {
  endContext();

  ascend();
}

void SimpleJSONProtocolReader::readSetBegin(
    TType& /*elemType*/, uint32_t& size) {
  descend();

  size = std::numeric_limits<uint32_t>::max();
  ensureAndBeginContext(ContextType::ARRAY);
}

void SimpleJSONProtocolReader::readSetEnd() {
  endContext();

  ascend();
}

void SimpleJSONProtocolReader::readBool(bool& value) {
  readInContext<bool>(value);
}

void SimpleJSONProtocolReader::readBool(std::vector<bool>::reference value) {
  bool tmp = false;
  readInContext<bool>(tmp);
  value = tmp;
}

bool SimpleJSONProtocolReader::peekMap() {
  skipWhitespace();
  return peekCharSafe() != apache::thrift::detail::json::kJSONObjectEnd;
}

bool SimpleJSONProtocolReader::peekList() {
  skipWhitespace();
  return peekCharSafe() != apache::thrift::detail::json::kJSONArrayEnd;
}

bool SimpleJSONProtocolReader::peekSet() {
  return peekList();
}

void SimpleJSONProtocolReader::skip(TType /*type*/, int depth) {
  if (depth >= FLAGS_thrift_protocol_max_depth) {
    protocol::TProtocolException::throwExceededDepthLimit();
  }
  bool keyish;
  ensureAndReadContext(keyish);
  readWhitespace();
  auto ch = peekCharSafe();
  if (ch == apache::thrift::detail::json::kJSONObjectStart) {
    descend();
    beginContext(ContextType::MAP);
    while (true) {
      skipWhitespace();
      if (peekCharSafe() == apache::thrift::detail::json::kJSONObjectEnd) {
        break;
      }
      skip(TType::T_VOID, depth + 1);
      skip(TType::T_VOID, depth + 1);
    }
    endContext();
    ascend();
  } else if (ch == apache::thrift::detail::json::kJSONArrayStart) {
    descend();
    beginContext(ContextType::ARRAY);
    while (true) {
      skipWhitespace();
      if (peekCharSafe() == apache::thrift::detail::json::kJSONArrayEnd) {
        break;
      }
      skip(TType::T_VOID, depth + 1);
    }
    endContext();
    ascend();
  } else if (ch == apache::thrift::detail::json::kJSONStringDelimiter) {
    std::string tmp;
    readJSONVal(tmp);
  } else if (ch == '-' || ch == '+' || (ch >= '0' && ch <= '9')) {
    double tmp;
    readJSONVal(tmp);
  } else if (ch == 't' || ch == 'f') {
    bool tmp;
    readJSONVal(tmp);
  } else if (ch == 'n') {
    readJSONNull();
  } else {
    throwInvalidFieldStart(ch);
  }
}

} // namespace thrift
} // namespace apache

#endif // #ifndef THRIFT2_PROTOCOL_TSIMPLEJSONPROTOCOL_TCC_
