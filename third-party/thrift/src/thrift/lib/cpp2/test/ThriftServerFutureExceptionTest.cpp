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

#include <thrift/lib/cpp2/test/gen-cpp2/Raiser.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

#include <gtest/gtest.h>

using namespace std;
using namespace folly;
using namespace apache::thrift;
using namespace apache::thrift::test;

class Lulz : public runtime_error {
 public:
  explicit Lulz(const string& msg) : runtime_error(msg) {}
};

namespace {

using AppExn = TApplicationException;
using AppExnType = AppExn::TApplicationExceptionType;

class RaiserHandler : public apache::thrift::ServiceHandler<Raiser> {
 public:
  explicit RaiserHandler(function<Future<Unit>()> go) : go_(std::move(go)) {}

 private:
  Future<Unit> future_doRaise() override { return go_(); }

  function<Future<Unit>()> go_;
};

class ThriftServerFutureExceptionTest : public testing::Test {
 public:
  EventBase eb;

  template <class V, class F>
  bool exn(Future<V> fv, F&& f) {
    exception_wrapper wrap = fv.waitVia(&eb).result().exception();
    return wrap.with_exception(std::forward<F>(f));
  }
};

Fiery makeFiery(std::string msg) {
  Fiery e;
  *e.message() = std::move(msg);
  return e;
}

TEST_F(ThriftServerFutureExceptionTest, fiery_return) {
  auto go = [&] { return makeFuture<Unit>(makeFiery("rofl")); };

  auto handler = make_shared<RaiserHandler>(go);
  ScopedServerInterfaceThread runner(handler);

  auto client = runner.newClient<RaiserAsyncClient>(&eb);

  EXPECT_TRUE(exn(client->future_doRaise(), [&](const Fiery& e) {
    EXPECT_EQ("rofl", *e.message());
    EXPECT_EQ("::apache::thrift::test::Fiery", string(e.what()));
  }));
}

TEST_F(ThriftServerFutureExceptionTest, fiery_throw) {
  auto go = [&]() -> Future<Unit> { throw makeFiery("rofl"); };

  auto handler = make_shared<RaiserHandler>(go);
  ScopedServerInterfaceThread runner(handler);

  auto client = runner.newClient<RaiserAsyncClient>(&eb);

  EXPECT_TRUE(exn(client->future_doRaise(), [&](const Fiery& e) {
    EXPECT_EQ("rofl", *e.message());
    EXPECT_EQ("::apache::thrift::test::Fiery", string(e.what()));
  }));
}

TEST_F(ThriftServerFutureExceptionTest, lulz_return) {
  auto go = [&] { return makeFuture<Unit>(Lulz("rofl")); };

  auto handler = make_shared<RaiserHandler>(go);
  ScopedServerInterfaceThread runner(handler);

  auto client = runner.newClient<RaiserAsyncClient>(&eb);

  EXPECT_TRUE(exn(client->future_doRaise(), [&](const AppExn& e) {
    EXPECT_EQ(AppExnType::UNKNOWN, e.getType());
    EXPECT_EQ("Lulz: rofl", string(e.what()));
  }));
}

TEST_F(ThriftServerFutureExceptionTest, lulz_throw) {
  auto go = [&]() -> Future<Unit> { throw Lulz("rofl"); };

  auto handler = make_shared<RaiserHandler>(go);
  ScopedServerInterfaceThread runner(handler);

  auto client = runner.newClient<RaiserAsyncClient>(&eb);

  EXPECT_TRUE(exn(client->future_doRaise(), [&](const AppExn& e) {
    EXPECT_EQ(AppExnType::UNKNOWN, e.getType());
    EXPECT_EQ("Lulz: rofl", string(e.what()));
  }));
}

} // namespace
