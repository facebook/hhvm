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

#include <folly/coro/GtestHelpers.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/server/test/gen-cpp2/TestService_clients.h>
#include <thrift/lib/cpp2/server/test/gen-cpp2/TestService_handlers.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace ::testing;
using namespace apache::thrift;

namespace {

class TestServicehandler
    : public apache::thrift::ServiceHandler<test::TestService> {
  folly::coro::Task<std::unique_ptr<std::string>> co_echo(
      std::unique_ptr<std::string> str) override {
    co_return std::move(str);
  }
};

} // namespace

CO_TEST(ThriftServerCpuCCTest, CPUConcurrencyControllerCanBeNull) {
  auto handler = std::make_shared<TestServicehandler>();
  auto server = std::make_shared<ScopedServerInterfaceThread>(
      std::move(handler), [&](ThriftServer& thriftServer) {
        thriftServer.setCPUConcurrencyController(nullptr);
      });
  EXPECT_EQ(server->getThriftServer().getCPUConcurrencyController(), nullptr);

  std::unique_ptr<Client<test::TestService>> client =
      server->newClient<Client<test::TestService>>();

  std::string result = co_await client->co_echo("hello");
  EXPECT_EQ(result, "hello");
}
