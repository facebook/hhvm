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

#include <functional>
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
  MOCK_METHOD(void, noResponse, (int64_t), (override));
  MOCK_METHOD(
      apache::thrift::ServerStream<int32_t>,
      range,
      (int32_t, int32_t),
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

class ReconnectingRequestChannelTest : public Test {
 public:
  folly::EventBase* eb{folly::EventBaseManager::get()->getEventBase()};
  folly::ScopedBoundPort bound;
  std::shared_ptr<TestServiceServerMock> handler{
      std::make_shared<TestServiceServerMock>()};
  std::unique_ptr<apache::thrift::ScopedServerInterfaceThread> runner{
      std::make_unique<apache::thrift::ScopedServerInterfaceThread>(handler)};

  folly::SocketAddress up_addr{runner->getAddress()};
  folly::SocketAddress dn_addr{bound.getAddress()};
  uint32_t connection_count_ = 0;

  void runReconnect(TestServiceAsyncClient& client, bool testStreaming);

#if FOLLY_HAS_COROUTINES
  folly::coro::Task<std::shared_ptr<TestServiceAsyncClient>> co_getClient() {
    auto executor = co_await folly::coro::co_current_executor;
    auto channel = PooledRequestChannel::newChannel(
        executor, ioThread_, [this](folly::EventBase& evb) {
          return ReconnectingRequestChannel::newChannel(
              evb,
              [this](
                  folly::EventBase& evb,
                  folly::AsyncSocket::ConnectCallback& cb) {
                SCOPE_EXIT {
                  cb.connectSuccess();
                };
                auto socket =
                    AsyncSocket::UniquePtr(new AsyncSocket(&evb, up_addr));
                return RocketClientChannel::newChannel(std::move(socket));
              });
        });
    co_return std::make_shared<TestServiceAsyncClient>(std::move(channel));
  }
  std::shared_ptr<folly::IOExecutor> ioThread_{
      std::make_shared<folly::ScopedEventBaseThread>()};
#endif
};

TEST_F(ReconnectingRequestChannelTest, ReconnectHeader) {
  auto channel = ReconnectingRequestChannel::newChannel(
      *eb,
      [this](
          folly::EventBase& eb,
          folly::AsyncSocket::ConnectCallback& cb) mutable {
        connection_count_++;
        SCOPE_EXIT {
          cb.connectSuccess();
        };
        return HeaderClientChannel::newChannel(
            AsyncSocket::newSocket(&eb, up_addr));
      });
  TestServiceAsyncClient client(std::move(channel));
  runReconnect(client, false);
}

TEST_F(ReconnectingRequestChannelTest, ReconnectRocket) {
  auto channel = ReconnectingRequestChannel::newChannel(
      *eb,
      [this](
          folly::EventBase& eb,
          folly::AsyncSocket::ConnectCallback& cb) mutable {
        connection_count_++;
        SCOPE_EXIT {
          cb.connectSuccess();
        };

        return RocketClientChannel::newChannel(
            folly::AsyncSocket::UniquePtr(
                new folly::AsyncSocket(&eb, up_addr)));
      });
  TestServiceAsyncClient client(std::move(channel));
  runReconnect(client, true);
}

void ReconnectingRequestChannelTest::runReconnect(
    TestServiceAsyncClient& client, bool testStreaming) {
  EXPECT_CALL(*handler, echoInt(_))
      .WillOnce(Return(1))
      .WillOnce(Return(3))
      .WillOnce(Return(4));
  EXPECT_EQ(client.sync_echoInt(1), 1);
  EXPECT_EQ(connection_count_, 1);

  EXPECT_CALL(*handler, noResponse(_));
  client.sync_noResponse(0);
  EXPECT_EQ(connection_count_, 1);

  auto checkStream = [](auto&& stream, int from, int to) {
    std::move(stream).subscribeInline([idx = from, to](auto nextTry) mutable {
      DCHECK(!nextTry.hasException());
      if (!nextTry.hasValue()) {
        EXPECT_EQ(to, idx);
      } else {
        EXPECT_EQ(idx++, *nextTry);
      }
    });
  };

  if (testStreaming) {
    EXPECT_CALL(*handler, range(_, _))
        .WillRepeatedly(Invoke(
            [](int32_t from,
               int32_t to) -> apache::thrift::ServerStream<int32_t> {
              auto [serverStream, publisher] =
                  apache::thrift::ServerStream<int32_t>::createPublisher();
              for (auto idx = from; idx < to; ++idx) {
                publisher.next(idx);
              }
              std::move(publisher).complete();
              return std::move(serverStream);
            }));
    checkStream(client.sync_range(0, 1), 0, 1);
    EXPECT_EQ(connection_count_, 1);
  }

  // bounce the server
  runner =
      std::make_unique<apache::thrift::ScopedServerInterfaceThread>(handler);
  up_addr = runner->getAddress();

  EXPECT_THROW(client.sync_echoInt(2), TTransportException);
  EXPECT_EQ(client.sync_echoInt(3), 3);
  EXPECT_EQ(connection_count_, 2);
  EXPECT_EQ(client.sync_echoInt(4), 4);
  EXPECT_EQ(connection_count_, 2);

  if (testStreaming) {
    checkStream(client.sync_range(0, 2), 0, 2);
    EXPECT_EQ(connection_count_, 2);
    checkStream(client.sync_range(4, 42), 4, 42);
    EXPECT_EQ(connection_count_, 2);
  }
}

#if FOLLY_HAS_COROUTINES
TEST_F(ReconnectingRequestChannelTest, sinkReconnect) {
  folly::coro::blockingWait([&]() -> folly::coro::Task<void> {
    auto client = co_await co_getClient();
    auto consumer = co_await client->co_sumSink();
    auto res =
        co_await consumer.sink([]() -> folly::coro::AsyncGenerator<int32_t&&> {
          for (int32_t i = 1; i <= 5; ++i) {
            co_yield int(i);
          }
        }());
    EXPECT_EQ(res, 15);

    // bounce
    runner =
        std::make_unique<apache::thrift::ScopedServerInterfaceThread>(handler);
    up_addr = runner->getAddress();

    // no exception here - the underlying impl is marked !good so the next
    // request ends up triggering the reconnect
    consumer = co_await client->co_sumSink();
    res =
        co_await consumer.sink([]() -> folly::coro::AsyncGenerator<int32_t&&> {
          for (int32_t i = 1; i <= 5; ++i) {
            co_yield int(i);
          }
        }());
    EXPECT_EQ(res, 15);
  }());
}
#endif

#if FOLLY_HAS_COROUTINES
TEST_F(ReconnectingRequestChannelTest, RequestsDuringReconnect) {
  folly::coro::blockingWait([&]() -> folly::coro::Task<void> {
    folly::AsyncSocket::ConnectCallback* cbPtr;
    folly::coro::Baton baton;

    auto executor = co_await folly::coro::co_current_executor;
    auto channel = PooledRequestChannel::newChannel(
        executor, ioThread_, [&](folly::EventBase& evb) {
          return ReconnectingRequestChannel::newChannel(
              evb,
              [&](folly::EventBase& evb,
                  folly::AsyncSocket::ConnectCallback& cb) {
                // Do not connect yet
                cbPtr = &cb;
                baton.post();

                auto socket =
                    AsyncSocket::UniquePtr(new AsyncSocket(&evb, up_addr));
                return RocketClientChannel::newChannel(std::move(socket));
              });
        });
    auto client = std::make_shared<TestServiceAsyncClient>(std::move(channel));

    // This will 'hang' due to connection not being connected
    auto request1 = client->semifuture_echoInt(1);

    // To make sure connect attempt is in progress
    co_await baton;

    auto request2 = client->semifuture_echoInt(1);
    ioThread_->getEventBase()->runImmediatelyOrRunInEventBaseThreadAndWait(
        [&] { cbPtr->connectSuccess(); });

    co_await std::move(request1);
    co_await std::move(request2);
  }());
}
#endif
