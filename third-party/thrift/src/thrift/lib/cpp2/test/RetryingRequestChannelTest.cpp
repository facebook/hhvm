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

#include <thrift/lib/cpp2/async/RetryingRequestChannel.h>

#include <folly/Portability.h>
#include <folly/coro/AsyncGenerator.h>
#include <folly/coro/BlockingWait.h>
#include <folly/coro/Task.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/io/async/test/ScopedBoundPort.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/lib/cpp2/async/PooledRequestChannel.h>
#include <thrift/lib/cpp2/async/ReconnectingRequestChannel.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestService.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

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

#if FOLLY_HAS_COROUTINES
  folly::coro::Task<SinkConsumer<int32_t, int32_t>> co_sumSink() override {
    SinkConsumer<int32_t, int32_t> sink;
    sink.consumer = [](folly::coro::AsyncGenerator<int32_t&&> gen)
        -> folly::coro::Task<int32_t> {
      int32_t res = 0;
      while (auto val = co_await gen.next()) {
        res += *val;
      }
      co_return res;
    };
    sink.bufferSize = 10;
    co_return sink;
  }
#endif
};

class RetryingRequestChannelTest : public Test {
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

#if FOLLY_HAS_COROUTINES
  folly::coro::Task<std::shared_ptr<TestServiceAsyncClient>> co_getClient(
      int numRetries) {
    auto executor = co_await folly::coro::co_current_executor;
    auto channel = PooledRequestChannel::newChannel(
        executor, ioThread_, [numRetries, this](folly::EventBase& evb) {
          auto socket = AsyncSocket::UniquePtr(new AsyncSocket(&evb, up_addr));
          auto rocket = RocketClientChannel::newChannel(std::move(socket));
          return RetryingRequestChannel::newChannel(
              evb, numRetries, std::move(rocket));
        });
    co_return std::make_shared<TestServiceAsyncClient>(std::move(channel));
  }
  std::shared_ptr<folly::IOExecutor> ioThread_{
      std::make_shared<folly::ScopedEventBaseThread>()};
#endif
};

TEST_F(RetryingRequestChannelTest, noRetrySuccess) {
  std::shared_ptr<RequestChannel> up_chan =
      HeaderClientChannel::newChannel(AsyncSocket::newSocket(eb, up_addr));
  auto channel = RetryingRequestChannel::newChannel(*eb, 0, up_chan);

  TestServiceAsyncClient client(std::move(channel));
  EXPECT_CALL(*handler, echoInt(_)).WillOnce(Return(1)).WillOnce(Return(2));
  EXPECT_EQ(client.sync_echoInt(1), 1);
  EXPECT_EQ(client.sync_echoInt(2), 2);
}

TEST_F(RetryingRequestChannelTest, noRetryStreamSuccess) {
  auto socket = AsyncSocket::UniquePtr(new AsyncSocket(eb, up_addr));
  auto channel = RetryingRequestChannel::newChannel(
      *eb, 1, RocketClientChannel::newChannel(std::move(socket)));

  TestServiceAsyncClient client(std::move(channel));
  EXPECT_CALL(*handler, semifuture_echoIOBufAsByteStream(_, _))
      .WillOnce(InvokeWithoutArgs([] {
        return folly::makeSemiFuture(ServerStream<int8_t>::createEmpty());
      }));

  auto iobuf = folly::IOBuf::copyBuffer(std::string(1, 'x'));
  client.sync_echoIOBufAsByteStream(*iobuf, 0);
}

TEST_F(RetryingRequestChannelTest, noRetryFail) {
  auto channel = RetryingRequestChannel::newChannel(
      *eb,
      0,
      ReconnectingRequestChannel::newChannel(
          *eb, [this](folly::EventBase& eb) mutable {
            return HeaderClientChannel::newChannel(
                AsyncSocket::newSocket(&eb, up_addr));
          }));

  TestServiceAsyncClient client(std::move(channel));
  EXPECT_CALL(*handler, echoInt(_)).WillOnce(Return(1)).WillOnce(Return(3));
  EXPECT_EQ(client.sync_echoInt(1), 1);

  bounceServer();

  EXPECT_THROW(client.sync_echoInt(2), TTransportException);
  EXPECT_EQ(client.sync_echoInt(3), 3);
}

