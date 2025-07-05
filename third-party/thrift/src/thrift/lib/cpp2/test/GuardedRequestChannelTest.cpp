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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <folly/coro/BlockingWait.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/ScopedEventBaseThread.h>

#include <thrift/lib/cpp2/async/GuardedRequestChannel.h>
#include <thrift/lib/cpp2/async/PooledRequestChannel.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/test/gen-cpp2/Calculator.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestService.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace testing;
using namespace apache::thrift;
using namespace apache::thrift::test;
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
  MOCK_METHOD(
      folly::SemiFuture<folly::Unit>,
      semifuture_noResponse,
      (int64_t),
      (override));
};

struct CalculatorHandler : apache::thrift::ServiceHandler<Calculator> {
  struct AdditionHandler
      : apache::thrift::ServiceHandler<Calculator>::AdditionIf {
    int acc_{0};
    Point pacc_;

#if FOLLY_HAS_COROUTINES
    folly::coro::Task<void> co_accumulatePrimitive(int32_t a) override {
      acc_ += a;
      co_return;
    }
    folly::coro::Task<void> co_noop() override { co_return; }
    folly::coro::Task<void> co_accumulatePoint(
        std::unique_ptr<::apache::thrift::test::Point> a) override {
      *pacc_.x() += *a->x();
      *pacc_.y() += *a->y();
      co_return;
    }
    folly::coro::Task<int32_t> co_getPrimitive() override { co_return acc_; }
    folly::coro::Task<std::unique_ptr<::apache::thrift::test::Point>>
    co_getPoint() override {
      co_return folly::copy_to_unique_ptr(pacc_);
    }
#endif
  };

  std::unique_ptr<AdditionIf> createAddition() override {
    return std::make_unique<AdditionHandler>();
  }

  folly::SemiFuture<int32_t> semifuture_addPrimitive(
      int32_t a, int32_t b) override {
    return a + b;
  }
};

class GuardedRequestChannelTest : public Test {
 private:
  RequestChannel::Ptr createGuardedRequestChannel() {
    auto pooledChannel = PooledRequestChannel::newChannel(
        evbThread->getEventBase(), evbThread, [&](folly::EventBase& evb) {
          auto socket =
              AsyncSocket::UniquePtr(new AsyncSocket(&evb, serverAddress));
          auto rocketChannel =
              RocketClientChannel::newChannel(std::move(socket));
          return rocketChannel;
        });

    return GuardedRequestChannel<folly::Unit, folly::Unit>::newChannel(
        std::move(pooledChannel));
  }

 protected:
  std::shared_ptr<TestServiceServerMock> handler{
      std::make_shared<TestServiceServerMock>()};
  std::unique_ptr<apache::thrift::ScopedServerInterfaceThread> runner{
      std::make_unique<apache::thrift::ScopedServerInterfaceThread>(handler)};
  folly::SocketAddress serverAddress{runner->getAddress()};
  std::shared_ptr<folly::ScopedEventBaseThread> evbThread{
      std::make_shared<folly::ScopedEventBaseThread>()};
  apache::thrift::Client<TestService> testClient{createGuardedRequestChannel()};
};

TEST_F(GuardedRequestChannelTest, normalSingleRequestSuccess) {
  EXPECT_CALL(*handler, echoInt(_)).WillOnce(Return(1)).WillOnce(Return(2));
  EXPECT_EQ(testClient.sync_echoInt(1), 1);
  EXPECT_EQ(testClient.sync_echoInt(2), 2);
}

TEST_F(GuardedRequestChannelTest, sendRequestNoResponse) {
  EXPECT_NO_THROW(testClient.semifuture_noResponse(1));
  EXPECT_NO_THROW(testClient.semifuture_noResponse(100));
}

TEST_F(GuardedRequestChannelTest, normalStreamResponseAndComplete) {
  EXPECT_CALL(*handler, semifuture_echoIOBufAsByteStream(_, _))
      .WillOnce(Invoke([&](std::unique_ptr<folly::IOBuf> buf, int32_t delayMs) {
        auto [stream, publisher] = ServerStream<int8_t>::createPublisher();
        folly::io::Cursor cursor(buf.get());
        int8_t byte;
        while (cursor.tryRead(byte)) {
          publisher.next(byte);
        }
        std::move(publisher).complete();
        return folly::makeSemiFuture(std::move(stream))
            .delayed(std::chrono::milliseconds(delayMs));
      }));

  auto payloadLength = 25;
  auto iobuf = folly::IOBuf::copyBuffer(std::string(payloadLength, 'x'));
  auto stream = testClient.sync_echoIOBufAsByteStream(*iobuf, 5);
  auto returnPayload = 0;
  std::move(stream).subscribeInline([&](auto&& val) {
    if (val.hasValue()) {
      returnPayload++;
      EXPECT_EQ(*val, 'x');
    }
  });
  EXPECT_EQ(returnPayload, payloadLength);
}

TEST_F(GuardedRequestChannelTest, streamErrorFromServer) {
  EXPECT_CALL(*handler, semifuture_echoIOBufAsByteStream(_, _))
      .WillOnce(
          Invoke([&](std::unique_ptr<folly::IOBuf> /*buf*/, int32_t delayMs) {
            auto [stream, publisher] = ServerStream<int8_t>::createPublisher();
            auto ew = folly::exception_wrapper{
                std::runtime_error("end stream immediately")};
            std::move(publisher).complete(ew);
            return folly::makeSemiFuture(std::move(stream))
                .delayed(std::chrono::milliseconds(delayMs));
          }));

  auto iobuf = folly::IOBuf::copyBuffer(std::string(1, 'x'));
  auto stream = testClient.sync_echoIOBufAsByteStream(*iobuf, 0);
  std::move(stream).subscribeInline([&](auto&& val) {
    if (val.hasValue()) {
      FAIL() << "No real payload should be received";
    }
    EXPECT_TRUE(val.hasException());
    auto exception = val.tryGetExceptionObject();
    EXPECT_THAT(exception->what(), HasSubstr("end stream immediately"));
    EXPECT_THAT(exception->what(), HasSubstr("std::runtime_error"));
  });
}

