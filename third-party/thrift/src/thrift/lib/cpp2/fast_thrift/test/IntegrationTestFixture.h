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

#include <gtest/gtest.h>

#include <folly/SocketAddress.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/connection/ConnectionManager.h>
#include <thrift/lib/cpp2/fast_thrift/test/TestClientAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/test/TestServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>

#include <memory>

namespace apache::thrift::fast_thrift::test {

using ServerTransportHandler =
    apache::thrift::fast_thrift::transport::TransportHandler;
using ClientTransportHandler =
    apache::thrift::fast_thrift::transport::TransportHandler;

using TestConnectionManager =
    apache::thrift::fast_thrift::rocket::server::connection::ConnectionManager<
        ServerTransportHandler>;

/**
 * ClientConnection holds all client-side resources.
 */
struct ClientConnection {
  ClientTransportHandler::Ptr transportHandler;
  apache::thrift::fast_thrift::channel_pipeline::PipelineImpl::Ptr pipeline;
  TestClientAppAdapter appAdapter;
  std::unique_ptr<folly::AsyncSocket::ConnectCallback> connectCallback;
};

/**
 * IntegrationTestFixture provides a complete client-server test environment.
 *
 * This fixture uses:
 * - Real TransportHandler components that read directly from sockets
 * - ConnectionManager for server-side connection management
 *
 * The wiring is:
 *
 *   Socket -> TransportHandler (ReadCallback)
 *          -> Pipeline::fireRead
 *          -> TestAppAdapter::onMessage
 */
class IntegrationTestFixture : public ::testing::Test {
 protected:
  void SetUp() override;
  void TearDown() override;

  ClientConnection& connectClient();
  void disconnectClient();
  TestServerAppAdapter& serverAppAdapter();

 private:
  void startServer();
  void stopServer();

  // Server resources
  std::shared_ptr<folly::IOThreadPoolExecutor> executor_;
  TestConnectionManager::Ptr connectionManager_;
  folly::SocketAddress serverAddress_;
  TestServerAppAdapter serverAppAdapter_;
  apache::thrift::fast_thrift::channel_pipeline::SimpleBufferAllocator
      serverAllocator_;

  // Client resources
  std::unique_ptr<folly::ScopedEventBaseThread> clientThread_;
  ClientConnection clientConnection_;
  apache::thrift::fast_thrift::channel_pipeline::SimpleBufferAllocator
      clientAllocator_;
};

} // namespace apache::thrift::fast_thrift::test
