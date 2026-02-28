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

#include <proxygen/httpserver/HTTPServerOptions.h>
#include <thrift/lib/cpp2/server/ThriftProcessor.h>
#include <thrift/lib/cpp2/server/TransportRoutingHandler.h>

namespace apache::thrift {

class Cpp2Worker;

/*
 * This handler is used to determine if a client is talking HTTP2 and
 * routes creates the handler to route the socket to Proxygen
 */
class HTTP2RoutingHandler : public TransportRoutingHandler {
 public:
  explicit HTTP2RoutingHandler(
      std::unique_ptr<proxygen::HTTPServerOptions> options,
      ThriftProcessor* processor,
      const apache::thrift::server::ServerConfigs& serverConfigs)
      : options_(std::move(options)),
        processor_(processor),
        serverConfigs_(serverConfigs) {}
  ~HTTP2RoutingHandler() override = default;
  HTTP2RoutingHandler(const HTTP2RoutingHandler&) = delete;

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
      std::shared_ptr<Cpp2Worker>) override;

 private:
  // HTTPServerOptions are set outside out HTTP2RoutingHandler.
  // Since one of the internal members of this class is a unique_ptr
  // we need to set this object as a unique_ptr as well in order to properly
  // move it into the class.
  std::unique_ptr<proxygen::HTTPServerOptions> options_;

  ThriftProcessor* processor_;

  const apache::thrift::server::ServerConfigs& serverConfigs_;

  bool listening_{true};
};

} // namespace apache::thrift
