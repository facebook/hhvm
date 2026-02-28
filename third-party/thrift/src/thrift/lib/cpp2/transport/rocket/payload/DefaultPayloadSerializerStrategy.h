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

#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/transport/rocket/compression/CompressionManager.h>
#include <thrift/lib/cpp2/transport/rocket/payload/PayloadSerializerStrategy.h>

namespace apache::thrift::rocket {

/**
 * Port of PayloadUtils.h header free functions into a strategy class.
 */
class DefaultPayloadSerializerStrategy final
    : public PayloadSerializerStrategy<DefaultPayloadSerializerStrategy> {
 public:
  DefaultPayloadSerializerStrategy() : PayloadSerializerStrategy(*this) {}

  bool supportsChecksum() { return false; }

  template <class T>
  folly::Try<T> unpackAsCompressed(
      rocket::Payload&& payload, bool decodeMetadataUsingBinary) {
    return folly::makeTryWith([&]() {
      return unpackImpl<T>(std::move(payload), decodeMetadataUsingBinary);
    });
  }

  template <class T>
  folly::Try<T> unpack(
      rocket::Payload&& payload, bool decodeMetadataUsingBinary) {
    return folly::makeTryWith([&]() {
      T t = unpackImpl<T>(std::move(payload), decodeMetadataUsingBinary);
      if (auto compression = t.metadata.compression()) {
        const auto compressionAlgorithm = *compression;
        // Custom compression is supported in
        // CustomCompressionPayloadSerializerStrategy
        if (compressionAlgorithm != CompressionAlgorithm::NONE &&
            compressionAlgorithm != CompressionAlgorithm::CUSTOM) {
          t.payload =
              uncompressBuffer(std::move(t.payload), compressionAlgorithm);
        }
      }
      return t;
    });
  }

  template <typename Metadata>
  rocket::Payload packWithFds(
      Metadata* metadata,
      std::unique_ptr<folly::IOBuf>&& payload,
      folly::SocketFds fds,
      bool encodeMetadataUsingBinary,
      folly::AsyncTransport* transport,
      folly::IOBufFactory* ioBufFactory = nullptr);

  template <typename T>
  std::unique_ptr<folly::IOBuf> packCompact(const T& data) {
    CompactProtocolWriter writer;
    folly::IOBufQueue queue;
    writer.setOutput(&queue);
    data.write(&writer);
    return queue.move();
  }

  template <typename T>
  size_t unpackCompact(T& output, const folly::IOBuf* buffer) {
    if (FOLLY_UNLIKELY(!buffer)) {
      folly::throw_exception<std::runtime_error>("Underflow");
    }
    CompactProtocolReader reader;
    reader.setInput(buffer);
    output.read(&reader);
    return reader.getCursorPosition();
  }

  template <typename T>
  size_t unpackCompact(T& output, const folly::io::Cursor& cursor) {
    CompactProtocolReader reader;
    reader.setInput(cursor);
    output.read(&reader);
    return reader.getCursorPosition();
  }

  template <typename T>
  size_t unpackBinary(T& output, const folly::IOBuf* buffer) {
    if (FOLLY_UNLIKELY(!buffer)) {
      folly::throw_exception<std::runtime_error>("Underflow");
    }
    BinaryProtocolReader reader;
    reader.setInput(buffer);
    output.read(&reader);
    return reader.getCursorPosition();
  }

  template <typename T>
  size_t unpackBinary(T& output, const folly::io::Cursor& cursor) {
    BinaryProtocolReader reader;
    reader.setInput(cursor);
    output.read(&reader);
    return reader.getCursorPosition();
  }

  template <class PayloadType>
  rocket::Payload pack(
      PayloadType&& payload,
      bool encodeMetadataUsingBinary,
      folly::AsyncTransport* transport) {
    auto metadata = std::forward<PayloadType>(payload).metadata;
    return packWithFds(
        &metadata,
        std::forward<PayloadType>(payload).payload,
        std::forward<PayloadType>(payload).fds,
        encodeMetadataUsingBinary,
        transport);
  }

  std::unique_ptr<folly::IOBuf> compressBuffer(
      std::unique_ptr<folly::IOBuf>&& buffer,
      CompressionAlgorithm compressionAlgorithm) {
    return CompressionManager().compressBuffer(
        std::move(buffer), compressionAlgorithm);
  }

  std::unique_ptr<folly::IOBuf> uncompressBuffer(
      std::unique_ptr<folly::IOBuf>&& buffer,
      CompressionAlgorithm compressionAlgorithm) {
    return CompressionManager().uncompressBuffer(
        std::move(buffer), compressionAlgorithm);
  }

 private:
  static constexpr size_t kHeadroomBytes = 16;

  template <typename Metadata>
  rocket::Payload finalizePayload(
      std::unique_ptr<folly::IOBuf>&& payload,
      Metadata* metadata,
      folly::SocketFds fds,
      bool encodeMetadataUsingBinary,
      folly::IOBufFactory* ioBufFactory);

  bool canSerializeMetadataIntoDataBufferHeadroom(
      const std::unique_ptr<folly::IOBuf>& data, const size_t serSize) const;

  template <class Metadata, class ProtocolWriter>
  Payload makePayloadWithHeadroom(
      ProtocolWriter& writer,
      const Metadata& metadata,
      std::unique_ptr<folly::IOBuf> data,
      folly::IOBufFactory* ioBufFactory);

  template <class Metadata, class ProtocolWriter>
  Payload makePayloadWithoutHeadroom(
      size_t serSize,
      ProtocolWriter& writer,
      const Metadata& metadata,
      std::unique_ptr<folly::IOBuf> data,
      folly::IOBufFactory* ioBufFactory);

  template <class Metadata, class ProtocolWriter>
  Payload makePayload(
      const Metadata& metadata,
      std::unique_ptr<folly::IOBuf> data,
      folly::IOBufFactory* ioBufFactory);

  void verifyMetadataSize(size_t metadataSize, size_t expectedSize) {
    if (metadataSize != expectedSize) {
      folly::throw_exception<std::out_of_range>("metadata size mismatch");
    }
  }

  template <typename T>
  T unpackImpl(rocket::Payload&& payload, bool decodeMetadataUsingBinary) {
    T t{{}, {}};
    unpackPayloadMetadata(t, payload, decodeMetadataUsingBinary);
    t.payload = std::move(payload).data();
    return t;
  }

  template <typename T>
  void unpackPayloadMetadata(
      T& t, rocket::Payload& payload, bool decodeMetadataUsingBinary) {
    if (payload.hasNonemptyMetadata()) {
      size_t metadataSize;
      if (decodeMetadataUsingBinary) {
        metadataSize = unpackBinary(t.metadata, payload.buffer());
      } else {
        metadataSize = unpackCompact(t.metadata, payload.buffer());
      }

      if (metadataSize != payload.metadataSize()) {
        folly::throw_exception<std::out_of_range>("metadata size mismatch");
      }
    }
  }
};
} // namespace apache::thrift::rocket
