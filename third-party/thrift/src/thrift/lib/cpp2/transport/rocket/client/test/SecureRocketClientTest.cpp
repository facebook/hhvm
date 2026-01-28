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

#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <folly/container/F14Map.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp2/transport/rocket/client/RocketClientLogger.h>
#include <thrift/lib/cpp2/transport/rocket/client/SecureRocketClient.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

THRIFT_FLAG_DECLARE_bool(client_authwall_client_enabled);
THRIFT_FLAG_DECLARE_bool(client_authwall_client_enforced);

namespace apache::thrift::rocket {

using TTransportExceptionType =
    transport::TTransportException::TTransportExceptionType;

class MockRocketClientLogger : public RocketClientLogger {
 public:
  void logSecurityPolicies(
      const folly::F14FastMap<std::string, std::string>& policies) override {
    loggedPolicies_ = policies;
  }

  void logException(const transport::TTransportException& exception) override {
    loggedExceptions_.emplace_back(
        exception.getType(), std::string(exception.what()));
  }

  const folly::F14FastMap<std::string, std::string>& getLoggedPolicies() const {
    return loggedPolicies_;
  }

  const std::vector<std::pair<TTransportExceptionType, std::string>>&
  getLoggedExceptions() const {
    return loggedExceptions_;
  }

 private:
  folly::F14FastMap<std::string, std::string> loggedPolicies_;
  std::vector<std::pair<TTransportExceptionType, std::string>>
      loggedExceptions_;
};

class SecureRocketClientTest : public testing::Test {
 protected:
  void SetUp() override {
    mockLogger_ = std::make_shared<MockRocketClientLogger>();
  }

  void TearDown() override {
    THRIFT_FLAG_SET_MOCK(client_authwall_client_enabled, false);
    THRIFT_FLAG_SET_MOCK(client_authwall_client_enforced, false);
  }

  std::shared_ptr<SecureRocketClient> createClient() {
    auto socket = folly::AsyncSocket::newSocket(&evb_);
    RequestSetupMetadata setupMetadata;
    return SecureRocketClient::create(
        evb_, std::move(socket), std::move(setupMetadata), mockLogger_);
  }

  // Helper method with friend access to call private validateSecurityPolicy
  bool callValidateSecurityPolicy(
      SecureRocketClient& client,
      optional_field_ref<const SecurityPolicy&> securityPolicy) {
    return client.validateSecurityPolicy(securityPolicy);
  }

  SetupResponse createSetupResponse(
      std::optional<SecurityPolicyStatus> authorization,
      std::optional<SecurityPolicyStatus> authWall) {
    SetupResponse setupResponse;

    if (authorization.has_value() || authWall.has_value()) {
      SecurityPolicy securityPolicy;
      if (authorization.has_value()) {
        securityPolicy.authorization() = *authorization;
      }
      if (authWall.has_value()) {
        securityPolicy.authWall() = *authWall;
      }
      setupResponse.securityPolicy() = std::move(securityPolicy);
    }

    return setupResponse;
  }

