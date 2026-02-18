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

#ifndef CPP2_PROTOCOL_TBINARYPROTOCOL_H_
#define CPP2_PROTOCOL_TBINARYPROTOCOL_H_ 1

#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <folly/lang/Bits.h>
#include <folly/portability/GFlags.h>
#include <thrift/lib/cpp/protocol/TProtocol.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>
FOLLY_GFLAGS_DECLARE_int32(thrift_cpp2_protocol_reader_string_limit);
FOLLY_GFLAGS_DECLARE_int32(thrift_cpp2_protocol_reader_container_limit);

namespace apache::thrift {

using folly::IOBuf;
using folly::IOBufQueue;

using RWCursor = folly::io::RWPrivateCursor;
using folly::io::Cursor;
using folly::io::QueueAppender;

class BinaryProtocolReader;

/**
 * The default binary protocol for thrift. Writes all data in a very basic
 * binary format, essentially just spitting out the raw bytes.
 *
 */
class BinaryProtocolWriter : public detail::ProtocolBase {
 public:
  static const int32_t VERSION_1 = 0x80010000;

  using ProtocolReader = BinaryProtocolReader;

  explicit BinaryProtocolWriter(
      ExternalBufferSharing sharing = COPY_EXTERNAL_BUFFER)
      : out_(nullptr, 0), sharing_(sharing) {}

  static constexpr ProtocolType protocolType() {
    return ProtocolType::T_BINARY_PROTOCOL;
  }

  static constexpr bool kSortKeys() { return false; }

  static constexpr bool kHasIndexSupport() { return true; }

  static constexpr bool kSupportsArithmeticVectors() { return true; }

  /**
   * ...
   * The IOBuf itself is managed by the caller.
   * It must exist for the life of the BinaryProtocol as well,
   * or until the output is reset with setOutput/Input(NULL), or
   * set to some other buffer.
   */
  void setOutput(
      IOBufQueue* queue,
      size_t maxGrowth = std::numeric_limits<size_t>::max()) {
    // Allocate 16KB at a time; leave some room for the IOBuf overhead
    constexpr size_t kDesiredGrowth = (1 << 14) - 64;
    out_.reset(queue, std::min(maxGrowth, kDesiredGrowth));
  }

  void setOutput(QueueAppender&& output) { out_ = std::move(output); }

  uint32_t writeMessageBegin(
      folly::StringPiece name, MessageType messageType, int32_t seqid);
  uint32_t writeMessageEnd();
  uint32_t writeStructBegin(const char* name);
  uint32_t writeStructEnd();
  uint32_t writeFieldBegin(const char* name, TType fieldType, int16_t fieldId);
  uint32_t writeFieldEnd();
  uint32_t writeFieldStop();
  uint32_t writeMapBegin(TType keyType, TType valType, uint32_t size);
  uint32_t writeMapEnd();
  uint32_t writeListBegin(TType elemType, uint32_t size);
  uint32_t writeListEnd();
  uint32_t writeSetBegin(TType elemType, uint32_t size);
  uint32_t writeSetEnd();
  uint32_t writeBool(bool value);
  uint32_t writeByte(int8_t byte);
  uint32_t writeI16(int16_t i16);
  uint32_t writeI32(int32_t i32);
  uint32_t writeI64(int64_t i64);
  template <typename T>
  size_t writeArithmeticVector(const T* inputPtr, size_t numElements);
  uint32_t writeDouble(double dub);
  uint32_t writeFloat(float flt);
  uint32_t writeString(folly::StringPiece str);
  uint32_t writeBinary(folly::StringPiece str);
  uint32_t writeBinary(folly::ByteRange str);
  uint32_t writeBinary(
      const std::unique_ptr<folly::IOBuf>& str, bool pack = true);
  uint32_t writeBinary(const folly::IOBuf& str, bool pack = true);
  uint32_t writeRaw(const IOBuf& buf);
  uint32_t writeRaw(folly::io::Cursor cursor, uint32_t size);

