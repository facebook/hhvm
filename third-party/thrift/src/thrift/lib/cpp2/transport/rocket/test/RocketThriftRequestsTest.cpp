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

#include <thrift/lib/cpp2/transport/rocket/server/RocketThriftRequests.h>

#include <gtest/gtest.h>
#include <thrift/lib/cpp2/PluggableFunction.h>
#include <thrift/lib/cpp2/transport/rocket/test/util/TestUtil.h>

namespace apache::thrift::rocket {

std::atomic<bool> invoked{false};

namespace detail {
THRIFT_PLUGGABLE_FUNC_SET(
    void,
    onRocketThriftRequestReceived,
    const IRocketServerConnection& /* connection */,
    StreamId streamId,
    RpcKind rpcKind,
    const transport::THeader::StringToStringMap& headers) {
  EXPECT_EQ(StreamId{1}, streamId);
  EXPECT_EQ(RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE, rpcKind);
  EXPECT_EQ("hello", headers.at("some_header"));
  invoked.store(true);
}
} // namespace detail

class RocketThriftRequestsTest : public TestSetup {
 public:
  void SetUp() override {
    handler_ = std::make_shared<TestStreamServiceMock>();
    server_ = createServer(handler_, port_);
  }

  void TearDown() override {
    if (server_) {
      server_->cleanUp();
      server_.reset();
      handler_.reset();
    }
  }

  void connectToServer(
      folly::Function<void(std::unique_ptr<Client<StreamService>>)> callMe) {
    callMe(std::make_unique<Client<StreamService>>(
        TestSetup::connectToServer(port_)));
  }

 private:
  std::unique_ptr<ThriftServer> server_;
  std::shared_ptr<TestStreamServiceMock> handler_;
  uint16_t port_;
};

TEST_F(RocketThriftRequestsTest, foo) {
  connectToServer([](std::unique_ptr<Client<StreamService>> client) {
    RpcOptions rpcOptions;
    rpcOptions.setWriteHeader("some_header", "hello");
    client->sync_echo(rpcOptions, 42);
    EXPECT_TRUE(invoked.load());
  });
}

} // namespace apache::thrift::rocket
