/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

// Make sure that cpp_name_types.h can be included with conflicting_name
// defined to something problematic.
#define conflicting_name 0

#include <folly/test/JsonTestUtil.h>
#include <thrift/lib/cpp/util/EnumUtils.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>
#include <thrift/test/gen-cpp2/MyService.h>
#include <thrift/test/gen-cpp2/cpp_name_types.h>

#include <folly/portability/GTest.h>

using apache::thrift::SimpleJSONSerializer;
using namespace apache::thrift;
using namespace apache::thrift::test;

TEST(cpp_name_test, rename) {
  auto s = MyStruct();
  s.unique_name() = 42;
  s.opt_unique_name() = 4; // chosen by fair dice roll
  EXPECT_EQ(42, *s.unique_name());
  EXPECT_EQ(42, s.get_unique_name());
  EXPECT_EQ(4, *s.get_opt_unique_name());
  EXPECT_EQ(4, *s.opt_unique_name());
}

TEST(cpp_name_test, json_serialization) {
  auto in = MyStruct();
  in.unique_name() = 42;
  in.opt_unique_name() = 4; // chosen by fair dice roll
  auto json = SimpleJSONSerializer::serialize<std::string>(in);
  FOLLY_EXPECT_JSON_EQ(
      json, R"({"conflicting_name": 42, "opt_conflicting_name": 4})");
  auto out = MyStruct();
  SimpleJSONSerializer::deserialize(json, out);
  EXPECT_EQ(*out.unique_name(), 42);
  EXPECT_EQ(*out.opt_unique_name(), 4);
}

TEST(cpp_name_test, enum_value) {
  EXPECT_EQ(static_cast<int>(MyEnum::REALM), 1);
  EXPECT_STREQ(apache::thrift::util::enumName(MyEnum::REALM), "DOMAIN");
}

// Make sure the server code uses the renamed method.  If the following code
// compiles, that should be sufficient to verify this.
class MyServiceImpl : public apache::thrift::ServiceHandler<MyService> {
 public:
  int getCallCount() { return callCount_; }

  void cppDoNothing() override { callCount_++; }

  folly::Future<folly::Unit> future_cppDoNothing() override {
    callCount_++;
    return folly::Future<folly::Unit>();
  }

  void async_tm_cppDoNothing(
      std::unique_ptr<apache::thrift::HandlerCallback<void>> callback)
      override {
    callCount_++;
    callback->done();
  }

  folly::SemiFuture<folly::Unit> semifuture_cppDoNothing() override {
    callCount_++;
    return folly::SemiFuture<folly::Unit>();
  }

#if FOLLY_HAS_COROUTINES
  folly::coro::Task<void> co_cppDoNothing() override {
    callCount_++;
    co_return;
  }
#endif // FOLLY_HAS_COROUTINES

 private:
  int callCount_ = 0;
};

// Make sure the null implementation function can be called.
TEST(cpp_name_test, null_service) {
  MyServiceSvNull service;
  service.cppDoNothing();
}

// Make sure all the variations of the function name compile. This function
// doesn't actually need to be called - it just needs to compile.
void verifyCompiles(MyServiceAsyncClient* client) {
  client->cppDoNothing(std::unique_ptr<apache::thrift::RequestCallback>());
  client->future_cppDoNothing();
  client->sync_cppDoNothing();
  client->semifuture_cppDoNothing();
}

TEST(cpp_name_test, send_request) {
  auto server = std::make_shared<MyServiceImpl>();
  ScopedServerInterfaceThread runner(server);
  auto eb = folly::EventBaseManager::get()->getEventBase();
  auto client = runner.newClient<MyServiceAsyncClient>(eb);

  client->sync_cppDoNothing();
  EXPECT_EQ(server->getCallCount(), 1);
}
