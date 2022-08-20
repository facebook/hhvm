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

#ifndef THRIFT_PROTOCOL_TPROTOCOL_H_
#define THRIFT_PROTOCOL_TPROTOCOL_H_ 1

#include <folly/Conv.h>
#include <thrift/lib/cpp/protocol/TProtocolException.h>
#include <thrift/lib/cpp/protocol/TType.h>
#include <thrift/lib/cpp/transport/TTransport.h>

#include <memory>

#include <sys/types.h>
#include <map>
#include <string>
#include <vector>

#include <folly/FBString.h>

namespace apache {
namespace thrift {
namespace reflection {
class Schema;
}
} // namespace thrift
} // namespace apache

namespace apache {
namespace thrift {
namespace protocol {

using apache::thrift::transport::TTransport;

/**
 * Enumerated definition of the message types that the Thrift protocol
 * supports.
 */
enum TMessageType { T_CALL = 1, T_REPLY = 2, T_EXCEPTION = 3, T_ONEWAY = 4 };

/**
 * Helper template for implementing TProtocol::skip().
 *
 * Templatized to avoid having to make virtual function calls.
 */
template <class Protocol_>
uint32_t skip(Protocol_& prot, TType arg_type) {
  switch (arg_type) {
    case T_BOOL: {
      bool boolv;
      return prot.readBool(boolv);
    }
    case T_BYTE: {
      int8_t bytev = 0;
      return prot.readByte(bytev);
    }
    case T_I16: {
      int16_t i16;
      return prot.readI16(i16);
    }
    case T_I32: {
      int32_t i32;
      return prot.readI32(i32);
    }
    case T_I64: {
      int64_t i64;
      return prot.readI64(i64);
    }
    case T_DOUBLE: {
      double dub;
      return prot.readDouble(dub);
    }
    case T_FLOAT: {
      float flt;
      return prot.readFloat(flt);
    }
    case T_STRING: {
      std::string str;
      return prot.readBinary(str);
    }
    case T_STRUCT: {
      uint32_t result = 0;
      std::string name;
      int16_t fid;
      TType ftype;
      result += prot.readStructBegin(name);
      while (true) {
        result += prot.readFieldBegin(name, ftype, fid);
        if (ftype == T_STOP) {
          break;
        }
        result += skip(prot, ftype);
        result += prot.readFieldEnd();
      }
      result += prot.readStructEnd();
      return result;
    }
    case T_MAP: {
      uint32_t result = 0;
      TType keyType;
      TType valType;
      uint32_t i, size;
      bool sizeUnknown;
      result += prot.readMapBegin(keyType, valType, size, sizeUnknown);
      if (!sizeUnknown) {
        for (i = 0; i < size; i++) {
          result += skip(prot, keyType);
          result += skip(prot, valType);
        }
      } else {
        while (prot.peekMap()) {
          result += skip(prot, keyType);
          result += skip(prot, valType);
        }
      }
      result += prot.readMapEnd();
      return result;
    }
    case T_SET: {
      uint32_t result = 0;
      TType elemType;
      uint32_t i, size;
      bool sizeUnknown;
      result += prot.readSetBegin(elemType, size, sizeUnknown);
      if (!sizeUnknown) {
        for (i = 0; i < size; i++) {
          result += skip(prot, elemType);
        }
      } else {
        while (prot.peekSet()) {
          result += skip(prot, elemType);
        }
      }
      result += prot.readSetEnd();
      return result;
    }
    case T_LIST: {
      uint32_t result = 0;
      TType elemType;
      uint32_t i, size;
      bool sizeUnknown;
      result += prot.readListBegin(elemType, size, sizeUnknown);
      if (!sizeUnknown) {
        for (i = 0; i < size; i++) {
          result += skip(prot, elemType);
        }
      } else {
        while (prot.peekList()) {
          result += skip(prot, elemType);
        }
      }
      result += prot.readListEnd();
      return result;
    }
    default: {
      TProtocolException::throwInvalidSkipType(arg_type);
    }
  }
}

/**
 * Abstract class for a thrift protocol driver. These are all the methods that
 * a protocol must implement. Essentially, there must be some way of reading
 * and writing all the base types, plus a mechanism for writing out structs
 * with indexed fields.
 *
 * TProtocol objects should not be shared across multiple encoding contexts,
 * as they may need to maintain internal state in some protocols (i.e. XML).
 * Note that it is acceptable for the TProtocol module to do its own internal
 * buffered reads/writes to the underlying TTransport where appropriate (i.e.
 * when parsing an input XML stream, reading should be batched rather than
 * looking ahead character by character for a close tag).
 *
 */
class TProtocol {
 public:
  virtual ~TProtocol() {}

  virtual void setVersion_virt(const int8_t version) = 0;