  /**
   * Functions that return the [estimated] serialized size
   * Notes:
   * * Serialized size estimates for Binary protocol are generally accurate,
   *   but this is not the case for other protocols, e.g. Compact.
   *   Don't use these values as more than an estimate.
   *
   * * ZC versions are the preallocated estimate if any IOBufs are shared (i.e.
   *   there are IOBuf fields, and their sizes aren't too small to be packed),
   *   and won't count in the ZC estimate.
   */
  uint32_t serializedMessageSize(folly::StringPiece name) const;
  uint32_t serializedFieldSize(
      const char* name, TType fieldType, int16_t fieldId) const;
  uint32_t serializedStructSize(const char* name) const;
  uint32_t serializedSizeMapBegin(
      TType keyType, TType valType, uint32_t size) const;
  uint32_t serializedSizeMapEnd() const;
  uint32_t serializedSizeListBegin(TType elemType, uint32_t size) const;
  uint32_t serializedSizeListEnd() const;
  uint32_t serializedSizeSetBegin(TType elemType, uint32_t size) const;
  uint32_t serializedSizeSetEnd() const;
  uint32_t serializedSizeStop() const;
  uint32_t serializedSizeBool(bool = false) const;
  uint32_t serializedSizeByte(int8_t = 0) const;
  uint32_t serializedSizeI16(int16_t = 0) const;
  uint32_t serializedSizeI32(int32_t = 0) const;
  uint32_t serializedSizeI64(int64_t = 0) const;
  uint32_t serializedSizeDouble(double = 0.0) const;
  uint32_t serializedSizeFloat(float = 0) const;
  uint32_t serializedSizeString(folly::StringPiece str) const;
  uint32_t serializedSizeBinary(folly::StringPiece str) const;
  uint32_t serializedSizeBinary(folly::ByteRange) const;
  uint32_t serializedSizeBinary(
      const std::unique_ptr<folly::IOBuf>& v, bool pack = true) const;
  uint32_t serializedSizeBinary(const folly::IOBuf& v, bool pack = true) const;
  uint32_t serializedSizeZCBinary(folly::StringPiece str) const;
  uint32_t serializedSizeZCBinary(folly::ByteRange v) const;
  uint32_t serializedSizeZCBinary(
      const std::unique_ptr<folly::IOBuf>& v, bool pack = true) const;
  uint32_t serializedSizeZCBinary(
      const folly::IOBuf& v, bool pack = true) const;

  void rewriteDouble(double dub, int64_t offset);

  // Get last n bytes we just wrote
  folly::io::Cursor tail(size_t n);

  uint8_t* ensure(size_t n);
  void advance(size_t n);

 private:
  static void checkBinarySize(uint64_t size);
  template <bool kWriteSize>
  FOLLY_ERASE uint32_t writeBinaryImpl(const folly::IOBuf& str, bool pack);

  /**
   * Cursor to write the data out to.
   */
  QueueAppender out_;
  ExternalBufferSharing sharing_;
};

class BinaryProtocolReader : public detail::ProtocolBase {
 public:
  static const int32_t VERSION_MASK = 0xffff0000;
  static const int32_t VERSION_1 = 0x80010000;

  using ProtocolWriter = BinaryProtocolWriter;

  explicit BinaryProtocolReader(
      ExternalBufferSharing sharing = COPY_EXTERNAL_BUFFER)
      : string_limit_(FLAGS_thrift_cpp2_protocol_reader_string_limit),
        container_limit_(FLAGS_thrift_cpp2_protocol_reader_container_limit),
        sharing_(sharing),
        strict_read_(true),
        in_(nullptr) {}

  BinaryProtocolReader(
      int32_t string_limit,
      int32_t container_limit,
      ExternalBufferSharing sharing = COPY_EXTERNAL_BUFFER)
      : string_limit_(string_limit),
        container_limit_(container_limit),
        sharing_(sharing),
        strict_read_(true),
        in_(nullptr) {}

  static constexpr ProtocolType protocolType() {
    return ProtocolType::T_BINARY_PROTOCOL;
  }

  static constexpr bool kUsesFieldNames() { return false; }

  static constexpr bool kOmitsContainerSizes() { return false; }

  static constexpr bool kOmitsStringSizes() { return false; }

  static constexpr bool kHasDeferredRead() { return false; }

  static constexpr bool kCanReadStringView() { return true; }

  static constexpr bool kSupportsArithmeticVectors() { return true; }

  void setStringSizeLimit(int32_t string_limit) {
    string_limit_ = string_limit;
  }

  void setContainerSizeLimit(int32_t container_limit) {
    container_limit_ = container_limit;
  }

  void setStrict(bool strict_read = true) { strict_read_ = strict_read; }

  /**
   * The IOBuf itself is managed by the caller.
   * It must exist for the life of the BinaryProtocol as well,
   * or until the output is reset with setOutput/Input(NULL), or
   * set to some other buffer.
   */
  void setInput(const Cursor& cursor) { in_ = cursor; }
  void setInput(const IOBuf* buf) { in_.reset(buf); }

