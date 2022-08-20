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

#ifndef _THRIFT_PROTOCOL_TVIRTUALPROTOCOL_H_
#define _THRIFT_PROTOCOL_TVIRTUALPROTOCOL_H_ 1

#include <thrift/lib/cpp/protocol/TProtocol.h>

namespace apache {
namespace thrift {
namespace protocol {

/**
 * Helper class that provides default implementations of TProtocol methods.
 *
 * This class provides default implementations of the non-virtual TProtocol
 * methods.  It exists primarily so TVirtualProtocol can derive from it.  It
 * prevents TVirtualProtocol methods from causing infinite recursion if the
 * non-virtual methods are not overridden by the TVirtualProtocol subclass.
 *
 * You probably don't want to use this class directly.  Use TVirtualProtocol
 * instead.
 */
class TProtocolDefaults : public TProtocol {
 public:
  [[noreturn]] void setVersion(const int8_t /*version*/) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support setVersion (yet)");
  }

  ::apache::thrift::reflection::Schema* getSchema() { return nullptr; }

  [[noreturn]] uint32_t readMessageBegin(
      std::string& /*name*/,
      TMessageType& /*messageType*/,
      int32_t& /*seqid*/) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support reading (yet).");
  }

  [[noreturn]] uint32_t readMessageEnd() {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support reading (yet).");
  }

  [[noreturn]] void setNextStructType(uint64_t /*reflection_id*/) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support setting struct type (yet).");
  }

