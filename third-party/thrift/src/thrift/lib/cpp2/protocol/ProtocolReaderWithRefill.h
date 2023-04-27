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

#include <folly/FBString.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/VirtualProtocol.h>

namespace apache {
namespace thrift {

template <typename ProtocolT>
class ProtocolReaderWithRefill;

using VirtualBinaryReader = ProtocolReaderWithRefill<BinaryProtocolReader>;
using VirtualCompactReader = ProtocolReaderWithRefill<CompactProtocolReader>;
using Refiller =
    std::function<std::unique_ptr<folly::IOBuf>(const uint8_t*, int, int, int)>;

/**
 * Used by python fastproto module. Read methods must check if there are
 * enough bytes to read. If not, call the refiller to read more from python.
 */
template <typename ProtocolT>
class ProtocolReaderWithRefill : public VirtualReader<ProtocolT> {
 public:
  explicit ProtocolReaderWithRefill(Refiller refiller)
      : refiller_(std::move(refiller)) {}

  uint32_t totalBytesRead() {
    return bufferLength_ - this->protocol_.in_.length();
  }

  using VirtualReader<ProtocolT>::setInput;
  void setInput(const folly::io::Cursor& cursor) override {
    VirtualReader<ProtocolT>::setInput(cursor);
    bufferLength_ = cursor.length();
  }

 protected:
  void ensureBuffer(uint32_t size) {
    if (this->protocol_.in_.length() >= size) {
      return;
    }

    auto avail = this->protocol_.in_.peekBytes();
    buffer_ = refiller_(avail.data(), avail.size(), totalBytesRead(), size);
    if (!buffer_) {
      throw std::runtime_error("Empty buffer returned when refilling");
    }
    setInput(buffer_.get());
  }

  Refiller refiller_;
  std::unique_ptr<folly::IOBuf> buffer_;
  uint32_t bufferLength_ = 0;
};

class CompactProtocolReaderWithRefill : public VirtualCompactReader {
 public:
  explicit CompactProtocolReaderWithRefill(Refiller refiller)
      : VirtualCompactReader(std::move(refiller)) {}

  inline void readMessageBegin(
      std::string& /* name */,
      MessageType& /* messageType */,
      int32_t& /* seqid */) override {
    // Only called in python so leave it unimplemented.
    throw std::runtime_error("not implemented");
  }

  inline void readFieldBegin(
      std::string& name, TType& fieldType, int16_t& fieldId) override {
    ensureFieldBegin();
    protocol_.readFieldBegin(name, fieldType, fieldId);
  }

  inline void readMapBegin(
      TType& keyType, TType& valType, uint32_t& size) override {
    ensureMapBegin();
    protocol_.readMapBegin(keyType, valType, size);
  }

  inline void readListBegin(TType& elemType, uint32_t& size) override {
    ensureListBegin();
    protocol_.readListBegin(elemType, size);
  }

  inline void readBool(bool& value) override {
    if (!protocol_.boolValue_.hasBoolValue) {
      ensureBuffer(1);
    }
    protocol_.readBool(value);
  }

  inline void readBool(std::vector<bool>::reference value) override {
    bool ret = false;
    readBool(ret);
    value = ret;
  }

  inline void readByte(int8_t& byte) override {
    ensureBuffer(1);
    protocol_.readByte(byte);
  }

  inline void readI16(int16_t& i16) override {
    ensureInteger();
    protocol_.readI16(i16);
  }

  inline void readI32(int32_t& i32) override {
    ensureInteger();
    protocol_.readI32(i32);
  }

  inline void readI64(int64_t& i64) override {
    ensureInteger();
    protocol_.readI64(i64);
  }

  inline void readDouble(double& dub) override {
    ensureBuffer(8);
    protocol_.readDouble(dub);
  }

  inline void readFloat(float& flt) override {
    ensureBuffer(4);
    protocol_.readFloat(flt);
  }

  inline void readString(std::string& str) override { readStringImpl(str); }

  inline void readString(folly::fbstring& str) override { readStringImpl(str); }

  inline void readBinary(std::string& str) override { readStringImpl(str); }

