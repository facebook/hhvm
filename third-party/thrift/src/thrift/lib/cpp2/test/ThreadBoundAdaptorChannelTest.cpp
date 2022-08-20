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

#include <thrift/lib/cpp2/async/ThreadBoundAdaptorChannel.h>

#include <gtest/gtest.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/io/async/test/ScopedBoundPort.h>
#include <thrift/lib/cpp2/async/PooledRequestChannel.h>
#include <thrift/lib/cpp2/async/RetryingRequestChannel.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestService.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

using namespace testing;
using namespace apache::thrift;
using namespace apache::thrift::test;
using apache::thrift::transport::TTransportException;
using folly::AsyncSocket;

class TestServiceServerMock
    : public apache::thrift::ServiceHandler<TestService> {
 public:
  MOCK_METHOD(int32_t, echoInt, (int32_t), (override));
  MOCK_METHOD(
      folly::SemiFuture<std::unique_ptr<std::string>>,
      semifuture_echoRequest,
      (std::unique_ptr<std::string>),
      (override));

  MOCK_METHOD(
      folly::SemiFuture<ServerStream<int8_t>>,
      semifuture_echoIOBufAsByteStream,
      (std::unique_ptr<folly::IOBuf>, int32_t),
      (override));
};

class ThreadBoundAdaptorChannelTest : public Test {
 public:
  void bounceServer() {
    runner =
        std::make_unique<apache::thrift::ScopedServerInterfaceThread>(handler);
    up_addr = runner->getAddress();
  }

  folly::EventBase* eb{folly::EventBaseManager::get()->getEventBase()};
  folly::ScopedBoundPort bound;
  std::shared_ptr<TestServiceServerMock> handler{
      std::make_shared<TestServiceServerMock>()};

  std::unique_ptr<apache::thrift::ScopedServerInterfaceThread> runner{
      std::make_unique<apache::thrift::ScopedServerInterfaceThread>(handler)};

  folly::SocketAddress up_addr{runner->getAddress()};
  folly::SocketAddress dn_addr{bound.getAddress()};
};

TEST_F(ThreadBoundAdaptorChannelTest, normalStreamResponseAndComplete) {
  auto evbThread = std::make_shared<folly::ScopedEventBaseThread>();
  auto pooledChannel = PooledRequestChannel::newSyncChannel(
      evbThread, [&](folly::EventBase& evb) {
        auto socket = AsyncSocket::UniquePtr(new AsyncSocket(&evb, up_addr));
        auto rocketChannel = RocketClientChannel::newChannel(std::move(socket));
        return rocketChannel;
      });

  folly::EventBase clientEvb;

  auto adaptorChannel = std::make_shared<ThreadBoundAdaptorChannel>(
      &clientEvb, std::move(pooledChannel));

  auto retryingChannel = RetryingRequestChannel::newChannel(
      clientEvb, 0, std::move(adaptorChannel));

  TestServiceAsyncClient client(std::move(retryingChannel));
  EXPECT_CALL(*handler, semifuture_echoIOBufAsByteStream(_, _))
      .WillOnce(
          Invoke([&](std::unique_ptr<folly::IOBuf> buf, int32_t /* delay */) {
            auto [stream, publisher] = ServerStream<int8_t>::createPublisher();
            folly::io::Cursor cursor(buf.get());
            int8_t byte;
            while (cursor.tryRead(byte)) {
              publisher.next(byte);
            }
            std::move(publisher).complete();
            return folly::makeSemiFuture(std::move(stream));
          }));

  auto iobuf = folly::IOBuf::copyBuffer(std::string(1, 'x'));
  client.sync_echoIOBufAsByteStream(*iobuf, 0);
}

TEST_F(ThreadBoundAdaptorChannelTest, failStreamAfterBounce) {
  auto evbThread = std::make_shared<folly::ScopedEventBaseThread>();
  auto pooledChannel = PooledRequestChannel::newSyncChannel(
      evbThread, [&](folly::EventBase& evb) {
        auto socket = AsyncSocket::UniquePtr(new AsyncSocket(&evb, up_addr));
        auto rocketChannel = RocketClientChannel::newChannel(std::move(socket));
        return rocketChannel;
      });

  folly::EventBase clientEvb;

  auto adaptorChannel = std::make_shared<ThreadBoundAdaptorChannel>(
      &clientEvb, std::move(pooledChannel));

  auto retryingChannel = RetryingRequestChannel::newChannel(
      clientEvb, 0, std::move(adaptorChannel));

  TestServiceAsyncClient client(std::move(retryingChannel));

  EXPECT_CALL(*handler, semifuture_echoIOBufAsByteStream(_, _))
      .WillOnce(InvokeWithoutArgs([] {
        auto [stream, publisher] = ServerStream<int8_t>::createPublisher();
        std::move(publisher).complete();
        return folly::makeSemiFuture(std::move(stream));
      }));

  auto iobuf = folly::IOBuf::copyBuffer(std::string(1, 'x'));
  client.sync_echoIOBufAsByteStream(*iobuf, 0);

  bounceServer();

  EXPECT_THROW(
      client.sync_echoIOBufAsByteStream(*iobuf, 0), TTransportException);
}

