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

#include <chrono>
#include <future>
#include <memory>
#include <string>

#include <folly/Try.h>
#include <folly/io/async/AsyncServerSocket.h>
#include <folly/io/async/ScopedEventBaseThread.h>

#include <thrift/lib/cpp2/async/ServerStream.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/client/RocketClient.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

#include <wangle/acceptor/ConnectionManager.h>

namespace folly {
class EventBase;
class IOBuf;
class SocketAddress;

namespace fibers {
class FiberManager;
} // namespace fibers
} // namespace folly

namespace wangle {
class Acceptor;
} // namespace wangle

namespace apache::thrift {
class RequestClientCallback;
class StreamClientCallback;
class ChannelClientCallback;
class SinkClientCallback;

namespace rocket::test {

class RocketTestClient {
 public:
  explicit RocketTestClient(const folly::SocketAddress& serverAddr);
  ~RocketTestClient();

  folly::Try<Payload> sendRequestResponseSync(
      Payload request,
      std::chrono::milliseconds timeout = std::chrono::milliseconds(250),
      RocketClient::WriteSuccessCallback* writeSuccessCallback = nullptr);

  folly::Try<void> sendRequestFnfSync(Payload request);

  folly::Try<ClientBufferedStream<Payload>> sendRequestStreamSync(
      Payload request);
  void sendRequestSink(SinkClientCallback* callback, Payload request);

  RequestSetupMetadata makeTestSetupMetadata(
      MetadataOpaqueMap<std::string, std::string> md =
          MetadataOpaqueMap<std::string, std::string>{
              {"rando_key", "setup_data"}});

  void reconnect();
  void connect();
  void disconnect();

  RocketClient& getRawClient() { return *client_; }

  folly::EventBase& getEventBase() { return evb_; }

  void verifyVersion();

 private:
  folly::ScopedEventBaseThread evbThread_;
  folly::EventBase& evb_;
  folly::fibers::FiberManager& fm_;
  std::unique_ptr<RocketClient, folly::DelayedDestruction::Destructor> client_;
  const folly::SocketAddress serverAddr_;
};

class RocketTestServer {
 public:
  RocketTestServer();
  ~RocketTestServer();

  uint16_t getListeningPort() const;
  wangle::ConnectionManager* getConnectionManager() const;
  void setExpectedRemainingStreams(size_t n);

  void setExpectedSetupMetadata(MetadataOpaqueMap<std::string, std::string> md);

  folly::EventBase& getEventBase() const { return evb_; }

 private:
  class RocketTestServerHandler;

  folly::ScopedEventBaseThread ioThread_;
  folly::EventBase& evb_;
  folly::AsyncServerSocket::UniquePtr listeningSocket_;
  MetadataOpaqueMap<std::string, std::string> expectedSetupMetadata_{
      {"rando_key", "setup_data"}};
  std::unique_ptr<wangle::Acceptor> acceptor_;
  std::future<void> shutdownFuture_;

  void start();
  void stop();
};

} // namespace rocket::test

} // namespace apache::thrift