  inline void readBinary(folly::fbstring& str) override { readStringImpl(str); }

  inline void readBinary(apache::thrift::detail::SkipNoopString& str) override {
    readStringImpl(str);
  }

  inline void readBinary(std::unique_ptr<folly::IOBuf>& str) override {
    readBinaryIOBufImpl(str);
  }

  inline void readBinary(folly::IOBuf& str) override {
    readBinaryIOBufImpl(str);
  }

  inline void skip(TType type) override { apache::thrift::skip(*this, type); }

  inline void skipBytes(size_t bytes) override {
    ensureBuffer(bytes);
    protocol_.skipBytes(bytes);
  }

 private:
  /**
   * Make sure a varint can be read from the current buffer after idx bytes.
   * If not, call the refiller to read more bytes.
   *
   * A varint is stored with up to 10 bytes and only the last byte's
   * MSB isn't set. If the current buffer size is >= idx + 10, return. The
   * following call to readVarint may still fail if the first 10 bytes
   * all have MSB set, but it's not the problem to be addressed here.
   *
   * Otherwise, check if a byte with MSB not set can be found. If so, return.
   * Otherwise, call the refiller to ask for 1 more byte because the exact
   * size of the varint is still unknown but at least 1 more byte is required.
   * A sane transport reads more data even if asked for just 1 byte so this
   * should not cause any performance problem. After the new buffer is ready,
   * start all over again.
   **/
  void ensureInteger(size_t idx = 0) {
    while (protocol_.in_.length() - idx < 10) {
      if (protocol_.in_.length() <= idx) {
        ensureBuffer(idx + 1);
      } else {
        auto avail = protocol_.in_.peekBytes();
        const uint8_t* b = avail.data() + idx;
        while (idx < avail.size()) {
          if (!(*b++ & 0x80)) {
            return;
          }
          idx++;
        }

        ensureBuffer(avail.size() + 1);
      }
    }
  }

  void ensureFieldBegin() {
    // Fast path: at most 4 bytes are needed to read field begin.
    if (protocol_.in_.length() >= 4) {
      return;
    }

    // At least 1 byte is needed to read ftype.
    ensureBuffer(1);
    if (protocol_.in_.length() >= 4) {
      return;
    }
    auto avail = protocol_.in_.peekBytes();
    const uint8_t* b = avail.data();
    int8_t byte = folly::Endian::big(*b);
    int8_t type = (byte & 0x0f);

    if (type == TType::T_STOP) {
      return;
    }

    int16_t modifier = (int16_t)(((uint8_t)byte & 0xf0) >> 4);
    if (modifier == 0) {
      ensureInteger(1);
    }
  }

  void ensureMapBegin() {
    // Fast path: at most 11 bytes are needed to read map begin.
    if (protocol_.in_.length() >= 11) {
      return;
    }

    ensureInteger();
    if (protocol_.in_.length() >= 11) {
      return;
    }

    auto avail = protocol_.in_.peekBytes();
    const uint8_t* b = avail.data();
    size_t bytes = 1;
    while (bytes <= avail.size()) {
      if (!(*b++ & 0x80)) {
        break;
      }
      bytes++;
    }
    // Non-empty maps have an additional byte for the key/value type.
    if (bytes == avail.size() && *avail.data()) {
      ensureBuffer(avail.size() + 1);
    }
  }

  void ensureListBegin() {
    // Fast path: at most 11 bytes are needed to read list begin.
    if (protocol_.in_.length() >= 11) {
      return;
    }

    ensureBuffer(1);
    auto avail = protocol_.in_.peekBytes();
    const uint8_t* b = avail.data();
    int8_t size_and_type = folly::Endian::big(*b);
    int32_t lsize = ((uint8_t)size_and_type >> 4) & 0x0f;
    if (lsize == 15) {
      ensureInteger(1);
    }
  }

  template <typename StrType>
  void readStringImpl(StrType& str) {
    ensureInteger();
    int32_t size = 0;
    protocol_.readStringSize(size);

    ensureBuffer(size);
    protocol_.readStringBody(str, size);
  }

