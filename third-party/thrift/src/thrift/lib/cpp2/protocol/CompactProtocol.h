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

#ifndef CPP2_PROTOCOL_COMPACTPROTOCOL_H_
#define CPP2_PROTOCOL_COMPACTPROTOCOL_H_ 1

#include <stack>

#include <folly/FBVector.h>
#include <folly/Portability.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <folly/lang/Bits.h>
#include <folly/portability/GFlags.h>
#include <folly/small_vector.h>
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

namespace detail::compact {

static const int8_t COMPACT_PROTOCOL_VERSION = 0x02;
static const int32_t VERSION_2 = 0x82020000;
static const int8_t PROTOCOL_ID = static_cast<int8_t>(0x82);
static const int8_t TYPE_MASK = static_cast<int8_t>(0xE0);
static const int32_t TYPE_SHIFT_AMOUNT = 5;

} // namespace detail::compact

class CompactProtocolReader;

/**
 * C++ Implementation of the Compact Protocol as described in THRIFT-110
 */
class CompactProtocolWriter : public detail::ProtocolBase {
 public:
  using ProtocolReader = CompactProtocolReader;

  explicit CompactProtocolWriter(
      ExternalBufferSharing sharing = COPY_EXTERNAL_BUFFER)
      : out_(nullptr, 0),
        sharing_(sharing),
        booleanField_({nullptr, TType::T_BOOL, 0}) {}

  static constexpr ProtocolType protocolType() {
    return ProtocolType::T_COMPACT_PROTOCOL;
  }

  static constexpr bool kSortKeys() { return false; }

  static constexpr bool kHasIndexSupport() { return true; }

  static constexpr bool kSupportsArithmeticVectors() {
#if FOLLY_ARM_FEATURE_NEON_SVE_BRIDGE
    return true;
#else
    return false;
#endif
  }

  /**
   * The IOBufQueue itself is managed by the caller.
   * It must exist for the life of the CompactProtocol as well,
   * or until the output is reset with setOutput/Input(NULL), or
   * set to some other backing storage buffer.
   */
  inline void setOutput(
      IOBufQueue* storage,
      size_t maxGrowth = std::numeric_limits<size_t>::max()) {
    // Allocate 16KB at a time; leave some room for the IOBuf overhead
    constexpr size_t kDesiredGrowth = (1 << 14) - 64;
    out_.reset(storage, std::min(kDesiredGrowth, maxGrowth));
  }

  inline void setOutput(QueueAppender&& output) { out_ = std::move(output); }

  uint32_t writeMessageBegin(
      folly::StringPiece name, MessageType messageType, int32_t seqid);
  uint32_t writeMessageEnd();
  uint32_t writeStructBegin(const char* name);
  uint32_t writeStructEnd();
  inline uint32_t writeFieldBegin(
      const char* name, TType fieldType, int16_t fieldId) {
    return writeFieldBegin(name, fieldType, fieldId, lastFieldId_);
  }
  FOLLY_ALWAYS_INLINE uint32_t writeFieldBegin(
      const char* name, TType fieldType, int16_t fieldId, int16_t previousId);
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
  uint32_t writeBinary(const std::unique_ptr<IOBuf>& str);
  uint32_t writeBinary(const IOBuf& str);
  uint32_t writeRaw(const IOBuf& buf);

