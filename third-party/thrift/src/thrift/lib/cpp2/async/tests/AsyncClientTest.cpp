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

#include <gtest/gtest.h>
#include <folly/coro/GtestHelpers.h>

#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/TrustedServerException.h>
#include <thrift/lib/cpp2/async/AsyncClient.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

#include <thrift/lib/cpp2/async/tests/gen-cpp2/test_clients.h>
#include <thrift/lib/cpp2/async/tests/gen-cpp2/test_handlers.h>

class TestHandler
    : public apache::thrift::ServiceHandler<apache::thrift::test::Test> {
  folly::coro::Task<void> co_func() override { co_return; }
};

namespace {
constexpr auto kLoadsheddingMessage = "shedding load";

// Handler whose method raises a TrustedServerException, the sanctioned way for
// a handler to propagate a non-UNKNOWN TApplicationException type to the
// client. appOverloadError maps to TApplicationException::LOADSHEDDING. (A
// plain handler-thrown TApplicationException would have its type discarded and
// arrive as UNKNOWN.)
class ThrowingTestHandler
    : public apache::thrift::ServiceHandler<apache::thrift::test::Test> {
  folly::coro::Task<void> co_func() override {
    throw apache::thrift::TrustedServerException::appOverloadError(
        kLoadsheddingMessage);
  }
};
} // namespace

CO_TEST(AsyncClientTest, ZeroDependency) {
  class TestEventHandler : public apache::thrift::TProcessorEventHandler {
    void preRead(void* /*ctx*/, std::string_view /*fn_name*/) override {
      preReadCallCount++;
    }
    void postWrite(
        void* /*ctx*/,
        std::string_view /*fn_name*/,
        uint32_t /*bytes*/) override {
      postWriteCallCount++;
    }

   public:
    int preReadCallCount = 0;
    int postWriteCallCount = 0;
  };
  auto eventHandler = std::make_shared<TestEventHandler>();
  apache::thrift::TClientBase::addClientEventHandler(eventHandler);
  SCOPE_EXIT {
    apache::thrift::TClientBase::removeClientEventHandler(eventHandler);
  };

  using ClientType = apache::thrift::Client<apache::thrift::test::Test>;
  std::unique_ptr<ClientType> client =
      apache::thrift::makeTestClient(std::make_shared<TestHandler>());

  std::unique_ptr<ClientType> zeroDepClient = std::make_unique<ClientType>(
      client->getChannelShared(), ClientType::Options::zeroDependency());

  co_await zeroDepClient->co_func();
  EXPECT_EQ(eventHandler->preReadCallCount, 0);
  EXPECT_EQ(eventHandler->postWriteCallCount, 0);

  co_await client->co_func();
  EXPECT_EQ(eventHandler->preReadCallCount, 1);
  EXPECT_EQ(eventHandler->postWriteCallCount, 1);
}

// When a handler raises a TrustedServerException, the client observes a
// TApplicationException with the corresponding type. The error code is mapped
// to a ResponseRpcErrorCode on the server and back to a
// TApplicationExceptionType on the client, so appOverloadError surfaces as
// TApplicationException::LOADSHEDDING.
CO_TEST(AsyncClientTest, ApplicationExceptionTypeIsPreserved) {
  using ClientType = apache::thrift::Client<apache::thrift::test::Test>;
  std::unique_ptr<ClientType> client =
      apache::thrift::makeTestClient(std::make_shared<ThrowingTestHandler>());

  try {
    co_await client->co_func();
    ADD_FAILURE() << "expected TApplicationException to be thrown";
  } catch (const apache::thrift::TApplicationException& ex) {
    EXPECT_EQ(
        ex.getType(), apache::thrift::TApplicationException::LOADSHEDDING);
    EXPECT_EQ(ex.getMessage(), kLoadsheddingMessage);
  }
}