  template <typename StrType>
  void readBinaryIOBufImpl(StrType& str) {
    ensureInteger();
    int32_t size = 0;
    protocol_.readStringSize(size);

    ensureBuffer(size);
    protocol_.in_.clone(str, size);
  }
};

class BinaryProtocolReaderWithRefill : public VirtualBinaryReader {
 public:
  explicit BinaryProtocolReaderWithRefill(Refiller refiller)
      : VirtualBinaryReader(std::move(refiller)) {}

  inline void readMessageBegin(
      std::string& /* name */,
      MessageType& /* messageType */,
      int32_t& /* seqid */) override {
    // This is only called in python so leave it unimplemented.
    throw std::runtime_error("not implemented");
  }

  inline void readFieldBegin(
      std::string& /* name */, TType& fieldType, int16_t& fieldId) override {
    int8_t type;
    readByte(type);
    fieldType = (TType)type;
    if (fieldType == TType::T_STOP) {
      fieldId = 0;
      return;
    }
    readI16(fieldId);
  }

  inline void readMapBegin(
      TType& keyType, TType& valType, uint32_t& size) override {
    ensureBuffer(6);
    protocol_.readMapBegin(keyType, valType, size);
  }

  inline void readListBegin(TType& elemType, uint32_t& size) override {
    ensureBuffer(5);
    protocol_.readListBegin(elemType, size);
  }

  inline void readSetBegin(TType& elemType, uint32_t& size) override {
    readListBegin(elemType, size);
  }

  inline void readBool(bool& value) override {
    ensureBuffer(1);
    protocol_.readBool(value);
  }

  inline void readBool(std::vector<bool>::reference value) override {
    ensureBuffer(1);
    protocol_.readBool(value);
  }

  inline void readByte(int8_t& byte) override {
    ensureBuffer(1);
    protocol_.readByte(byte);
  }

  inline void readI16(int16_t& i16) override {
    ensureBuffer(2);
    protocol_.readI16(i16);
  }

  inline void readI32(int32_t& i32) override {
    ensureBuffer(4);
    protocol_.readI32(i32);
  }

  inline void readI64(int64_t& i64) override {
    ensureBuffer(8);
    protocol_.readI64(i64);
  }

  inline void readDouble(double& dub) override {
    ensureBuffer(8);
    protocol_.readDouble(dub);
  }

  inline void readFloat(float& flt) override {
    ensureBuffer(4);
    protocol_.readFloat(flt);
  }

  inline void readString(std::string& str) override { readStringImpl(str); }

  inline void readString(folly::fbstring& str) override { readStringImpl(str); }

  inline void readBinary(std::string& str) override { readStringImpl(str); }

  inline void readBinary(folly::fbstring& str) override { readStringImpl(str); }

  inline void readBinary(apache::thrift::detail::SkipNoopString& str) override {
    readStringImpl(str);
  }

  inline void readBinary(std::unique_ptr<folly::IOBuf>& str) override {
    readBinaryIOBufImpl(str);
  }

  inline void readBinary(folly::IOBuf& str) override {
    readBinaryIOBufImpl(str);
  }

  inline void skip(TType type) override { apache::thrift::skip(*this, type); }

  inline void skipBytes(size_t bytes) override {
    ensureBuffer(bytes);
    protocol_.skipBytes(bytes);
  }

 private:
  template <typename StrType>
  void readStringImpl(StrType& str) {
    int32_t size;
    readI32(size);
    protocol_.checkStringSize(size);

    ensureBuffer(size);
    protocol_.readStringBody(str, size);
  }

  template <typename StrType>
  void readBinaryIOBufImpl(StrType& str) {
    int32_t size;
    readI32(size);
    protocol_.checkStringSize(size);

    ensureBuffer(size);
    protocol_.in_.clone(str, size);
  }
};

template <>
inline bool canReadNElements(
    CompactProtocolReaderWithRefill& /* prot */,
    uint32_t /* n */,
    std::initializer_list<TType> /* types */) {
  return true;
}

template <>
inline bool canReadNElements(
    BinaryProtocolReaderWithRefill& /* prot */,
    uint32_t /* n */,
    std::initializer_list<TType> /* types */) {
  return true;
}

} // namespace thrift
} // namespace apache
