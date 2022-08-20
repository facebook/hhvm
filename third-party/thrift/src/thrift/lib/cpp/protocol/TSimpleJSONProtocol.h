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

#ifndef THRIFT_PROTOCOL_TSIMPLEJSONPROTOCOL_H_
#define THRIFT_PROTOCOL_TSIMPLEJSONPROTOCOL_H_ 1

#include <thrift/lib/cpp/Reflection.h>
#include <thrift/lib/cpp/protocol/TJSONProtocol.h>

#include <memory>
#include <stack>
#include <string>

namespace apache {
namespace thrift {
namespace protocol {

/*
 * TSimpleJSONProtocol overrides parts of the regular JSON serialization to
 * comply with the Simple JSON format.
 * Namely, spitting only field names without verbose field type output
 */

class TSimpleJSONProtocol
    : public TVirtualProtocol<TSimpleJSONProtocol, TJSONProtocol> {
 public:
  explicit TSimpleJSONProtocol(std::shared_ptr<TTransport> ptrans);

  explicit TSimpleJSONProtocol(TTransport* ptrans);

  ~TSimpleJSONProtocol() override;

 public:
  ::apache::thrift::reflection::Schema* getSchema();

  /**
   * Writing functions.
   */

  uint32_t writeFieldBegin(
      const char* name, const TType fieldType, const int16_t fieldId);

  uint32_t writeFieldEnd();

  uint32_t writeMapBegin(
      const TType keyType, const TType valType, const uint32_t size);

  uint32_t writeMapEnd();

  uint32_t writeListBegin(const TType elemType, const uint32_t size);

  uint32_t writeSetBegin(const TType elemType, const uint32_t size);

  uint32_t writeBool(const bool value);

  /**
   * Reading functions
   */

  void setNextStructType(uint64_t reflection_id);

  uint32_t readStructBegin(std::string& name);

  uint32_t readStructEnd();

  uint32_t readFieldBegin(
      std::string& name, TType& fieldType, int16_t& fieldId);

  uint32_t readFieldEnd();

  uint32_t readMapBegin(
      TType& keyType, TType& valType, uint32_t& size, bool& sizeUnknown);

  bool peekMap();

  uint32_t readMapEnd();

  uint32_t readListBegin(TType& elemType, uint32_t& size, bool& sizeUnknown);

  bool peekList();

  uint32_t readListEnd();

  uint32_t readSetBegin(TType& elemType, uint32_t& size, bool& sizeUnknown);

  bool peekSet();

  uint32_t readSetEnd();

  uint32_t readBool(bool& value);

  // Provide the default readBool() implementation for std::vector<bool>
  using TVirtualProtocol<TSimpleJSONProtocol, TJSONProtocol>::readBool;

 private:
  ::apache::thrift::reflection::Schema schema_;

  std::stack<const ::apache::thrift::reflection::DataType*> typeStack_;

  const ::apache::thrift::reflection::DataType* nextType_;

  uint32_t numSkipped_;

 private:
  ::apache::thrift::reflection::DataType* getDataTypeFromTypeNum(int64_t num);

  TType getTypeIdFromTypeNum(int64_t typeNum);
  TType guessTypeIdFromFirstByte();

  void enterType();
  void exitType();
  const ::apache::thrift::reflection::DataType* getCurrentDataType();

  bool isCompoundType(int64_t typeNum);

  void skipWhitespace();
  uint32_t getNumSkippedChars();
};

/**
 * Constructs input and output protocol objects given transports.
 */
class TSimpleJSONProtocolFactory : public TProtocolFactory {
 public:
  TSimpleJSONProtocolFactory() {}

  ~TSimpleJSONProtocolFactory() override {}

  std::shared_ptr<TProtocol> getProtocol(
      std::shared_ptr<TTransport> trans) override {
    return std::shared_ptr<TProtocol>(new TSimpleJSONProtocol(trans));
  }
};

} // namespace protocol
} // namespace thrift
} // namespace apache

#endif // #define THRIFT_PROTOCOL_TSIMPLEJSONPROTOCOL_H_ 1
