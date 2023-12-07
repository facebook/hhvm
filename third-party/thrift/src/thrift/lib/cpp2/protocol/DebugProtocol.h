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

#ifndef CPP2_PROTOCOL_DEBUGPROTOCOL_H_
#define CPP2_PROTOCOL_DEBUGPROTOCOL_H_

#include <fmt/core.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <folly/portability/GFlags.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/protocol/Cpp2Ops.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>
#include <thrift/lib/cpp2/type/NativeType.h>

FOLLY_GFLAGS_DECLARE_bool(thrift_cpp2_debug_skip_list_indices);
FOLLY_GFLAGS_DECLARE_int64(thrift_cpp2_debug_string_limit);

namespace apache {
namespace thrift {

namespace op {
namespace detail {
template <class T>
struct Encode;
}
} // namespace op

class DebugProtocolWriter {
 public:
  struct Options {
    Options() {}
    bool skipListIndices = FLAGS_thrift_cpp2_debug_skip_list_indices;
    size_t stringLengthLimit = FLAGS_thrift_cpp2_debug_string_limit;
    bool skipFieldId = false;
    bool skipFieldType = false;

    static Options simple() {
      Options opt;
      opt.skipListIndices = true;
      opt.skipFieldId = true;
      opt.skipFieldType = true;
      return opt;
    }
  };

  using ProtocolReader = void;

  explicit DebugProtocolWriter(
      ExternalBufferSharing sharing = COPY_EXTERNAL_BUFFER /* ignored */,
      Options options = Options{});

  static constexpr ProtocolType protocolType() {
    return ProtocolType::T_DEBUG_PROTOCOL;
  }

  static constexpr bool kSortKeys() { return true; }

  static constexpr bool kHasIndexSupport() { return false; }

  void setOutput(
      folly::IOBufQueue* queue,
      size_t maxGrowth = std::numeric_limits<size_t>::max());

  void setOutput(folly::io::QueueAppender&& output);

  uint32_t writeMessageBegin(
      const std::string& name, MessageType messageType, int32_t seqid);
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
  uint32_t writeDouble(double dub);
  uint32_t writeFloat(float flt);
  uint32_t writeString(folly::StringPiece str);
  uint32_t writeBinary(folly::StringPiece str);
  uint32_t writeBinary(folly::ByteRange v);
  uint32_t writeBinary(const std::unique_ptr<folly::IOBuf>& str);
  uint32_t writeBinary(const folly::IOBuf& str);

  /**
   * Functions that return the serialized size
   */
  uint32_t serializedMessageSize(const std::string& name);
  uint32_t serializedFieldSize(
      const char* name, TType fieldName, int16_t fieldId);
  uint32_t serializedStructSize(const char* name);
  uint32_t serializedSizeMapBegin(TType keyType, TType valType, uint32_t size);
  uint32_t serializedSizeMapEnd();
  uint32_t serializedSizeListBegin(TType elemType, uint32_t size);
  uint32_t serializedSizeListEnd();
  uint32_t serializedSizeSetBegin(TType elemType, uint32_t size);
  uint32_t serializedSizeSetEnd();
  uint32_t serializedSizeStop();
  uint32_t serializedSizeBool(bool value);
  uint32_t serializedSizeByte(int8_t byte);
  uint32_t serializedSizeI16(int16_t i16);
  uint32_t serializedSizeI32(int32_t i32);
  uint32_t serializedSizeI64(int64_t i64);
  uint32_t serializedSizeDouble(double dub);
  uint32_t serializedSizeFloat(float flt);
  uint32_t serializedSizeString(folly::StringPiece str);
  uint32_t serializedSizeBinary(folly::StringPiece str);
  uint32_t serializedSizeBinary(folly::ByteRange v);
  uint32_t serializedSizeBinary(const std::unique_ptr<folly::IOBuf>& v);
  uint32_t serializedSizeBinary(const folly::IOBuf& v);
  uint32_t serializedSizeZCBinary(folly::StringPiece str);
  uint32_t serializedSizeZCBinary(folly::ByteRange v);
  uint32_t serializedSizeZCBinary(const std::unique_ptr<folly::IOBuf>&);
  uint32_t serializedSizeZCBinary(const folly::IOBuf& /*v*/);

 private:
  void indentUp();
  void indentDown();
  void startItem();
  void endItem();

  void writeByteRange(folly::ByteRange v);

  void writeRaw(folly::StringPiece sp) {
    out_.push(reinterpret_cast<const uint8_t*>(sp.data()), sp.size());
  }

  template <class... Args>
  void writePlain(fmt::string_view fmt, const Args&... args) {
    writeRaw(fmt::vformat(fmt, {fmt::make_format_args(args...)}));
  }

  void writeIndent() { writeRaw(indent_); }

  template <class... Args>
  void writeIndented(Args&&... args) {
    writeIndent();
    writePlain(std::forward<Args>(args)...);
  }

  template <class... Args>
  void writeItem(Args&&... args) {
    startItem();
    writePlain(std::forward<Args>(args)...);
    endItem();
  }

  enum ItemType { STRUCT, SET, MAP_KEY, MAP_VALUE, LIST };

  struct WriteState {
    /* implicit */ WriteState(ItemType t) : type(t), index(0) {}

    ItemType type;
    int index;
  };

  void pushState(ItemType t);
  void popState();

  folly::io::QueueAppender out_;
  std::string indent_;
  std::vector<WriteState> writeState_;
  Options options_;
};

template <class T>
std::string debugString(
    const T& obj, DebugProtocolWriter::Options options = {}) {
  folly::IOBufQueue queue;
  DebugProtocolWriter proto(
      COPY_EXTERNAL_BUFFER, // Ignored by constructor.
      options);
  proto.setOutput(&queue);
  Cpp2Ops<T>::write(&proto, &obj);
  auto buf = queue.move();
  auto br = buf->coalesce();
  return std::string(reinterpret_cast<const char*>(br.data()), br.size());
}

// TODO: Replace `debugString()` with this function
template <class T, class..., template <class> class Encode = op::detail::Encode>
std::string debugStringViaEncode(
    const T& obj, DebugProtocolWriter::Options options = {}) {
  folly::IOBufQueue queue;
  DebugProtocolWriter proto(
      COPY_EXTERNAL_BUFFER, // Ignored by constructor.
      options);
  proto.setOutput(&queue);
  Encode<type::infer_tag<T>>{}(proto, obj);
  auto buf = queue.move();
  auto br = buf->coalesce();
  return std::string(reinterpret_cast<const char*>(br.data()), br.size());
}

} // namespace thrift
} // namespace apache

#endif /* CPP2_PROTOCOL_DEBUGPROTOCOL_H_ */