  [[noreturn]] uint32_t readStructBegin(std::string& /*name*/) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support reading (yet).");
  }

  [[noreturn]] uint32_t readStructEnd() {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support reading (yet).");
  }

  [[noreturn]] uint32_t readFieldBegin(
      std::string& /*name*/, TType& /*fieldType*/, int16_t& /*fieldId*/) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support reading (yet).");
  }

  [[noreturn]] uint32_t readFieldEnd() {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support reading (yet).");
  }

  [[noreturn]] uint32_t readMapBegin(
      TType& /*keyType*/,
      TType& /*valType*/,
      uint32_t& /*size*/,
      bool& /*sizeUnknown*/) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support reading (yet).");
  }

  [[noreturn]] bool peekMap() {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support peeking (yet).");
  }

  [[noreturn]] uint32_t readMapEnd() {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support reading (yet).");
  }

  [[noreturn]] uint32_t readListBegin(
      TType& /*elemType*/, uint32_t& /*size*/, bool& /*sizeUnknown*/) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support reading (yet).");
  }

  [[noreturn]] bool peekList() {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support peeking (yet).");
  }

  [[noreturn]] uint32_t readListEnd() {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support reading (yet).");
  }

  [[noreturn]] uint32_t readSetBegin(
      TType& /*elemType*/, uint32_t& /*size*/, bool& /*sizeUnknown*/) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support reading (yet).");
  }

  [[noreturn]] bool peekSet() {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support peeking (yet).");
  }

  [[noreturn]] uint32_t readSetEnd() {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support reading (yet).");
  }

  [[noreturn]] uint32_t readBool(bool& /*value*/) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support reading (yet).");
  }

  [[noreturn]] uint32_t readBool(std::vector<bool>::reference /*value*/) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support reading (yet).");
  }

  [[noreturn]] uint32_t readByte(int8_t& /*byte*/) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support reading (yet).");
  }

  [[noreturn]] uint32_t readI16(int16_t& /*i16*/) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support reading (yet).");
  }

  [[noreturn]] uint32_t readI32(int32_t& /*i32*/) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support reading (yet).");
  }

  [[noreturn]] uint32_t readI64(int64_t& /*i64*/) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support reading (yet).");
  }

  [[noreturn]] uint32_t readDouble(double& /*dub*/) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support reading (yet).");
  }

  [[noreturn]] uint32_t readFloat(float& /*flt*/) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support reading (yet).");
  }

  [[noreturn]] uint32_t readString(std::string& /*str*/) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support reading (yet).");
  }

  [[noreturn]] uint32_t readBinary(std::string& /*str*/) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support reading (yet).");
  }

  [[noreturn]] uint32_t writeMessageBegin(
      const std::string& /*name*/,
      const TMessageType /*messageType*/,
      const int32_t /*seqid*/) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support writing (yet).");
  }

  [[noreturn]] uint32_t writeMessageEnd() {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support writing (yet).");
  }

  [[noreturn]] uint32_t writeStructBegin(const char* /*name*/) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support writing (yet).");
  }

  [[noreturn]] uint32_t writeStructEnd() {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support writing (yet).");
  }

  [[noreturn]] uint32_t writeFieldBegin(
      const char* /*name*/,
      const TType /*fieldType*/,
      const int16_t /*fieldId*/) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support writing (yet).");
  }

  [[noreturn]] uint32_t writeFieldEnd() {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support writing (yet).");
  }

  [[noreturn]] uint32_t writeFieldStop() {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support writing (yet).");
  }

  [[noreturn]] uint32_t writeMapBegin(
      const TType /*keyType*/,
      const TType /*valType*/,
      const uint32_t /*size*/) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support writing (yet).");
  }

  [[noreturn]] uint32_t writeMapEnd() {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support writing (yet).");
  }

  [[noreturn]] uint32_t writeListBegin(
      const TType /*elemType*/, const uint32_t /*size*/) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support writing (yet).");
  }

  [[noreturn]] uint32_t writeListEnd() {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support writing (yet).");
  }

  [[noreturn]] uint32_t writeSetBegin(
      const TType /*elemType*/, const uint32_t /*size*/) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support writing (yet).");
  }

  [[noreturn]] uint32_t writeSetEnd() {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support writing (yet).");
  }

  [[noreturn]] uint32_t writeBool(const bool /*value*/) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support writing (yet).");
  }

  [[noreturn]] uint32_t writeByte(const int8_t /*byte*/) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support writing (yet).");
  }

  [[noreturn]] uint32_t writeI16(const int16_t /*i16*/) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support writing (yet).");
  }

  [[noreturn]] uint32_t writeI32(const int32_t /*i32*/) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support writing (yet).");
  }

  [[noreturn]] uint32_t writeI64(const int64_t /*i64*/) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support writing (yet).");
  }

  [[noreturn]] uint32_t writeDouble(const double /*dub*/) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support writing (yet).");
  }

  [[noreturn]] uint32_t writeFloat(const float /*flt*/) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support writing (yet).");
  }

  [[noreturn]] uint32_t writeString(const std::string& /*str*/) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support writing (yet).");
  }

  [[noreturn]] uint32_t writeBinary(const std::string& /*str*/) {
    throw TProtocolException(
        TProtocolException::NOT_IMPLEMENTED,
        "this protocol does not support writing (yet).");
  }

  uint32_t skip(TType type) {
    return ::apache::thrift::protocol::skip(*this, type);
  }

 protected:
  explicit TProtocolDefaults(
      const std::shared_ptr<transport::TTransport>& ptrans)
      : TProtocol(ptrans) {}

  explicit TProtocolDefaults(transport::TTransport* ptrans)
      : TProtocol(ptrans) {}
};

/**
 * Concrete TProtocol classes should inherit from TVirtualProtocol
 * so they don't have to manually override virtual methods.
 *
 * @author David Reiss <dreiss@facebook.com>
 */
template <class Protocol_, class Super_ = TProtocolDefaults>
class TVirtualProtocol : public Super_ {
 public:
  void setVersion_virt(const int8_t version) override {
    return static_cast<Protocol_*>(this)->setVersion(version);
  }

  ::apache::thrift::reflection::Schema* getSchema_virt() override {
    return static_cast<Protocol_*>(this)->getSchema();
  }

  /**
   * Writing functions.
   */

  uint32_t writeMessageBegin_virt(
      const std::string& name,
      const TMessageType messageType,
      const int32_t seqid) override {
    return static_cast<Protocol_*>(this)->writeMessageBegin(
        name, messageType, seqid);
  }

  uint32_t writeMessageEnd_virt() override {
    return static_cast<Protocol_*>(this)->writeMessageEnd();
  }

  uint32_t writeStructBegin_virt(const char* name) override {
    return static_cast<Protocol_*>(this)->writeStructBegin(name);
  }

