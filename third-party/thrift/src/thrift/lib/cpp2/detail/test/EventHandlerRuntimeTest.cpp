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

#include <initializer_list>
#include <memory>

#include <thrift/lib/cpp/EventHandlerBase.h>
#include <thrift/lib/cpp2/async/MultiplexAsyncProcessor.h>
#include <thrift/lib/cpp2/detail/EventHandlerRuntime.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

#include <thrift/lib/cpp2/detail/test/gen-cpp2/EventHandler_clients.h>
#include <thrift/lib/cpp2/detail/test/gen-cpp2/EventHandler_handlers.h>

using apache::thrift::ScopedServerInterfaceThread;
using apache::thrift::TProcessorEventHandler;
using apache::thrift::detail::EventHandlerRuntime;

namespace {
EventHandlerRuntime::MethodNameSet methodNames(
    std::initializer_list<std::string_view> serviceNames,
    std::initializer_list<std::string_view> methodNames) {
  auto methods = EventHandlerRuntime::MethodNameSet();
  for (auto service : serviceNames) {
    methods.serviceNames.emplace_back(service);
  }
  for (auto method : methodNames) {
    methods.methodNames.emplace_back(method);
  }
  return methods;
}

class EventHandlerRuntimeTest : public testing::Test {
 public:
  class CountingEventHandler : public TProcessorEventHandler {
   private:
    void preRead(void* /* ctx */, const char* /* fn_name */) override {
      ++callCount;
    }

   public:
    int callCount = 0;
  };

 private:
  void SetUp() override {
    class FooHandler
        : public apache::thrift::ServiceHandler<apache::thrift::test::Foo> {
      void sync_foo() override {}
    };
    class BarHandler
        : public apache::thrift::ServiceHandler<apache::thrift::test::Bar> {
      void sync_bar1() override {}
      void sync_bar2() override {}
    };

    std::vector<std::shared_ptr<apache::thrift::AsyncProcessorFactory>>
        handlers = {
            std::make_shared<FooHandler>(), std::make_shared<BarHandler>()};
    server_ = std::make_unique<ScopedServerInterfaceThread>(
        std::make_shared<apache::thrift::MultiplexAsyncProcessorFactory>(
            std::move(handlers)),
        "::1");

    clientEvents = std::make_shared<CountingEventHandler>();
    serverEvents = std::make_shared<CountingEventHandler>();

    apache::thrift::TProcessorBase::addProcessorEventHandler(serverEvents);
    apache::thrift::TClientBase::addClientEventHandler(clientEvents);
  }

  void TearDown() override {
    apache::thrift::TProcessorBase::removeProcessorEventHandler(serverEvents);
    apache::thrift::TClientBase::removeClientEventHandler(clientEvents);
  }

  std::unique_ptr<ScopedServerInterfaceThread> server_;

 public:
  template <typename ServiceTag>
  std::unique_ptr<apache::thrift::Client<ServiceTag>> makeClient() {
    return server_->newClient<apache::thrift::Client<ServiceTag>>();
  }

  std::shared_ptr<CountingEventHandler> clientEvents, serverEvents;
};
} // namespace

TEST_F(EventHandlerRuntimeTest, MethodNames) {
  EventHandlerRuntime::setClientMethodsToBypass(methodNames({}, {}));
  auto foo = makeClient<apache::thrift::test::Foo>();
  auto bar = makeClient<apache::thrift::test::Bar>();

  bar->sync_bar1();
  EXPECT_EQ(clientEvents->callCount, 1);
  EXPECT_EQ(serverEvents->callCount, 1);

  EventHandlerRuntime::setClientMethodsToBypass(methodNames({}, {"Bar.bar1"}));

  bar->sync_bar1();
  EXPECT_EQ(clientEvents->callCount, 1);
  EXPECT_EQ(serverEvents->callCount, 2);

  bar->sync_bar2();
  EXPECT_EQ(clientEvents->callCount, 2);
  EXPECT_EQ(serverEvents->callCount, 3);

  foo->sync_foo();
  EXPECT_EQ(clientEvents->callCount, 3);
  EXPECT_EQ(serverEvents->callCount, 4);

  EventHandlerRuntime::setClientMethodsToBypass(
      methodNames({}, {"Bar.bar1", "Foo.foo"}));

  foo->sync_foo();
  EXPECT_EQ(clientEvents->callCount, 3);
  EXPECT_EQ(serverEvents->callCount, 5);

  EventHandlerRuntime::setClientMethodsToBypass(methodNames({}, {}));

  foo->sync_foo();
  EXPECT_EQ(clientEvents->callCount, 4);
  EXPECT_EQ(serverEvents->callCount, 6);

  bar->sync_bar1();
  EXPECT_EQ(clientEvents->callCount, 5);
  EXPECT_EQ(serverEvents->callCount, 7);
}

TEST_F(EventHandlerRuntimeTest, ServiceNames) {
  EventHandlerRuntime::setClientMethodsToBypass(methodNames({}, {}));
  auto foo = makeClient<apache::thrift::test::Foo>();
  auto bar = makeClient<apache::thrift::test::Bar>();

  bar->sync_bar1();
  EXPECT_EQ(clientEvents->callCount, 1);
  EXPECT_EQ(serverEvents->callCount, 1);

  bar->sync_bar2();
  EXPECT_EQ(clientEvents->callCount, 2);
  EXPECT_EQ(serverEvents->callCount, 2);

  EventHandlerRuntime::setClientMethodsToBypass(methodNames({"Bar"}, {}));

  foo->sync_foo();
  EXPECT_EQ(clientEvents->callCount, 3);
  EXPECT_EQ(serverEvents->callCount, 3);

  bar->sync_bar1();
  EXPECT_EQ(clientEvents->callCount, 3);
  EXPECT_EQ(serverEvents->callCount, 4);

  bar->sync_bar2();
  EXPECT_EQ(clientEvents->callCount, 3);
  EXPECT_EQ(serverEvents->callCount, 5);

  EventHandlerRuntime::setClientMethodsToBypass(
      methodNames({"Bar", "Foo"}, {}));

  foo->sync_foo();
  EXPECT_EQ(clientEvents->callCount, 3);
  EXPECT_EQ(serverEvents->callCount, 6);

  bar->sync_bar1();
  EXPECT_EQ(clientEvents->callCount, 3);
  EXPECT_EQ(serverEvents->callCount, 7);

  bar->sync_bar2();
  EXPECT_EQ(clientEvents->callCount, 3);
  EXPECT_EQ(serverEvents->callCount, 8);

  EventHandlerRuntime::setClientMethodsToBypass(
      methodNames({"Foo"}, {"Bar.bar1"}));

  foo->sync_foo();
  EXPECT_EQ(clientEvents->callCount, 3);
  EXPECT_EQ(serverEvents->callCount, 9);

  bar->sync_bar1();
  EXPECT_EQ(clientEvents->callCount, 3);
  EXPECT_EQ(serverEvents->callCount, 10);

  bar->sync_bar2();
  EXPECT_EQ(clientEvents->callCount, 4);
  EXPECT_EQ(serverEvents->callCount, 11);
}
