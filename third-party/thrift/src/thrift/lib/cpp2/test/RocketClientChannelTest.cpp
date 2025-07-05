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

#include <folly/io/async/AsyncSocket.h>

#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/PluggableFunction.h>
#include <thrift/lib/cpp2/async/ClientChannel.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestService.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

using namespace ::testing;
using namespace apache::thrift::test;
using namespace apache::thrift;

namespace {
constexpr auto kTestAgentName = "testAgentName";
constexpr auto kTestClientKey = "client_id";
constexpr auto kTestClientVal = "client123";
constexpr auto kTestHostname = "completely_made_up_hostname";
constexpr auto kBuildRuleKey = "build_rule";
constexpr auto kCustomBuildRule = "//test_dir:test_target";
constexpr auto kDefaultBuildRule = "//default_dir:default_target";
constexpr auto kTwJobKey = "tw_job_name";
constexpr auto kTwJobVal = "tsp_global/thrift/RocketChannelTest";
} // namespace

// set custom pluggable function implementation to test merging of
// ClientMetadata
namespace apache::thrift::detail {
THRIFT_PLUGGABLE_FUNC_SET(ClientHostMetadata, getClientHostMetadata) {
  ClientHostMetadata md;
  md.hostname = kTestHostname;
  apache::thrift::transport::THeader::StringToStringMap otherMetadata;
  // Add a key that we will test collision against
  otherMetadata.emplace(kBuildRuleKey, kDefaultBuildRule);
  // Add a key that we won't test collision against
  otherMetadata.emplace(kTwJobKey, kTwJobVal);
  md.otherMetadata = otherMetadata;
  return md;
}
} // namespace apache::thrift::detail

class TestServiceHandler : public ServiceHandler<TestService> {
 public:
  folly::coro::Task<int32_t> co_processHeader(RequestParams param) override {
    auto connContext = param.getRequestContext()->getConnectionContext();
    auto clientMetadata = connContext->getClientMetadataRef();

    // Test defaults
    DCHECK(clientMetadata.has_value());
    DCHECK(clientMetadata->getHostname() == kTestHostname);
    DCHECK(clientMetadata->getOtherMetadataField(kTwJobKey) == kTwJobVal);

    if (clientMetadata.value().getAgent().value() == kTestAgentName) {
      co_return 1;
    }

    if (auto otherMetaData =
            clientMetadata.value().getOtherMetadataField(kTestClientKey)) {
      if (otherMetaData.value() == kTestClientVal) {
        co_return 2;
      }
    }

    if (auto otherMetaData =
            clientMetadata.value().getOtherMetadataField(kBuildRuleKey)) {
      if (otherMetaData.value() == kCustomBuildRule) {
        co_return 3;
      } else {
        DCHECK(otherMetaData.value() == kDefaultBuildRule);
      }
    }

    co_return 0;
  }
};

class RocketClientChannelTest : public Test {
 protected:
  RequestChannel::Ptr createRequestChannel(
      std::optional<RequestSetupMetadata> setupMetaData = {}) {
    auto socket = folly::AsyncSocket::UniquePtr(
        new folly::AsyncSocket(&evb, serverAddress));
    return setupMetaData ? RocketClientChannel::newChannelWithMetadata(
                               std::move(socket), std::move(*setupMetaData))
                         : RocketClientChannel::newChannel(std::move(socket));
  }

  std::shared_ptr<TestServiceHandler> handler{
      std::make_shared<TestServiceHandler>()};
  std::unique_ptr<ScopedServerInterfaceThread> runner{
      std::make_unique<ScopedServerInterfaceThread>(handler)};
  folly::SocketAddress serverAddress{runner->getAddress()};
  folly::EventBase evb;
};

TEST_F(RocketClientChannelTest, DefaultConstructor) {
  Client<TestService> testClient{this->createRequestChannel()};
  EXPECT_EQ(testClient.sync_processHeader(), 0);
}

TEST_F(RocketClientChannelTest, ConstructorWithDefaultMetaData) {
  RequestSetupMetadata setupMetadata;
  Client<TestService> testClient{
      this->createRequestChannel(std::move(setupMetadata))};
  EXPECT_EQ(testClient.sync_processHeader(), 0);
}

TEST_F(RocketClientChannelTest, ConstructorWithClientAgentData) {
  RequestSetupMetadata setupMetadata;
  ClientMetadata clientMetadata;
  clientMetadata.agent() = kTestAgentName;
  setupMetadata.clientMetadata().ensure() = clientMetadata;

  Client<TestService> testClient{
      this->createRequestChannel(std::move(setupMetadata))};
  EXPECT_EQ(testClient.sync_processHeader(), 1);
}

TEST_F(RocketClientChannelTest, ConstructorWithClientOtherData) {
  RequestSetupMetadata setupMetadata;
  ClientMetadata clientMetadata;
  apache::thrift::transport::THeader::StringToStringMap otherMetadata;
  otherMetadata.emplace(kTestClientKey, kTestClientVal);
  clientMetadata.otherMetadata().ensure() = otherMetadata;
  setupMetadata.clientMetadata().ensure() = clientMetadata;

  Client<TestService> testClient{
      this->createRequestChannel(std::move(setupMetadata))};
  EXPECT_EQ(testClient.sync_processHeader(), 2);
}

TEST_F(
    RocketClientChannelTest,
    ConstructorWithClientOtherDataClashWithHostMetadata) {
  RequestSetupMetadata setupMetadata;
  ClientMetadata clientMetadata;
  apache::thrift::transport::THeader::StringToStringMap otherMetadata;
  // test the build_rule does not get overrwritten by client host metadata
  otherMetadata.emplace(kBuildRuleKey, kCustomBuildRule);
  clientMetadata.otherMetadata().ensure() = otherMetadata;
  setupMetadata.clientMetadata().ensure() = clientMetadata;

  Client<TestService> testClient{
      this->createRequestChannel(std::move(setupMetadata))};
  EXPECT_EQ(testClient.sync_processHeader(), 3);
}
