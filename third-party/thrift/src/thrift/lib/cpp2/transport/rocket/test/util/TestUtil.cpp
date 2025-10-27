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

#include <thrift/lib/cpp2/transport/rocket/test/util/TestUtil.h>

#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/transport/core/testutil/TAsyncSocketIntercepted.h>

DECLARE_int32(num_client_connections);
DECLARE_string(transport); // ConnectionManager depends on this flag.

namespace apache::thrift {

std::unique_ptr<ThriftServer> TestSetup::createServer(
    std::shared_ptr<AsyncProcessorFactory> processorFactory,
    uint16_t& port,
    int maxRequests,
    std::string transport) {
  // override the default
  FLAGS_transport = transport; // client's transport
  observer_ = std::make_shared<FakeServerObserver>();

  auto server = std::make_unique<ThriftServer>();
  if (maxRequests > 0) {
    server->setMaxRequests(maxRequests);
  }
  server->setObserver(observer_);
  server->setPort(0);
  server->setNumIOWorkerThreads(numIOThreads_);
  server->setNumCPUWorkerThreads(numWorkerThreads_);
  if (queueTimeout_.has_value()) {
    server->setQueueTimeout(*queueTimeout_);
  }
  if (idleTimeout_.has_value()) {
    server->setIdleTimeout(*idleTimeout_);
  }
  if (taskExpireTime_.has_value()) {
    server->setTaskExpireTime(*taskExpireTime_);
  }
  if (streamExpireTime_.has_value()) {
    server->setStreamExpireTime(*streamExpireTime_);
  }

  server->setInterface(processorFactory);

  server->disableInfoLogging();

  auto eventHandler = std::make_shared<TestEventHandler>();
  server->setServerEventHandler(eventHandler);
  server->setup();

  // Get the port that the server has bound to
  port = eventHandler->waitForPortAssignment();
  return server;
}

RequestChannel::Ptr TestSetup::connectToServer(
    uint16_t port,
    folly::Function<void()> onDetachable,
    folly::Function<void(TAsyncSocketIntercepted&)> socketSetup) {
  CHECK_GT(port, 0) << "Check if the server has started already";
  return PooledRequestChannel::newChannel(
      evbThread_.getEventBase(),
      ioThread_,
      [port,
       onDetachable = std::move(onDetachable),
       socketSetup = std::move(socketSetup)](folly::EventBase& evb) mutable
          -> std::
              unique_ptr<ClientChannel, folly::DelayedDestruction::Destructor> {
                auto socket = folly::AsyncSocket::UniquePtr(
                    new TAsyncSocketIntercepted(&evb, "::1", port));
                if (socketSetup) {
                  socketSetup(
                      *static_cast<TAsyncSocketIntercepted*>(socket.get()));
                }

                ClientChannel::Ptr channel =
                    RocketClientChannel::newChannel(std::move(socket));

                if (onDetachable) {
                  channel->setOnDetachable(std::move(onDetachable));
                }
                return channel;
              });
}

} // namespace apache::thrift
