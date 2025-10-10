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

#include <thrift/lib/cpp2/PluggableFunction.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/server/TransportRoutingHandler.h>
#include <thrift/lib/cpp2/server/metrics/StreamMetricCallback.h>
#include <thrift/lib/cpp2/transport/rocket/server/IRocketServerConnection.h>
#include <thrift/lib/cpp2/transport/rocket/server/SetupFrameHandler.h>
#include <thrift/lib/cpp2/transport/rocket/server/SetupFrameInterceptor.h>

namespace apache::thrift {

class Cpp2Worker;
namespace rocket {
class RocketServerConnection;
}

namespace detail {

#define THRIFT_DETAIL_DECLARE_SERVER_EXTENSION(FUNC)              \
  THRIFT_PLUGGABLE_FUNC_DECLARE(                                  \
      std::unique_ptr<apache::thrift::rocket::SetupFrameHandler>, \
      FUNC,                                                       \
      apache::thrift::ThriftServer&);

THRIFT_DETAIL_DECLARE_SERVER_EXTENSION(createRocketDebugSetupFrameHandler)
THRIFT_DETAIL_DECLARE_SERVER_EXTENSION(createRocketMonitoringSetupFrameHandler)
THRIFT_DETAIL_DECLARE_SERVER_EXTENSION(createRocketProfilingSetupFrameHandler)
THRIFT_DETAIL_DECLARE_SERVER_EXTENSION(createSecuritySetupFrameHandler)

#undef THRIFT_DETAIL_DECLARE_EXTENSION_DEFAULT

THRIFT_PLUGGABLE_FUNC_DECLARE(
    std::unique_ptr<apache::thrift::rocket::SetupFrameInterceptor>,
    createSecuritySetupFrameInterceptor,
    apache::thrift::ThriftServer&);

THRIFT_PLUGGABLE_FUNC_DECLARE(
    std::string, getSocketParser, folly::AsyncTransport&);
} // namespace detail

class RocketRoutingHandler : public TransportRoutingHandler {
 public:
  explicit RocketRoutingHandler(ThriftServer&);
  ~RocketRoutingHandler() override;
  RocketRoutingHandler(const RocketRoutingHandler&) = delete;
  RocketRoutingHandler& operator=(const RocketRoutingHandler&) = delete;

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

 protected:
  virtual void onConnection(rocket::IRocketServerConnection&) {}

 private:
  std::atomic<bool> listening_{true};
  std::vector<std::unique_ptr<rocket::SetupFrameHandler>> setupFrameHandlers_;
  std::vector<std::unique_ptr<rocket::SetupFrameInterceptor>>
      setupFrameInterceptors_;
  StreamMetricCallback& streamMetricCallback_;
};
} // namespace apache::thrift
