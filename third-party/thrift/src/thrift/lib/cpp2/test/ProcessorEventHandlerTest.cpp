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

#include <folly/portability/GTest.h>

#include <folly/experimental/coro/GtestHelpers.h>
#include <thrift/lib/cpp/EventHandlerBase.h>
#include <thrift/lib/cpp2/server/ServerModule.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/test/gen-cpp2/HandlerGeneric.h>
#include <thrift/lib/cpp2/test/util/TrackingTProcessorEventHandler.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace apache::thrift;
using apache::thrift::test::TrackingTProcessorEventHandler;

namespace {

struct TestHandler : apache::thrift::ServiceHandler<test::HandlerGeneric> {
  folly::coro::Task<std::unique_ptr<std::string>> co_get_string() override {
    co_return std::make_unique<std::string>("reply");
  }
};

class TestModule : public apache::thrift::ServerModule {
 public:
  TrackingTProcessorEventHandler*& eventHandlerRef_;
  explicit TestModule(TrackingTProcessorEventHandler*& eventHandlerRef)
      : eventHandlerRef_(eventHandlerRef) {}

  std::string getName() const override { return "TestModule"; }

  std::vector<std::shared_ptr<TProcessorEventHandler>> getLegacyEventHandlers()
      override {
    std::vector<std::shared_ptr<TProcessorEventHandler>> result;
    auto evtHandler = std::make_unique<TrackingTProcessorEventHandler>();
    eventHandlerRef_ = evtHandler.get();
    result.emplace_back(std::move(evtHandler));
    return result;
  }
};

const std::vector<std::string> kGetStringRpcOutput{
    "getServiceContext('HandlerGeneric', 'HandlerGeneric.get_string')",
    "preRead('HandlerGeneric.get_string')",
    "onReadData('HandlerGeneric.get_string')",
    "postRead('HandlerGeneric.get_string')",
    "preWrite('HandlerGeneric.get_string')",
    "onWriteData('HandlerGeneric.get_string')",
    "postWrite('HandlerGeneric.get_string')",
    "freeContext('HandlerGeneric.get_string')",
};

} // namespace

CO_TEST(TProcessorEventHandlerTest, BasicRead) {
  auto testEventHandler = std::make_shared<TrackingTProcessorEventHandler>();
  TProcessorBase::addProcessorEventHandler(testEventHandler);

  TrackingTProcessorEventHandler* scopedTestEventHandler = nullptr;
  ScopedServerInterfaceThread runner(
      std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(scopedTestEventHandler));
      });
  EXPECT_NE(scopedTestEventHandler, nullptr);

  auto client =
      runner.newClient<apache::thrift::Client<test::HandlerGeneric>>();
  co_await client->co_get_string();
  EXPECT_EQ(testEventHandler->getHistory(), kGetStringRpcOutput);
  EXPECT_EQ(scopedTestEventHandler->getHistory(), kGetStringRpcOutput);
}

CO_TEST(TProcessorEventHandlerTest, ClearEventHandlers) {
  auto testEventHandler = std::make_shared<TrackingTProcessorEventHandler>();
  TProcessorBase::addProcessorEventHandler(testEventHandler);

  class TestHandlerWithClearEventHandlers : public TestHandler {
    std::unique_ptr<AsyncProcessor> getProcessor() override {
      auto processor = TestHandler::getProcessor();
      processor->clearEventHandlers();
      return processor;
    }
  };
  TrackingTProcessorEventHandler* scopedTestEventHandler = nullptr;
  ScopedServerInterfaceThread runner(
      std::make_shared<TestHandlerWithClearEventHandlers>(),
      [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(scopedTestEventHandler));
      });
  EXPECT_NE(scopedTestEventHandler, nullptr);

  auto client =
      runner.newClient<apache::thrift::Client<test::HandlerGeneric>>();
  co_await client->co_get_string();
  EXPECT_EQ(testEventHandler->getHistory(), std::vector<std::string>{});
  EXPECT_EQ(scopedTestEventHandler->getHistory(), kGetStringRpcOutput);
}
