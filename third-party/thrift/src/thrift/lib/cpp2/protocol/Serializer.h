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

#ifndef CPP2_SERIALIZER_H
#define CPP2_SERIALIZER_H

#include <folly/GLog.h>
#include <folly/io/IOBuf.h>
#include <folly/lang/Pretty.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/Cpp2Ops.h>
#include <thrift/lib/cpp2/protocol/JSONProtocol.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>
#include <thrift/lib/cpp2/protocol/SimpleJSONProtocol.h>

namespace apache {
namespace thrift {

template <typename Reader, typename Writer>
struct Serializer {
 private:
  template <typename T>
  using is_thrift_class = folly::bool_constant<is_thrift_class_v<T>>;

  template <typename T>
  static void warn_unless(folly::tag_t<T>, const char* which, std::false_type) {
    FB_LOG_ONCE(WARNING)
        << "Thrift serialization is only defined for structs and unions, not"
        << " containers thereof. Attempting to " << which
        << " a value of type `" << folly::pretty_name<T>() << "`.";
  }
  template <typename T>
  static void warn_unless(folly::tag_t<T>, const char*, std::true_type) {}

 public:
  using ProtocolReader = Reader;
  using ProtocolWriter = Writer;

  template <class T>
  static folly::io::Cursor deserialize(
      const folly::io::Cursor& cursor,
      T& obj,
      ExternalBufferSharing sharing = COPY_EXTERNAL_BUFFER) {
    return deserializeImpl(cursor, obj, sharing);
  }

  template <class T>
  static size_t deserialize(
      const folly::IOBuf* buf,
      T& obj,
      ExternalBufferSharing sharing = COPY_EXTERNAL_BUFFER) {
    // Create Reader and assign/read Cursor object in the same method to avoid
    // store-load forwarding block penalty vs calling deserializer from Cursor
    // object directly.
    return deserializeImpl(folly::io::Cursor{buf}, obj, sharing)
        .getCurrentPosition();
  }

  template <class T>
  static size_t deserialize(
      folly::ByteRange range,
      T& obj,
      ExternalBufferSharing sharing = COPY_EXTERNAL_BUFFER) {
    folly::IOBuf buf(folly::IOBuf::WRAP_BUFFER, range);
    return deserialize(&buf, obj, sharing);
  }

  template <class T>
  static size_t deserialize(
      folly::StringPiece range,
      T& obj,
      ExternalBufferSharing sharing = COPY_EXTERNAL_BUFFER) {
    return deserialize(folly::ByteRange(range), obj, sharing);
  }

  /**
   * Deserialize an object from a folly::io::Cursor.
   *
   * When given a non-const Cursor reference we will update the input cursor
   * to point at the end of the deserialized data.
   */
  template <class T>
  static T deserialize(folly::io::Cursor& cursor) {
    return returning<T>([&](T& obj) { cursor = deserialize(cursor, obj); });
  }

  /**
   * Deserialize an object from a folly::io::Cursor.
   *
   * When given a reference to a const Cursor we cannot return the size of data
   * that was deserialized.  Pass in a non-const Cursor to use the API above if
   * you do need to determine the length of the deserialized data.
   */
  template <class T>
  static T deserialize(const folly::io::Cursor& cursor) {
    return returning<T>([&](T& obj) { deserialize(cursor, obj); });
  }

  template <class T>
  static T deserialize(const folly::IOBuf* buf, size_t* size = nullptr) {
    return returning<T>([&](T& obj) { set(size, deserialize(buf, obj)); });
  }

  template <class T>
  static T deserialize(
      folly::ByteRange range,
      size_t* size = nullptr,
      ExternalBufferSharing sharing = COPY_EXTERNAL_BUFFER) {
    return returning<T>(
        [&](T& obj) { set(size, deserialize(range, obj, sharing)); });
  }

  template <class T>
  static T deserialize(
      folly::StringPiece range,
      size_t* size = nullptr,
      ExternalBufferSharing sharing = COPY_EXTERNAL_BUFFER) {
    return deserialize<T>(folly::ByteRange(range), size, sharing);
  }

  template <class T>
  static void serialize(
      const T& obj,
      folly::IOBufQueue* out,
      ExternalBufferSharing sharing = COPY_EXTERNAL_BUFFER) {
    warn_unless(folly::tag<T>, "serialize", is_thrift_class<T>{});
    Writer writer(sharing);
    writer.setOutput(out);

    // This can be obj.write(&writer);
    // if you don't need to support thrift1-compatibility types
    apache::thrift::Cpp2Ops<T>::write(&writer, &obj);
  }

  template <class T>
  static void serialize(
      const T& obj,
      folly::io::QueueAppender&& out,
      ExternalBufferSharing sharing = COPY_EXTERNAL_BUFFER) {
    Writer writer(sharing);
    writer.setOutput(std::move(out));

    // This can be obj.write(&writer);
    // if you don't need to support thrift1-compatibility types
    apache::thrift::Cpp2Ops<T>::write(&writer, &obj);
  }

  template <class T>
  static void serialize(const T& obj, std::string* out) {
    folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
    // Okay to share any external buffers, as we'll copy them to *out
    // immediately afterwards.
    serialize(obj, &queue, SHARE_EXTERNAL_BUFFER);
    queue.appendToString(*out);
  }

  template <class T>
  static void serialize(const T& obj, folly::fbstring* out) {
    folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
    // Okay to share any external buffers, as we'll copy them to *out
    // immediately afterwards.
    if (!out->empty()) {
      queue.append(out->data(), out->size());
    }
    serialize(obj, &queue, SHARE_EXTERNAL_BUFFER);
    *out = queue.move()->moveToFbString();
  }

