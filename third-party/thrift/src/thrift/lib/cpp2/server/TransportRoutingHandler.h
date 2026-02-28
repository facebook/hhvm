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

#include <vector>

#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncTransport.h>
#include <wangle/acceptor/ConnectionManager.h>
#include <wangle/acceptor/TransportInfo.h>

namespace apache::thrift {

class Cpp2Worker;

/*
 * An interface used by ThriftServer to route the
 * socket to different Transports.
 */
class TransportRoutingHandler {
 public:
  TransportRoutingHandler() = default;
  virtual ~TransportRoutingHandler() = default;
  TransportRoutingHandler(const TransportRoutingHandler&) = delete;

  /**
   * Stop accepting new connections as the server shuts down.
   */
  virtual void stopListening() = 0;

  /*
   * Performs a check on the first bytes read from the wire
   * and determines if this protocol is supported by this routing handler.
   * Transport info is provided in case additional validation is needed
   * around security and other properties of the transport.
   */
  virtual bool canAcceptConnection(
      const std::vector<uint8_t>& bytes,
      const wangle::TransportInfo& tinfo) = 0;

  /*
   * Determines if the protocol indicated by the protocol name is supported by
   * this routing handler.
   */
  virtual bool canAcceptEncryptedConnection(
      const std::string& protocolName) = 0;

  /*
   * Creates the correct session to route the socket to the appropriate
   * protocol handler
   */
  virtual void handleConnection(
      wangle::ConnectionManager* connectionManager,
      folly::AsyncTransport::UniquePtr sock,
      const folly::SocketAddress* peerAddress,
      const wangle::TransportInfo& tinfo,
      std::shared_ptr<Cpp2Worker> cpp2Worker) = 0;
};

} // namespace apache::thrift
