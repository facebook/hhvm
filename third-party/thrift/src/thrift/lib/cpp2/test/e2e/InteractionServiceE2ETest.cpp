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

#include <chrono>
#include <memory>
#include <thread>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <folly/coro/BlockingWait.h>
#include <folly/coro/Collect.h>
#include <folly/coro/FutureUtil.h>
#include <folly/coro/GtestHelpers.h>
#include <folly/coro/Sleep.h>
#include <folly/coro/Task.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/cpp2/server/ParallelConcurrencyController.h>
#include <thrift/lib/cpp2/server/RoundRobinRequestPile.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/test/e2e/E2ETestFixture.h>
#include <thrift/lib/cpp2/test/e2e/gen-cpp2/Calculator.h>
#include <thrift/lib/cpp2/test/e2e/gen-cpp2/Streamer.h>
#include <thrift/lib/cpp2/transport/core/ManagedConnectionIf.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace ::testing;
using namespace apache::thrift;
using namespace apache::thrift::detail::test;

struct SemiCalculatorHandler : ServiceHandler<Calculator> {
  struct SemiAdditionHandler : ServiceHandler<Calculator>::AdditionIf {
    int acc_{0};
    Point pacc_;

    void accumulatePrimitive(int32_t a) override { acc_ += a; }
    folly::SemiFuture<folly::Unit> semifuture_noop() override {
      return folly::makeSemiFuture();
    }
    folly::SemiFuture<folly::Unit> semifuture_accumulatePoint(
        std::unique_ptr<Point> a) override {
      *pacc_.x() += *a->x();
      *pacc_.y() += *a->y();
      return folly::makeSemiFuture();
    }
    int32_t getPrimitive() override { return acc_; }
    folly::SemiFuture<std::unique_ptr<Point>> semifuture_getPoint() override {
      return folly::copy_to_unique_ptr(pacc_);
    }
  };

  std::unique_ptr<AdditionIf> createAddition() override {
    return std::make_unique<SemiAdditionHandler>();
  }

  folly::SemiFuture<int32_t> semifuture_addPrimitive(
      int32_t a, int32_t b) override {
    return a + b;
  }

  TileAndResponse<AdditionIf, int> initializedAddition(int x) override {
    auto handler = std::make_unique<SemiAdditionHandler>();
    handler->acc_ = x;
    return {std::move(handler), x};
  }

  struct FastAdditionHandler : ServiceHandler<Calculator>::AdditionFastIf {
    int acc_{0};
    void async_eb_accumulatePrimitive(
        HandlerCallbackPtr<void> cb, int32_t a) override {
      cb->getEventBase()->checkIsInEventBaseThread();
      acc_ += a;
      cb->exception(std::runtime_error("Not Implemented Yet"));
    }
    void async_eb_getPrimitive(HandlerCallbackPtr<int32_t> cb) override {
      cb->getEventBase()->checkIsInEventBaseThread();
      cb->result(acc_);
    }
  };

  TileAndResponse<AdditionFastIf, void> fastAddition() override {
    CHECK(!getEventBase()->isInEventBaseThread());
    return {std::make_unique<FastAdditionHandler>()};
  }

  void async_eb_veryFastAddition(
      HandlerCallbackPtr<TileAndResponse<AdditionFastIf, void>> cb) override {
    cb->getEventBase()->checkIsInEventBaseThread();
    cb->result({std::make_unique<FastAdditionHandler>()});
  }
};

struct CalculatorHandler : ServiceHandler<Calculator> {
  struct AdditionHandler : ServiceHandler<Calculator>::AdditionIf {
    int acc_{0};
    Point pacc_;

    folly::coro::Task<void> co_accumulatePrimitive(int32_t a) override {
      acc_ += a;
      co_return;
    }
    folly::coro::Task<void> co_noop() override { co_return; }
    folly::coro::Task<void> co_accumulatePoint(
        std::unique_ptr<Point> a) override {
      *pacc_.x() += *a->x();
      *pacc_.y() += *a->y();
      co_return;
    }
    folly::coro::Task<int32_t> co_getPrimitive() override { co_return acc_; }
    folly::coro::Task<std::unique_ptr<Point>> co_getPoint() override {
      co_return folly::copy_to_unique_ptr(pacc_);
    }
  };

  std::unique_ptr<AdditionIf> createAddition() override {
    return std::make_unique<AdditionHandler>();
  }

  folly::SemiFuture<int32_t> semifuture_addPrimitive(
      int32_t a, int32_t b) override {
    return a + b;
  }
};

