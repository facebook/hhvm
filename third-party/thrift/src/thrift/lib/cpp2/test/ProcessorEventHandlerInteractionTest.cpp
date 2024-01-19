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

#include <folly/Synchronized.h>
#include <folly/portability/GTest.h>

#include <thrift/lib/cpp/EventHandlerBase.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/test/gen-cpp2/Calculator.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace apache::thrift;

namespace {

/**
 * In this test, we are interested in interaction events.
 */
class TestEventHandler : public TProcessorEventHandler {
  // An interaction ID must be unique per connection
  using UniqueInteractionId = std::pair<folly::SocketAddress, int64_t>;

 public:
  void* getServiceContext(
      const char* service_name,
      const char* fn_name,
      TConnectionContext* conn) override {
    LOG(INFO) << fmt::format(
        "getServiceContext(\"{}\", \"{}\")", service_name, fn_name);
    return conn;
  }

  void freeContext(void*, const char* fn_name) override {
    LOG(INFO) << fmt::format("freeContext(\"{}\")", fn_name);
  }

  void onReadData(
      void* ctx, const char* fn_name, const SerializedMessage&) override {
    LOG(INFO) << fmt::format("onReadData(\"{}\")", fn_name);
    ASSERT_TRUE(ctx);
    auto* reqCtx = dynamic_cast<Cpp2RequestContext*>((TConnectionContext*)ctx);
    ASSERT_TRUE(reqCtx);
    ASSERT_GT(reqCtx->getInteractionId(), 0);
    UniqueInteractionId uniqueId{
        *reqCtx->getPeerAddress(), reqCtx->getInteractionId()};
    if (reqCtx->getInteractionCreate()) {
      auto [_, added] = ids_.wlock()->emplace(uniqueId);
      ASSERT_TRUE(added);
    } else {
      ASSERT_TRUE(ids_.rlock()->count(uniqueId));
    }
    if (std::string_view{"Calculator.Addition.noop"} == fn_name) {
      ASSERT_EQ(reqCtx->getRpcKind(), RpcKind::SINGLE_REQUEST_NO_RESPONSE);
    } else {
      ASSERT_EQ(reqCtx->getRpcKind(), RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);
    }
  }

  bool wantNonPerRequestCallbacks() const override {
    return wantNonPerRequestCallbacks_.load();
  }
  void onInteractionTerminate(void* ctx, int64_t id) override {
    LOG(INFO) << fmt::format("onInteractionTerminate({})", id);
    ASSERT_TRUE(ctx);
    auto* conn = dynamic_cast<Cpp2ConnContext*>((TConnectionContext*)ctx);
    ASSERT_TRUE(conn);
    ASSERT_GT(id, 0);
    UniqueInteractionId uniqueId{*conn->getPeerAddress(), id};
    ASSERT_EQ(1, ids_.wlock()->erase(uniqueId));
  }

  size_t countInteractions() const { return ids_.rlock()->size(); }
  void setWantNonPerRequestCallbacks(bool val) {
    wantNonPerRequestCallbacks_.store(val);
  }

 private:
  folly::Synchronized<std::unordered_set<UniqueInteractionId>> ids_;
  std::atomic_bool wantNonPerRequestCallbacks_{true};
};

class TestHandler : public ServiceHandler<test::Calculator> {
 public:
  class Addition : public AdditionIf {
   public:
    int32_t sync_getPrimitive() override { return acc; }
    void sync_accumulatePrimitive(int32_t val) override { acc += val; }

   private:
    int32_t acc{0};
  };

  TileAndResponse<ServiceHandler<test::Calculator>::AdditionIf, void>
  sync_newAddition() override {
    return {std::make_unique<Addition>()};
  }
};

} // namespace

TEST(TProcessorEventHandlerTest, BasicInteraction) {
  auto eventHandler = std::make_shared<TestEventHandler>();
  TProcessorBase::addProcessorEventHandler(eventHandler);
  {
    ScopedServerInterfaceThread runner(std::make_shared<TestHandler>());
    auto client = runner.newClient<apache::thrift::Client<test::Calculator>>();
    auto add = client->sync_newAddition();
    add.sync_accumulatePrimitive(7);
    EXPECT_EQ(add.sync_getPrimitive(), 7);
    add.sync_accumulatePrimitive(5);
    EXPECT_EQ(add.sync_getPrimitive(), 12);
  }
  EXPECT_EQ(eventHandler->countInteractions(), 0);

  eventHandler->setWantNonPerRequestCallbacks(false);
  {
    ScopedServerInterfaceThread runner(std::make_shared<TestHandler>());
    auto client = runner.newClient<apache::thrift::Client<test::Calculator>>();
    client->sync_newAddition();
    // destruct and trigger interaction termination
  }
  EXPECT_EQ(eventHandler->countInteractions(), 1)
      << "onInteractionTerminate shouldn't be called "
         "when wantNonPerRequestCallbacks is false";
}

