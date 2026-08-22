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
#include <string>

#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncTransport.h>

namespace apache::thrift::fast_thrift::connection {

// What the peer proved about itself during the security handshake.
//
// Captured once, when it is negotiated, and carried alongside the transport
// rather than re-read from it later: a StopTLS downgrade replaces the
// transport with a plaintext one that can report neither. Null on a connection
// that negotiated no security.
//
// Held behind a shared_ptr so the messages carrying it stay inside the
// pipeline's inline-message budget; the allocation is once per handshake.
struct PeerSecurityInfo {
  std::shared_ptr<const folly::AsyncTransportCertificate> peerCertificate;
  std::string securityProtocol;
};

// Single message type that flows through the acceptance pipeline. Each
// accepted socket enters the pipeline as one ConnectionMessage at the head;
// each handler may upgrade the transport in-place (e.g. plain AsyncSocket →
// AsyncFizzServer after TLS) and forward the same message type. The tail
// invokes the installation callback to hand the ready transport off to the
// connection factory.
struct ConnectionMessage {
  folly::AsyncTransport::UniquePtr transport;
  folly::SocketAddress clientAddr;
  std::shared_ptr<const PeerSecurityInfo> peerSecurity;
};

} // namespace apache::thrift::fast_thrift::connection