namespace apache::thrift {
namespace {

class InteractionServiceE2ETest : public test::E2ETestFixture {};

// --- E2ETestFixture-based tests ---

TEST_F(InteractionServiceE2ETest, PrioritizedInteractionRequest) {
  gflags::FlagSaver flag_saver;
  FLAGS_thrift_experimental_use_resource_pools = true;
  struct BlockingCalculatorHandler : public SemiCalculatorHandler {
    folly::Baton<> blockBlocker, blockNormal, startedBlocker;

    folly::SemiFuture<int32_t> semifuture_addPrimitive(
        int32_t a, int32_t b) override {
      if (a == 0 && b == 0) {
        startedBlocker.post();
        EXPECT_TRUE(blockBlocker.try_wait_for(std::chrono::seconds(5)));
      } else {
        EXPECT_TRUE(blockNormal.try_wait_for(std::chrono::seconds(5)));
        blockNormal.reset();
      }
      return a + b;
    }
  };

  auto handler = std::make_shared<BlockingCalculatorHandler>();
  ScopedServerInterfaceThread runner{
      handler, [](ThriftServer& server) { server.setNumCPUWorkerThreads(1); }};

  if (runner.getThriftServer().resourcePoolSet().empty()) {
    return;
  }
  auto client = runner.newClient<Client<Calculator>>(
      nullptr, RocketClientChannel::newChannel);

  RpcOptions opts;
  auto [adder, sf1] = client->eager_semifuture_initializedAddition(opts, 42);
  EXPECT_EQ(std::move(sf1).get(), 42);

  auto blocker = client->semifuture_addPrimitive(0, 0);
  handler->startedBlocker.wait();

  auto sf2 = adder.semifuture_getPrimitive();
  auto normal = client->semifuture_addPrimitive(1, 2);

  auto start = std::chrono::steady_clock::now();
  while (runner.getThriftServer().resourcePoolSet().numQueued() != 2) {
    /* sleep override */
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    if (std::chrono::steady_clock::now() - start > std::chrono::seconds(5)) {
      FAIL() << "Timed out waiting for requests to be queued";
    }
  }

  handler->blockBlocker.post();
  EXPECT_EQ(std::move(blocker).get(), 0);

  EXPECT_EQ(std::move(sf2).get(), 42);

  handler->blockNormal.post();
  EXPECT_EQ(std::move(normal).get(), 3);
}

TEST_F(InteractionServiceE2ETest, TerminateUsed) {
  testConfig({std::make_shared<SemiCalculatorHandler>()});
  auto& runner = getThriftServer();
  folly::EventBase eb;
  Client<Calculator> client(
      RocketClientChannel::newChannel(
          folly::AsyncSocket::UniquePtr(
              new folly::AsyncSocket(&eb, runner.getAddress()))));

  auto adder = client.createAddition();
  adder.semifuture_getPrimitive().via(&eb).getVia(&eb);
}

TEST_F(InteractionServiceE2ETest, TerminateActive) {
  testConfig({std::make_shared<SemiCalculatorHandler>()});
  auto& runner = getThriftServer();
  folly::EventBase eb;
  Client<Calculator> client(
      RocketClientChannel::newChannel(
          folly::AsyncSocket::UniquePtr(
              new folly::AsyncSocket(&eb, runner.getAddress()))));

  auto adder = client.createAddition();
  adder.semifuture_noop().via(&eb).getVia(&eb);
}

TEST_F(InteractionServiceE2ETest, TerminateUnused) {
  testConfig({std::make_shared<SemiCalculatorHandler>()});
  auto& runner = getThriftServer();
  folly::EventBase eb;
  Client<Calculator> client(
      RocketClientChannel::newChannel(
          folly::AsyncSocket::UniquePtr(
              new folly::AsyncSocket(&eb, runner.getAddress()))));

  client.sync_addPrimitive(0, 0);
  auto adder = client.createAddition();
}

TEST_F(InteractionServiceE2ETest, TerminateWithoutSetup) {
  testConfig({std::make_shared<SemiCalculatorHandler>()});
  auto& runner = getThriftServer();
  folly::EventBase eb;
  Client<Calculator> client(
      RocketClientChannel::newChannel(
          folly::AsyncSocket::UniquePtr(
              new folly::AsyncSocket(&eb, runner.getAddress()))));

  auto adder = client.createAddition();
}

TEST_F(InteractionServiceE2ETest, TerminateUsedPRC) {
  testConfig({std::make_shared<SemiCalculatorHandler>()});
  auto client = makeClient<Calculator>();

  auto adder = client->createAddition();
  adder.semifuture_noop().get();
}

TEST_F(InteractionServiceE2ETest, TerminateUnusedPRC) {
  testConfig({std::make_shared<SemiCalculatorHandler>()});
  auto client = makeClient<Calculator>();

  client->sync_addPrimitive(0, 0);
  auto adder = client->createAddition();
}

TEST_F(InteractionServiceE2ETest, TerminateWithoutSetupPRC) {
  testConfig({std::make_shared<SemiCalculatorHandler>()});
  auto client = makeClient<Calculator>();

  auto adder = client->createAddition();
}

TEST_F(InteractionServiceE2ETest, TerminateStressPRC) {
  testConfig({std::make_shared<SemiCalculatorHandler>()});
  auto client = makeClient<Calculator>();

  for (int i = 0; i < 100; i++) {
    auto adder = client->createAddition();
    adder.semifuture_noop();
  }
}

TEST_F(InteractionServiceE2ETest, IsDetachable) {
  testConfig({std::make_shared<SemiCalculatorHandler>()});
  auto& runner = getThriftServer();
  folly::EventBase eb;
  Client<Calculator> client(
      RocketClientChannel::newChannel(
          folly::AsyncSocket::UniquePtr(
              new folly::AsyncSocket(&eb, runner.getAddress()))));
  auto channel = static_cast<ClientChannel*>(client.getChannel());

  bool detached = false;
  channel->setOnDetachable([&] { detached = true; });
  EXPECT_TRUE(channel->isDetachable());

  {
    auto adder = client.createAddition();
    EXPECT_FALSE(channel->isDetachable());

    adder.semifuture_noop().via(&eb).getVia(&eb);
    EXPECT_FALSE(channel->isDetachable());
  }

  client.sync_addPrimitive(0, 0);
  EXPECT_TRUE(channel->isDetachable());
  EXPECT_TRUE(detached);
}

TEST_F(InteractionServiceE2ETest, QueueTimeout) {
  struct SlowCalculatorHandler : ServiceHandler<Calculator> {
    struct SemiAdditionHandler : ServiceHandler<Calculator>::AdditionIf {
      folly::SemiFuture<int32_t> semifuture_getPrimitive() override {
        /* sleep override: testing timeout */
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return 0;
      }
    };

    std::unique_ptr<AdditionIf> createAddition() override {
      return std::make_unique<SemiAdditionHandler>();
    }
  };

  ScopedServerInterfaceThread runner{std::make_shared<SlowCalculatorHandler>()};
  runner.getThriftServer().setQueueTimeout(std::chrono::milliseconds(50));
  auto client = runner.newClient<Client<Calculator>>(
      nullptr, RocketClientChannel::newChannel);

  auto adder = client->createAddition();
  adder.sync_getPrimitive();

  auto f1 = adder.semifuture_getPrimitive(),
       f2 = adder.semifuture_getPrimitive();
  EXPECT_FALSE(std::move(f1).getTry().hasException());
  EXPECT_TRUE(std::move(f2).getTry().hasException());
}

CO_TEST_F(InteractionServiceE2ETest, BasicCoroutine) {
  testConfig({std::make_shared<CalculatorHandler>()});
  auto client = makeClient<Calculator>();

  auto adder = client->createAddition();
  co_await adder.co_accumulatePrimitive(1);
  co_await adder.semifuture_accumulatePrimitive(2);
  co_await adder.co_noop();
  auto acc = co_await adder.co_getPrimitive();
  EXPECT_EQ(acc, 3);

  auto sum = co_await client->co_addPrimitive(20, 22);
  EXPECT_EQ(sum, 42);

  Point p;
  p.x() = 1;
  co_await adder.co_accumulatePoint(p);
  p.y() = 2;
  co_await adder.co_accumulatePoint(p);
  auto pacc = co_await adder.co_getPoint();
  EXPECT_EQ(*pacc.x(), 2);
  EXPECT_EQ(*pacc.y(), 2);
}

CO_TEST_F(InteractionServiceE2ETest, RpcOptions) {
  testConfig({std::make_shared<CalculatorHandler>()});
  auto client = makeClient<Calculator>();

  RpcOptions rpcOptions;
  auto adder = client->createAddition();
  co_await adder.co_accumulatePrimitive(rpcOptions, 1);
  co_await adder.semifuture_accumulatePrimitive(rpcOptions, 2);
  co_await adder.co_noop(rpcOptions);
  auto acc = co_await adder.co_getPrimitive(rpcOptions);
  EXPECT_EQ(acc, 3);

  auto sum = co_await client->co_addPrimitive(rpcOptions, 20, 22);
  EXPECT_EQ(sum, 42);

  Point p;
  p.x() = 1;
  co_await adder.co_accumulatePoint(rpcOptions, p);
  p.y() = 2;
  co_await adder.co_accumulatePoint(rpcOptions, p);
  auto pacc = co_await adder.co_getPoint(rpcOptions);
  EXPECT_EQ(*pacc.x(), 2);
  EXPECT_EQ(*pacc.y(), 2);
}

TEST_F(InteractionServiceE2ETest, BasicSemiFuture) {
  testConfig({std::make_shared<SemiCalculatorHandler>()});
  auto client = makeClient<Calculator>();

  auto adder = client->createAddition();
  adder.semifuture_accumulatePrimitive(1).get();
  adder.semifuture_accumulatePrimitive(2).get();
  adder.semifuture_noop().get();
  auto acc = adder.semifuture_getPrimitive().get();
  EXPECT_EQ(acc, 3);

  auto sum = client->semifuture_addPrimitive(20, 22).get();
  EXPECT_EQ(sum, 42);

  Point p;
  p.x() = 1;
  adder.semifuture_accumulatePoint(p).get();
  p.y() = 2;
  adder.semifuture_accumulatePoint(p).get();
  auto pacc = adder.semifuture_getPoint().get();
  EXPECT_EQ(*pacc.x(), 2);
  EXPECT_EQ(*pacc.y(), 2);
}

TEST_F(InteractionServiceE2ETest, BasicSync) {
  testConfig({std::make_shared<SemiCalculatorHandler>()});
  auto client = makeClient<Calculator>();

  auto adder = client->createAddition();
  adder.sync_accumulatePrimitive(1);
  adder.sync_accumulatePrimitive(2);
  adder.sync_noop();
  auto acc = adder.sync_getPrimitive();
  EXPECT_EQ(acc, 3);

  auto sum = client->sync_addPrimitive(20, 22);
  EXPECT_EQ(sum, 42);

  Point p;
  p.x() = 1;
  adder.sync_accumulatePoint(p);
  p.y() = 2;
  adder.sync_accumulatePoint(p);
  Point pacc;
  adder.sync_getPoint(pacc);
  EXPECT_EQ(*pacc.x(), 2);
  EXPECT_EQ(*pacc.y(), 2);
}

CO_TEST_F(InteractionServiceE2ETest, ConstructorError) {
  struct BrokenCalculatorHandler : CalculatorHandler {
    std::unique_ptr<AdditionIf> createAddition() override {
      throw std::runtime_error("Plus key is broken");
    }
  };
  testConfig({std::make_shared<BrokenCalculatorHandler>()});
  auto client = makeClient<Calculator>();

  const char* kExpectedErr =
      "apache::thrift::TApplicationException:"
      " Interaction constructor failed with std::runtime_error: Plus key is broken";

  auto adder = client->createAddition();
  auto t = co_await folly::coro::co_awaitTry(adder.co_accumulatePrimitive(1));
  EXPECT_STREQ(t.exception().what().c_str(), kExpectedErr);
  auto t2 = co_await folly::coro::co_awaitTry(adder.co_getPrimitive());
  EXPECT_STREQ(t.exception().what().c_str(), kExpectedErr);
  co_await adder.co_noop();

  auto sum = co_await client->co_addPrimitive(20, 22);
  EXPECT_EQ(sum, 42);
}

CO_TEST_F(InteractionServiceE2ETest, MethodException) {
  struct ExceptionCalculatorHandler : ServiceHandler<Calculator> {
    struct AdditionHandler : ServiceHandler<Calculator>::AdditionIf {
      int acc_{0};
      folly::coro::Task<void> co_accumulatePrimitive(int32_t a) override {
        acc_ += a;
        co_yield folly::coro::co_error(
            std::runtime_error("Not Implemented Yet"));
      }
    };
    std::unique_ptr<AdditionIf> createAddition() override {
      return std::make_unique<AdditionHandler>();
    }
  };

  testConfig({std::make_shared<ExceptionCalculatorHandler>()});
  auto client = makeClient<Calculator>();

  const char* kExpectedErr =
      "apache::thrift::TApplicationException: std::runtime_error: Not Implemented Yet";

  auto adder = client->createAddition();
  auto t = co_await folly::coro::co_awaitTry(adder.co_accumulatePrimitive(1));
  EXPECT_STREQ(t.exception().what().c_str(), kExpectedErr);
}

CO_TEST_F(InteractionServiceE2ETest, SlowConstructor) {
  struct SlowCalculatorHandler : CalculatorHandler {
    std::unique_ptr<AdditionIf> createAddition() override {
      b.wait();
      return std::make_unique<AdditionHandler>();
    }

    folly::Baton<> b;
  };
  auto handler = std::make_shared<SlowCalculatorHandler>();
  ScopedServerInterfaceThread runner{handler};
  auto client = runner.newClient<Client<Calculator>>(
      nullptr, RocketClientChannel::newChannel);

  auto adder = client->createAddition();
  folly::EventBase eb;
  co_withExecutor(&eb, adder.co_noop()).start();
  co_withExecutor(&eb, adder.co_accumulatePrimitive(1)).start();
  folly::via(&eb, [&] { handler->b.post(); }).getVia(&eb);
  auto acc = co_await adder.co_getPrimitive();
  EXPECT_EQ(acc, 1);
}

TEST_F(InteractionServiceE2ETest, FastTermination) {
  struct SlowCalculatorHandler : CalculatorHandler {
    struct SlowAdditionHandler : AdditionHandler {
      folly::coro::Baton &baton_, &destroyed_;
      SlowAdditionHandler(
          folly::coro::Baton& baton, folly::coro::Baton& destroyed)
          : baton_(baton), destroyed_(destroyed) {}
      ~SlowAdditionHandler() override { destroyed_.post(); }

      folly::coro::Task<int32_t> co_getPrimitive() override {
        co_await baton_;
        co_return acc_;
      }
      folly::coro::Task<void> co_noop() override {
        co_await baton_;
        co_return;
      }
    };

    std::unique_ptr<AdditionIf> createAddition() override {
      return std::make_unique<SlowAdditionHandler>(baton, destroyed);
    }

    folly::coro::Baton baton, destroyed;
  };
  auto handler = std::make_shared<SlowCalculatorHandler>();
  ScopedServerInterfaceThread runner{handler};
  auto client = runner.newClient<Client<Calculator>>(
      nullptr, RocketClientChannel::newChannel);

  auto adder = folly::copy_to_unique_ptr(client->createAddition());
  folly::EventBase eb;
  auto semi = co_withExecutor(&eb, adder->co_getPrimitive()).start();
  adder->co_accumulatePrimitive(1).semi().via(&eb).getVia(&eb);
  adder->co_noop().semi().via(&eb).getVia(&eb);
  adder.reset();
  EXPECT_FALSE(handler->destroyed.ready());
  handler->baton.post();
  EXPECT_EQ(std::move(semi).via(&eb).getVia(&eb), 1);
  folly::coro::blockingWait(handler->destroyed);
}

TEST_F(InteractionServiceE2ETest, ConstructorExceptionPropagated) {
  struct SlowCalculatorHandler : CalculatorHandler {
    std::unique_ptr<AdditionIf> createAddition() override {
      b.wait();
      throw std::runtime_error("Custom exception");
    }

    folly::Baton<> b;
  };
  auto handler = std::make_shared<SlowCalculatorHandler>();
  ScopedServerInterfaceThread runner{handler};
  auto client = runner.newClient<Client<Calculator>>(
      nullptr, RocketClientChannel::newChannel);

  auto adder = client->createAddition();
  auto fut1 = adder.semifuture_accumulatePrimitive(1);
  auto fut2 = adder.semifuture_accumulatePrimitive(1);
  handler->b.post();
  auto fut3 = adder.semifuture_accumulatePrimitive(1);

  EXPECT_THAT(
      std::move(fut1).getTry().exception().what().toStdString(),
      HasSubstr("Custom exception"));
  EXPECT_THAT(
      std::move(fut2).getTry().exception().what().toStdString(),
      HasSubstr("Custom exception"));
  EXPECT_TRUE(std::move(fut3).getTry().hasException());
}

TEST_F(InteractionServiceE2ETest, SerialInteraction) {
  struct SerialCalculatorHandler : CalculatorHandler {
    struct SerialAdditionHandler
        : ServiceHandler<Calculator>::SerialAdditionIf {
      int acc_{0};
      folly::coro::Baton &baton2_, &baton3_;
      SerialAdditionHandler(
          folly::coro::Baton& baton2, folly::coro::Baton& baton3)
          : baton2_(baton2), baton3_(baton3) {}

      folly::coro::Task<void> co_accumulatePrimitive(int a) override {
        co_await baton2_;
        acc_ += a;
      }
      folly::coro::Task<int32_t> co_getPrimitive() override {
        co_await baton3_;
        co_return acc_;
      }
      folly::coro::Task<ServerStream<int32_t>> co_waitForCancel() override {
        co_return []() -> folly::coro::AsyncGenerator<int32_t&&> {
          folly::coro::Baton b;
          folly::CancellationCallback cb{
              co_await folly::coro::co_current_cancellation_token,
              [&] { b.post(); }};
          co_await b;
        }();
      }
    };

    std::unique_ptr<SerialAdditionIf> createSerialAddition() override {
      folly::coro::blockingWait(baton1);
      return std::make_unique<SerialAdditionHandler>(baton2, baton3);
    }

    folly::coro::Baton baton1, baton2, baton3;
  };
  auto handler = std::make_shared<SerialCalculatorHandler>();
  ScopedServerInterfaceThread runner{handler};
  auto client = runner.newClient<Client<Calculator>>(
      nullptr, RocketClientChannel::newChannel);

  auto adder = client->createSerialAddition();
  folly::EventBase eb;
  auto stream = adder.semifuture_waitForCancel();
  auto accSemi = co_withExecutor(&eb, adder.co_accumulatePrimitive(1)).start();
  auto getSemi = co_withExecutor(&eb, adder.co_getPrimitive()).start();

  auto semi = client->semifuture_addPrimitive(0, 0);
  EXPECT_FALSE(accSemi.isReady());
  EXPECT_FALSE(getSemi.isReady());
  handler->baton1.post();
  std::move(semi).via(&eb).getVia(&eb);
  client->co_addPrimitive(0, 0).semi().via(&eb).getVia(&eb);
  EXPECT_FALSE(accSemi.isReady());
  EXPECT_FALSE(getSemi.isReady());

  handler->baton2.post();
  client->co_addPrimitive(0, 0).semi().via(&eb).getVia(&eb);
  std::move(accSemi).via(&eb).getVia(&eb);
  EXPECT_FALSE(getSemi.isReady());

  accSemi = co_withExecutor(&eb, adder.co_accumulatePrimitive(1)).start();
  client->co_addPrimitive(0, 0).semi().via(&eb).getVia(&eb);
  EXPECT_FALSE(accSemi.isReady());
  EXPECT_FALSE(getSemi.isReady());

  handler->baton3.post();
  client->co_addPrimitive(0, 0).semi().via(&eb).getVia(&eb);
  std::move(accSemi).via(&eb).getVia(&eb);

  EXPECT_EQ(std::move(getSemi).via(&eb).getVia(&eb), 1);
}

TEST_F(InteractionServiceE2ETest, StreamExtendsInteractionLifetime) {
  struct StreamingHandler : ServiceHandler<Streamer> {
    StreamingHandler()
        : publisherPair(ServerStream<int>::createPublisher([&] {
            streamBaton.post();
            EXPECT_FALSE(
                tileBaton2.try_wait_for(std::chrono::milliseconds(100)));
          })) {}
    struct StreamTile : ServiceHandler<Streamer>::StreamingIf {
      folly::coro::Task<ServerStream<int>> co_generatorStream() override {
        co_return folly::coro::co_invoke(
            [&]() -> folly::coro::AsyncGenerator<int&&> {
              SCOPE_EXIT {
                streamBaton.post();
                EXPECT_FALSE(
                    tileBaton2.try_wait_for(std::chrono::milliseconds(100)));
              };
              while (true) {
                co_yield 0;
              }
            });
      }

      folly::coro::Task<ServerStream<int>> co_publisherStream() override {
        co_return std::move(publisherStreamRef);
      }

      folly::coro::Task<SinkConsumer<int32_t, int8_t>> co__sink() override {
        SinkConsumer<int32_t, int8_t> sink;
        sink.consumer = [&](auto gen) -> folly::coro::Task<int8_t> {
          SCOPE_EXIT {
            streamBaton.post();
            EXPECT_FALSE(
                tileBaton2.try_wait_for(std::chrono::milliseconds(100)));
          };
          co_await gen.next();
          co_return 0;
        };
        sink.bufferSize = 5;
        sink.setChunkTimeout(std::chrono::milliseconds(500));
        co_return sink;
      }

      StreamTile(
          folly::Baton<>& tileBaton_,
          folly::Baton<>& tileBaton2_,
          folly::Baton<>& streamBaton_,
          ServerStream<int>& publisherStream)
          : tileBaton(tileBaton_),
            tileBaton2(tileBaton2_),
            streamBaton(streamBaton_),
            publisherStreamRef(publisherStream) {}

      ~StreamTile() override {
        tileBaton.post();
        tileBaton2.post();
        EXPECT_TRUE(streamBaton.try_wait_for(std::chrono::milliseconds(100)));
      }

      folly::Baton<>&tileBaton, &tileBaton2, &streamBaton;
      ServerStream<int>& publisherStreamRef;
    };

    std::unique_ptr<StreamingIf> createStreaming() override {
      return std::make_unique<StreamTile>(
          tileBaton, tileBaton2, streamBaton, publisherPair.first);
    }

    folly::Baton<> tileBaton, tileBaton2, streamBaton;
    std::pair<ServerStream<int>, ServerStreamPublisher<int>> publisherPair;
  };

  auto handler = std::make_shared<StreamingHandler>();
  ScopedServerInterfaceThread runner{handler};
  runner.getThriftServer().getThreadManager_deprecated()->addWorker();
  auto client = runner.newClient<StreamerAsyncClient>(nullptr, [](auto socket) {
    return RocketClientChannel::newChannel(std::move(socket));
  });

  // Generator test
  {
    auto handle = folly::copy_to_unique_ptr(client->createStreaming());
    auto stream = handle->semifuture_generatorStream().get();
    EXPECT_FALSE(
        handler->tileBaton.try_wait_for(std::chrono::milliseconds(100)));
    handler->tileBaton.reset();
    handle.reset();
    EXPECT_FALSE(
        handler->tileBaton.try_wait_for(std::chrono::milliseconds(100)));
    handler->tileBaton.reset();
  }
  EXPECT_TRUE(handler->tileBaton.try_wait_for(std::chrono::milliseconds(300)));
  handler->tileBaton.reset();
  handler->tileBaton2.reset();
  handler->streamBaton.reset();

  // Publisher test
  {
    auto handle = folly::copy_to_unique_ptr(client->createStreaming());
    auto stream = handle->semifuture_publisherStream().get();
    EXPECT_FALSE(
        handler->tileBaton.try_wait_for(std::chrono::milliseconds(100)));
    handler->tileBaton.reset();
    handle.reset();
    EXPECT_FALSE(
        handler->tileBaton.try_wait_for(std::chrono::milliseconds(100)));
    handler->tileBaton.reset();
  }
  std::move(handler->publisherPair.second).complete();
  EXPECT_TRUE(handler->tileBaton.try_wait_for(std::chrono::milliseconds(300)));
  handler->tileBaton.reset();
  handler->tileBaton2.reset();
  handler->streamBaton.reset();

  // Sink test
  {
    auto handle = folly::copy_to_unique_ptr(client->createStreaming());
    auto sink = handle->co__sink().semi().get();
    EXPECT_FALSE(
        handler->tileBaton.try_wait_for(std::chrono::milliseconds(100)));
    handler->tileBaton.reset();
    handle.reset();
    EXPECT_FALSE(
        handler->tileBaton.try_wait_for(std::chrono::milliseconds(100)));
    handler->tileBaton.reset();
  }
  EXPECT_TRUE(handler->tileBaton.try_wait_for(std::chrono::milliseconds(300)));
  handler->tileBaton.reset();
  handler->tileBaton2.reset();
  handler->streamBaton.reset();
}

CO_TEST_F(InteractionServiceE2ETest, ShutdownDuringStreamTeardown) {
  struct StreamingHandler : ServiceHandler<Streamer> {
    struct StreamTile : ServiceHandler<Streamer>::StreamingIf {
      folly::coro::Task<ServerStream<int>> co_generatorStream() override {
        co_return folly::coro::co_invoke(
            [&]() -> folly::coro::AsyncGenerator<int&&> {
              while (true) {
                co_yield 0;
                co_await folly::coro::sleep(std::chrono::milliseconds(100));
              }
            });
      }
    };

    std::unique_ptr<StreamingIf> createStreaming() override {
      return std::make_unique<StreamTile>();
    }
  };

  auto handler = std::make_shared<StreamingHandler>();
  ScopedServerInterfaceThread runner{handler};
  auto client = runner.newClient<StreamerAsyncClient>(nullptr, [](auto socket) {
    return RocketClientChannel::newChannel(std::move(socket));
  });

  co_await client->createStreaming().co_generatorStream();
}

CO_TEST_F(InteractionServiceE2ETest, BasicEB) {
  struct ExceptionCalculatorHandler : ServiceHandler<Calculator> {
    struct AdditionHandler : ServiceHandler<Calculator>::AdditionFastIf {
      int acc_{0};
      void async_eb_accumulatePrimitive(
          HandlerCallbackPtr<void> cb, int32_t a) override {
        acc_ += a;
        cb->exception(std::runtime_error("Not Implemented Yet"));
      }
      void async_eb_getPrimitive(HandlerCallbackPtr<int32_t> cb) override {
        cb->result(acc_);
      }
    };
    std::unique_ptr<AdditionFastIf> createAdditionFast() override {
      return std::make_unique<AdditionHandler>();
    }
  };

  testConfig({std::make_shared<ExceptionCalculatorHandler>()});
  auto client = makeClient<Calculator>();

  auto adder = client->createAdditionFast();
  auto [r1, r2] = co_await folly::coro::collectAllTry(
      adder.co_accumulatePrimitive(1), adder.co_getPrimitive());
  EXPECT_TRUE(r1.hasException());
  EXPECT_EQ(*r2, 1);
}

CO_TEST_F(InteractionServiceE2ETest, ErrorEB) {
  struct ExceptionCalculatorHandler : ServiceHandler<Calculator> {
    std::unique_ptr<AdditionFastIf> createAdditionFast() override {
      throw std::runtime_error("Unimplemented");
    }
  };

  testConfig({std::make_shared<ExceptionCalculatorHandler>()});
  auto client = makeClient<Calculator>();

  auto adder = client->createAdditionFast();
  auto [r1, r2] = co_await folly::coro::collectAllTry(
      adder.co_accumulatePrimitive(1), adder.co_getPrimitive());
  EXPECT_TRUE(r1.hasException());
  EXPECT_TRUE(r2.hasException());
}

TEST_F(InteractionServiceE2ETest, ClientCrashDuringInteraction) {
  struct SlowCalculatorHandler : CalculatorHandler {
    struct SlowAdditionHandler : AdditionHandler {
      folly::coro::Baton &baton_, &destroyed_;
      SlowAdditionHandler(
          folly::coro::Baton& baton, folly::coro::Baton& destroyed)
          : baton_(baton), destroyed_(destroyed) {}
      ~SlowAdditionHandler() override { destroyed_.post(); }

      folly::coro::Task<void> co_noop() override {
        co_await baton_;
        co_return;
      }
    };

    std::unique_ptr<AdditionIf> createAddition() override {
      return std::make_unique<SlowAdditionHandler>(baton, destroyed);
    }

    folly::coro::Baton baton, destroyed;
  };
  auto handler = std::make_shared<SlowCalculatorHandler>();
  ScopedServerInterfaceThread runner{handler};
  folly::EventBase eb;
  folly::AsyncSocket* sock = new folly::AsyncSocket(&eb, runner.getAddress());
  Client<Calculator> client(
      RocketClientChannel::newChannel(folly::AsyncSocket::UniquePtr(sock)));

  auto adder = client.createAddition();
  auto fut = adder.co_noop().semi().via(&eb);
  adder.co_getPrimitive().semi().via(&eb).getVia(&eb);
  sock->closeNow();
  handler->baton.post();
  fut.waitVia(&eb);
  folly::coro::blockingWait(handler->destroyed);
}

TEST_F(InteractionServiceE2ETest, ClientCrashDuringConstructor) {
  struct SlowCalculatorHandler : CalculatorHandler {
    struct SlowAdditionHandler : AdditionHandler {
      folly::coro::Baton &baton_, &destroyed_;
      bool& executed_;
      SlowAdditionHandler(
          folly::coro::Baton& baton,
          folly::coro::Baton& destroyed,
          bool& executed)
          : baton_(baton), destroyed_(destroyed), executed_(executed) {}
      ~SlowAdditionHandler() override { destroyed_.post(); }

      folly::coro::Task<void> co_noop() override {
        executed_ = true;
        co_return;
      }
    };

    std::unique_ptr<AdditionIf> createAddition() override {
      folly::coro::blockingWait(baton);
      return std::make_unique<SlowAdditionHandler>(baton, destroyed, executed);
    }

    folly::coro::Baton baton, destroyed;
    bool executed = false;
  };
  auto handler = std::make_shared<SlowCalculatorHandler>();
  ScopedServerInterfaceThread runner{handler};
  runner.getThriftServer().getThreadManager_deprecated()->addWorker();
  folly::EventBase eb;
  folly::AsyncSocket* sock = new folly::AsyncSocket(&eb, runner.getAddress());
  Client<Calculator> client(
      RocketClientChannel::newChannel(folly::AsyncSocket::UniquePtr(sock)));

  auto adder = client.createAddition();
  auto fut = adder.co_noop().semi().via(&eb);
  client.co_addPrimitive(0, 0).semi().via(&eb).getVia(&eb);
  sock->closeNow();
  handler->baton.post();
  fut.waitVia(&eb);
  folly::coro::blockingWait(handler->destroyed);
  EXPECT_TRUE(handler->executed);
}

TEST_F(InteractionServiceE2ETest, ReuseIdDuringConstructor) {
  struct SlowCalculatorHandler : CalculatorHandler {
    std::unique_ptr<AdditionIf> createAddition() override {
      if (first) {
        first = false;
        b1.post();
        b2.wait();
      }
      return std::make_unique<AdditionHandler>();
    }

    folly::Baton<> b1, b2;
    bool first = true;
  };
  auto handler = std::make_shared<SlowCalculatorHandler>();
  ScopedServerInterfaceThread runner{handler};
  folly::EventBase eb;
  Client<Calculator> client(
      RocketClientChannel::newChannel(
          folly::AsyncSocket::UniquePtr(
              new folly::AsyncSocket(&eb, runner.getAddress()))));

  {
    auto id = client.getChannel()->registerInteraction("Addition", 1);
    Client<Calculator>::Addition adder(
        client.getChannelShared(),
        std::move(id),
        nullptr /* clientInterceptors */);
    adder.semifuture_noop().via(&eb).getVia(&eb);
    handler->b1.wait();
  }
  eb.loopOnce();

  auto id = client.getChannel()->registerInteraction("Addition", 1);
  Client<Calculator>::Addition adder(
      client.getChannelShared(),
      std::move(id),
      nullptr /* clientInterceptors */);

  auto fut = adder.semifuture_accumulatePrimitive(1);
  handler->b2.post();
  std::move(fut).via(&eb).getVia(&eb);
}

TEST_F(InteractionServiceE2ETest, ResourcePoolsGeneratedFactory) {
  if (!FLAGS_thrift_experimental_use_resource_pools) {
    return;
  }
  auto handler = std::make_shared<SemiCalculatorHandler>();
  ScopedServerInterfaceThread server(handler);
  EXPECT_TRUE(server.getThriftServer().resourcePoolEnabled());
}

TEST_F(InteractionServiceE2ETest, ResourcePoolsCustomFactory) {
  if (!FLAGS_thrift_experimental_use_resource_pools) {
    return;
  }
  class CustomAsyncProcessorFactory : public AsyncProcessorFactory {
   public:
    std::unique_ptr<AsyncProcessor> getProcessor() override {
      return handler_.getProcessor();
    }
    std::vector<ServiceHandlerBase*> getServiceHandlers() override {
      return handler_.getServiceHandlers();
    }
    CreateMethodMetadataResult createMethodMetadata() override {
      return handler_.createMethodMetadata();
    }
    SemiCalculatorHandler handler_;
  };
  auto handler = std::make_shared<CustomAsyncProcessorFactory>();
  ScopedServerInterfaceThread server(handler);
  EXPECT_TRUE(server.getThriftServer().resourcePoolEnabled());
}

TEST_F(InteractionServiceE2ETest, InteractionSnapshots) {
  testConfig({std::make_shared<SemiCalculatorHandler>()});
  auto& thriftServer = getThriftServer();

  folly::EventBase eb;
  Client<Calculator> client(
      RocketClientChannel::newChannel(
          folly::AsyncSocket::UniquePtr(
              new folly::AsyncSocket(&eb, thriftServer.getAddress()))));

  auto adder = client.createAddition();
  adder.semifuture_accumulatePrimitive(5).via(&eb).getVia(&eb);

  auto snapshot = thriftServer.getServerSnapshot().get();

  bool foundInteraction = false;
  for (const auto& [addr, connSnapshot] : snapshot.connections) {
    if (!connSnapshot.interactions.empty()) {
      foundInteraction = true;
      EXPECT_EQ(connSnapshot.interactions.size(), 1);
      const auto& interaction = connSnapshot.interactions[0];
      EXPECT_GT(interaction.interactionId, 0);
      EXPECT_GT(interaction.creationTime.time_since_epoch().count(), 0);
      EXPECT_GE(interaction.refCount, 0);
    }
  }
  EXPECT_TRUE(foundInteraction)
      << "Expected to find at least one connection with an active interaction";
}

TEST_F(InteractionServiceE2ETest, InteractionSnapshotLastActivityTime) {
  testConfig({std::make_shared<SemiCalculatorHandler>()});
  auto& thriftServer = getThriftServer();

  folly::EventBase eb;
  Client<Calculator> client(
      RocketClientChannel::newChannel(
          folly::AsyncSocket::UniquePtr(
              new folly::AsyncSocket(&eb, thriftServer.getAddress()))));

  auto adder = client.createAddition();
  adder.semifuture_accumulatePrimitive(1).via(&eb).getVia(&eb);

  auto snapshot1 = thriftServer.getServerSnapshot().get();

  std::chrono::steady_clock::time_point firstLastActivity{};
  for (const auto& [addr, connSnapshot] : snapshot1.connections) {
    if (!connSnapshot.interactions.empty()) {
      const auto& interaction = connSnapshot.interactions[0];
      firstLastActivity = interaction.lastActivityTime;
      EXPECT_GT(firstLastActivity.time_since_epoch().count(), 0);
      EXPECT_GE(firstLastActivity, interaction.creationTime);
    }
  }

  /* sleep override */
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  adder.semifuture_accumulatePrimitive(2).via(&eb).getVia(&eb);

  auto snapshot2 = thriftServer.getServerSnapshot().get();
  for (const auto& [addr, connSnapshot] : snapshot2.connections) {
    if (!connSnapshot.interactions.empty()) {
      const auto& interaction = connSnapshot.interactions[0];
      EXPECT_GT(interaction.lastActivityTime, firstLastActivity)
          << "lastActivityTime should advance after a new call";
    }
  }
}

TEST_F(InteractionServiceE2ETest, TerminateInteraction) {
  testConfig({std::make_shared<SemiCalculatorHandler>()});
  auto& thriftServer = getThriftServer();

  folly::EventBase eb;
  Client<Calculator> client(
      RocketClientChannel::newChannel(
          folly::AsyncSocket::UniquePtr(
              new folly::AsyncSocket(&eb, thriftServer.getAddress()))));

  auto adder = client.createAddition();
  adder.semifuture_accumulatePrimitive(1).via(&eb).getVia(&eb);

  auto snapshot1 = thriftServer.getServerSnapshot().get();
  int64_t interactionId = 0;
  for (const auto& [addr, connSnapshot] : snapshot1.connections) {
    if (!connSnapshot.interactions.empty()) {
      interactionId = connSnapshot.interactions[0].interactionId;
      break;
    }
  }
  ASSERT_GT(interactionId, 0) << "Expected to find an active interaction";

  bool terminated = false;
  thriftServer.forEachWorker([&](wangle::Acceptor* acceptor) {
    auto* worker = dynamic_cast<Cpp2Worker*>(acceptor);
    if (!worker || terminated) {
      return;
    }
    worker->getEventBase()->runInEventBaseThreadAndWait([&] {
      if (auto* connectionManager = worker->getConnectionManager()) {
        connectionManager->forEachConnection(
            [&](wangle::ManagedConnection* wangleConn) {
              if (auto* managedConn =
                      dynamic_cast<ManagedConnectionIf*>(wangleConn)) {
                managedConn->terminateInteraction(interactionId);
                terminated = true;
              }
            });
      }
    });
  });
  ASSERT_TRUE(terminated) << "Failed to find connection to terminate on";

  auto snapshot2 = thriftServer.getServerSnapshot().get();
  for (const auto& [addr, connSnapshot] : snapshot2.connections) {
    for (const auto& interaction : connSnapshot.interactions) {
      EXPECT_NE(interaction.interactionId, interactionId)
          << "Interaction should have been terminated";
    }
  }
}

// --- New tests for coverage gaps ---

CO_TEST_F(InteractionServiceE2ETest, DeclaredException) {
  struct ExceptionCalculatorHandler : ServiceHandler<Calculator> {
    struct AdditionHandler : ServiceHandler<Calculator>::AdditionIf {
      folly::coro::Task<void> co_accumulatePrimitive(int32_t) override {
        co_return;
      }
      folly::coro::Task<int32_t> co_getPrimitive() override {
        co_yield folly::coro::co_error(AdditionException());
      }
    };
    std::unique_ptr<AdditionIf> createAddition() override {
      return std::make_unique<AdditionHandler>();
    }
  };

  testConfig({std::make_shared<ExceptionCalculatorHandler>()});
  auto client = makeClient<Calculator>();

  auto adder = client->createAddition();
  auto t = co_await folly::coro::co_awaitTry(adder.co_getPrimitive());
  EXPECT_TRUE(t.hasException<AdditionException>());
}

CO_TEST_F(InteractionServiceE2ETest, MultipleConcurrentInteractions) {
  testConfig({std::make_shared<CalculatorHandler>()});
  auto client = makeClient<Calculator>();

  auto adder1 = client->createAddition();
  auto adder2 = client->createAddition();

  co_await adder1.co_accumulatePrimitive(10);
  co_await adder2.co_accumulatePrimitive(20);
  co_await adder1.co_accumulatePrimitive(5);
  co_await adder2.co_accumulatePrimitive(3);

  EXPECT_EQ(co_await adder1.co_getPrimitive(), 15);
  EXPECT_EQ(co_await adder2.co_getPrimitive(), 23);
}

CO_TEST_F(InteractionServiceE2ETest, DifferentInteractionTypesConcurrently) {
  struct DualHandler : CalculatorHandler {
    struct FastHandler : ServiceHandler<Calculator>::AdditionFastIf {
      int acc_{0};
      void async_eb_accumulatePrimitive(
          HandlerCallbackPtr<void> cb, int32_t a) override {
        acc_ += a;
        cb->done();
      }
      void async_eb_getPrimitive(HandlerCallbackPtr<int32_t> cb) override {
        cb->result(acc_);
      }
    };

    std::unique_ptr<AdditionFastIf> createAdditionFast() override {
      return std::make_unique<FastHandler>();
    }
  };

  testConfig({std::make_shared<DualHandler>()});
  auto client = makeClient<Calculator>();

  auto addition = client->createAddition();
  auto additionFast = client->createAdditionFast();

  co_await addition.co_accumulatePrimitive(10);
  additionFast.sync_accumulatePrimitive(100);

  EXPECT_EQ(co_await addition.co_getPrimitive(), 10);
  EXPECT_EQ(additionFast.sync_getPrimitive(), 100);
}

// --- Standalone tests (using makeTestClient) ---

TEST(InteractionServiceStandalone, OnTermination) {
  struct TerminationHandler : ServiceHandler<Calculator> {
    folly::coro::Baton create;
    folly::coro::Baton terminate;
    folly::coro::Baton terminated;
    folly::coro::Baton destroyed;
    struct AdditionHandler : ServiceHandler<Calculator>::AdditionIf {
      TerminationHandler& handler;

      explicit AdditionHandler(TerminationHandler& parent) : handler(parent) {}
      ~AdditionHandler() override { handler.destroyed.post(); }

      folly::coro::Task<void> co_onTermination() override {
        co_await handler.terminate;
        handler.terminated.post();
      }

      folly::coro::Task<void> co_accumulatePrimitive(int32_t) override {
        co_return;
      }

      folly::coro::Task<int32_t> co_getPrimitive() override {
        co_await handler.terminated;
        co_return 42;
      }
    };

    struct AdditionFastHandler : ServiceHandler<Calculator>::AdditionFastIf {
      TerminationHandler& handler;

      explicit AdditionFastHandler(TerminationHandler& parent)
          : handler(parent) {}
      ~AdditionFastHandler() override { handler.destroyed.post(); }

      folly::coro::Task<void> co_onTermination() override {
        co_await handler.terminate;
        handler.terminated.post();
      }

      void async_eb_accumulatePrimitive(
          HandlerCallbackPtr<void> cb, int32_t) override {
        cb->done();
      }

      void async_eb_getPrimitive(HandlerCallbackPtr<int32_t> cb) override {
        folly::coro::toSemiFuture(std::ref(handler.terminated))
            .toUnsafeFuture()
            .thenValue([cb = std::move(cb)](auto&&) { cb->result(42); });
      }
    };

    std::unique_ptr<AdditionIf> createAddition() override {
      return std::make_unique<AdditionHandler>(*this);
    }
    std::unique_ptr<AdditionFastIf> createAdditionFast() override {
      return std::make_unique<AdditionFastHandler>(*this);
    }

    folly::SemiFuture<
        TileAndResponse<ServiceHandler<Calculator>::AdditionIf, void>>
    semifuture_newAddition() override {
      return folly::coro::toSemiFuture(std::ref(create))
          .deferValue([&](auto&&) {
            return TileAndResponse<
                ServiceHandler<Calculator>::AdditionIf,
                void>{std::make_unique<AdditionHandler>(*this)};
          });
    }

    folly::SemiFuture<int32_t> semifuture_addPrimitive(
        int32_t a, int32_t b) override {
      return a + b;
    }
  };

  auto handler = std::make_shared<TerminationHandler>();
  auto client = makeTestClient<Client<Calculator>>(handler);
  handler->terminate.post();
  auto checkAndReset = [](auto& baton) {
    EXPECT_TRUE(baton.ready());
    baton.reset();
  };

  // No active request
  {
    auto adder = client->createAddition();
    adder.sync_accumulatePrimitive(0);
  }
  client->sync_addPrimitive(0, 0);
  checkAndReset(handler->terminated);
  checkAndReset(handler->destroyed);

  // Active request
  auto fut = folly::SemiFuture<int>::makeEmpty();
  {
    auto adder = client->createAddition();
    fut = adder.semifuture_getPrimitive();
    client->sync_addPrimitive(0, 0);
  }
  client->sync_addPrimitive(0, 0);
  checkAndReset(handler->terminated);
  checkAndReset(handler->destroyed);
  std::move(fut).get();

  // Client crash
  {
    ScopedServerInterfaceThread runner{handler};
    folly::EventBase eb;
    folly::AsyncSocket* sock = new folly::AsyncSocket(&eb, runner.getAddress());
    Client<Calculator> localClient(
        RocketClientChannel::newChannel(folly::AsyncSocket::UniquePtr(sock)));

    auto adder = localClient.createAddition();
    adder.sync_accumulatePrimitive(0);
    sock->closeNow();
    folly::coro::blockingWait(handler->destroyed);
    checkAndReset(handler->destroyed);
    EXPECT_FALSE(handler->terminated.ready());
  }

  // Slow onTermination
  {
    handler->terminate.reset();
    auto adder = client->createAddition();
    adder.sync_accumulatePrimitive(0);
  }
  for (int i = 0; i < 10; i++) {
    client->sync_addPrimitive(0, 0);
    EXPECT_FALSE(handler->destroyed.ready());
  }
  handler->terminate.post();
  folly::coro::blockingWait(handler->destroyed);
  checkAndReset(handler->terminated);
  checkAndReset(handler->destroyed);

  // Slow factory
  RpcOptions opts;
  std::ignore = client->eager_semifuture_newAddition(opts);
  client->sync_addPrimitive(0, 0);
  EXPECT_FALSE(handler->terminated.ready());
  handler->create.post();
  folly::coro::blockingWait(handler->destroyed);
  checkAndReset(handler->terminated);
  checkAndReset(handler->destroyed);

  // No active request, eb
  {
    auto adder = client->createAdditionFast();
    adder.sync_accumulatePrimitive(0);
  }
  client->sync_addPrimitive(0, 0);
  checkAndReset(handler->terminated);
  checkAndReset(handler->destroyed);

  // Active request, eb
  {
    auto adder = client->createAdditionFast();
    fut = adder.semifuture_getPrimitive();
    client->sync_addPrimitive(0, 0);
  }
  client->sync_addPrimitive(0, 0);
  checkAndReset(handler->terminated);
  checkAndReset(handler->destroyed);
  std::move(fut).get();
}

CO_TEST(InteractionServiceStandalone, Factory) {
  auto client = makeTestClient<Client<Calculator>>(
      std::make_shared<SemiCalculatorHandler>());

  {
    auto [adder, ret] = client->sync_initializedAddition(42);
    EXPECT_EQ(ret, 42);
    adder.sync_accumulatePrimitive(1);
    EXPECT_EQ(adder.sync_getPrimitive(), 43);
  }

  {
    auto [adder, ret] = co_await client->semifuture_initializedAddition(42);
    EXPECT_EQ(ret, 42);
    co_await adder.semifuture_accumulatePrimitive(1);
    EXPECT_EQ(co_await adder.semifuture_getPrimitive(), 43);
  }

  {
    auto [adder, ret] = co_await client->co_initializedAddition(42);
    EXPECT_EQ(ret, 42);
    co_await adder.co_accumulatePrimitive(1);
    EXPECT_EQ(co_await adder.co_getPrimitive(), 43);
  }
}

CO_TEST(InteractionServiceStandalone, FactoryError) {
  auto client = makeTestClient<Client<Calculator>>(
      std::make_shared<SemiCalculatorHandler>());

  EXPECT_THROW(client->sync_newAddition(), TApplicationException);
  EXPECT_THROW(
      co_await client->semifuture_newAddition(), TApplicationException);
  EXPECT_THROW(co_await client->co_newAddition(), TApplicationException);
}

CO_TEST(InteractionServiceStandalone, FactoryEager) {
  auto client = makeTestClient<Client<Calculator>>(
      std::make_shared<SemiCalculatorHandler>());

  {
    RpcOptions opts;
    auto [adder, sf1] = client->eager_semifuture_initializedAddition(opts, 42);
    auto sf2 = adder.semifuture_accumulatePrimitive(1);
    EXPECT_FALSE(sf1.isReady());
    auto [ret1, ret2] =
        co_await folly::coro::collectAll(std::move(sf1), std::move(sf2));
    EXPECT_EQ(ret1, 42);
    EXPECT_EQ(co_await adder.co_getPrimitive(), 43);
  }
}

TEST(InteractionServiceStandalone, FactoryEb) {
  auto client = makeTestClient<Client<Calculator>>(
      std::make_shared<SemiCalculatorHandler>());

  {
    auto adder = client->sync_fastAddition();
    EXPECT_THROW(adder.sync_accumulatePrimitive(1), TApplicationException);
    EXPECT_EQ(adder.sync_getPrimitive(), 1);
  }

  {
    auto adder = client->sync_veryFastAddition();
    EXPECT_THROW(adder.sync_accumulatePrimitive(1), TApplicationException);
    EXPECT_EQ(adder.sync_getPrimitive(), 1);
  }

  {
    RpcOptions opts;
    auto [adder, sf1] = client->eager_semifuture_fastAddition(opts);
    auto sf2 = adder.semifuture_accumulatePrimitive(1);
    auto sf3 = adder.semifuture_getPrimitive();
    auto [res1, res2, res3] =
        folly::collectAll(std::move(sf1), std::move(sf2), std::move(sf3)).get();
    EXPECT_TRUE(res1.hasValue());
    EXPECT_FALSE(res2.hasValue());
    EXPECT_TRUE(res3.hasValue());
  }

  {
    RpcOptions opts;
    auto [adder, sf1] = client->eager_semifuture_veryFastAddition(opts);
    auto sf2 = adder.semifuture_accumulatePrimitive(1);
    auto sf3 = adder.semifuture_getPrimitive();
    auto [res1, res2, res3] =
        folly::collectAll(std::move(sf1), std::move(sf2), std::move(sf3)).get();
    EXPECT_TRUE(res1.hasValue());
    EXPECT_FALSE(res2.hasValue());
    EXPECT_TRUE(res3.hasValue());
  }
}

TEST(InteractionServiceStandalone, FactoryHandlerCallback) {
  struct HandlerResult : ServiceHandler<Calculator> {
    void async_tm_newAddition(
        HandlerCallbackPtr<TileAndResponse<AdditionIf, void>> cb) override {
      auto handler =
          std::make_unique<SemiCalculatorHandler::SemiAdditionHandler>();
      cb->result({std::move(handler)});
    }

    void async_tm_initializedAddition(
        HandlerCallbackPtr<TileAndResponse<AdditionIf, int>> cb,
        int x) override {
      auto handler =
          std::make_unique<SemiCalculatorHandler::SemiAdditionHandler>();
      handler->acc_ = x;
      cb->result({std::move(handler), x});
    }

    void async_tm_stringifiedAddition(
        HandlerCallbackPtr<
            TileAndResponse<AdditionIf, std::unique_ptr<std::string>>> cb,
        int x) override {
      auto handler =
          std::make_unique<SemiCalculatorHandler::SemiAdditionHandler>();
      handler->acc_ = x;
      cb->result(
          {std::move(handler), folly::copy_to_unique_ptr(std::to_string(x))});
    }
  };

  struct HandlerComplete : ServiceHandler<Calculator> {
    void async_tm_newAddition(
        HandlerCallbackPtr<TileAndResponse<AdditionIf, void>> cb) override {
      auto handler =
          std::make_unique<SemiCalculatorHandler::SemiAdditionHandler>();
      cb->complete(
          folly::Try<TileAndResponse<AdditionIf, void>>{{std::move(handler)}});
    }

    void async_tm_initializedAddition(
        HandlerCallbackPtr<TileAndResponse<AdditionIf, int>> cb,
        int x) override {
      auto handler =
          std::make_unique<SemiCalculatorHandler::SemiAdditionHandler>();
      handler->acc_ = x;
      cb->complete(
          folly::Try<TileAndResponse<AdditionIf, int>>{
              {std::move(handler), x}});
    }

    void async_tm_stringifiedAddition(
        HandlerCallbackPtr<
            TileAndResponse<AdditionIf, std::unique_ptr<std::string>>> cb,
        int x) override {
      auto handler =
          std::make_unique<SemiCalculatorHandler::SemiAdditionHandler>();
      handler->acc_ = x;
      cb->complete(
          folly::Try<TileAndResponse<AdditionIf, std::unique_ptr<std::string>>>{
              {std::move(handler),
               folly::copy_to_unique_ptr(std::to_string(x))}});
    }
  };

  struct HandlerException : ServiceHandler<Calculator> {
    void async_tm_newAddition(
        HandlerCallbackPtr<TileAndResponse<AdditionIf, void>> cb) override {
      cb->exception(std::runtime_error("foo"));
    }

    void async_tm_initializedAddition(
        HandlerCallbackPtr<TileAndResponse<AdditionIf, int>> cb, int) override {
      cb->exception(std::runtime_error("foo"));
    }
  };

  struct HandlerDrop : ServiceHandler<Calculator> {
    void async_tm_newAddition(
        HandlerCallbackPtr<TileAndResponse<AdditionIf, void>> cb) override {
      (void)cb;
    }

    void async_tm_initializedAddition(
        HandlerCallbackPtr<TileAndResponse<AdditionIf, int>> cb, int) override {
      (void)cb;
    }
  };

  // Result
  {
    auto client =
        makeTestClient<Client<Calculator>>(std::make_shared<HandlerResult>());
    auto adder = client->sync_newAddition();
    adder.sync_accumulatePrimitive(1);
    EXPECT_EQ(adder.sync_getPrimitive(), 1);
  }
  {
    auto client =
        makeTestClient<Client<Calculator>>(std::make_shared<HandlerResult>());
    auto [adder, ret] = client->sync_initializedAddition(42);
    EXPECT_EQ(ret, 42);
    adder.sync_accumulatePrimitive(1);
    EXPECT_EQ(adder.sync_getPrimitive(), 43);
  }
  {
    auto client =
        makeTestClient<Client<Calculator>>(std::make_shared<HandlerResult>());
    auto [adder, ret] = client->sync_stringifiedAddition(42);
    EXPECT_EQ(ret, "42");
    adder.sync_accumulatePrimitive(1);
    EXPECT_EQ(adder.sync_getPrimitive(), 43);
  }

  // Complete
  {
    auto client =
        makeTestClient<Client<Calculator>>(std::make_shared<HandlerComplete>());
    auto adder = client->sync_newAddition();
    adder.sync_accumulatePrimitive(1);
    EXPECT_EQ(adder.sync_getPrimitive(), 1);
  }
  {
    auto client =
        makeTestClient<Client<Calculator>>(std::make_shared<HandlerComplete>());
    auto [adder, ret] = client->sync_initializedAddition(42);
    EXPECT_EQ(ret, 42);
    adder.sync_accumulatePrimitive(1);
    EXPECT_EQ(adder.sync_getPrimitive(), 43);
  }
  {
    auto client =
        makeTestClient<Client<Calculator>>(std::make_shared<HandlerComplete>());
    auto [adder, ret] = client->sync_stringifiedAddition(42);
    EXPECT_EQ(ret, "42");
    adder.sync_accumulatePrimitive(1);
    EXPECT_EQ(adder.sync_getPrimitive(), 43);
  }

  // Exception
  {
    auto client = makeTestClient<Client<Calculator>>(
        std::make_shared<HandlerException>());
    EXPECT_THROW(client->sync_newAddition(), TApplicationException);
  }
  {
    auto client = makeTestClient<Client<Calculator>>(
        std::make_shared<HandlerException>());
    EXPECT_THROW(client->sync_initializedAddition(42), TApplicationException);
  }

  // Drop
  {
    auto client =
        makeTestClient<Client<Calculator>>(std::make_shared<HandlerDrop>());
    EXPECT_THROW(client->sync_newAddition(), TApplicationException);
  }
  {
    auto client =
        makeTestClient<Client<Calculator>>(std::make_shared<HandlerDrop>());
    EXPECT_THROW(client->sync_initializedAddition(42), TApplicationException);
  }
}

// --- InternalPriorityE2ETest ---

class InternalPriorityE2ETest : public ::testing::Test {
 public:
  class TestRequestPile : public RoundRobinRequestPile {
   public:
    TestRequestPile()
        : RoundRobinRequestPile(RoundRobinRequestPile::Options()) {}