  uint32_t writeStructEnd_virt() override {
    return static_cast<Protocol_*>(this)->writeStructEnd();
  }

  uint32_t writeFieldBegin_virt(
      const char* name, const TType fieldType, const int16_t fieldId) override {
    return static_cast<Protocol_*>(this)->writeFieldBegin(
        name, fieldType, fieldId);
  }

  uint32_t writeFieldEnd_virt() override {
    return static_cast<Protocol_*>(this)->writeFieldEnd();
  }

  uint32_t writeFieldStop_virt() override {
    return static_cast<Protocol_*>(this)->writeFieldStop();
  }

  uint32_t writeMapBegin_virt(
      const TType keyType, const TType valType, const uint32_t size) override {
    return static_cast<Protocol_*>(this)->writeMapBegin(keyType, valType, size);
  }

  uint32_t writeMapEnd_virt() override {
    return static_cast<Protocol_*>(this)->writeMapEnd();
  }

  uint32_t writeListBegin_virt(
      const TType elemType, const uint32_t size) override {
    return static_cast<Protocol_*>(this)->writeListBegin(elemType, size);
  }

  uint32_t writeListEnd_virt() override {
    return static_cast<Protocol_*>(this)->writeListEnd();
  }

  uint32_t writeSetBegin_virt(
      const TType elemType, const uint32_t size) override {
    return static_cast<Protocol_*>(this)->writeSetBegin(elemType, size);
  }

  uint32_t writeSetEnd_virt() override {
    return static_cast<Protocol_*>(this)->writeSetEnd();
  }

  uint32_t writeBool_virt(const bool value) override {
    return static_cast<Protocol_*>(this)->writeBool(value);
  }

  uint32_t writeByte_virt(const int8_t byte) override {
    return static_cast<Protocol_*>(this)->writeByte(byte);
  }

  uint32_t writeI16_virt(const int16_t i16) override {
    return static_cast<Protocol_*>(this)->writeI16(i16);
  }

  uint32_t writeI32_virt(const int32_t i32) override {
    return static_cast<Protocol_*>(this)->writeI32(i32);
  }

  uint32_t writeI64_virt(const int64_t i64) override {
    return static_cast<Protocol_*>(this)->writeI64(i64);
  }

  uint32_t writeDouble_virt(const double dub) override {
    return static_cast<Protocol_*>(this)->writeDouble(dub);
  }

  uint32_t writeFloat_virt(const float flt) override {
    return static_cast<Protocol_*>(this)->writeFloat(flt);
  }

  uint32_t writeString_virt(const std::string& str) override {
    return static_cast<Protocol_*>(this)->writeString(str);
  }

  uint32_t writeBinary_virt(const std::string& str) override {
    return static_cast<Protocol_*>(this)->writeBinary(str);
  }

  /**
   * Reading functions
   */

  uint32_t readMessageBegin_virt(
      std::string& name, TMessageType& messageType, int32_t& seqid) override {
    return static_cast<Protocol_*>(this)->readMessageBegin(
        name, messageType, seqid);
  }

  uint32_t readMessageEnd_virt() override {
    return static_cast<Protocol_*>(this)->readMessageEnd();
  }

  void setNextStructType_virt(uint64_t reflection_id) override {
    return static_cast<Protocol_*>(this)->setNextStructType(reflection_id);
  }

  uint32_t readStructBegin_virt(std::string& name) override {
    return static_cast<Protocol_*>(this)->readStructBegin(name);
  }

  uint32_t readStructEnd_virt() override {
    return static_cast<Protocol_*>(this)->readStructEnd();
  }

  uint32_t readFieldBegin_virt(
      std::string& name, TType& fieldType, int16_t& fieldId) override {
    return static_cast<Protocol_*>(this)->readFieldBegin(
        name, fieldType, fieldId);
  }

  uint32_t readFieldEnd_virt() override {
    return static_cast<Protocol_*>(this)->readFieldEnd();
  }

  uint32_t readMapBegin_virt(
      TType& keyType,
      TType& valType,
      uint32_t& size,
      bool& sizeUnknown) override {
    return static_cast<Protocol_*>(this)->readMapBegin(
        keyType, valType, size, sizeUnknown);
  }

