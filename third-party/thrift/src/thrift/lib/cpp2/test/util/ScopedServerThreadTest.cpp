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

#include <thrift/lib/cpp2/util/ScopedServerThread.h>

#include <common/fb303/cpp/FacebookBase2.h>
#include <folly/Memory.h>
#include <folly/SocketAddress.h>
#include <folly/io/async/test/ScopedBoundPort.h>
#include <thrift/lib/cpp2/server/BaseThriftServer.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>

#include <iostream>
#include <stdexcept>
#include <folly/portability/GTest.h>

using namespace apache::thrift;
using namespace apache::thrift::util;
using namespace facebook::fb303;
using namespace std;

class DummyServiceHandler
    : virtual public ::facebook::fb303::FacebookBase2DeprecationMigration {
 public:
  DummyServiceHandler()
      : ::facebook::fb303::FacebookBase2DeprecationMigration("dummy") {}
  cpp2::fb_status getStatus() override { return cpp2::fb_status::ALIVE; }
};

TEST(ScopedServerThreadTest, BindFailure) {
  // Start a server so a port is already bound.
  auto squattingServer = make_shared<ThriftServer>();
  squattingServer->setInterface(make_shared<DummyServiceHandler>());
  folly::SocketAddress squattingAddress;
  squattingAddress.setFromLocalPort(static_cast<uint16_t>(0));
  squattingServer->setAddress(squattingAddress);
  auto squattingThread = std::make_unique<ScopedServerThread>(squattingServer);

  // Try to start another server on the same port.
  auto server = make_shared<ThriftServer>();
  server->setInterface(make_shared<DummyServiceHandler>());
  folly::SocketAddress address;
  address.setFromLocalPort(squattingServer->getAddress().getPort());
  server->setAddress(address);
  EXPECT_THROW(std::make_unique<ScopedServerThread>(server), exception);
  // Make sure there wasn't a leak of the ThriftServer, (cf. t13139338).
  EXPECT_TRUE(server.unique());
}
