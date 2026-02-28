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

#include <thrift/lib/cpp2/transport/rocket/server/detail/RocketRequestProcessor.h>

#include <folly/ExceptionString.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/logging/xlog.h>

#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/cpp2/server/LoggingEvent.h>
#include <thrift/lib/cpp2/transport/rocket/FdSocket.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>
#include <thrift/lib/cpp2/transport/rocket/payload/PayloadSerializer.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerConnection.h>
#include <thrift/lib/cpp2/util/Checksum.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::rocket {

RocketRequestProcessor::RocketRequestProcessor(folly::AsyncTransport* transport)
    : transport_(transport) {}

folly::Try<RequestPayload> RocketRequestProcessor::parseRequestPayload(
    Payload&& payload, RocketServerConnection& connection) {
  return connection.getPayloadSerializer()->unpackAsCompressed<RequestPayload>(
      std::move(payload), connection.isDecodingMetadataUsingBinaryProtocol());
}

ChecksumAlgorithm RocketRequestProcessor::setupChecksumHandling(
    RequestRpcMetadata& metadata, RocketServerConnection& connection) {
  ChecksumAlgorithm checksumAlgorithm = ChecksumAlgorithm::NONE;
  if (connection.getPayloadSerializer()->supportsChecksum()) {
    if (auto checksum = metadata.checksum()) {
      checksumAlgorithm = *checksum->algorithm();
    }
  } else if (
      metadata.checksum().has_value() &&
      metadata.checksum()->algorithm().value() != ChecksumAlgorithm::NONE) {
    FB_LOG_ONCE(WARNING)
        << "Checksum is not supported, but checksum on the client was set";
    Checksum c;
    c.algorithm() = ChecksumAlgorithm::NONE;
    metadata.checksum() = c;
  }
  return checksumAlgorithm;
}

bool RocketRequestProcessor::extractFileDescriptors(
    const RequestRpcMetadata& metadata, folly::Try<folly::SocketFds>& tryFds) {
  if (metadata.fdMetadata().has_value()) {
    const auto& fdMetadata = *metadata.fdMetadata();
    tryFds = popReceivedFdsFromSocket(
        transport_,
        fdMetadata.numFds().value_or(0),
        fdMetadata.fdSeqNum().value_or(folly::SocketFds::kNoSeqNum));
    if (tryFds.hasException()) {
      return false;
    }
  }
  return true;
}

bool RocketRequestProcessor::validateRequestMetadata(
    const RequestRpcMetadata& metadata, RpcKind& expectedKind) {
  if (metadata.kind() == RpcKind::BIDIRECTIONAL_STREAM &&
      expectedKind == RpcKind::SINK) {
    expectedKind = RpcKind::BIDIRECTIONAL_STREAM;
  }

  return metadata.protocol() && metadata.name() && metadata.kind() &&
      metadata.kind() == expectedKind;
}

std::string RocketRequestProcessor::processPayloadCompression(
    std::unique_ptr<folly::IOBuf>& data,
    const RequestRpcMetadata& metadata,
    RocketServerConnection& connection) {
  if (!metadata.crc32c()) {
    return std::string();
  }

  auto compression = metadata.compression();
  if (!compression) {
    return std::string();
  }

  try {
    data = connection.getPayloadSerializer()->uncompressBuffer(
        std::move(data), *compression);
  } catch (...) {
    return folly::exceptionStr(folly::current_exception()).toStdString();
  }

  return std::string();
}

bool RocketRequestProcessor::validateChecksum(
    const std::unique_ptr<folly::IOBuf>& data,
    const RequestRpcMetadata& metadata) {
  const bool badChecksum =
      metadata.crc32c() && (*metadata.crc32c() != checksum::crc32c(*data));
  return !badChecksum;
}

void RocketRequestProcessor::logApplicationEvents(
    const RequestRpcMetadata& metadata) {
  THRIFT_APPLICATION_EVENT(server_read_headers).log([&] {
    auto size = metadata.otherMetadata() ? metadata.otherMetadata()->size() : 0;
    std::vector<folly::dynamic> keys;
    if (size) {
      keys.reserve(size);
      for (auto& [k, v] : *metadata.otherMetadata()) {
        keys.emplace_back(k);
      }
    }
    int fmd_sz = 0;
    if (auto fmd = metadata.frameworkMetadata()) {
      DCHECK(*fmd) << "initialized IOBuf is null";
      fmd_sz = static_cast<int>((**fmd).computeChainDataLength());
    }
    return folly::dynamic::object("size", static_cast<int>(size))(
        "keys", folly::dynamic::array(std::move(keys)))(
        "frameworkMetadataSize", fmd_sz);
  });
}

} // namespace apache::thrift::rocket
