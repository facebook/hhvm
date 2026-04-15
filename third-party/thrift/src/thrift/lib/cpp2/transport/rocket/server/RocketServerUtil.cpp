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

#include <thrift/lib/cpp2/transport/rocket/server/RocketServerUtil.h>

#include <folly/io/IOBufQueue.h>
#include <folly/io/async/AsyncSocket.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/transport/rocket/FdSocket.h>
#include <thrift/lib/cpp2/transport/rocket/server/IRocketServerConnection.h>

namespace apache::thrift::rocket {

Payload makePreCompressedPayload(
    IRocketServerConnection& connection,
    ResponseRpcMetadata& metadata,
    std::unique_ptr<folly::IOBuf> data,
    folly::SocketFds fds) {
  // Set FD metadata before serializing (matching packWithFds behavior).
  if (!fds.empty()) {
    metadata.fdMetadata() = makeFdMetadata(fds, connection.getRawSocket());
  }

  std::unique_ptr<folly::IOBuf> serializedMetadata;
  if (connection.isDecodingMetadataUsingBinaryProtocol()) {
    BinaryProtocolWriter writer;
    folly::IOBufQueue queue;
    writer.setOutput(&queue);
    metadata.write(&writer);
    serializedMetadata = queue.move();
  } else {
    serializedMetadata =
        connection.getPayloadSerializer()->packCompact(metadata);
  }

  auto payload = Payload::makeFromMetadataAndData(
      std::move(serializedMetadata), std::move(data));
  if (fds.size()) {
    payload.fds = std::move(fds.dcheckToSendOrEmpty());
  }
  return payload;
}

Payload makePreCompressedPayload(
    IRocketServerConnection& connection,
    StreamPayloadMetadata& metadata,
    std::unique_ptr<folly::IOBuf> data) {
  std::unique_ptr<folly::IOBuf> serializedMetadata;
  if (connection.isDecodingMetadataUsingBinaryProtocol()) {
    BinaryProtocolWriter writer;
    folly::IOBufQueue queue;
    writer.setOutput(&queue);
    metadata.write(&writer);
    serializedMetadata = queue.move();
  } else {
    serializedMetadata =
        connection.getPayloadSerializer()->packCompact(metadata);
  }

  return Payload::makeFromMetadataAndData(
      std::move(serializedMetadata), std::move(data));
}

} // namespace apache::thrift::rocket
