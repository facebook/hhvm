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

#ifndef CPP2_PROTOCOL_VIRTUALPROTOCOL_H_
#define CPP2_PROTOCOL_VIRTUALPROTOCOL_H_ 1

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <folly/FBString.h>
#include <folly/io/Cursor.h>
#include <thrift/lib/cpp/Thrift.h>
#include <thrift/lib/cpp/protocol/TProtocol.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>

namespace apache::thrift {

class VirtualReaderBase {
 public:
  VirtualReaderBase() {}
  virtual ~VirtualReaderBase() = default;
  VirtualReaderBase(const VirtualReaderBase&) = delete;
  VirtualReaderBase& operator=(const VirtualReaderBase&) = delete;
  VirtualReaderBase(VirtualReaderBase&&) = default;
  VirtualReaderBase& operator=(VirtualReaderBase&&) = default;

  virtual ProtocolType protocolType() const = 0;
  virtual bool kUsesFieldNames() const = 0;
  virtual bool kOmitsContainerSizes() const = 0;
  virtual bool kOmitsStringSizes() const = 0;
  virtual bool kHasDeferredRead() const = 0;

  virtual void setInput(const folly::io::Cursor& cursor) = 0;
  void setInput(const folly::IOBuf* buf) { setInput(folly::io::Cursor(buf)); }

  virtual void readMessageBegin(
      std::string& name, MessageType& messageType, int32_t& seqid) = 0;
  virtual void readMessageEnd() = 0;
  virtual void readStructBegin(std::string& name) = 0;
  virtual void readStructEnd() = 0;
  virtual void readFieldBegin(
      std::string& name, TType& fieldType, int16_t& fieldId) = 0;
  virtual void readFieldEnd() = 0;
  virtual void readMapBegin(TType& keyType, TType& valType, uint32_t& size) = 0;
  virtual void readMapEnd() = 0;
  virtual void readListBegin(TType& elemType, uint32_t& size) = 0;
  virtual void readListEnd() = 0;
  virtual void readSetBegin(TType& elemType, uint32_t& size) = 0;
  virtual void readSetEnd() = 0;
  virtual void readBool(bool& value) = 0;
  virtual void readBool(std::vector<bool>::reference value) = 0;
  virtual void readByte(int8_t& byte) = 0;
  virtual void readI16(int16_t& i16) = 0;
  virtual void readI32(int32_t& i32) = 0;
  virtual void readI64(int64_t& i64) = 0;
  virtual void readDouble(double& dub) = 0;
  virtual void readFloat(float& flt) = 0;
  virtual void readString(std::string& str) = 0;
  virtual void readString(folly::fbstring& str) = 0;
  virtual void readBinary(std::string& str) = 0;
  virtual void readBinary(folly::fbstring& str) = 0;
  virtual void readBinary(apache::thrift::detail::SkipNoopString& str) = 0;
  virtual void readBinary(std::unique_ptr<folly::IOBuf>& str) = 0;
  virtual void readBinary(folly::IOBuf& str) = 0;
  virtual size_t fixedSizeInContainer(TType) const = 0;
  virtual void skipBytes(size_t bytes) = 0;
  virtual void skip(TType type, int depth = 0) = 0;
  virtual const folly::io::Cursor& getCursor() const = 0;
  virtual size_t getCursorPosition() const = 0;
  virtual bool peekMap() { return false; }
  virtual bool peekSet() { return false; }
  virtual bool peekList() { return false; }
};

std::unique_ptr<VirtualReaderBase> makeVirtualReader(ProtocolType type);

template <class ProtocolT>
class VirtualReader : public VirtualReaderBase {
 public:
  using ProtocolWriter = typename ProtocolT::ProtocolWriter;

