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

#include <thrift/lib/cpp2/async/HibernatingRequestChannel.h>

#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestService.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

#include <gtest/gtest.h>
#include <folly/portability/GMock.h>

using namespace testing;
using namespace apache::thrift;
using namespace apache::thrift::test;
using folly::AsyncSocket;

class TestServiceServerMock
    : public apache::thrift::ServiceHandler<TestService> {
 public:
  MOCK_METHOD(int32_t, echoInt, (int32_t));
};

class HibernatingRequestChannelTest : public Test {
 public:
  folly::EventBase* eb{folly::EventBaseManager::get()->getEventBase()};
  std::shared_ptr<TestServiceServerMock> handler{
      std::make_shared<TestServiceServerMock>()};
  std::unique_ptr<apache::thrift::ScopedServerInterfaceThread> runner{
      std::make_unique<apache::thrift::ScopedServerInterfaceThread>(handler)};

  folly::SocketAddress up_addr{runner->getAddress()};
};

TEST_F(HibernatingRequestChannelTest, immediateHibernate) {
  auto connection_count = 0;
  auto channel = apache::thrift::HibernatingRequestChannel::newChannel(
      *eb,
      std::chrono::milliseconds(0),
      [this, &connection_count](folly::EventBase& eb) mutable {
        connection_count++;
        return HeaderClientChannel::newChannel(
            AsyncSocket::newSocket(&eb, up_addr));
      });

  TestServiceAsyncClient client(std::move(channel));
  EXPECT_CALL(*handler, echoInt(_)).WillOnce(Return(1)).WillOnce(Return(2));
  EXPECT_EQ(client.sync_echoInt(1), 1);
  EXPECT_EQ(client.sync_echoInt(2), 2);
  EXPECT_EQ(connection_count, 2);
}

TEST_F(HibernatingRequestChannelTest, nonimmediateHibernate) {
  auto connection_count = 0;
  auto channel = apache::thrift::HibernatingRequestChannel::newChannel(
      *eb,
      std::chrono::seconds(5),
      [this, &connection_count](folly::EventBase& eb) mutable {
        connection_count++;
        return HeaderClientChannel::newChannel(
            AsyncSocket::newSocket(&eb, up_addr));
      });

  TestServiceAsyncClient client(std::move(channel));
  EXPECT_CALL(*handler, echoInt(_)).WillOnce(Return(1)).WillOnce(Return(2));
  EXPECT_EQ(client.sync_echoInt(1), 1);
  EXPECT_EQ(client.sync_echoInt(2), 2);
  EXPECT_EQ(connection_count, 1);
}