  /**
   * Functions that return the serialized size
   *
   * Notes:
   *  * Serialized size is intended to be an upper bound, rather than an exact
   *    value, since we don't want to unnecessarily pay varint encoding costs.
   *    Don't use rely on these values as more than an estimate.
   *
   *  * ZC versions are the preallocated estimate if any IOBufs are shared (i.e.
   *    there are IOBuf fields, and their sizes aren't too small to be packed),
   *    and won't count in the ZC estimate.
   *
   *    Note that we still may not pre-allocate ideally for the IOBuf case,
   *    since the IOBuf might be in the middle of the serialized stream.
   */
  uint32_t serializedMessageSize(folly::StringPiece name) const;
  uint32_t serializedFieldSize(
      const char* name, TType fieldType, int16_t fieldId) const;
  uint32_t serializedStructSize(const char* name) const;
  inline uint32_t serializedSizeMapBegin(
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
  uint32_t serializedSizeBinary(folly::ByteRange v) const;
  uint32_t serializedSizeBinary(const std::unique_ptr<IOBuf>& v) const;
  uint32_t serializedSizeBinary(const IOBuf& v) const;
  uint32_t serializedSizeZCBinary(folly::StringPiece str) const;
  uint32_t serializedSizeZCBinary(folly::ByteRange v) const;
  uint32_t serializedSizeZCBinary(const std::unique_ptr<IOBuf>& /*v*/) const;
  uint32_t serializedSizeZCBinary(const IOBuf& /*v*/) const;

  void rewriteDouble(double dub, int64_t offset);

  // Get last n bytes we just wrote
  folly::io::Cursor tail(size_t n);

 protected:
  /**
   * Cursor to write the data out to. Must support some of the interface of
   * folly::io::QueueAppender, notably, reset(), push(), and
   * write()/writeBE()/writeLE().
   */
  QueueAppender out_;

 private:
  ExternalBufferSharing sharing_;

  struct {
    const char* name;
    TType fieldType;
    int16_t fieldId;
  } booleanField_;

 private:
  std::stack<int16_t, folly::small_vector<int16_t, 10>> lastField_;
  int16_t lastFieldId_{-1};

  uint32_t writeCollectionBegin(int8_t elemType, int32_t size);
  static void checkBinarySize(uint64_t size);
  template <bool kWriteSize>
  FOLLY_ERASE uint32_t writeBinaryImpl(const folly::IOBuf& str);

  uint32_t writeFieldBeginInternal(
      const char* name,
      const TType fieldType,
      const int16_t fieldId,
      int8_t typeOverride,
      int16_t previousId);
};

class CompactProtocolReader : public detail::ProtocolBase {
 public:
  static const int8_t VERSION_MASK = 0x1f; // 0001 1111

  using ProtocolWriter = CompactProtocolWriter;

  explicit CompactProtocolReader(
      ExternalBufferSharing sharing = COPY_EXTERNAL_BUFFER)
      : in_(nullptr),
        string_limit_(FLAGS_thrift_cpp2_protocol_reader_string_limit),
        container_limit_(FLAGS_thrift_cpp2_protocol_reader_container_limit),
        sharing_(sharing),

        boolValue_({false, false}) {}

  CompactProtocolReader(
      int32_t string_limit,
      int32_t container_limit,
      ExternalBufferSharing sharing = COPY_EXTERNAL_BUFFER)
      : in_(nullptr),
        string_limit_(string_limit),
        container_limit_(container_limit),
        sharing_(sharing),
        boolValue_({false, false}) {}

  static constexpr ProtocolType protocolType() {
    return ProtocolType::T_COMPACT_PROTOCOL;
  }

  static constexpr bool kUsesFieldNames() { return false; }

  static constexpr bool kOmitsContainerSizes() { return false; }

  static constexpr bool kOmitsStringSizes() { return false; }

  static constexpr bool kHasDeferredRead() { return true; }

  static constexpr bool kSupportsArithmeticVectors() {
    // Disabling on x86 due to seeing regressions in some tests
#if FOLLY_ARM_FEATURE_NEON_SVE_BRIDGE
    return true;
#else
    return false;
#endif
  }

  void setStringSizeLimit(int32_t string_limit) {
    string_limit_ = string_limit;
  }

  void setContainerSizeLimit(int32_t container_limit) {
    container_limit_ = container_limit;
  }

  /**
   * The IOBuf itself is managed by the caller.
   * It must exist for the life of the CompactProtocol as well,
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
  void readDouble(double& dub);
  void readFloat(float& flt);
  template <typename T>
  void readArithmeticVector(T* outputPtr, size_t numElements);
  template <typename StrType>
  void readString(StrType& str);
  template <typename StrType>
  void readBinary(StrType& str);
  void readBinary(std::unique_ptr<IOBuf>& str);
  void readBinary(IOBuf& str);

  static constexpr std::size_t fixedSizeInContainer(TType type);
  void skipBytes(size_t bytes) { in_.skip(bytes); }
  void skip(TType type, int depth = 0) {
    apache::thrift::skip(*this, type, depth);
  }
  bool peekMap() { return false; }
  bool peekSet() { return false; }
  bool peekList() { return false; }

  const Cursor& getCursor() const { return in_; }

  size_t getCursorPosition() const { return in_.getCurrentPosition(); }

  struct StructReadState;

 protected:
  /**
   * Cursor to manipulate the buffer to read from.  Throws an exception if
   * there is not enough data tor ead the whole struct.
   */
  Cursor in_;