  template <class R, class T, typename... Args>
  static R serialize(const T& obj, Args&&... args) {
    R _return;
    serialize(obj, &_return, std::forward<Args>(args)...);
    return _return;
  }

 private:
  template <typename T>
  FOLLY_ALWAYS_INLINE static folly::io::Cursor deserializeImpl(
      const folly::io::Cursor& cursor, T& obj, ExternalBufferSharing sharing) {
    warn_unless(folly::tag<T>, "deserialize", is_thrift_class<T>{});
    Reader reader(sharing);
    reader.setInput(cursor);

    // This can be obj.read(&reader);
    // if you don't need to support thrift1-compatibility types
    apache::thrift::Cpp2Ops<T>::read(&reader, &obj);

    return reader.getCursor();
  }

  template <typename T>
  static void set(T* t, T&& v) {
    if (t != nullptr) {
      *t = std::forward<T>(v);
    }
  }

  template <typename R, typename F>
  static R returning(F&& f) {
    R _return;
    f(_return);
    return _return;
  }
};

typedef Serializer<CompactProtocolReader, CompactProtocolWriter>
    CompactSerializer;
typedef Serializer<BinaryProtocolReader, BinaryProtocolWriter> BinarySerializer;
typedef Serializer<JSONProtocolReader, JSONProtocolWriter> JSONSerializer;
typedef Serializer<SimpleJSONProtocolReader, SimpleJSONProtocolWriter>
    SimpleJSONSerializer;

// Serialization code specific to handling errors
template <typename ProtIn, typename ProtOut, bool includeEnvelope = true>
std::unique_ptr<folly::IOBuf> serializeErrorProtocol(
    const TApplicationException& obj, folly::IOBuf* req) {
  ProtOut prot;
  folly::IOBufQueue queue;
  size_t objSize = obj.serializedSizeZC(&prot);
  if /*constexpr*/ (includeEnvelope) {
    ProtIn iprot;
    std::string fname;
    apache::thrift::MessageType mtype;
    int32_t protoSeqId = 0;
    iprot.setInput(req);
    iprot.readMessageBegin(fname, mtype, protoSeqId);
    prot.setOutput(&queue, objSize + prot.serializedMessageSize(fname));
    prot.writeMessageBegin(fname, MessageType::T_EXCEPTION, protoSeqId);
  } else {
    prot.setOutput(&queue, objSize);
  }
  obj.write(&prot);
  prot.writeMessageEnd();
  return queue.move();
}

template <typename ProtOut, bool includeEnvelope = true>
std::unique_ptr<folly::IOBuf> serializeErrorProtocol(
    const TApplicationException& obj,
    const std::string& fname,
    int32_t protoSeqId) {
  ProtOut prot;
  folly::IOBufQueue queue;
  std::size_t objSize = obj.serializedSizeZC(&prot);
  if /*constexpr*/ (includeEnvelope) {
    prot.setOutput(&queue, objSize + prot.serializedMessageSize(fname));
    prot.writeMessageBegin(fname, MessageType::T_EXCEPTION, protoSeqId);
  } else {
    prot.setOutput(&queue, objSize);
  }
  obj.write(&prot);
  prot.writeMessageEnd();
  return queue.move();
}

template <bool includeEnvelope = true>
std::unique_ptr<folly::IOBuf> serializeError(
    int protId, const TApplicationException& obj, folly::IOBuf* buf) {
  switch (protId) {
    case apache::thrift::protocol::T_BINARY_PROTOCOL: {
      return serializeErrorProtocol<
          BinaryProtocolReader,
          BinaryProtocolWriter,
          includeEnvelope>(obj, buf);
    }
    case apache::thrift::protocol::T_COMPACT_PROTOCOL: {
      return serializeErrorProtocol<
          CompactProtocolReader,
          CompactProtocolWriter,
          includeEnvelope>(obj, buf);
    }
    default: {
      LOG(ERROR) << "Invalid protocol from client";
    }
  }

  return nullptr;
}

template <bool includeEnvelope = true>
std::unique_ptr<folly::IOBuf> serializeError(
    int protId,
    const TApplicationException& obj,
    const std::string& fname,
    int32_t protoSeqId) {
  switch (protId) {
    case apache::thrift::protocol::T_BINARY_PROTOCOL: {
      return serializeErrorProtocol<BinaryProtocolWriter, includeEnvelope>(
          obj, fname, protoSeqId);
    }
    case apache::thrift::protocol::T_COMPACT_PROTOCOL: {
      return serializeErrorProtocol<CompactProtocolWriter, includeEnvelope>(
          obj, fname, protoSeqId);
    }
    default: {
      LOG(ERROR) << "Invalid protocol from client";
    }
  }

  return nullptr;
}

// For places where we can't currently invoke the templated version directly,
inline std::unique_ptr<folly::IOBuf> serializeErrorWithEnvelope(
    int protId,
    const TApplicationException& obj,
    const std::string& fname,
    int32_t protoSeqId) {
  return serializeError<true>(protId, obj, fname, protoSeqId);
}

inline std::unique_ptr<folly::IOBuf> serializeErrorWithoutEnvelope(
    int protId,
    const TApplicationException& obj,
    const std::string& fname,
    int32_t protoSeqId) {
  return serializeError<false>(protId, obj, fname, protoSeqId);
}

// serialize TApplicationException without a protocol message envelop
std::unique_ptr<folly::IOBuf> serializeErrorStruct(
    protocol::PROTOCOL_TYPES protId, const TApplicationException& obj);

} // namespace thrift
} // namespace apache
#endif
