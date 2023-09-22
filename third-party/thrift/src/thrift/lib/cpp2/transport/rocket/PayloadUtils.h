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

// NB: If `fds` is non-empty, populates `metadata.{numFds,fdSeqNum}` and
// `fds.seqNum()`.  Then, moves the FDs into `Payload::fds`.
template <typename Payload, typename Metadata>
rocket::Payload packWithFds(
    Metadata* metadata,
    Payload&& payload,
    folly::SocketFds fds,
    folly::AsyncTransport* transport) {
  auto serializedPayload = packCompact(std::forward<Payload>(payload));
  if (auto compress = metadata->compression_ref()) {
    apache::thrift::rocket::detail::compressPayload(
        serializedPayload, *compress);
  }
  auto numFds = fds.size();
  if (numFds) {
    FdMetadata fdMetadata;

    // The kernel maximum is actually much lower (at least on Linux, and
    // MacOS doesn't seem to document it at all), but that will only fail in
    // in `AsyncFdSocket`.
    constexpr auto numFdsTypeMax = std::numeric_limits<
        op::get_native_type<FdMetadata, ident::numFds>>::max();
    if (UNLIKELY(numFds > numFdsTypeMax)) {
      LOG(DFATAL) << numFds << " would overflow FdMetadata::numFds";
      fdMetadata.numFds() = numFdsTypeMax;
      // This will cause "AsyncFdSocket::writeChainWithFds" to error out.
      fdMetadata.fdSeqNum() = folly::SocketFds::kNoSeqNum;
    } else {
      // When received, the request will know to retrieve this many FDs.
      fdMetadata.numFds() = numFds;
      // FD sequence numbers count the total number of FDs sent on this
      // socket, and are used to detect & fail on the dire class of bugs where
      // the wrong FDs are about to be associated with a message.
      //
      // We currently require message bytes and FDs to be both sent and
      // received in a coherent order, so sequence numbers here in `pack*` are
      // expected to exactly match the sequencing of socket sends, and also the
      // sequencing of `popNextReceivedFds` on the receiving side.
      //
      // NB: If `transport` is not backed by a `AsyncFdSocket*`, this will
      // store `fdSeqNum == -1`, which cannot happen otherwise, thanks to
      // AsyncFdSocket's 2^63 -> 0 wrap-around logic.  Furthermore, the
      // subsequent `writeChainWithFds` will discard `fds`.  As a result, the
      // recipient will see read errors on the FDs due to both `numFds` not
      // matching, and `fdSeqNum` not matching.
      fdMetadata.fdSeqNum() =
          injectFdSocketSeqNumIntoFdsToSend(transport, &fds);
    }

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
rocket::Payload pack(T&& payload, folly::AsyncTransport* transport) {
  auto metadata = std::forward<T>(payload).metadata;
  return packWithFds(
      &metadata,
      std::forward<T>(payload).payload,
      std::forward<T>(payload).fds,
      transport);
}
} // namespace rocket
} // namespace thrift
} // namespace apache
