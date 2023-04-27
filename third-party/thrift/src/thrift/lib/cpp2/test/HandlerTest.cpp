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

#include <thrift/lib/cpp2/test/gen-cpp2/HandlerGeneric.h>

#include <memory>

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace std;
using namespace testing;
using namespace folly;
using namespace folly::coro;
using namespace apache::thrift;
using namespace apache::thrift::test;

class HandlerTest : public Test {};

MATCHER(IsMissingResult, "") {
  try {
    arg.value();
    return false;
  } catch (const TApplicationException& e) {
    LOG(INFO) << e.getType();
    return e.getType() == TApplicationException::MISSING_RESULT;
  } catch (...) {
    return false;
  }
}

TEST_F(HandlerTest, async_eb_result_nullptr) {
  struct Handler : apache::thrift::ServiceHandler<HandlerGeneric> {
    void async_eb_get_string_eb(
        unique_ptr<HandlerCallback<unique_ptr<string>>> callback) override {
      callback->result(unique_ptr<string>(nullptr));
    }
  };
  ScopedServerInterfaceThread runner{make_shared<Handler>()};
  auto client =
      runner.newClient<HandlerGenericAsyncClient>(nullptr, [](auto socket) {
        return HeaderClientChannel::newChannel(std::move(socket));
      });
  EXPECT_THAT(client->semifuture_get_string_eb().getTry(), IsMissingResult());
}

TEST_F(HandlerTest, async_tm_result_nullptr) {
  struct Handler : apache::thrift::ServiceHandler<HandlerGeneric> {
    void async_tm_get_string(
        unique_ptr<HandlerCallback<unique_ptr<string>>> callback) override {
      callback->result(unique_ptr<string>(nullptr));
    }
  };
  ScopedServerInterfaceThread runner{make_shared<Handler>()};
  auto client =
      runner.newClient<HandlerGenericAsyncClient>(nullptr, [](auto socket) {
        return HeaderClientChannel::newChannel(std::move(socket));
      });
  EXPECT_THAT(client->semifuture_get_string().getTry(), IsMissingResult());
}

#if FOLLY_HAS_COROUTINES
TEST_F(HandlerTest, co_nullptr) {
  struct Handler : apache::thrift::ServiceHandler<HandlerGeneric> {
    Task<unique_ptr<string>> co_get_string() override { co_return nullptr; }
  };
  ScopedServerInterfaceThread runner{make_shared<Handler>()};
  auto client =
      runner.newClient<HandlerGenericAsyncClient>(nullptr, [](auto socket) {
        return HeaderClientChannel::newChannel(std::move(socket));
      });
  EXPECT_THAT(client->semifuture_get_string().getTry(), IsMissingResult());
}
#endif

TEST_F(HandlerTest, future_nullptr) {
  struct Handler : apache::thrift::ServiceHandler<HandlerGeneric> {
    Future<unique_ptr<string>> future_get_string() override { return nullptr; }
  };
  ScopedServerInterfaceThread runner{make_shared<Handler>()};
  auto client =
      runner.newClient<HandlerGenericAsyncClient>(nullptr, [](auto socket) {
        return HeaderClientChannel::newChannel(std::move(socket));
      });
  EXPECT_THAT(client->semifuture_get_string().getTry(), IsMissingResult());
}

TEST_F(HandlerTest, semifuture_nullptr) {
  struct Handler : apache::thrift::ServiceHandler<HandlerGeneric> {
    SemiFuture<unique_ptr<string>> semifuture_get_string() override {
      return nullptr;
    }
  };
  ScopedServerInterfaceThread runner{make_shared<Handler>()};
  auto client =
      runner.newClient<HandlerGenericAsyncClient>(nullptr, [](auto socket) {
        return HeaderClientChannel::newChannel(std::move(socket));
      });
  EXPECT_THAT(client->semifuture_get_string().getTry(), IsMissingResult());
}
