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

#ifndef THRIFT_PROTOCOL_TCOMPACTPROTOCOL_H_
#define THRIFT_PROTOCOL_TCOMPACTPROTOCOL_H_ 1

#include <folly/lang/Bits.h>
#include <thrift/lib/cpp/protocol/TVirtualProtocol.h>

#include <memory>
#include <stack>
#include <folly/FBVector.h>

namespace apache::thrift::protocol {

/**
 * C++ Implementation of the Compact Protocol as described in THRIFT-110
 */
template <class Transport_>
class TCompactProtocolT
    : public TVirtualProtocol<TCompactProtocolT<Transport_>> {
 public:
  static const int8_t VERSION_N = 2;
  static const int8_t VERSION_LOW = 1;
  static const int8_t VERSION_DOUBLE_BE = 2;
  static const int8_t PROTOCOL_ID = static_cast<int8_t>(0x82);
  static const int8_t VERSION_MASK = 0x1f; // 0001 1111

 protected:
  static const int8_t TYPE_MASK = 0xE0;
  static const int32_t TYPE_SHIFT_AMOUNT = 5;
  static const int8_t TYPE_SHIFTED_MASK = 0x07; // 0000 0111

  Transport_* trans_;

  /**
   * (Writing) If we encounter a boolean field begin, save the TField here
   * so it can have the value incorporated.
   */
  struct {
    const char* name;
    TType fieldType;
    int16_t fieldId;
  } booleanField_;

  /**
   * (Reading) If we read a field header, and it's a boolean field, save
   * the boolean value here so that readBool can use it.
   */
  struct {
    bool hasBoolValue;
    bool boolValue;
  } boolValue_;

  /**
   * Used to keep track of the last field for the current and previous structs,
   * so we can do the delta stuff.
   */

  std::stack<int16_t, folly::fbvector<int16_t>> lastField_;
  int16_t lastFieldId_;
  int8_t version_;

 public:
  explicit TCompactProtocolT(std::shared_ptr<Transport_> trans)
      : TVirtualProtocol<TCompactProtocolT<Transport_>>(trans),
        trans_(trans.get()),
        lastFieldId_(0),
        version_(VERSION_N),
        string_limit_(0),
        container_limit_(0) {
    booleanField_.name = nullptr;
    boolValue_.hasBoolValue = false;
  }

  TCompactProtocolT(
      std::shared_ptr<Transport_> trans,
      int32_t string_limit,
      int32_t container_limit)
      : TVirtualProtocol<TCompactProtocolT<Transport_>>(trans),
        trans_(trans.get()),
        lastFieldId_(0),
        version_(VERSION_N),
        string_limit_(string_limit),
        container_limit_(container_limit) {
    booleanField_.name = nullptr;
    boolValue_.hasBoolValue = false;
  }

  /**
   * Construct a TCompactProtocolT using a raw pointer to the transport.
   *
   * The caller is responsible for ensuring that the transport remains valid
   * for the lifetime of the protocol.
   */
  explicit TCompactProtocolT(Transport_* trans)
      : TVirtualProtocol<TCompactProtocolT<Transport_>>(trans),
        trans_(trans),
        lastFieldId_(0),
        version_(VERSION_N),
        string_limit_(0),
        container_limit_(0) {
    booleanField_.name = nullptr;
    boolValue_.hasBoolValue = false;
  }

  void setStringSizeLimit(int32_t string_limit) {
    string_limit_ = string_limit;
  }

  void setContainerSizeLimit(int32_t container_limit) {
    container_limit_ = container_limit;
  }

  int32_t getStringSizeLimit() { return string_limit_; }

  int32_t getContainerSizeLimit() { return container_limit_; }

  /** Set this if you need backwards compatibility with an old version */
  void setVersion(const int8_t version) { version_ = version; }

  /**
   * Writing functions
   */

  virtual uint32_t writeMessageBegin(
      const std::string& name,
      const TMessageType messageType,
      const int32_t seqid);

  uint32_t writeStructBegin(const char* name);

  uint32_t writeStructEnd();

  uint32_t writeFieldBegin(
      const char* name, const TType fieldType, const int16_t fieldId);

  uint32_t writeFieldStop();

  uint32_t writeListBegin(const TType elemType, const uint32_t size);

  uint32_t writeSetBegin(const TType elemType, const uint32_t size);

  virtual uint32_t writeMapBegin(
      const TType keyType, const TType valType, const uint32_t size);

  uint32_t writeBool(const bool value);

  uint32_t writeByte(const int8_t byte);

  uint32_t writeI16(const int16_t i16);

  uint32_t writeI32(const int32_t i32);

  uint32_t writeI64(const int64_t i64);

  uint32_t writeDouble(const double dub);

  uint32_t writeFloat(const float flt);

  uint32_t writeString(const char* str);

  template <class String_>
  uint32_t writeString(const String_& str);

  template <class String_>
  uint32_t writeBinary(const String_& str);

  /**
   * These methods are called by structs, but don't actually have any wired
   * output or purpose
   */
  virtual uint32_t writeMessageEnd() { return 0; }
  uint32_t writeMapEnd() { return 0; }
  uint32_t writeListEnd() { return 0; }
  uint32_t writeSetEnd() { return 0; }
  uint32_t writeFieldEnd() { return 0; }

 protected:
  int32_t writeFieldBeginInternal(
      const char* name,
      const TType fieldType,
      const int16_t fieldId,
      int8_t typeOverride);
  uint32_t writeCollectionBegin(int8_t elemType, int32_t size);
  uint32_t writeVarint32(uint32_t n);
  uint32_t writeVarint64(uint64_t n);
  inline int8_t getCompactType(int8_t ttype);

 public:
  uint32_t readMessageBegin(
      std::string& name, TMessageType& messageType, int32_t& seqid);

  uint32_t readStructBegin(std::string& name);

  uint32_t readStructEnd();

  uint32_t readFieldBegin(
      std::string& name, TType& fieldType, int16_t& fieldId);

  uint32_t readMapBegin(
      TType& keyType, TType& valType, uint32_t& size, bool& sizeUnknown);

  uint32_t readListBegin(TType& elemType, uint32_t& size, bool& sizeUnknown);

  uint32_t readSetBegin(TType& elemType, uint32_t& size, bool& sizeUnknown);

  uint32_t readBool(bool& value);
  // Provide the default readBool() implementation for std::vector<bool>
  using TVirtualProtocol<TCompactProtocolT<Transport_>>::readBool;

  uint32_t readByte(int8_t& byte);

  uint32_t readI16(int16_t& i16);

  uint32_t readI32(int32_t& i32);

  uint32_t readI64(int64_t& i64);

  uint32_t readDouble(double& dub);

  uint32_t readFloat(float& flt);

  template <class String_>
  uint32_t readString(String_& str);

  template <class String_>
  uint32_t readBinary(String_& str);

  /*
   *These methods are here for the struct to call, but don't have any wire
   * encoding.
   */
  uint32_t readMessageEnd() { return 0; }
  uint32_t readFieldEnd() { return 0; }
  uint32_t readMapEnd() { return 0; }
  uint32_t readListEnd() { return 0; }
  uint32_t readSetEnd() { return 0; }

 protected:
  uint32_t readVarint32(int32_t& i32);
  uint32_t readVarint64(int64_t& i64);
  TType getTType(int8_t type);

  int32_t string_limit_;
  int32_t container_limit_;
};

using TCompactProtocol = TCompactProtocolT<TTransport>;

/**
 * Constructs compact protocol handlers
 */
template <class Transport_>
class TCompactProtocolFactoryT : public TProtocolFactory {
 public:
  TCompactProtocolFactoryT() : string_limit_(0), container_limit_(0) {}

  TCompactProtocolFactoryT(int32_t string_limit, int32_t container_limit)
      : string_limit_(string_limit), container_limit_(container_limit) {}

  ~TCompactProtocolFactoryT() override {}

  void setStringSizeLimit(int32_t string_limit) {
    string_limit_ = string_limit;
  }

  void setContainerSizeLimit(int32_t container_limit) {
    container_limit_ = container_limit;
  }

  std::shared_ptr<TProtocol> getProtocol(
      std::shared_ptr<TTransport> trans) override {
    std::shared_ptr<Transport_> specific_trans =
        std::dynamic_pointer_cast<Transport_>(trans);
    TProtocol* prot;
    if (specific_trans) {
      prot = new TCompactProtocolT<Transport_>(
          specific_trans, string_limit_, container_limit_);
    } else {
      prot = new TCompactProtocol(trans, string_limit_, container_limit_);
    }

    return std::shared_ptr<TProtocol>(prot);
  }

 private:
  int32_t string_limit_;
  int32_t container_limit_;
};

using TCompactProtocolFactory = TCompactProtocolFactoryT<TTransport>;

} // namespace apache::thrift::protocol

#include <thrift/lib/cpp/protocol/TCompactProtocol-inl.h>

#endif
