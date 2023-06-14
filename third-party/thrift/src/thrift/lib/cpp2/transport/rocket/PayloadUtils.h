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
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/transport/rocket/Compression.h>
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

// NB: Populates `metadata.numFds` if `fds` is nonempty.
template <typename Payload, typename Metadata>
rocket::Payload packWithFds(
    Metadata* metadata, Payload&& payload, folly::SocketFds fds) {
  auto serializedPayload = packCompact(std::forward<Payload>(payload));
  if (auto compress = metadata->compression_ref()) {
    apache::thrift::rocket::detail::compressPayload(
        serializedPayload, *compress);
  }
  auto numFds = fds.size();
  if (numFds) {
    FdMetadata fdMetadata;

    // When received, the request will know to retrieve this many FDs.
    //
    // NB: The receiver could more confidently assert that the right FDs are
    // associated with the right request if we could:
    //  - Additionally store the "socket sequence number" of these FDs into
    //    the metadata here.
    //  - Have `AsyncFdSocket::writeIOBufsWithFds` check, via a token
    //    object, that the sequence number chosen at parse-time matches the
    //    actual write order).
    // Unfortunately, implementing this check would require adding
    // considerable plumbing to Rocket, so we skip it in favor of asserting
    // we got the right # of FDs, and documenting the correct FD+data
    // ordering invariant in the client & server code that interacts with
    // the socket.
    fdMetadata.numFds() = numFds;

    DCHECK(!metadata->fdMetadata().has_value());
    metadata->fdMetadata() = fdMetadata;
  }
  auto ret = apache::thrift::rocket::detail::makePayload(
      *metadata, std::move(serializedPayload));
  if (numFds) {
    ret.fds = std::move(fds.dcheckToSendOrEmpty());
  }
  return ret;
}

template <class T>
rocket::Payload pack(T&& payload) {
  auto metadata = std::forward<T>(payload).metadata;
  return packWithFds(
      &metadata,
      std::forward<T>(payload).payload,
      std::forward<T>(payload).fds);
}
} // namespace rocket
} // namespace thrift
} // namespace apache