  template <
      typename... Args,
      std::enable_if_t<std::is_constructible_v<ProtocolT, Args...>, bool> =
          false>
  explicit VirtualReader(Args&&... args)
      : protocol_(std::forward<Args>(args)...) {}
  ~VirtualReader() override = default;
  VirtualReader(const VirtualReader&) = delete;
  VirtualReader& operator=(const VirtualReader&) = delete;
  VirtualReader(VirtualReader&&) = default;
  VirtualReader& operator=(VirtualReader&&) = default;

  ProtocolType protocolType() const override {
    return protocol_.protocolType();
  }

  bool kUsesFieldNames() const override { return protocol_.kUsesFieldNames(); }

  bool kOmitsContainerSizes() const override {
    return protocol_.kOmitsContainerSizes();
  }

  bool kOmitsStringSizes() const override {
    return protocol_.kOmitsStringSizes();
  }

  bool kHasDeferredRead() const override {
    return protocol_.kHasDeferredRead();
  }

  using VirtualReaderBase::setInput;
  void setInput(const folly::io::Cursor& cursor) override {
    protocol_.setInput(cursor);
  }

  void readMessageBegin(
      std::string& name, MessageType& messageType, int32_t& seqid) override {
    protocol_.readMessageBegin(name, messageType, seqid);
  }
  void readMessageEnd() override { protocol_.readMessageEnd(); }
  void readStructBegin(std::string& name) override {
    protocol_.readStructBegin(name);
  }
  void readStructEnd() override { protocol_.readStructEnd(); }
  void readFieldBegin(
      std::string& name, TType& fieldType, int16_t& fieldId) override {
    protocol_.readFieldBegin(name, fieldType, fieldId);
  }
  void readFieldEnd() override { protocol_.readFieldEnd(); }
  void readMapBegin(TType& keyType, TType& valType, uint32_t& size) override {
    protocol_.readMapBegin(keyType, valType, size);
  }
  void readMapEnd() override { protocol_.readMapEnd(); }
  void readListBegin(TType& elemType, uint32_t& size) override {
    protocol_.readListBegin(elemType, size);
  }
  void readListEnd() override { protocol_.readListEnd(); }
  void readSetBegin(TType& elemType, uint32_t& size) override {
    protocol_.readSetBegin(elemType, size);
  }
  void readSetEnd() override { protocol_.readSetEnd(); }
  void readBool(bool& value) override { protocol_.readBool(value); }
  void readBool(std::vector<bool>::reference value) override {
    protocol_.readBool(value);
  }
  void readByte(int8_t& byte) override { protocol_.readByte(byte); }
  void readI16(int16_t& i16) override { protocol_.readI16(i16); }
  void readI32(int32_t& i32) override { protocol_.readI32(i32); }
  void readI64(int64_t& i64) override { protocol_.readI64(i64); }
  void readDouble(double& dub) override { protocol_.readDouble(dub); }
  void readFloat(float& flt) override { protocol_.readFloat(flt); }
  void readString(std::string& str) override { protocol_.readString(str); }
  void readString(folly::fbstring& str) override { protocol_.readString(str); }
  void readBinary(std::string& str) override { protocol_.readBinary(str); }
  void readBinary(folly::fbstring& str) override { protocol_.readBinary(str); }
  void readBinary(apache::thrift::detail::SkipNoopString& str) override {
    protocol_.readBinary(str);
  }
  void readBinary(std::unique_ptr<folly::IOBuf>& str) override {
    protocol_.readBinary(str);
  }
  void readBinary(folly::IOBuf& str) override { protocol_.readBinary(str); }
  size_t fixedSizeInContainer(TType type) const override {
    return protocol_.fixedSizeInContainer(type);
  }
  void skipBytes(size_t bytes) override { protocol_.skipBytes(bytes); }
  void skip(TType type, int depth = 0) override { protocol_.skip(type, depth); }
  const folly::io::Cursor& getCursor() const override {
    return protocol_.getCursor();
  }
  size_t getCursorPosition() const override {
    return protocol_.getCursorPosition();
  }

 protected:
  ProtocolT protocol_;
};

} // namespace apache::thrift

#endif
