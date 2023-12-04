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

#include <folly/experimental/coro/BlockingWait.h>
#include <folly/portability/GTest.h>

#include <thrift/lib/cpp/EventHandlerBase.h>
#include <thrift/lib/cpp2/server/ServerModule.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/test/gen-cpp2/HandlerGeneric.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace apache::thrift;

namespace {

class TestEventHandler : public TProcessorEventHandler {
 public:
  std::string history;

  void* getServiceContext(
      const char* service_name,
      const char* fn_name,
      TConnectionContext* /*connectionContext*/) override {
    history += fmt::format(
        "getServiceContext(\"{}\", \"{}\"),", service_name, fn_name);
    return this;
  }

  void freeContext(void* ctx, const char* fn_name) override {
    EXPECT_EQ(this, ctx);
    history += fmt::format("freeContext(\"{}\"),", fn_name);
  }

  void preRead(void* ctx, const char* fn_name) override {
    EXPECT_EQ(this, ctx);
    history += fmt::format("preRead(\"{}\"),", fn_name);
  }

  void onReadData(
      void* ctx,
      const char* fn_name,
      const SerializedMessage& /*msg*/) override {
    EXPECT_EQ(this, ctx);
    history += fmt::format("onReadData(\"{}\"),", fn_name);
  }

  void postRead(
      void* ctx,
      const char* fn_name,
      apache::thrift::transport::THeader* /*header*/,
      uint32_t /*bytes*/) override {
    EXPECT_EQ(this, ctx);
    history += fmt::format("postRead(\"{}\"),", fn_name);
  }

  void preWrite(void* ctx, const char* fn_name) override {
    EXPECT_EQ(this, ctx);
    history += fmt::format("preWrite(\"{}\"),", fn_name);
  }

  void onWriteData(
      void* ctx,
      const char* fn_name,
      const SerializedMessage& /*msg*/) override {
    EXPECT_EQ(this, ctx);
    history += fmt::format("onWriteData(\"{}\"),", fn_name);
  }

  void postWrite(void* ctx, const char* fn_name, uint32_t /*bytes*/) override {
    EXPECT_EQ(this, ctx);
    history += fmt::format("postWrite(\"{}\"),", fn_name);
  }
};

class TProcessorEventHandlerTest : public testing::Test {};

} // namespace

TEST_F(TProcessorEventHandlerTest, BasicRead) {
  auto testEventHandler = std::make_shared<TestEventHandler>();
  TProcessorBase::addProcessorEventHandler(testEventHandler);

  struct Handler : apache::thrift::ServiceHandler<test::HandlerGeneric> {
    folly::coro::Task<std::unique_ptr<std::string>> co_get_string() override {
      co_return std::make_unique<std::string>("reply");
    }
  };

  TestEventHandler* scopedTestEventHandler = nullptr;
  ScopedServerInterfaceThread runner(
      std::make_shared<Handler>(), [&](ThriftServer& server) {
        class TestModule : public apache::thrift::ServerModule {
         public:
          TestEventHandler*& eventHandlerRef_;
          explicit TestModule(TestEventHandler*& eventHandlerRef)
              : eventHandlerRef_(eventHandlerRef) {}

          std::string getName() const override { return "TestModule"; }

          std::vector<std::unique_ptr<TProcessorEventHandler>>
          getLegacyEventHandlers() override {
            std::vector<std::unique_ptr<TProcessorEventHandler>> result;
            auto evtHandler = std::make_unique<TestEventHandler>();
            eventHandlerRef_ = evtHandler.get();
            result.emplace_back(std::move(evtHandler));
            return result;
          }
        };
        server.addModule(std::make_unique<TestModule>(scopedTestEventHandler));
      });
  EXPECT_NE(scopedTestEventHandler, nullptr);

  std::string ret;
  auto client = runner.newClient<test::HandlerGenericAsyncClient>();
  folly::coro::blockingWait(client->co_get_string());
  EXPECT_EQ(
      testEventHandler->history,
      "getServiceContext(\"HandlerGeneric\", \"HandlerGeneric.get_string\"),"
      "preRead(\"HandlerGeneric.get_string\"),"
      "onReadData(\"HandlerGeneric.get_string\"),"
      "postRead(\"HandlerGeneric.get_string\"),"
      "preWrite(\"HandlerGeneric.get_string\"),"
      "onWriteData(\"HandlerGeneric.get_string\"),"
      "postWrite(\"HandlerGeneric.get_string\"),"
      "freeContext(\"HandlerGeneric.get_string\"),");
  EXPECT_EQ(scopedTestEventHandler->history, testEventHandler->history);
}