  void setVersion(const int8_t version) { return setVersion_virt(version); }

  virtual ::apache::thrift::reflection::Schema* getSchema_virt() = 0;

  ::apache::thrift::reflection::Schema* getSchema() { return getSchema_virt(); }

  /**
   * Writing functions.
   */

  virtual uint32_t writeMessageBegin_virt(
      const std::string& name,
      const TMessageType messageType,
      const int32_t seqid) = 0;

  virtual uint32_t writeMessageEnd_virt() = 0;

  virtual uint32_t writeStructBegin_virt(const char* name) = 0;

  virtual uint32_t writeStructEnd_virt() = 0;

  virtual uint32_t writeFieldBegin_virt(
      const char* name, const TType fieldType, const int16_t fieldId) = 0;

  virtual uint32_t writeFieldEnd_virt() = 0;

  virtual uint32_t writeFieldStop_virt() = 0;

  virtual uint32_t writeMapBegin_virt(
      const TType keyType, const TType valType, const uint32_t size) = 0;

  virtual uint32_t writeMapEnd_virt() = 0;

  virtual uint32_t writeListBegin_virt(
      const TType elemType, const uint32_t size) = 0;

  virtual uint32_t writeListEnd_virt() = 0;

  virtual uint32_t writeSetBegin_virt(
      const TType elemType, const uint32_t size) = 0;

  virtual uint32_t writeSetEnd_virt() = 0;

  virtual uint32_t writeBool_virt(const bool value) = 0;

  virtual uint32_t writeByte_virt(const int8_t byte) = 0;

  virtual uint32_t writeI16_virt(const int16_t i16) = 0;

  virtual uint32_t writeI32_virt(const int32_t i32) = 0;

  virtual uint32_t writeI64_virt(const int64_t i64) = 0;

  virtual uint32_t writeDouble_virt(const double dub) = 0;

  virtual uint32_t writeFloat_virt(const float flt) = 0;

  virtual uint32_t writeString_virt(const std::string& str) = 0;

  virtual uint32_t writeBinary_virt(const std::string& str) = 0;

  uint32_t writeMessageBegin(
      const std::string& name,
      const TMessageType messageType,
      const int32_t seqid) {
    return writeMessageBegin_virt(name, messageType, seqid);
  }

  uint32_t writeMessageEnd() { return writeMessageEnd_virt(); }

  uint32_t writeStructBegin(const char* name) {
    return writeStructBegin_virt(name);
  }

  uint32_t writeStructEnd() { return writeStructEnd_virt(); }

  uint32_t writeFieldBegin(
      const char* name, const TType fieldType, const int16_t fieldId) {
    return writeFieldBegin_virt(name, fieldType, fieldId);
  }

  uint32_t writeFieldEnd() { return writeFieldEnd_virt(); }

  uint32_t writeFieldStop() { return writeFieldStop_virt(); }

  uint32_t writeMapBegin(
      const TType keyType, const TType valType, const uint32_t size) {
    return writeMapBegin_virt(keyType, valType, size);
  }

  uint32_t writeMapEnd() { return writeMapEnd_virt(); }

  uint32_t writeListBegin(const TType elemType, const uint32_t size) {
    return writeListBegin_virt(elemType, size);
  }

  uint32_t writeListEnd() { return writeListEnd_virt(); }

  uint32_t writeSetBegin(const TType elemType, const uint32_t size) {
    return writeSetBegin_virt(elemType, size);
  }

  uint32_t writeSetEnd() { return writeSetEnd_virt(); }

  uint32_t writeBool(const bool value) { return writeBool_virt(value); }

  uint32_t writeByte(const int8_t byte) { return writeByte_virt(byte); }

  uint32_t writeI16(const int16_t i16) { return writeI16_virt(i16); }

  uint32_t writeI32(const int32_t i32) { return writeI32_virt(i32); }

  uint32_t writeI64(const int64_t i64) { return writeI64_virt(i64); }

  uint32_t writeDouble(const double dub) { return writeDouble_virt(dub); }

  uint32_t writeFloat(const float flt) { return writeFloat_virt(flt); }

  uint32_t writeString(const std::string& str) { return writeString_virt(str); }

  uint32_t writeString(const folly::fbstring& str) {
    return writeString_virt(str.toStdString());
  }

  uint32_t writeBinary(const std::string& str) { return writeBinary_virt(str); }

  uint32_t writeBinary(const folly::fbstring& str) {
    return writeBinary_virt(str.toStdString());
  }

  /**
   * Reading functions
   */

  virtual uint32_t readMessageBegin_virt(
      std::string& name, TMessageType& messageType, int32_t& seqid) = 0;

  virtual uint32_t readMessageEnd_virt() = 0;