TEST_F(RetryingRequestChannelTest, noRetryStreamFail) {
  auto socket = AsyncSocket::UniquePtr(new AsyncSocket(eb, up_addr));
  auto channel = RetryingRequestChannel::newChannel(
      *eb, 1, RocketClientChannel::newChannel(std::move(socket)));

  TestServiceAsyncClient client(std::move(channel));

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

TEST_F(RetryingRequestChannelTest, retry) {
  auto channel = RetryingRequestChannel::newChannel(
      *eb,
      1,
      ReconnectingRequestChannel::newChannel(
          *eb, [this](folly::EventBase& eb) mutable {
            return HeaderClientChannel::newChannel(
                AsyncSocket::newSocket(&eb, up_addr));
          }));

  TestServiceAsyncClient client(std::move(channel));
  EXPECT_CALL(*handler, echoInt(_))
      .WillOnce(Return(1))
      .WillOnce(Return(2))
      .WillOnce(Return(3));
  EXPECT_EQ(client.sync_echoInt(1), 1);

  bounceServer();

  EXPECT_EQ(client.sync_echoInt(2), 2);
  EXPECT_EQ(client.sync_echoInt(3), 3);
}

TEST_F(RetryingRequestChannelTest, retryStream) {
  auto channel = RetryingRequestChannel::newChannel(
      *eb,
      1,
      ReconnectingRequestChannel::newChannel(
          *eb, [this](folly::EventBase& eb) mutable {
            auto socket = AsyncSocket::UniquePtr(new AsyncSocket(&eb, up_addr));
            return RocketClientChannel::newChannel(std::move(socket));
          }));

  TestServiceAsyncClient client(std::move(channel));
  EXPECT_CALL(*handler, semifuture_echoIOBufAsByteStream(_, _))
      .WillOnce(InvokeWithoutArgs([] {
        auto [stream, publisher] = ServerStream<int8_t>::createPublisher();
        std::move(publisher).complete();
        return folly::makeSemiFuture(std::move(stream));
      }))
      .WillOnce(InvokeWithoutArgs([] {
        auto [stream, publisher] = ServerStream<int8_t>::createPublisher();
        std::move(publisher).complete();
        return folly::makeSemiFuture(std::move(stream));
      }));

  auto iobuf = folly::IOBuf::copyBuffer(std::string(1, 'x'));
  client.sync_echoIOBufAsByteStream(*iobuf, 0);

  bounceServer();

  client.sync_echoIOBufAsByteStream(*iobuf, 0);
}

TEST_F(RetryingRequestChannelTest, shutdown) {
  auto evbThread = std::make_shared<folly::ScopedEventBaseThread>();

  auto channel = PooledRequestChannel::newSyncChannel(
      evbThread, [&](folly::EventBase& evb) {
        auto headerChannel = HeaderClientChannel::newChannel(
            AsyncSocket::newSocket(&evb, up_addr));
        headerChannel->setTimeout(1000);
        return RetryingRequestChannel::newChannel(
            evb, 2, std::move(headerChannel));
      });

  TestServiceAsyncClient client(std::move(channel));

  folly::Promise<std::unique_ptr<std::string>> promise;

  EXPECT_CALL(*handler, semifuture_echoRequest(_))
      .WillOnce(InvokeWithoutArgs([&] { return promise.getFuture(); }))
      .WillOnce(InvokeWithoutArgs([] {
        return folly::makeSemiFuture(std::make_unique<std::string>("Expected"));
      }));

  auto sf = client.semifuture_echoRequest("");

  evbThread.reset();

  EXPECT_EQ("Expected", std::move(sf).get());

  promise.setValue(std::make_unique<std::string>("Slow"));
}

TEST_F(RetryingRequestChannelTest, shutdownStream) {
  auto evbThread = std::make_shared<folly::ScopedEventBaseThread>();

  auto channel = PooledRequestChannel::newSyncChannel(
      evbThread, [&](folly::EventBase& evb) {
        auto socket = AsyncSocket::UniquePtr(new AsyncSocket(&evb, up_addr));
        auto rocketChannel = RocketClientChannel::newChannel(std::move(socket));
        rocketChannel->setTimeout(1000);
        return RetryingRequestChannel::newChannel(
            evb, 2, std::move(rocketChannel));
      });

  TestServiceAsyncClient client(std::move(channel));

  folly::Promise<ServerStream<int8_t>> promise;

  EXPECT_CALL(*handler, semifuture_echoIOBufAsByteStream(_, _))
      .WillOnce(InvokeWithoutArgs([&] { return promise.getFuture(); }))
      .WillOnce(InvokeWithoutArgs([] {
        auto [stream, publisher] = ServerStream<int8_t>::createPublisher();
        std::move(publisher).complete();
        return folly::makeSemiFuture(std::move(stream));
      }));
  auto iobuf = folly::IOBuf::copyBuffer(std::string(1, 'x'));
  auto sf = client.semifuture_echoIOBufAsByteStream(*iobuf, 0);
  evbThread.reset();
  std::move(sf).get();
}

#if FOLLY_HAS_COROUTINES
TEST_F(RetryingRequestChannelTest, sinkSuccess) {
  folly::coro::blockingWait([&]() -> folly::coro::Task<void> {
    auto client = co_await co_getClient(0 /* numRetries */);
    auto consumer = co_await client->co_sumSink();
    auto res =
        co_await consumer.sink([]() -> folly::coro::AsyncGenerator<int32_t&&> {
          for (int32_t i = 1; i <= 5; ++i) {
            co_yield int32_t(i);
          }
        }());
    EXPECT_EQ(res, 15);
  }());
}
#endif