  bool peekMap_virt() override {
    return static_cast<Protocol_*>(this)->peekMap();
  }

  uint32_t readMapEnd_virt() override {
    return static_cast<Protocol_*>(this)->readMapEnd();
  }

  uint32_t readListBegin_virt(
      TType& elemType, uint32_t& size, bool& sizeUnkown) override {
    return static_cast<Protocol_*>(this)->readListBegin(
        elemType, size, sizeUnkown);
  }

  bool peekList_virt() override {
    return static_cast<Protocol_*>(this)->peekList();
  }

  uint32_t readListEnd_virt() override {
    return static_cast<Protocol_*>(this)->readListEnd();
  }

  uint32_t readSetBegin_virt(
      TType& elemType, uint32_t& size, bool& sizeUnknown) override {
    return static_cast<Protocol_*>(this)->readSetBegin(
        elemType, size, sizeUnknown);
  }

  bool peekSet_virt() override {
    return static_cast<Protocol_*>(this)->peekSet();
  }

  uint32_t readSetEnd_virt() override {
    return static_cast<Protocol_*>(this)->readSetEnd();
  }

  uint32_t readBool_virt(bool& value) override {
    return static_cast<Protocol_*>(this)->readBool(value);
  }

  uint32_t readBool_virt(std::vector<bool>::reference value) override {
    return static_cast<Protocol_*>(this)->readBool(value);
  }

  uint32_t readByte_virt(int8_t& byte) override {
    return static_cast<Protocol_*>(this)->readByte(byte);
  }

  uint32_t readI16_virt(int16_t& i16) override {
    return static_cast<Protocol_*>(this)->readI16(i16);
  }

  uint32_t readI32_virt(int32_t& i32) override {
    return static_cast<Protocol_*>(this)->readI32(i32);
  }

  uint32_t readI64_virt(int64_t& i64) override {
    return static_cast<Protocol_*>(this)->readI64(i64);
  }

  uint32_t readDouble_virt(double& dub) override {
    return static_cast<Protocol_*>(this)->readDouble(dub);
  }

  uint32_t readFloat_virt(float& flt) override {
    return static_cast<Protocol_*>(this)->readFloat(flt);
  }

  uint32_t readString_virt(std::string& str) override {
    return static_cast<Protocol_*>(this)->readString(str);
  }

  uint32_t readBinary_virt(std::string& str) override {
    return static_cast<Protocol_*>(this)->readBinary(str);
  }

  uint32_t skip_virt(TType type) override {
    return static_cast<Protocol_*>(this)->skip(type);
  }

  /*
   * Provide a default skip() implementation that uses non-virtual read
   * methods.
   *
   * Note: subclasses that use TVirtualProtocol to derive from another protocol
   * implementation (i.e., not TProtocolDefaults) should beware that this may
   * override any non-default skip() implementation provided by the parent
   * transport class.  They may need to explicitly redefine skip() to call the
   * correct parent implementation, if desired.
   */
  uint32_t skip(TType type) {
    Protocol_* const prot = static_cast<Protocol_*>(this);
    return ::apache::thrift::protocol::skip(*prot, type);
  }

  /*
   * Provide a default readBool() implementation for use with
   * std::vector<bool>, that behaves the same as reading into a normal bool.
   *
   * Subclasses can override this if desired, but there normally shouldn't
   * be a need to.
   */
  uint32_t readBool(std::vector<bool>::reference value) {
    bool b = false;
    uint32_t ret = static_cast<Protocol_*>(this)->readBool(b);
    value = b;
    return ret;
  }
  using Super_::readBool; // so we don't hide readBool(bool&)

 protected:
  explicit TVirtualProtocol(
      const std::shared_ptr<transport::TTransport>& ptrans)
      : Super_(ptrans) {}

  explicit TVirtualProtocol(transport::TTransport* ptrans) : Super_(ptrans) {}
};

} // namespace protocol
} // namespace thrift
} // namespace apache

#endif // #define _THRIFT_PROTOCOL_TVIRTUALPROTOCOL_H_ 1
