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

#include <thrift/lib/cpp2/async/RequestChannel.h>

#include <memory>
#include <thread>

#include <gtest/gtest.h>
#include <folly/Memory.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/test/ScopedBoundPort.h>
#include <folly/portability/GMock.h>

#include <folly/io/async/AsyncSocket.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestService.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace std;
using namespace std::chrono;
using namespace folly;
using namespace apache::thrift;
using namespace apache::thrift::test;
using namespace apache::thrift::transport;
using namespace testing;

using CSR = ClientReceiveState;

class TestServiceServerMock
    : public apache::thrift::ServiceHandler<TestService> {
 public:
  MOCK_METHOD(void, noResponse, (int64_t), (override));
  MOCK_METHOD(void, voidResponse, (), (override));
};

class FunctionSendCallbackTest : public Test {
 public:
  unique_ptr<TestServiceAsyncClient> getClient(
      const folly::SocketAddress& addr) {
    return make_unique<TestServiceAsyncClient>(HeaderClientChannel::newChannel(
        HeaderClientChannel::WithoutRocketUpgrade{},
        AsyncSocket::newSocket(&eb, addr)));
  }
  void sendOnewayMessage(
      const folly::SocketAddress& addr,
      function<void(ClientReceiveState&&)> cb) {
    auto client = getClient(addr);
    client->noResponse(
        make_unique<FunctionSendCallback>(std::move(cb)),
        68 /* without loss of generality */);
    eb.loop();
  }
  EventBase eb;
};

TEST_F(FunctionSendCallbackTest, with_missing_server_fails) {
  ScopedBoundPort bound;
  exception_wrapper exn;
  sendOnewayMessage(bound.getAddress(), [&](CSR&& state) {
    exn = std::move(state.exception());
  });
  EXPECT_TRUE(bool(exn));
  auto err = "TTransportException";
  EXPECT_NE(string::npos, exn.what().find(err));
}

TEST_F(FunctionSendCallbackTest, with_throwing_server_passes) {
  auto si = make_shared<TestServiceServerMock>();
  ScopedServerInterfaceThread ssit(si);
  Baton<> done;
  EXPECT_CALL(*si, noResponse(_))
      .WillOnce(DoAll(
          Invoke([&](int64_t) { done.post(); }), Throw(runtime_error("hi"))));
  exception_wrapper exn = make_exception_wrapper<runtime_error>("lo");
  sendOnewayMessage(ssit.getAddress(), [&](CSR&& state) {
    exn = std::move(state.exception());
  });
  done.try_wait_for(std::chrono::milliseconds(50));
  EXPECT_FALSE(exn);
}
