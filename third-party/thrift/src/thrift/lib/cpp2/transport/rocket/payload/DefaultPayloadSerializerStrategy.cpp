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

#include <thrift/lib/cpp2/transport/rocket/payload/DefaultPayloadSerializerStrategy.h>

#include <folly/io/async/AsyncTransport.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/transport/rocket/Compression.h>
#include <thrift/lib/cpp2/transport/rocket/FdSocket.h>

#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::rocket {

namespace {

template <typename Metadata>
void applyCompressionIfNeeded(
    std::unique_ptr<folly::IOBuf>& payload, Metadata* metadata) {
  if (auto compress = metadata->compression_ref()) {
    apache::thrift::rocket::detail::compressPayload(payload, *compress);
  }
}

bool validateFileDescriptor(size_t numFds, FdMetadata& fdMetadata) {
  // The kernel maximum is actually much lower (at least on Linux, and
  // MacOS doesn't seem to document it at all), but that will only fail in
  // in `AsyncFdSocket`.
  constexpr auto numFdsTypeMax = std::numeric_limits<
      op::get_native_type<FdMetadata, ident::numFds>>::max();

  if (LIKELY(numFdsTypeMax <= numFds)) {
    return true;
  } else {
    LOG(DFATAL) << numFds << " would overflow FdMetadata::numFds";
    fdMetadata.numFds() = numFdsTypeMax;
    // This will cause "AsyncFdSocket::writeChainWithFds" to error out.
    fdMetadata.fdSeqNum() = folly::SocketFds::kNoSeqNum;
    return false;
  }
}

template <typename Metadata>
void handleFds(
    folly::SocketFds& fds,
    Metadata* metadata,
    folly::AsyncTransport* transport) {
  auto numFds = fds.size();
  if (numFds) {
    FdMetadata fdMetadata;
    if (LIKELY(validateFileDescriptor(numFds, fdMetadata))) {
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
}

} // namespace

template <typename Metadata>
rocket::Payload DefaultPayloadSerializerStrategy::finalizePayload(
    std::unique_ptr<folly::IOBuf>&& payload,
    Metadata* metadata,
    folly::SocketFds fds) {
  auto ret = makePayload<Metadata, CompactProtocolWriter>(
      *metadata, std::move(payload));
  if (fds.size()) {
    ret.fds = std::move(fds.dcheckToSendOrEmpty());
  }
  return ret;
}

template <typename Metadata>
rocket::Payload DefaultPayloadSerializerStrategy::packWithFds(
    Metadata* metadata,
    std::unique_ptr<folly::IOBuf>&& payload,
    folly::SocketFds fds,
    folly::AsyncTransport* transport) {
  applyCompressionIfNeeded(payload, metadata);
  handleFds(fds, metadata, transport);
  return finalizePayload(std::move(payload), metadata, std::move(fds));
}

template rocket::Payload
DefaultPayloadSerializerStrategy::packWithFds<RequestRpcMetadata>(
    RequestRpcMetadata*,
    std::unique_ptr<folly::IOBuf>&&,
    folly::SocketFds,
    folly::AsyncTransport*);

template rocket::Payload
DefaultPayloadSerializerStrategy::packWithFds<ResponseRpcMetadata>(
    ResponseRpcMetadata*,
    std::unique_ptr<folly::IOBuf>&&,
    folly::SocketFds,
    folly::AsyncTransport*);

template rocket::Payload
DefaultPayloadSerializerStrategy::packWithFds<StreamPayloadMetadata>(
    StreamPayloadMetadata*,
    std::unique_ptr<folly::IOBuf>&&,
    folly::SocketFds,
    folly::AsyncTransport*);

bool DefaultPayloadSerializerStrategy::
    canSerializeMetadataIntoDataBufferHeadroom(
        const std::unique_ptr<folly::IOBuf>& data, const size_t serSize) const {
  return data && !data->isChained() &&
      data->headroom() >= serSize + kHeadroomBytes && !data->isSharedOne();
}

template <class Metadata, class ProtocolWriter>
Payload DefaultPayloadSerializerStrategy::makePayloadWithHeadroom(
    ProtocolWriter& writer,
    const Metadata& metadata,
    std::unique_ptr<folly::IOBuf> data) {
  folly::IOBufQueue queue;
  // Store previous state of the buffer pointers and rewind it.
  auto startBuffer = data->buffer();
  auto start = data->data();
  auto origLen = data->length();
  data->trimEnd(origLen);
  data->retreat(start - startBuffer);

  queue.append(std::move(data), false);
  writer.setOutput(&queue);
  auto metadataLen = metadata.write(&writer);

  // Move the new data to come right before the old data and restore the
  // old tail pointer.
  data = queue.move();
  data->advance(start - data->tail());
  data->append(origLen);

  return Payload::makeCombined(std::move(data), metadataLen);
}

template <class Metadata, class ProtocolWriter>
Payload DefaultPayloadSerializerStrategy::makePayloadWithoutHeadroom(
    size_t serSize,
    ProtocolWriter& writer,
    const Metadata& metadata,
    std::unique_ptr<folly::IOBuf> data) {
  folly::IOBufQueue queue;
  constexpr size_t kMinAllocBytes = 1024;
  auto buf =
      folly::IOBuf::create(std::max(kHeadroomBytes + serSize, kMinAllocBytes));
  buf->advance(kHeadroomBytes);
  queue.append(std::move(buf));
  writer.setOutput(&queue);
  auto metadataLen = metadata.write(&writer);
  queue.append(std::move(data));
  return Payload::makeCombined(queue.move(), metadataLen);
}

template <class Metadata, class ProtocolWriter>
Payload DefaultPayloadSerializerStrategy::makePayload(
    const Metadata& metadata, std::unique_ptr<folly::IOBuf> data) {
  ProtocolWriter writer;
  // Default is to leave some headroom for rsocket headers
  size_t serSize = metadata.serializedSizeZC(&writer);

  // If possible, serialize metadata into the headeroom of data.
  if (canSerializeMetadataIntoDataBufferHeadroom(data, serSize)) {
    return makePayloadWithHeadroom(writer, metadata, std::move(data));
  } else {
    return makePayloadWithoutHeadroom(
        serSize, writer, metadata, std::move(data));
  }
}

template Payload DefaultPayloadSerializerStrategy::
    makePayload<RequestRpcMetadata, BinaryProtocolWriter>(
        const RequestRpcMetadata&, std::unique_ptr<folly::IOBuf> data);
template Payload DefaultPayloadSerializerStrategy::
    makePayload<ResponseRpcMetadata, BinaryProtocolWriter>(
        const ResponseRpcMetadata&, std::unique_ptr<folly::IOBuf> data);
template Payload DefaultPayloadSerializerStrategy::
    makePayload<StreamPayloadMetadata, BinaryProtocolWriter>(
        const StreamPayloadMetadata&, std::unique_ptr<folly::IOBuf> data);

template Payload DefaultPayloadSerializerStrategy::
    makePayload<RequestRpcMetadata, CompactProtocolWriter>(
        const RequestRpcMetadata&, std::unique_ptr<folly::IOBuf> data);
template Payload DefaultPayloadSerializerStrategy::
    makePayload<ResponseRpcMetadata, CompactProtocolWriter>(
        const ResponseRpcMetadata&, std::unique_ptr<folly::IOBuf> data);
template Payload DefaultPayloadSerializerStrategy::
    makePayload<StreamPayloadMetadata, CompactProtocolWriter>(
        const StreamPayloadMetadata&, std::unique_ptr<folly::IOBuf> data);

} // namespace apache::thrift::rocket
