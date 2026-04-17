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

#include <thrift/lib/cpp2/fast_thrift/test/IntegrationTestFixture.h>

#include <folly/io/async/AsyncSocket.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>

namespace apache::thrift::fast_thrift::test {

namespace {

class ConnectCallback : public folly::AsyncSocket::ConnectCallback {
 public:
  explicit ConnectCallback(
      apache::thrift::fast_thrift::transport::TransportHandler* handler)
      : handler_(handler) {}

  void connectSuccess() noexcept override { handler_->onConnect(); }

  void connectErr(const folly::AsyncSocketException& ex) noexcept override {
    handler_->onClose(ex);
  }

 private:
  apache::thrift::fast_thrift::transport::TransportHandler* handler_;
};

} // namespace

void IntegrationTestFixture::SetUp() {
  executor_ = std::make_shared<folly::IOThreadPoolExecutor>(1);

  apache::thrift::fast_thrift::rocket::server::connection::ConnectionFactory
      connectionFactory = [this](folly::AsyncSocket::UniquePtr socket)
      -> apache::thrift::fast_thrift::rocket::server::connection::
          RocketServerConnection {
            auto* evb = socket->getEventBase();
            auto transportHandler = apache::thrift::fast_thrift::transport::
                TransportHandler::create(std::move(socket));

            auto pipeline =
                apache::thrift::fast_thrift::channel_pipeline::PipelineBuilder<
                    ServerTransportHandler,
                    TestServerAppAdapter,
                    apache::thrift::fast_thrift::channel_pipeline::
                        SimpleBufferAllocator>()
                    .setEventBase(evb)
                    .setHead(transportHandler.get())
                    .setTail(&serverAppAdapter_)
                    .setAllocator(&serverAllocator_)
                    .build();
            serverAppAdapter_.setPipeline(pipeline.get());
            transportHandler->setPipeline(*pipeline);

            return apache::thrift::fast_thrift::rocket::server::connection::
                RocketServerConnection{
                    .transportHandler = std::move(transportHandler),
                    .pipeline = std::move(pipeline),
                    .allocator = {},
                };
          };

  connectionManager_ = TestConnectionManager::create(
      folly::SocketAddress("::1", 0),
      folly::getKeepAliveToken(executor_.get()),
      std::move(connectionFactory));
  connectionManager_->start();

  serverAddress_ = connectionManager_->getAddress();
}

void IntegrationTestFixture::TearDown() {
  disconnectClient();
  if (connectionManager_) {
    connectionManager_->stop();
    connectionManager_.reset();
  }
  if (executor_) {
    executor_->join();
    executor_.reset();
  }
}

ClientConnection& IntegrationTestFixture::connectClient() {
  clientThread_ = std::make_unique<folly::ScopedEventBaseThread>();
  auto* evb = clientThread_->getEventBase();

  evb->runInEventBaseThreadAndWait([this, evb]() {
    auto socket = folly::AsyncSocket::newSocket(evb);
    auto* socketPtr = socket.get();

    auto transportHandler = ClientTransportHandler::create(std::move(socket));

    auto pipeline =
        apache::thrift::fast_thrift::channel_pipeline::PipelineBuilder<
            ClientTransportHandler,
            TestClientAppAdapter,
            apache::thrift::fast_thrift::channel_pipeline::
                SimpleBufferAllocator>()
            .setEventBase(evb)
            .setHead(transportHandler.get())
            .setTail(&clientConnection_.appAdapter)
            .setAllocator(&clientAllocator_)
            .build();

    transportHandler->setPipeline(*pipeline);
    clientConnection_.appAdapter.setPipeline(pipeline.get());
    clientConnection_.appAdapter.setEventBase(evb);

    clientConnection_.transportHandler = std::move(transportHandler);
    clientConnection_.pipeline = std::move(pipeline);

    clientConnection_.connectCallback = std::make_unique<ConnectCallback>(
        clientConnection_.transportHandler.get());
    socketPtr->connect(clientConnection_.connectCallback.get(), serverAddress_);
  });

  return clientConnection_;
}

void IntegrationTestFixture::disconnectClient() {
  if (clientThread_) {
    auto* evb = clientThread_->getEventBase();
    evb->runInEventBaseThreadAndWait([this]() {
      clientConnection_.connectCallback.reset();
      clientConnection_.transportHandler.reset();
      clientConnection_.pipeline.reset();
    });
    clientThread_.reset();
  }
}

TestServerAppAdapter& IntegrationTestFixture::serverAppAdapter() {
  return serverAppAdapter_;
}

} // namespace apache::thrift::fast_thrift::test
