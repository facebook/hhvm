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
#include <folly/ExceptionWrapper.h>
#include <folly/io/async/AsyncSocket.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/util/ScopedServerThread.h>

#include <thrift/test/gen-cpp2/ExceptionThrowingService.h>

using namespace apache::thrift;
using namespace apache::thrift::util;
using folly::EventBase;
using thrift::test::cpp2::ExceptionThrowingService;
using thrift::test::cpp2::ExceptionThrowingServiceAsyncClient;
using thrift::test::cpp2::SimpleException;

class ExceptionThrowingHandler
    : public apache::thrift::ServiceHandler<ExceptionThrowingService> {
 public:
  void echo(std::string& ret, std::unique_ptr<std::string> req) override {
    ret = *req;
  }

  void throwException(std::unique_ptr<std::string> msg) override {
    SimpleException e;
    *e.message() = *msg;
    throw e;
  }
};

std::unique_ptr<ScopedServerThread> createThriftServer() {
  auto server = std::make_shared<ThriftServer>();
  server->setPort(0);
  server->setInterface(
      std::unique_ptr<ExceptionThrowingHandler>(new ExceptionThrowingHandler));
  return std::make_unique<ScopedServerThread>(server);
}

TEST(ExceptionThrowingTest, Thrift2Client) {
  EventBase eb;
  auto serverThread = createThriftServer();
  auto* serverAddr = serverThread->getAddress();
  auto socket = folly::AsyncSocket::newSocket(&eb, *serverAddr);
  auto channel = HeaderClientChannel::newChannel(std::move(socket));
  ExceptionThrowingServiceAsyncClient client(std::move(channel));

  // Verify that recv_echo works
  bool echoDone = false;
  client.echo(
      [&echoDone, &eb](ClientReceiveState&& state) {
        EXPECT_FALSE(state.exception());
        try {
          std::string result;
          ExceptionThrowingServiceAsyncClient::recv_echo(result, state);
          EXPECT_EQ(result, "Hello World");
          echoDone = true;
        } catch (const std::exception&) {
        }
        eb.terminateLoopSoon();
      },
      "Hello World");
  eb.loop();
  EXPECT_TRUE(echoDone);

  // Verify that recv_wrapped_echo works
  echoDone = false;
  client.echo(
      [&echoDone, &eb](ClientReceiveState&& state) {
        EXPECT_FALSE(state.exception());
        std::string result;
        auto ew = ExceptionThrowingServiceAsyncClient::recv_wrapped_echo(
            result, state);
        if (!ew) {
          EXPECT_EQ(result, "Hello World");
          echoDone = true;
        }
        eb.terminateLoopSoon();
      },
      "Hello World");
  eb.loop();
  EXPECT_TRUE(echoDone);

  // recv_throwException
  bool exceptionThrown = false;
  client.throwException(
      [&exceptionThrown, &eb](ClientReceiveState&& state) {
        EXPECT_FALSE(state.exception());
        try {
          ExceptionThrowingServiceAsyncClient::recv_throwException(state);
        } catch (const SimpleException& e) {
          EXPECT_EQ(*e.message(), "Hello World");
          exceptionThrown = true;
        }
        eb.terminateLoopSoon();
      },
      "Hello World");
  eb.loop();
  EXPECT_TRUE(exceptionThrown);

  // recv_wrapped_throwException
  exceptionThrown = false;
  client.throwException(
      [&exceptionThrown, &eb](ClientReceiveState&& state) {
        EXPECT_FALSE(state.exception());
        auto ew =
            ExceptionThrowingServiceAsyncClient::recv_wrapped_throwException(
                state);
        if (ew && ew.with_exception([](const SimpleException& e) {
              EXPECT_EQ(*e.message(), "Hello World");
            })) {
          exceptionThrown = true;
        }
        eb.terminateLoopSoon();
      },
      "Hello World");
  eb.loop();
  EXPECT_TRUE(exceptionThrown);
}
