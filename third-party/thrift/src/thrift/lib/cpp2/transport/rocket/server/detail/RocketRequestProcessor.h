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

#include <memory>

#include <folly/Try.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/AsyncTransport.h>

#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/util/Checksum.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace folly {
class SocketFds;
} // namespace folly

namespace apache::thrift::rocket {

class Payload;
class RocketServerConnection;

/**
 * Result of payload parsing containing parsed metadata and data.
 */
struct RequestPayload {
  RequestRpcMetadata metadata;
  std::unique_ptr<folly::IOBuf> payload;
};

/**
 * RocketRequestProcessor handles all request payload processing logic for
 * Rocket server connections. It parses payloads, validates metadata, handles
 * compression, validates checksums, and extracts file descriptors.
 */
class RocketRequestProcessor {
 public:
  explicit RocketRequestProcessor(folly::AsyncTransport* transport);

  /**
   * Parse a request payload into metadata and data buffer.
   */
  folly::Try<RequestPayload> parseRequestPayload(
      Payload&& payload, RocketServerConnection& connection);

  /**
   * Setup checksum handling for the request metadata.
   */
  ChecksumAlgorithm setupChecksumHandling(
      RequestRpcMetadata& metadata, RocketServerConnection& connection);

  /**
   * Extract file descriptors from socket if present in metadata.
   */
  bool extractFileDescriptors(
      const RequestRpcMetadata& metadata, folly::Try<folly::SocketFds>& tryFds);

  /**
   * Validate request metadata against expected RPC kind.
   */
  bool validateRequestMetadata(
      const RequestRpcMetadata& metadata, RpcKind& expectedKind);

  /**
   * Process payload compression if needed.
   * Returns error message on failure, empty string on success.
   */
  std::string processPayloadCompression(
      std::unique_ptr<folly::IOBuf>& data,
      const RequestRpcMetadata& metadata,
      RocketServerConnection& connection);

  /**
   * Validate payload checksum if present in metadata.
   */
  bool validateChecksum(
      const std::unique_ptr<folly::IOBuf>& data,
      const RequestRpcMetadata& metadata);

  /**
   * Log application-level events from request metadata.
   */
  void logApplicationEvents(const RequestRpcMetadata& metadata);

 private:
  folly::AsyncTransport* transport_;
};

} // namespace apache::thrift::rocket
