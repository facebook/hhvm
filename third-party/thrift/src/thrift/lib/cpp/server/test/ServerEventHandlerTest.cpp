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
#include <thrift/lib/cpp/server/test/TrackingTServerEventHandler.h>
#include <thrift/lib/cpp2/server/ServerModule.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/test/gen-cpp2/HandlerGeneric.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using apache::thrift::ScopedServerInterfaceThread;
using apache::thrift::ThriftServer;
using apache::thrift::server::TServerEventHandler;
using apache::thrift::server::test::TrackingTServerEventHandler;
using apache::thrift::test::HandlerGeneric;

namespace {

struct TestHandler : apache::thrift::ServiceHandler<HandlerGeneric> {
  folly::coro::Task<std::unique_ptr<std::string>> co_get_string() override {
    co_return std::make_unique<std::string>("reply");
  }
};

class TestModule : public apache::thrift::ServerModule {
 public:
  std::shared_ptr<TrackingTServerEventHandler>& eventHandlerRef_;
  explicit TestModule(
      std::shared_ptr<TrackingTServerEventHandler>& eventHandlerRef)
      : eventHandlerRef_(eventHandlerRef) {}

  std::string getName() const override { return "TestModule"; }

  std::vector<std::shared_ptr<TServerEventHandler>>
  getLegacyServerEventHandlers() override {
    std::vector<std::shared_ptr<TServerEventHandler>> result;
    auto evtHandler = std::make_shared<TrackingTServerEventHandler>();
    eventHandlerRef_ = evtHandler;
    result.emplace_back(std::move(evtHandler));
    return result;
  }
};

const std::vector<std::string> kGetStringRpcOutput{
    "preStart()",
    "preStart()",
    "newConnection()",
    "connectionDestroyed()",
    "postStop()",
};

} // namespace

CO_TEST(TServerEventHandlerTest, BasicRead) {
  auto testEventHandler = std::make_shared<TrackingTServerEventHandler>();

  std::shared_ptr<TrackingTServerEventHandler> scopedTestEventHandler = nullptr;
  auto runner = std::make_unique<ScopedServerInterfaceThread>(
      std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addServerEventHandler(testEventHandler);
        server.addModule(std::make_unique<TestModule>(scopedTestEventHandler));
      });
  EXPECT_NE(scopedTestEventHandler, nullptr);

  auto client = runner->newClient<apache::thrift::Client<HandlerGeneric>>();
  co_await client->co_get_string();
  runner.reset();

  EXPECT_EQ(testEventHandler->getHistory(), kGetStringRpcOutput);
  EXPECT_EQ(scopedTestEventHandler->getHistory(), kGetStringRpcOutput);
}
