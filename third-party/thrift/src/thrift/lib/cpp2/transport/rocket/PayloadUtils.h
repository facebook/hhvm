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

#include <fmt/core.h>
#include <folly/Expected.h>
#include <folly/Try.h>
#include <folly/compression/Compression.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache {
namespace thrift {

struct RequestPayload {
  RequestPayload(std::unique_ptr<folly::IOBuf> p, RequestRpcMetadata md)
      : payload(std::move(p)), metadata(std::move(md)) {}

  std::unique_ptr<folly::IOBuf> payload;
  RequestRpcMetadata metadata;
};

namespace rocket {
namespace detail {
template <class Metadata>
Payload makePayload(
    const Metadata& metadata, std::unique_ptr<folly::IOBuf> data);

extern template Payload makePayload<>(
    const RequestRpcMetadata&, std::unique_ptr<folly::IOBuf> data);
extern template Payload makePayload<>(
    const ResponseRpcMetadata&, std::unique_ptr<folly::IOBuf> data);
extern template Payload makePayload<>(
    const StreamPayloadMetadata&, std::unique_ptr<folly::IOBuf> data);

template <typename Metadata>
void setCompressionCodec(
    CompressionConfig compressionConfig,
    Metadata& metadata,
    size_t payloadSize);

extern template void setCompressionCodec<>(
    CompressionConfig compressionConfig,
    RequestRpcMetadata& metadata,
    size_t payloadSize);
extern template void setCompressionCodec<>(
    CompressionConfig compressionConfig,
    ResponseRpcMetadata& metadata,
    size_t payloadSize);
extern template void setCompressionCodec<>(
    CompressionConfig compressionConfig,
    StreamPayloadMetadata& metadata,
    size_t payloadSize);

/**
 * Helper method to compress the payload before sending to the remote endpoint.
 */
void compressPayload(
    std::unique_ptr<folly::IOBuf>& data, CompressionAlgorithm compression);

/**
 * Helper method to uncompress the payload from remote endpoint.
 */
folly::Expected<std::unique_ptr<folly::IOBuf>, std::string> uncompressPayload(
    CompressionAlgorithm compression, std::unique_ptr<folly::IOBuf> data);
} // namespace detail

template <typename T>
size_t unpackCompact(T& output, const folly::IOBuf* buffer) {
  if (!buffer) {
    folly::throw_exception<std::runtime_error>("Underflow");
  }
  CompactProtocolReader reader;
  reader.setInput(buffer);
  output.read(&reader);
  return reader.getCursorPosition();
}

template <typename T>
size_t unpackCompact(T& output, std::unique_ptr<folly::IOBuf> buffer) {
  return unpackCompact(output, buffer.get());
}

template <>
inline size_t unpackCompact(
    std::unique_ptr<folly::IOBuf>& output,
    std::unique_ptr<folly::IOBuf> buffer) {
  output = std::move(buffer);
  return 0;
}

std::unique_ptr<folly::IOBuf> uncompressBuffer(
    std::unique_ptr<folly::IOBuf>&& buffer, CompressionAlgorithm compression);

namespace detail {
template <class T, bool uncompressPayload>
inline T unpackPayload(rocket::Payload&& payload) {
  T t{{}, {}};
  if (payload.hasNonemptyMetadata()) {
    if (unpackCompact(t.metadata, payload.buffer()) != payload.metadataSize()) {
      folly::throw_exception<std::out_of_range>("metadata size mismatch");
    }
  }
  if constexpr (uncompressPayload) {
    auto data = std::move(payload).data();
    if (auto compression = t.metadata.compression()) {
      data = uncompressBuffer(std::move(data), *compression);
    }
    unpackCompact(t.payload, std::move(data));
  } else {
    t.payload = std::move(payload).data();
  }
  return t;
}
} // namespace detail

template <class T>
folly::Try<T> unpackAsCompressed(rocket::Payload&& payload) {
  return folly::makeTryWith(
      [&] { return detail::unpackPayload<T, false>(std::move(payload)); });
}

template <class T>
folly::Try<T> unpack(rocket::Payload&& payload) {
  return folly::makeTryWith(
      [&] { return detail::unpackPayload<T, true>(std::move(payload)); });
}

template <typename T>
std::unique_ptr<folly::IOBuf> packCompact(T&& data) {
  CompactProtocolWriter writer;
  folly::IOBufQueue queue;
  writer.setOutput(&queue);
  data.write(&writer);
  return queue.move();
}

template <>
inline std::unique_ptr<folly::IOBuf> packCompact(
    std::unique_ptr<folly::IOBuf>&& data) {
  return std::move(data);
}

template <typename Payload, typename Metadata>
rocket::Payload pack(const Metadata& metadata, Payload&& payload) {
  auto serializedPayload = packCompact(std::forward<Payload>(payload));
  if (auto compress = metadata.compression_ref()) {
    apache::thrift::rocket::detail::compressPayload(
        serializedPayload, *compress);
  }
  return apache::thrift::rocket::detail::makePayload(
      metadata, std::move(serializedPayload));
}

template <class T>
rocket::Payload pack(T&& payload) {
  return pack(
      std::forward<T>(payload).metadata, std::forward<T>(payload).payload);
}
} // namespace rocket
} // namespace thrift
} // namespace apache
