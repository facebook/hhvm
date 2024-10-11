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
#include <thrift/lib/cpp2/transport/rocket/FdSocket.h>
#include <thrift/lib/cpp2/transport/rocket/RequestPayload.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/compression/Compression.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::rocket {
namespace detail {
template <class Metadata, class ProtocolWriter>
Payload makePayload(
    const Metadata& metadata, std::unique_ptr<folly::IOBuf> data);
} // namespace detail

// TODO(yuhanhao): hide the following methods behind PayloadSerializer.
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
size_t unpackCompact(T& output, const folly::io::Cursor& cursor) {
  CompactProtocolReader reader;
  reader.setInput(cursor);
  output.read(&reader);
  return reader.getCursorPosition();
}

template <typename T>
size_t unpackBinary(T& output, const folly::IOBuf* buffer) {
  if (!buffer) {
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

namespace detail {
template <class PayloadType, bool uncompressPayload>
inline PayloadType unpackPayload(rocket::Payload&& payload) {
  PayloadType t{{}, {}};
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
    t.payload = std::move(data);
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
} // namespace apache::thrift::rocket