TEST_F(GuardedRequestChannelTest, createInteractionTest) {
  ScopedServerInterfaceThread runner{std::make_shared<CalculatorHandler>()};
  apache::thrift::Client<Calculator> client(
      GuardedRequestChannel<folly::Unit, folly::Unit>::newChannel(
          PooledRequestChannel::newChannel([&](folly::EventBase& evb) {
            return RocketClientChannel::newChannel(
                folly::AsyncSocket::UniquePtr(
                    new folly::AsyncSocket(&evb, runner.getAddress())));
          })));

  auto adder = client.createAddition();
}

TEST_F(GuardedRequestChannelTest, basicRequestResponseMethodCallInteraction) {
  ScopedServerInterfaceThread runner{std::make_shared<CalculatorHandler>()};
  apache::thrift::Client<Calculator> client(
      GuardedRequestChannel<folly::Unit, folly::Unit>::newChannel(
          PooledRequestChannel::newChannel([&](folly::EventBase& evb) {
            return RocketClientChannel::newChannel(
                folly::AsyncSocket::UniquePtr(
                    new folly::AsyncSocket(&evb, runner.getAddress())));
          })));

  auto adder = client.createAddition();
#if FOLLY_HAS_COROUTINES
  folly::coro::blockingWait([&]() -> folly::coro::Task<void> {
    co_await adder.co_accumulatePrimitive(1);
    co_await adder.semifuture_accumulatePrimitive(2);
    auto acc = co_await adder.co_getPrimitive();
    EXPECT_EQ(acc, 3);

    auto sum = co_await client.co_addPrimitive(20, 22);
    EXPECT_EQ(sum, 42);

    Point p;
    p.x() = 1;
    co_await adder.co_accumulatePoint(p);
    p.y() = 2;
    co_await adder.co_accumulatePoint(p);
    auto pacc = co_await adder.co_getPoint();
    EXPECT_EQ(*pacc.x(), 2);
    EXPECT_EQ(*pacc.y(), 2);
  }());
#endif
}

TEST_F(GuardedRequestChannelTest, interactionConstructorError) {
  struct BrokenCalculatorHandler : CalculatorHandler {
    std::unique_ptr<AdditionIf> createAddition() override {
      throw std::runtime_error("Plus key is broken");
    }
  };
  ScopedServerInterfaceThread runner{
      std::make_shared<BrokenCalculatorHandler>()};
  apache::thrift::Client<Calculator> client(
      GuardedRequestChannel<folly::Unit, folly::Unit>::newChannel(
          PooledRequestChannel::newChannel([&](folly::EventBase& evb) {
            return RocketClientChannel::newChannel(
                folly::AsyncSocket::UniquePtr(
                    new folly::AsyncSocket(&evb, runner.getAddress())));
          })));
  const char* kExpectedErr =
      "apache::thrift::TApplicationException:"
      " Interaction constructor failed with std::runtime_error: Plus key is broken";

  auto adder = client.createAddition();
#if FOLLY_HAS_COROUTINES
  folly::coro::blockingWait([&]() -> folly::coro::Task<void> {
    auto t = co_await folly::coro::co_awaitTry(adder.co_accumulatePrimitive(1));
    EXPECT_STREQ(t.exception().what().c_str(), kExpectedErr);
    auto t2 = co_await folly::coro::co_awaitTry(adder.co_getPrimitive());
    EXPECT_STREQ(t.exception().what().c_str(), kExpectedErr);
    auto sum = co_await client.co_addPrimitive(20, 22);
    EXPECT_EQ(sum, 42);
  }());
#endif
}

TEST_F(GuardedRequestChannelTest, interactionMethodException) {
  struct ExceptionCalculatorHandler
      : apache::thrift::ServiceHandler<Calculator> {
    struct AdditionHandler
        : apache::thrift::ServiceHandler<Calculator>::AdditionIf {
      int acc_{0};
#if FOLLY_HAS_COROUTINES
      folly::coro::Task<void> co_accumulatePrimitive(int32_t a) override {
        acc_ += a;
        co_yield folly::coro::co_error(
            std::runtime_error("Not Implemented Yet"));
      }
#endif
    };
    std::unique_ptr<AdditionIf> createAddition() override {
      return std::make_unique<AdditionHandler>();
    }
  };

  ScopedServerInterfaceThread runner{
      std::make_shared<ExceptionCalculatorHandler>()};
  apache::thrift::Client<Calculator> client(
      GuardedRequestChannel<folly::Unit, folly::Unit>::newChannel(
          PooledRequestChannel::newChannel([&](folly::EventBase& evb) {
            return RocketClientChannel::newChannel(
                folly::AsyncSocket::UniquePtr(
                    new folly::AsyncSocket(&evb, runner.getAddress())));
          })));
  const char* kExpectedErr =
      "apache::thrift::TApplicationException: std::runtime_error: Not Implemented Yet";

  auto adder = client.createAddition();
#if FOLLY_HAS_COROUTINES
  folly::coro::blockingWait([&]() -> folly::coro::Task<void> {
    auto t = co_await folly::coro::co_awaitTry(adder.co_accumulatePrimitive(1));
    EXPECT_STREQ(t.exception().what().c_str(), kExpectedErr);
  }());
#endif
}
