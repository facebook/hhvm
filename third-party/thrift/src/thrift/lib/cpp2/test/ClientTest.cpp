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

#include <thrift/lib/cpp2/test/gen-cpp2/HandlerGeneric.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace apache::thrift;

TEST(ClientTest, ReadHeaderAvailableToEventProcessor) {
  struct EventHandler : TProcessorEventHandler {
    void postRead(
        void*, const char*, transport::THeader* header, uint32_t) override {
      ASSERT_EQ(header->getHeaders().at("header"), "value");
    }
  };

  TClientBase::addClientEventHandler(std::make_shared<EventHandler>());

  struct Handler : apache::thrift::ServiceHandler<test::HandlerGeneric> {
    void get_string(std::string&) override {
      getRequestContext()->setHeader("header", "value");
    }
  };

  ScopedServerInterfaceThread runner(std::make_shared<Handler>());
  std::string ret;
  auto client = runner.newClient<test::HandlerGenericAsyncClient>();
  client->sync_get_string(ret);
  client->semifuture_get_string();
  folly::coro::blockingWait(client->co_get_string());
}