TEST(TProcessorEventHandlerTest, MultipleInteractions) {
  auto eventHandler = std::make_shared<TestEventHandler>();
  TProcessorBase::addProcessorEventHandler(eventHandler);
  {
    ScopedServerInterfaceThread runner(std::make_shared<TestHandler>());
    auto client = runner.newClient<apache::thrift::Client<test::Calculator>>();
    {
      auto add = client->sync_newAddition();
      add.sync_accumulatePrimitive(7);
      EXPECT_EQ(add.sync_getPrimitive(), 7);
      add.sync_accumulatePrimitive(5);
      EXPECT_EQ(add.sync_getPrimitive(), 12);
    }
    {
      auto add = client->sync_newAddition();
      add.sync_accumulatePrimitive(3);
      EXPECT_EQ(add.sync_getPrimitive(), 3);
      add.sync_accumulatePrimitive(7);
      EXPECT_EQ(add.sync_getPrimitive(), 10);
    }
  }
  EXPECT_EQ(eventHandler->countInteractions(), 0);
}

TEST(TProcessorEventHandlerTest, MultipleConcurrentInteractions) {
  auto eventHandler = std::make_shared<TestEventHandler>();
  TProcessorBase::addProcessorEventHandler(eventHandler);
  {
    ScopedServerInterfaceThread runner(std::make_shared<TestHandler>());
    auto client = runner.newClient<apache::thrift::Client<test::Calculator>>();
    /* 1 */
    auto add1 = client->sync_newAddition();
    add1.sync_accumulatePrimitive(7);
    EXPECT_EQ(add1.sync_getPrimitive(), 7);
    add1.sync_accumulatePrimitive(5);
    EXPECT_EQ(add1.sync_getPrimitive(), 12);
    /* 2 */
    auto add2 = client->sync_newAddition();
    add2.sync_accumulatePrimitive(3);
    EXPECT_EQ(add2.sync_getPrimitive(), 3);
    add2.sync_accumulatePrimitive(7);
    EXPECT_EQ(add2.sync_getPrimitive(), 10);
    EXPECT_EQ(eventHandler->countInteractions(), 2);
  }
  EXPECT_EQ(eventHandler->countInteractions(), 0);
}

TEST(TProcessorEventHandlerTest, ConnectionClose) {
  using namespace std::chrono;

  auto eventHandler = std::make_shared<TestEventHandler>();
  TProcessorBase::addProcessorEventHandler(eventHandler);

  // server
  ScopedServerInterfaceThread runner(std::make_shared<TestHandler>());

  // client
  folly::EventBase evb;
  auto socket = folly::AsyncSocket::newSocket(&evb, runner.getAddress());
  auto channel = RocketClientChannel::newChannel(std::move(socket));
  auto channelPtr = channel.get();
  auto client = std::make_unique<apache::thrift::Client<test::Calculator>>(
      std::move(channel));

  // 1st interaction
  auto add1 = client->sync_newAddition();
  add1.sync_accumulatePrimitive(7);
  EXPECT_EQ(add1.sync_getPrimitive(), 7);
  add1.sync_accumulatePrimitive(5);
  EXPECT_EQ(add1.sync_getPrimitive(), 12);

  // 2nd interaction
  auto add2 = client->sync_newAddition();
  add2.sync_accumulatePrimitive(3);
  EXPECT_EQ(add2.sync_getPrimitive(), 3);
  add2.sync_accumulatePrimitive(7);
  EXPECT_EQ(add2.sync_getPrimitive(), 10);

  EXPECT_EQ(eventHandler->countInteractions(), 2);

  // drop connection to the server
  channelPtr->closeNow();

  // wait for termination events
  for (auto n = 10; n && eventHandler->countInteractions(); n--) {
    /* sleep override */ std::this_thread::sleep_for(1s);
  }

  EXPECT_EQ(eventHandler->countInteractions(), 0);
}

TEST(TProcessorEventHandlerTest, RpcKind) {
  auto eventHandler = std::make_shared<TestEventHandler>();
  TProcessorBase::addProcessorEventHandler(eventHandler);
  {
    ScopedServerInterfaceThread runner(std::make_shared<TestHandler>());
    auto client = runner.newClient<apache::thrift::Client<test::Calculator>>();
    auto add = client->sync_newAddition();
    add.sync_noop();
  }
  EXPECT_EQ(eventHandler->countInteractions(), 0);
}