  virtual void setNextStructType_virt(uint64_t reflection_id) = 0;

  virtual uint32_t readStructBegin_virt(std::string& name) = 0;

  virtual uint32_t readStructEnd_virt() = 0;

  virtual uint32_t readFieldBegin_virt(
      std::string& name, TType& fieldType, int16_t& fieldId) = 0;

  virtual uint32_t readFieldEnd_virt() = 0;

  virtual uint32_t readMapBegin_virt(
      TType& keyType, TType& valType, uint32_t& size, bool& sizeUnknown) = 0;

  virtual bool peekMap_virt() = 0;

  virtual uint32_t readMapEnd_virt() = 0;

  virtual uint32_t readListBegin_virt(
      TType& elemType, uint32_t& size, bool& sizeUnknown) = 0;

  virtual bool peekList_virt() = 0;

  virtual uint32_t readListEnd_virt() = 0;

  virtual uint32_t readSetBegin_virt(
      TType& elemType, uint32_t& size, bool& sizeUnknown) = 0;

  virtual bool peekSet_virt() = 0;

  virtual uint32_t readSetEnd_virt() = 0;

  virtual uint32_t readBool_virt(bool& value) = 0;

  virtual uint32_t readBool_virt(std::vector<bool>::reference value) = 0;

  virtual uint32_t readByte_virt(int8_t& byte) = 0;

  virtual uint32_t readI16_virt(int16_t& i16) = 0;

  virtual uint32_t readI32_virt(int32_t& i32) = 0;

  virtual uint32_t readI64_virt(int64_t& i64) = 0;

  virtual uint32_t readDouble_virt(double& dub) = 0;

  virtual uint32_t readFloat_virt(float& flt) = 0;

  virtual uint32_t readString_virt(std::string& str) = 0;

  virtual uint32_t readBinary_virt(std::string& str) = 0;

  uint32_t readMessageBegin(
      std::string& name, TMessageType& messageType, int32_t& seqid) {
    return readMessageBegin_virt(name, messageType, seqid);
  }

  uint32_t readMessageEnd() { return readMessageEnd_virt(); }

  void setNextStructType(uint64_t reflection_id) {
    return setNextStructType_virt(reflection_id);
  }

  uint32_t readStructBegin(std::string& name) {
    return readStructBegin_virt(name);
  }

  uint32_t readStructEnd() { return readStructEnd_virt(); }

  uint32_t readFieldBegin(
      std::string& name, TType& fieldType, int16_t& fieldId) {
    return readFieldBegin_virt(name, fieldType, fieldId);
  }

  uint32_t readFieldEnd() { return readFieldEnd_virt(); }

  uint32_t readMapBegin(
      TType& keyType, TType& valType, uint32_t& size, bool& sizeUnknown) {
    return readMapBegin_virt(keyType, valType, size, sizeUnknown);
  }

  bool peekMap() { return peekMap_virt(); }

  uint32_t readMapEnd() { return readMapEnd_virt(); }

  uint32_t readListBegin(TType& elemType, uint32_t& size, bool& sizeUnknown) {
    return readListBegin_virt(elemType, size, sizeUnknown);
  }

  bool peekList() { return peekList_virt(); }

  uint32_t readListEnd() { return readListEnd_virt(); }

  uint32_t readSetBegin(TType& elemType, uint32_t& size, bool& sizeUnknown) {
    return readSetBegin_virt(elemType, size, sizeUnknown);
  }

  bool peekSet() { return peekSet_virt(); }

  uint32_t readSetEnd() { return readSetEnd_virt(); }

  uint32_t readBool(bool& value) { return readBool_virt(value); }

  uint32_t readByte(int8_t& byte) { return readByte_virt(byte); }

  uint32_t readI16(int16_t& i16) { return readI16_virt(i16); }

  uint32_t readI32(int32_t& i32) { return readI32_virt(i32); }

  uint32_t readI64(int64_t& i64) { return readI64_virt(i64); }

  uint32_t readDouble(double& dub) { return readDouble_virt(dub); }

  uint32_t readFloat(float& flt) { return readFloat_virt(flt); }

  uint32_t readString(std::string& str) { return readString_virt(str); }

  uint32_t readString(folly::fbstring& str) {
    std::string data;
    uint32_t ret = readString_virt(data);
    str = data;
    return ret;
  }

  uint32_t readBinary(std::string& str) { return readBinary_virt(str); }

  uint32_t readBinary(folly::fbstring& str) {
    std::string data;
    uint32_t ret = readBinary_virt(data);
    str = data;
    return ret;
  }

  int32_t getStringSizeLimit() {
    return 0; // No limit
  }

  int32_t getContainerSizeLimit() {
    return 0; // No limit
  }