TEST_F(ThreadBoundAdaptorChannelTest, streamErrorFromServer) {
  auto evbThread = std::make_shared<folly::ScopedEventBaseThread>();
  auto pooledChannel = PooledRequestChannel::newSyncChannel(
      evbThread, [&](folly::EventBase& evb) {
        auto socket = AsyncSocket::UniquePtr(new AsyncSocket(&evb, up_addr));
        auto rocketChannel = RocketClientChannel::newChannel(std::move(socket));
        return rocketChannel;
      });

  folly::EventBase clientEvb;

  auto adaptorChannel = std::make_shared<ThreadBoundAdaptorChannel>(
      &clientEvb, std::move(pooledChannel));

  auto retryingChannel = RetryingRequestChannel::newChannel(
      clientEvb, 0, std::move(adaptorChannel));

  TestServiceAsyncClient client(std::move(retryingChannel));

  EXPECT_CALL(*handler, semifuture_echoIOBufAsByteStream(_, _))
      .WillOnce(InvokeWithoutArgs([] {
        auto [stream, publisher] = ServerStream<int8_t>::createPublisher();
        auto ew = folly::exception_wrapper{std::runtime_error("abcd")};
        std::move(publisher).complete(ew);
        return folly::makeSemiFuture(std::move(stream));
      }));

  auto iobuf = folly::IOBuf::copyBuffer(std::string(1, 'x'));
  client.sync_echoIOBufAsByteStream(*iobuf, 0);
}

TEST_F(ThreadBoundAdaptorChannelTest, normalSingleRequestSuccess) {
  auto evbThread = std::make_shared<folly::ScopedEventBaseThread>();
  auto pooledChannel = PooledRequestChannel::newChannel(
      evbThread->getEventBase(), evbThread, [&](folly::EventBase& evb) {
        auto socket = AsyncSocket::UniquePtr(new AsyncSocket(&evb, up_addr));
        auto rocketChannel = RocketClientChannel::newChannel(std::move(socket));
        return rocketChannel;
      });

  folly::EventBase clientEvb;

  auto adaptorChannel = std::make_shared<ThreadBoundAdaptorChannel>(
      &clientEvb, std::move(pooledChannel));

  auto retryingChannel = RetryingRequestChannel::newChannel(
      clientEvb, 0, std::move(adaptorChannel));

  TestServiceAsyncClient client(std::move(retryingChannel));
  EXPECT_CALL(*handler, echoInt(_)).WillOnce(Return(1)).WillOnce(Return(2));
  EXPECT_EQ(client.sync_echoInt(1), 1);
  EXPECT_EQ(client.sync_echoInt(2), 2);
}

TEST_F(ThreadBoundAdaptorChannelTest, singleRequestFailOnBounce) {
  auto evbThread = std::make_shared<folly::ScopedEventBaseThread>();
  auto pooledChannel = PooledRequestChannel::newChannel(
      evbThread->getEventBase(), evbThread, [&](folly::EventBase& evb) {
        auto socket = AsyncSocket::UniquePtr(new AsyncSocket(&evb, up_addr));
        auto rocketChannel = RocketClientChannel::newChannel(std::move(socket));
        return rocketChannel;
      });

  folly::EventBase clientEvb;

  auto adaptorChannel = std::make_shared<ThreadBoundAdaptorChannel>(
      &clientEvb, std::move(pooledChannel));

  auto retryingChannel = RetryingRequestChannel::newChannel(
      clientEvb, 0, std::move(adaptorChannel));

  TestServiceAsyncClient client(std::move(retryingChannel));

  EXPECT_CALL(*handler, echoInt(_)).WillOnce(Return(1));
  EXPECT_EQ(client.sync_echoInt(1), 1);

  bounceServer();

  EXPECT_THROW(client.sync_echoInt(2), TTransportException);
}