  folly::EventBase evb_;
  std::shared_ptr<MockRocketClientLogger> mockLogger_;
};

TEST_F(SecureRocketClientTest, ValidateSecurityPolicyWhenFlagsDisabled) {
  THRIFT_FLAG_SET_MOCK(client_authwall_client_enabled, false);
  THRIFT_FLAG_SET_MOCK(client_authwall_client_enforced, false);

  auto secureRocketClient = createClient();

  auto setupResponse = createSetupResponse(
      SecurityPolicyStatus::ENFORCED, SecurityPolicyStatus::ENFORCED);

  bool result = callValidateSecurityPolicy(
      *secureRocketClient, setupResponse.securityPolicy());

  EXPECT_TRUE(result);
  EXPECT_TRUE(mockLogger_->getLoggedPolicies().empty());
  EXPECT_TRUE(mockLogger_->getLoggedExceptions().empty());
}

TEST_F(SecureRocketClientTest, ValidateSecurityPolicyWhenEnabledFlagSet) {
  THRIFT_FLAG_SET_MOCK(client_authwall_client_enabled, true);
  THRIFT_FLAG_SET_MOCK(client_authwall_client_enforced, false);

  auto secureRocketClient = createClient();

  auto setupResponse = createSetupResponse(
      SecurityPolicyStatus::ENFORCED, SecurityPolicyStatus::ENABLED);

  bool result = callValidateSecurityPolicy(
      *secureRocketClient, setupResponse.securityPolicy());

  EXPECT_TRUE(result);
  auto& policies = mockLogger_->getLoggedPolicies();
  EXPECT_EQ(policies.at("authorization"), "ENFORCED");
  EXPECT_EQ(policies.at("authWall"), "ENABLED");
}

TEST_F(SecureRocketClientTest, ValidateSecurityPolicyWhenAuthWallMissing) {
  THRIFT_FLAG_SET_MOCK(client_authwall_client_enabled, true);
  THRIFT_FLAG_SET_MOCK(client_authwall_client_enforced, false);

  auto secureRocketClient = createClient();

  auto setupResponse =
      createSetupResponse(SecurityPolicyStatus::ENFORCED, std::nullopt);

  bool result = callValidateSecurityPolicy(
      *secureRocketClient, setupResponse.securityPolicy());

  EXPECT_TRUE(result);
  auto& policies = mockLogger_->getLoggedPolicies();
  EXPECT_EQ(policies.at("authorization"), "ENFORCED");
  EXPECT_EQ(policies.at("authWall"), "UNDEFINED");
}

TEST_F(
    SecureRocketClientTest,
    ValidateSecurityPolicyWhenPolicyNotSetAndOnlyEnabledFlag) {
  THRIFT_FLAG_SET_MOCK(client_authwall_client_enabled, true);
  THRIFT_FLAG_SET_MOCK(client_authwall_client_enforced, false);

  auto secureRocketClient = createClient();

  auto setupResponse = createSetupResponse(std::nullopt, std::nullopt);

  bool result = callValidateSecurityPolicy(
      *secureRocketClient, setupResponse.securityPolicy());

  // When only enabled flag is set and policy is missing, log exception but
  // return true to continue processing
  EXPECT_TRUE(result);
  EXPECT_TRUE(mockLogger_->getLoggedPolicies().empty());
  ASSERT_EQ(mockLogger_->getLoggedExceptions().size(), 1);
  EXPECT_EQ(
      mockLogger_->getLoggedExceptions()[0].first,
      TTransportExceptionType::SECURITY_POLICY_MISSING);
}

TEST_F(
    SecureRocketClientTest,
    ValidateSecurityPolicyWhenPolicyNotSetAndEnforcedFlag) {
  THRIFT_FLAG_SET_MOCK(client_authwall_client_enabled, false);
  THRIFT_FLAG_SET_MOCK(client_authwall_client_enforced, true);

  auto secureRocketClient = createClient();

  auto setupResponse = createSetupResponse(std::nullopt, std::nullopt);

  bool result = callValidateSecurityPolicy(
      *secureRocketClient, setupResponse.securityPolicy());

  // When enforced flag is set and policy is missing, log violation and
  // return false to terminate connection
  EXPECT_FALSE(result);
  EXPECT_TRUE(mockLogger_->getLoggedPolicies().empty());
  ASSERT_EQ(mockLogger_->getLoggedExceptions().size(), 1);
  EXPECT_EQ(
      mockLogger_->getLoggedExceptions()[0].first,
      TTransportExceptionType::SECURITY_POLICY_VIOLATION);
}

TEST_F(SecureRocketClientTest, ValidateSecurityPolicyWhenEnforcedFlagSet) {
  THRIFT_FLAG_SET_MOCK(client_authwall_client_enabled, false);
  THRIFT_FLAG_SET_MOCK(client_authwall_client_enforced, true);

  auto secureRocketClient = createClient();

  auto setupResponse = createSetupResponse(
      SecurityPolicyStatus::ENFORCED, SecurityPolicyStatus::ENFORCED);

  bool result = callValidateSecurityPolicy(
      *secureRocketClient, setupResponse.securityPolicy());

  EXPECT_TRUE(result);
  EXPECT_FALSE(mockLogger_->getLoggedPolicies().empty());
  EXPECT_TRUE(mockLogger_->getLoggedExceptions().empty());
}

TEST_F(SecureRocketClientTest, ValidateSecurityPolicyBothFlagsEnabled) {
  THRIFT_FLAG_SET_MOCK(client_authwall_client_enabled, true);
  THRIFT_FLAG_SET_MOCK(client_authwall_client_enforced, true);

  auto secureRocketClient = createClient();

  auto setupResponse = createSetupResponse(
      SecurityPolicyStatus::ENFORCED, SecurityPolicyStatus::ENABLED);

  bool result = callValidateSecurityPolicy(
      *secureRocketClient, setupResponse.securityPolicy());

  EXPECT_TRUE(result);
  auto& policies = mockLogger_->getLoggedPolicies();
  EXPECT_EQ(policies.at("authorization"), "ENFORCED");
  EXPECT_EQ(policies.at("authWall"), "ENABLED");
  EXPECT_TRUE(mockLogger_->getLoggedExceptions().empty());
}

} // namespace apache::thrift::rocket