  /*
   * std::vector is specialized for bool, and its elements are individual bits
   * rather than bools.   We need to define a different version of readBool()
   * to work with std::vector<bool>.
   */
  uint32_t readBool(std::vector<bool>::reference value) {
    return readBool_virt(value);
  }

  /**
   * Method to arbitrarily skip over data.
   */
  uint32_t skip(TType type) { return skip_virt(type); }
  virtual uint32_t skip_virt(TType type) {
    return ::apache::thrift::protocol::skip(*this, type);
  }

  inline std::shared_ptr<TTransport> getTransport() { return ptrans_; }

  // TODO: remove these two calls, they are for backwards
  // compatibility
  inline std::shared_ptr<TTransport> getInputTransport() { return ptrans_; }
  inline std::shared_ptr<TTransport> getOutputTransport() { return ptrans_; }

 protected:
  explicit TProtocol(std::shared_ptr<TTransport> ptrans) : ptrans_(ptrans) {}

  /**
   * Construct a TProtocol using a raw TTransport pointer.
   *
   * It is the callers responsibility to ensure that the TTransport remains
   * valid for the lifetime of the TProtocol object.
   */
  explicit TProtocol(TTransport* ptrans)
      : ptrans_(ptrans, [](TTransport*) {}) {}

  std::shared_ptr<TTransport> ptrans_;

 private:
  TProtocol() {}
};

/**
 * Constructs protocol objects given transports.
 */
class TProtocolFactory {
 public:
  TProtocolFactory() {}

  virtual ~TProtocolFactory() {}

  virtual std::shared_ptr<TProtocol> getProtocol(
      std::shared_ptr<TTransport> trans) = 0;
};

/**
 * Constructs both input and output protocol objects with a given pair of
 * input and output transports.
 *
 * TProtocolPair.first = Input Protocol
 * TProtocolPair.second = Output Protocol
 */
typedef std::pair<std::shared_ptr<TProtocol>, std::shared_ptr<TProtocol>>
    TProtocolPair;

class TDuplexProtocolFactory {
 public:
  TDuplexProtocolFactory() {}

  virtual ~TDuplexProtocolFactory() {}

  virtual TProtocolPair getProtocol(transport::TTransportPair transports) = 0;

  virtual std::shared_ptr<TProtocolFactory> getInputProtocolFactory() {
    return std::shared_ptr<TProtocolFactory>();
  }

  virtual std::shared_ptr<TProtocolFactory> getOutputProtocolFactory() {
    return std::shared_ptr<TProtocolFactory>();
  }
};

/**
 * Adapts a TProtocolFactory to a TDuplexProtocolFactory that returns
 * a new protocol object for both input and output
 */
template <class Factory_>
class TSingleProtocolFactory : public TDuplexProtocolFactory {
 public:
  TSingleProtocolFactory() { factory_.reset(new Factory_()); }

  explicit TSingleProtocolFactory(std::shared_ptr<Factory_> factory)
      : factory_(factory) {}

  TProtocolPair getProtocol(transport::TTransportPair transports) override {
    return std::make_pair(
        factory_->getProtocol(transports.first),
        factory_->getProtocol(transports.second));
  }

  std::shared_ptr<TProtocolFactory> getInputProtocolFactory() override {
    return factory_;
  }

  std::shared_ptr<TProtocolFactory> getOutputProtocolFactory() override {
    return factory_;
  }

 private:
  std::shared_ptr<Factory_> factory_;
};

/**
 * Use TDualProtocolFactory to construct input and output protocols from
 * different factories.
 */
class TDualProtocolFactory : public TDuplexProtocolFactory {
 public:
  TDualProtocolFactory(
      std::shared_ptr<TProtocolFactory> inputFactory,
      std::shared_ptr<TProtocolFactory> outputFactory)
      : inputFactory_(inputFactory), outputFactory_(outputFactory) {}

  TProtocolPair getProtocol(transport::TTransportPair transports) override {
    return std::make_pair(
        inputFactory_->getProtocol(transports.first),
        outputFactory_->getProtocol(transports.second));
  }

  std::shared_ptr<TProtocolFactory> getInputProtocolFactory() override {
    return inputFactory_;
  }

  std::shared_ptr<TProtocolFactory> getOutputProtocolFactory() override {
    return outputFactory_;
  }

 private:
  std::shared_ptr<TProtocolFactory> inputFactory_;
  std::shared_ptr<TProtocolFactory> outputFactory_;
};

/**
 * Dummy protocol class.
 *
 * This class does nothing, and should never be instantiated.
 * It is used only by the generator code.
 */
class TDummyProtocol : public TProtocol {};

} // namespace protocol
} // namespace thrift
} // namespace apache

#endif // #define _THRIFT_PROTOCOL_TPROTOCOL_H_ 1
