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

#include <atomic>
#include <memory>

#include <boost/lexical_cast.hpp>

#include <gtest/gtest.h>
#include <folly/Executor.h>
#include <folly/MapUtil.h>
#include <folly/executors/ManualExecutor.h>
#include <folly/io/async/EventBase.h>

#include <folly/io/async/AsyncSocket.h>
#include <thrift/lib/cpp2/async/FutureRequest.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/test/gen-cpp2/FutureService.h>
#include <thrift/lib/cpp2/test/util/TestThriftServerFactory.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>
#include <thrift/lib/cpp2/util/ScopedServerThread.h>

using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::test::cpp2;
using namespace apache::thrift::util;
using namespace folly;

class TestInterface : public apache::thrift::ServiceHandler<FutureService> {
  Future<std::unique_ptr<std::string>> future_sendResponse(
      int64_t size) override {
    EXPECT_NE("", getConnectionContext()->getPeerAddress()->describe());

    Promise<std::unique_ptr<std::string>> p;
    auto f = p.getFuture();

    auto func = [p = std::move(p), size]() mutable {
      std::unique_ptr<std::string> _return(
          new std::string("test" + boost::lexical_cast<std::string>(size)));
      p.setValue(std::move(_return));
    };

    RequestEventBase::get()->runInEventBaseThread(
        [func = std::move(func), size]() mutable {
          RequestEventBase::get()->tryRunAfterDelay(std::move(func), size);
        });

    return f;
  }

  Future<Unit> future_noResponse(int64_t size) override {
    Promise<Unit> p;
    auto f = p.getFuture();

    auto func = [p = std::move(p)]() mutable { p.setValue(); };
    RequestEventBase::get()->runInEventBaseThread(
        [func = std::move(func), size]() mutable {
          RequestEventBase::get()->tryRunAfterDelay(std::move(func), size);
        });
    return f;
  }

  Future<std::unique_ptr<std::string>> future_echoRequest(
      std::unique_ptr<std::string> req) override {
    *req += "ccccccccccccccccccccccccccccccccccccccccccccc";

    auto header = getConnectionContext()->getHeader();

    if (header->getHeaders().count("foo")) {
      header->setHeader("header_from_server", "1");
    }

    return makeFuture<std::unique_ptr<std::string>>(std::move(req));
  }

  SemiFuture<std::unique_ptr<std::string>> semifuture_echoRequestSlow(
      std::unique_ptr<std::string> req, int64_t sleepMs) override {
    return folly::futures::sleep(std::chrono::milliseconds{sleepMs})

        .deferValue(
            [req = std::move(req)](auto&&) mutable { return std::move(req); });
  }

  Future<int> future_throwing() override {
    Promise<int> p;
    auto f = p.getFuture();

    Xception x;
    *x.errorCode() = 32;
    *x.message() = "test";

    p.setException(x);

    return f;
  }

  Future<Unit> future_voidThrowing() override {
    Promise<Unit> p;
    auto f = p.getFuture();

    Xception x;
    *x.errorCode() = 42;
    *x.message() = "test2";

    p.setException(x);

    return f;
  }
};

TEST(ThriftServer, FutureExceptions) {
  apache::thrift::TestThriftServerFactory<TestInterface> factory;
  ScopedServerThread sst(factory.create());
  EventBase base;
  auto socket = AsyncSocket::newSocket(&base, *sst.getAddress());

  auto channel = HeaderClientChannel::newChannel(std::move(socket));
  FutureServiceAsyncClient client(std::move(channel));
  auto f = client.future_throwing().waitVia(&base);

  EXPECT_THROW(f.value(), Xception);

  auto vf = client.future_voidThrowing().waitVia(&base);

  EXPECT_THROW(vf.value(), Xception);
}

TEST(ThriftServer, SemiFutureExceptions) {
  apache::thrift::TestThriftServerFactory<TestInterface> factory;
  ScopedServerThread sst(factory.create());
  EventBase base;
  auto socket = AsyncSocket::newSocket(&base, *sst.getAddress());
  auto channel = HeaderClientChannel::newChannel(std::move(socket));
  FutureServiceAsyncClient client(std::move(channel));

  auto f = client.semifuture_throwing().via(&base).waitVia(&base);
  EXPECT_THROW(f.value(), Xception);

  auto vf = client.semifuture_voidThrowing().via(&base).waitVia(&base);
  EXPECT_THROW(std::move(vf).get(), Xception);
}

TEST(ThriftServer, FutureClientTest) {
  using std::chrono::steady_clock;

  apache::thrift::TestThriftServerFactory<TestInterface> factory;
  ScopedServerThread sst(factory.create());
  EventBase base;
  auto socket = AsyncSocket::newSocket(&base, *sst.getAddress());

  auto channel = HeaderClientChannel::newChannel(std::move(socket));
  channel->setTimeout(10000);
  FutureServiceAsyncClient client(std::move(channel));

  // only once you call wait() should you start looping,
  // so the wait time in value() (which calls wait()) should
  // be much higher than the time for future_sendResponse(0)
  steady_clock::time_point start = steady_clock::now();

  auto future = client.future_sendResponse(1000);
  steady_clock::time_point sent = steady_clock::now();

  auto value = std::move(future).getVia(&base);
  steady_clock::time_point got = steady_clock::now();

  EXPECT_EQ(value, "test1000");

  steady_clock::duration sentTime = sent - start;
  steady_clock::duration waitTime = got - sent;

  int factor = 2;
  EXPECT_GE(waitTime, factor * sentTime);

  auto len = client.future_sendResponse(64).thenTry(
      [](folly::Try<std::string>&& response) {
        EXPECT_TRUE(response.hasValue());
        EXPECT_EQ(response.value(), "test64");
        return response.value().size();
      });

  EXPECT_EQ(std::move(len).getVia(&base), 6);

  RpcOptions options;
  options.setTimeout(std::chrono::milliseconds(1));
  try {
    // should timeout
    auto f = client.future_sendResponse(options, 10000);

    // Wait for future to finish
    std::move(f).getVia(&base);
    ADD_FAILURE();
  } catch (...) {
    return;
  }
}