  void readStringSize(int32_t& size);

 private:
  template <typename StrType>
  void readStringBody(StrType& str, int32_t size);

  TType getType(int8_t type);

  void readStructBeginWithState(StructReadState& state);
  void readFieldBeginWithState(StructReadState& state);
  FOLLY_NOINLINE void readFieldBeginWithStateMediumSlow(
      StructReadState& state, int16_t prevFieldId);
  FOLLY_ALWAYS_INLINE void readFieldBeginWithStateImpl(
      StructReadState& state, int16_t prevFieldId, uint8_t firstByte);

  FOLLY_ALWAYS_INLINE bool matchTypeHeader(
      uint8_t byte, TType type, uint8_t diff);

  FOLLY_ALWAYS_INLINE bool advanceToNextField(
      int16_t currFieldId,
      int16_t nextFieldId,
      TType type,
      StructReadState& state);

  [[noreturn]] static void throwBadProtocolIdentifier();
  [[noreturn]] static void throwBadProtocolVersion();
  [[noreturn]] static void throwBadType(uint8_t type);

  int32_t string_limit_;
  int32_t container_limit_;
  ExternalBufferSharing sharing_;

  std::stack<int16_t, folly::small_vector<int16_t, 10>> lastField_;
  int16_t lastFieldId_{-1};

  struct {
    bool hasBoolValue;
    bool boolValue;
  } boolValue_;

  template <typename T>
  friend class ProtocolReaderWithRefill;
  friend class CompactProtocolReaderWithRefill;
};

struct CompactProtocolReader::StructReadState {
  int16_t fieldId;
  apache::thrift::protocol::TType fieldType;
  // bool boolValue;

  constexpr static bool kAcceptsContext = false;

  void readStructBegin(CompactProtocolReader* iprot) {
    iprot->readStructBeginWithState(*this);
  }

  void readStructEnd(CompactProtocolReader* /*iprot*/) {}

  void readFieldBegin(CompactProtocolReader* iprot) {
    iprot->readFieldBeginWithState(*this);
  }

  FOLLY_NOINLINE void readFieldBeginNoInline(CompactProtocolReader* iprot) {
    iprot->readFieldBeginWithState(*this);
  }

  void readFieldEnd(CompactProtocolReader* /*iprot*/) {}

  FOLLY_ALWAYS_INLINE bool advanceToNextField(
      CompactProtocolReader* iprot,
      int16_t currFieldId,
      int16_t nextFieldId,
      TType nextFieldType) {
    return iprot->advanceToNextField(
        currFieldId, nextFieldId, nextFieldType, *this);
  }

  void afterAdvanceFailure(CompactProtocolReader* /*iprot*/) {}

  void beforeSubobject(CompactProtocolReader* /* iprot */) {}
  void afterSubobject(CompactProtocolReader* /* iprot */) {}

  bool atStop() { return fieldType == apache::thrift::protocol::T_STOP; }

  /*
   * This is used in generated deserialization code only. When deserializing
   * fields in "non-advanceToNextField" case, we delegate the type check to
   * each protocol since some protocol may not encode type information.
   */
  FOLLY_ALWAYS_INLINE bool isCompatibleWithType(
      CompactProtocolReader* /*iprot*/, TType expectedFieldType) {
    return fieldType == expectedFieldType;
  }

  void skip(CompactProtocolReader* iprot) { iprot->skip(fieldType); }

  std::string& fieldName() {
    throw std::logic_error("CompactProtocol doesn't support field names");
  }

  template <typename StructTraits>
  void fillFieldTraitsFromName() {
    throw std::logic_error("CompactProtocol doesn't support field names");
  }
};

static_assert(!usesFieldNames<CompactProtocolReader>());
static_assert(!usesFieldNames<CompactProtocolWriter>());

namespace detail {

template <class Protocol>
struct ProtocolReaderStructReadState;

template <>
struct ProtocolReaderStructReadState<CompactProtocolReader>
    : CompactProtocolReader::StructReadState {};

} // namespace detail
} // namespace apache::thrift

#include <thrift/lib/cpp2/protocol/CompactProtocol-inl.h>

#endif // #ifndef CPP2_PROTOCOL_COMPACTPROTOCOL_H_
