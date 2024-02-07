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

#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncTransport.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/server/TransportRoutingHandler.h>
#include <wangle/acceptor/ConnectionManager.h>
#include <wangle/acceptor/TransportInfo.h>

#include <atomic>
#include <vector>

namespace apache {
namespace thrift {

/**
 * A TransportRoutingHandler implementation that handles Header transport,
 * the legacy transports, Framed and Unframed, as well as HTTP/1.1.
 *
 * For plaintext connections, it relies on parsing similar to that found in
 * THeader class to detect and accept the transport.
 *
 * For encrypted connections, it looks for and accepts either the "thrift" or
 * the "http/1.1" protocol name and ALPN value.
 */
class LegacyHeaderRoutingHandler : public TransportRoutingHandler {
 public:
  explicit LegacyHeaderRoutingHandler(ThriftServer&);
  ~LegacyHeaderRoutingHandler() override;
  LegacyHeaderRoutingHandler(const LegacyHeaderRoutingHandler&) = delete;
  LegacyHeaderRoutingHandler& operator=(const LegacyHeaderRoutingHandler&) =
      delete;

  void stopListening() override;
  bool canAcceptConnection(
      const std::vector<uint8_t>& bytes,
      const wangle::TransportInfo& tinfo) override;
  bool canAcceptEncryptedConnection(const std::string& protocolName) override;
  void handleConnection(
      wangle::ConnectionManager* connectionManager,
      folly::AsyncTransport::UniquePtr sock,
      const folly::SocketAddress* peerAddress,
      const wangle::TransportInfo& tinfo,
      std::shared_ptr<Cpp2Worker> worker) override;

 private:
  std::atomic<bool> listening_{true};
};

} // namespace thrift
} // namespace apache
