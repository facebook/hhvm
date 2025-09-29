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

#ifndef CPP2_PROTOCOL_TSIMPLEJSONPROTOCOL_H_
#define CPP2_PROTOCOL_TSIMPLEJSONPROTOCOL_H_ 1

#include <string>

#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp/protocol/TProtocol.h>
#include <thrift/lib/cpp2/protocol/JSONProtocolCommon.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>

namespace apache::thrift {

using folly::IOBuf;
using folly::IOBufQueue;

using RWCursor = folly::io::RWPrivateCursor;
using folly::io::Cursor;
using folly::io::QueueAppender;

class SimpleJSONProtocolReader;

class SimpleJSONProtocolWriter : public JSONProtocolWriterCommon {
 public:
  static const int32_t VERSION_1 = 0x80010000;

  using ProtocolReader = SimpleJSONProtocolReader;

  using JSONProtocolWriterCommon::JSONProtocolWriterCommon;

  static constexpr ProtocolType protocolType() {
    return ProtocolType::T_SIMPLE_JSON_PROTOCOL;
  }

  static constexpr bool kSortKeys() { return false; }

  static constexpr bool kHasIndexSupport() { return false; }

  static constexpr bool kSupportsArithmeticVectors() { return false; }

  inline uint32_t writeStructBegin(const char* name);
  inline uint32_t writeStructEnd();
  inline uint32_t writeFieldBegin(
      const char* name, TType fieldType, int16_t fieldId);
  inline uint32_t writeFieldEnd();
  inline uint32_t writeFieldStop();
  inline uint32_t writeMapBegin(TType keyType, TType valType, uint32_t size);
  inline uint32_t writeMapEnd();
  inline uint32_t writeListBegin(TType elemType, uint32_t size);
  inline uint32_t writeListEnd();
  inline uint32_t writeSetBegin(TType elemType, uint32_t size);
  inline uint32_t writeSetEnd();
  inline uint32_t writeBool(bool value);

  /**
   * Functions that return the serialized size
   */

  inline uint32_t serializedMessageSize(const std::string& name) const;
  inline uint32_t serializedFieldSize(
      const char* name, TType fieldType, int16_t fieldId) const;
  inline uint32_t serializedStructSize(const char* name) const;
  inline uint32_t serializedSizeMapBegin(
      TType keyType, TType valType, uint32_t size) const;
  inline uint32_t serializedSizeMapEnd() const;
  inline uint32_t serializedSizeListBegin(TType elemType, uint32_t size) const;
  inline uint32_t serializedSizeListEnd() const;
  inline uint32_t serializedSizeSetBegin(TType elemType, uint32_t size) const;
  inline uint32_t serializedSizeSetEnd() const;
  inline uint32_t serializedSizeStop() const;
  inline uint32_t serializedSizeBool(bool = false) const;
};

class SimpleJSONProtocolReader : public JSONProtocolReaderCommon {
 public:
  static const int32_t VERSION_MASK = 0xffff0000;
  static const int32_t VERSION_1 = 0x80010000;

  using ProtocolWriter = SimpleJSONProtocolWriter;

  using JSONProtocolReaderCommon::JSONProtocolReaderCommon;

  static constexpr ProtocolType protocolType() {
    return ProtocolType::T_SIMPLE_JSON_PROTOCOL;
  }

  static constexpr bool kUsesFieldNames() { return true; }

  static constexpr bool kOmitsContainerSizes() { return true; }

  static constexpr bool kOmitsStringSizes() { return true; }

  static constexpr bool kOmitsContainerElemTypes() { return true; }

  static constexpr bool kHasDeferredRead() { return false; }

  static constexpr bool kSupportsArithmeticVectors() { return false; }

  /**
   * Reading functions
   */
  inline void readStructBegin(std::string& name);
  inline void readStructEnd();
  inline void readFieldBegin(
      std::string& name, TType& fieldType, int16_t& fieldId);
  inline void readFieldEnd();
  inline void readMapBegin(TType& keyType, TType& valType, uint32_t& size);
  inline void readMapEnd();
  inline void readListBegin(TType& elemType, uint32_t& size);
  inline void readListEnd();
  inline void readSetBegin(TType& elemType, uint32_t& size);
  inline void readSetEnd();
  inline void readBool(bool& value);
  inline void readBool(std::vector<bool>::reference value);
  inline bool peekMap();
  inline bool peekList();
  inline bool peekSet();

  inline void skip(TType type, int depth = 0);
};

static_assert(usesFieldNames<SimpleJSONProtocolReader>());
static_assert(usesFieldNames<SimpleJSONProtocolWriter>());

} // namespace apache::thrift

#include <thrift/lib/cpp2/protocol/SimpleJSONProtocol-inl.h>

#endif // #ifndef CPP2_PROTOCOL_TSIMPLEJSONPROTOCOL_H_