  /**
   * Reading functions
   */
  void readMessageBegin(
      std::string& name, MessageType& messageType, int32_t& seqid);
  void readMessageEnd();
  void readStructBegin(std::string& name);
  void readStructEnd();
  void readFieldBegin(std::string& name, TType& fieldType, int16_t& fieldId);
  void readFieldEnd();
  void readMapBegin(TType& keyType, TType& valType, uint32_t& size);
  void readMapEnd();
  void readListBegin(TType& elemType, uint32_t& size);
  void readListEnd();
  void readSetBegin(TType& elemType, uint32_t& size);
  void readSetEnd();
  void readBool(bool& value);
  void readBool(std::vector<bool>::reference value);
  void readByte(int8_t& byte);
  void readI16(int16_t& i16);
  void readI32(int32_t& i32);
  void readI64(int64_t& i64);
  template <typename T>
  void readArithmeticVector(T* outputPtr, size_t numElements);
  void readDouble(double& dub);
  void readFloat(float& flt);
  template <typename StrType>
  void readString(StrType& str);
  template <typename StrType>
  void readBinary(StrType& str);
  void readBinary(std::unique_ptr<folly::IOBuf>& str);
  void readBinary(folly::IOBuf& str);
  void readStringSize(int32_t& size) {
    readI32(size);
    checkStringSize(size);
  }
  bool peekMap() { return false; }
  bool peekSet() { return false; }
  bool peekList() { return false; }

  static constexpr std::size_t fixedSizeInContainer(TType type);
  void skipBytes(size_t bytes) { in_.skip(bytes); }
  void skip(TType type, int depth = 0);

  const Cursor& getCursor() const { return in_; }

  size_t getCursorPosition() const { return in_.getCurrentPosition(); }

  struct StructReadState;

 protected:
  template <typename StrType>
  void readStringBody(StrType& str, int32_t sz);

  FOLLY_ALWAYS_INLINE bool advanceToNextField(
      int16_t nextFieldId, TType nextFieldType, StructReadState& state);

  void readFieldBeginWithState(StructReadState& state);

  void checkStringSize(int32_t size);
  void checkContainerSize(int32_t size);

  [[noreturn]] static void throwBadVersionIdentifier(int32_t sz);
  [[noreturn]] static void throwMissingVersionIdentifier(int32_t sz);

  int32_t string_limit_;
  int32_t container_limit_;
  ExternalBufferSharing sharing_;

  // Enforce presence of version identifier
  bool strict_read_;

  /**
   * Cursor to manipulate the buffer to read from.  Throws an exception if
   * there is not enough data to read the whole struct.
   */
  Cursor in_;

  template <typename T>
  friend class ProtocolReaderWithRefill;
  friend class BinaryProtocolReaderWithRefill;

 private:
  bool readBoolSafe();
};

struct BinaryProtocolReader::StructReadState {
  int16_t fieldId;
  apache::thrift::protocol::TType fieldType;

  constexpr static bool kAcceptsContext = false;

  void readStructBegin(BinaryProtocolReader* /*iprot*/) {}

  void readStructEnd(BinaryProtocolReader* /*iprot*/) {}

  void readFieldBegin(BinaryProtocolReader* iprot) {
    iprot->readFieldBeginWithState(*this);
  }

  FOLLY_NOINLINE void readFieldBeginNoInline(BinaryProtocolReader* iprot) {
    iprot->readFieldBeginWithState(*this);
  }

  void readFieldEnd(BinaryProtocolReader* /*iprot*/) {}

  FOLLY_ALWAYS_INLINE bool advanceToNextField(
      BinaryProtocolReader* iprot,
      int16_t /*currFieldId*/,
      int16_t nextFieldId,
      TType nextFieldType) {
    return iprot->advanceToNextField(nextFieldId, nextFieldType, *this);
  }

  /*
   * This is used in generated deserialization code only. When deserializing
   * fields in "non-advanceToNextField" case, we delegate the type check to
   * each protocol since some protocol may not encode type information.
   */
  FOLLY_ALWAYS_INLINE bool isCompatibleWithType(
      BinaryProtocolReader* /*iprot*/, TType expectedFieldType) {
    return fieldType == expectedFieldType;
  }

  void skip(BinaryProtocolReader* iprot) { iprot->skip(fieldType); }

  std::string& fieldName() {
    throw std::logic_error("BinaryProtocol doesn't support field names");
  }

  void afterAdvanceFailure(BinaryProtocolReader* /*iprot*/) {}

  void beforeSubobject(BinaryProtocolReader* /* iprot */) {}
  void afterSubobject(BinaryProtocolReader* /* iprot */) {}

  bool atStop() { return fieldType == apache::thrift::protocol::T_STOP; }

  template <typename StructTraits>
  void fillFieldTraitsFromName() {
    throw std::logic_error("BinaryProtocol doesn't support field names");
  }
};

static_assert(!usesFieldNames<BinaryProtocolReader>());
static_assert(!usesFieldNames<BinaryProtocolWriter>());

namespace detail {

template <class Protocol>
struct ProtocolReaderStructReadState;

template <>
struct ProtocolReaderStructReadState<BinaryProtocolReader>
    : BinaryProtocolReader::StructReadState {};

} // namespace detail
} // namespace apache::thrift

#include <thrift/lib/cpp2/protocol/BinaryProtocol-inl.h>

#endif // #ifndef CPP2_PROTOCOL_TBINARYPROTOCOL_H_