TEST(ThriftServer, SemiFutureClientTest) {
  auto handler = std::make_shared<TestInterface>();
  apache::thrift::ScopedServerInterfaceThread runner(handler);

  EventBase base;
  ManualExecutor executor;
  auto client = runner.newClient<FutureServiceAsyncClient>();
  auto future = client->semifuture_sendResponse(1).via(&executor);
  EXPECT_FALSE(future.isReady());
  future.waitVia(&executor);
  EXPECT_TRUE(future.isReady());
  auto value = future.value();

  EXPECT_EQ(value, "test1");

  auto len = client->semifuture_sendResponse(4)
                 .via(&base)
                 .thenTry([](folly::Try<std::string>&& response) {
                   EXPECT_TRUE(response.hasValue());
                   EXPECT_EQ(response.value(), "test4");
                   return response.value().size();
                 })
                 .waitVia(&base);
  EXPECT_EQ(len.value(), 5);

  RpcOptions options;
  options.setTimeout(std::chrono::milliseconds(1));

  auto f =
      client->semifuture_sendResponse(options, 3000).via(&base).waitVia(&base);

  EXPECT_TRUE(f.hasException());
}

// Needs wait()
TEST(ThriftServer, FutureGetOrderTest) {
  using std::chrono::steady_clock;

  apache::thrift::TestThriftServerFactory<TestInterface> factory;
  ScopedServerThread sst(factory.create());
  EventBase base;
  auto socket = AsyncSocket::newSocket(&base, *sst.getAddress());

  auto channel = HeaderClientChannel::newChannel(std::move(socket));
  channel->setTimeout(10000);
  FutureServiceAsyncClient client(std::move(channel));

  auto future0 = client.future_sendResponse(0);
  auto future1 = client.future_sendResponse(10);
  auto future2 = client.future_sendResponse(20);
  auto future3 = client.future_sendResponse(30);
  auto future4 = client.future_sendResponse(40);

  steady_clock::time_point start = steady_clock::now();

  EXPECT_EQ(std::move(future3).getVia(&base), "test30");
  steady_clock::time_point sent = steady_clock::now();
  EXPECT_EQ(std::move(future4).getVia(&base), "test40");
  EXPECT_EQ(std::move(future0).getVia(&base), "test0");
  EXPECT_EQ(std::move(future2).getVia(&base), "test20");
  EXPECT_EQ(std::move(future1).getVia(&base), "test10");
  steady_clock::time_point gets = steady_clock::now();

  steady_clock::duration sentTime = sent - start;
  steady_clock::duration getsTime = gets - sent;

  int factor = 2;
  EXPECT_GE(sentTime, factor * getsTime);
}

TEST(ThriftServer, OnewayFutureClientTest) {
  using std::chrono::steady_clock;

  apache::thrift::TestThriftServerFactory<TestInterface> factory;
  ScopedServerThread sst(factory.create());
  EventBase base;
  auto socket = AsyncSocket::newSocket(&base, *sst.getAddress());

  auto channel = HeaderClientChannel::newChannel(
      HeaderClientChannel::WithoutRocketUpgrade{}, std::move(socket));
  FutureServiceAsyncClient client(std::move(channel));

  auto future = client.future_noResponse(100);
  steady_clock::time_point sent = steady_clock::now();

  // wait for future to finish.
  base.loop();
  steady_clock::time_point waited = steady_clock::now();

  future.value();
  steady_clock::time_point got = steady_clock::now();

  steady_clock::duration waitTime = waited - sent;
  steady_clock::duration gotTime = got - waited;

  int factor = 1;
  EXPECT_GE(waitTime, factor * gotTime);
  // Client returns quickly because it is oneway, need to sleep for
  // some time so at least when the request reaches the server it is
  // not already stopped.
  // Also consider use a Baton if this is still flaky under stress run.
  /* sleep override */ std::this_thread::sleep_for(
      std::chrono::milliseconds(200));
}

TEST(ThriftServer, FutureHeaderClientTest) {
  ScopedServerInterfaceThread runner(std::make_shared<TestInterface>());
  EventBase eb;
  auto client = runner.newClient<FutureServiceAsyncClient>(&eb);

  RpcOptions rpcOptions;
  rpcOptions.setWriteHeader("foo", "bar");
  auto future =
      client->header_future_echoRequest(rpcOptions, "hi").waitVia(&eb);

  const auto& headers = future.value().second->getHeaders();
  EXPECT_EQ(get_default(headers, "header_from_server"), "1");
}

TEST(ThriftServer, SemiFutureServerTest) {
  auto handler = std::make_shared<TestInterface>();
  apache::thrift::ScopedServerInterfaceThread runner(handler);

  auto client = runner.newClient<FutureServiceAsyncClient>();

  auto startTime = std::chrono::steady_clock::now();

  auto request = "request";
  auto response = client->semifuture_echoRequestSlow(request, 100).get();
  EXPECT_EQ(request, response);
  EXPECT_GE(
      std::chrono::steady_clock::now() - startTime,
      std::chrono::milliseconds{100});
}
