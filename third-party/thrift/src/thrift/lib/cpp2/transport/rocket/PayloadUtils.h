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
#include <folly/Try.h>
#include <folly/io/async/AsyncTransport.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/transport/rocket/Compression.h>
#include <thrift/lib/cpp2/transport/rocket/FdSocket.h>
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
template <class Metadata, class ProtocolWriter>
Payload makePayload(
    const Metadata& metadata, std::unique_ptr<folly::IOBuf> data);
} // namespace detail

template <typename T, typename ProtocolType, typename BufferType>
size_t unpack(T& output, const BufferType& input) {
  if constexpr (std::is_same_v<BufferType, const folly::IOBuf*>) {
    if (!input) {
      throw std::runtime_error("Underflow");
    }
  }
  ProtocolType reader;
  reader.setInput(input);
  output.read(&reader);
  return reader.getCursorPosition();
}

template <typename T, typename BufferType>
size_t unpack(T& output, const BufferType& buffer, bool useBinary) {
  if (useBinary) {
    return unpack<T, BinaryProtocolReader, BufferType>(output, buffer);
  }
  return unpack<T, CompactProtocolReader, BufferType>(output, buffer);
}

namespace detail {
template <class PayloadType, bool uncompressPayload>
inline PayloadType unpackPayload(rocket::Payload&& payload, bool useBinary) {
  PayloadType t{{}, {}};
  if (payload.hasNonemptyMetadata()) {
    if (unpack(t.metadata, payload.buffer(), useBinary) !=
        payload.metadataSize()) {
      folly::throw_exception<std::out_of_range>("metadata size mismatch");
    }
  }
  if constexpr (uncompressPayload) {
    auto data = std::move(payload).data();
    if (auto compression = t.metadata.compression()) {
      data = uncompressBuffer(std::move(data), *compression);
    }
    t.payload = std::move(data);
  } else {
    t.payload = std::move(payload).data();
  }
  return t;
}
} // namespace detail

template <class T>
folly::Try<T> unpackAsCompressed(rocket::Payload&& payload, bool useBinary) {
  return folly::makeTryWith([&] {
    return detail::unpackPayload<T, false>(std::move(payload), useBinary);
  });
}

template <class T>
folly::Try<T> unpack(rocket::Payload&& payload, bool useBinary) {
  return folly::makeTryWith([&] {
    return detail::unpackPayload<T, true>(std::move(payload), useBinary);
  });
}

template <typename T>
std::unique_ptr<folly::IOBuf> packCompact(T&& data) {
  CompactProtocolWriter writer;
  folly::IOBufQueue queue;
  writer.setOutput(&queue);
  data.write(&writer);
  return queue.move();
}

// NB: If `fds` is non-empty, populates `metadata.{numFds,fdSeqNum}` and
// `fds.seqNum()`.  Then, moves the FDs into `Payload::fds`.
template <typename Metadata>
rocket::Payload packWithFds(
    Metadata* metadata,
    std::unique_ptr<folly::IOBuf>&& payload,
    folly::SocketFds fds,
    folly::AsyncTransport* transport);

template <class PayloadType>
rocket::Payload pack(PayloadType&& payload, folly::AsyncTransport* transport) {
  auto metadata = std::forward<PayloadType>(payload).metadata;
  return packWithFds(
      &metadata,
      std::forward<PayloadType>(payload).payload,
      std::forward<PayloadType>(payload).fds,
      transport);
}
} // namespace rocket
} // namespace thrift
} // namespace apache