    ~TestRequestPile() override {
      EXPECT_TRUE(expectedInternalPriorities_.empty());
    }

    std::optional<ServerRequestRejection> enqueue(
        ServerRequest&& request) override {
      auto expectedInternalPriority = expectedInternalPriorities_.front();
      auto internalPriority =
          detail::ServerRequestHelper::internalPriority(request);
      EXPECT_EQ(expectedInternalPriority, internalPriority);
      expectedInternalPriorities_.pop();
      return RoundRobinRequestPile::enqueue(std::move(request));
    }

   private:
    std::queue<int8_t> expectedInternalPriorities_{
        {folly::Executor::LO_PRI,
         folly::Executor::MID_PRI,
         folly::Executor::MID_PRI}};
  };
  void SetUp() override {
    if (!FLAGS_thrift_experimental_use_resource_pools) {
      return;
    }
    auto pile = std::make_unique<TestRequestPile>();
    auto cc = std::make_unique<ParallelConcurrencyController>(
        *pile.get(), *folly::getGlobalCPUExecutor().get());
    server_ = std::make_unique<ScopedServerInterfaceThread>(
        std::make_shared<SemiCalculatorHandler>(),
        "::1",
        0,
        [&](ThriftServer& thriftServer) {
          thriftServer.resourcePoolSet().setResourcePool(
              ResourcePoolHandle::defaultAsync(),
              std::move(pile),
              folly::getUnsafeMutableGlobalCPUExecutor(),
              std::move(cc));
        });
  }

  std::unique_ptr<ScopedServerInterfaceThread> server_;
};

CO_TEST_F(InternalPriorityE2ETest, FactoryFunction) {
  if (!FLAGS_thrift_experimental_use_resource_pools) {
    co_return;
  }
  auto client = server_->newClient<Client<Calculator>>();
  auto [addition, _] = co_await client->co_initializedAddition(0);
  co_await addition.co_getPrimitive();
  co_await addition.co_getPrimitive();
}

CO_TEST_F(InternalPriorityE2ETest, Constructor) {
  if (!FLAGS_thrift_experimental_use_resource_pools) {
    co_return;
  }
  auto client = server_->newClient<Client<Calculator>>();
  auto addition = client->createAddition();
  co_await addition.co_getPrimitive();
  co_await addition.co_getPrimitive();
  co_await addition.co_getPrimitive();
}

} // namespace
} // namespace apache::thrift
