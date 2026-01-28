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

#include <thrift/lib/cpp2/transport/rocket/client/SecureRocketClient.h>

#include <folly/container/F14Map.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

THRIFT_FLAG_DEFINE_bool(client_authwall_client_enabled, false);
THRIFT_FLAG_DEFINE_bool(client_authwall_client_enforced, false);

namespace {
const std::string kAuthorization = "authorization";
const std::string kAuthWall = "authWall";
const std::string kUndefined = "UNDEFINED";
} // namespace

namespace apache::thrift::rocket {

void SecureRocketClient::handleSetupResponse(
    const ServerPushMetadata& serverMeta) {
  auto& setupResponse = serverMeta.get_setupResponse();
  if (validateSecurityPolicy(setupResponse.securityPolicy())) {
    RocketClient::handleSetupResponse(serverMeta);
  }
}

bool SecureRocketClient::validateSecurityPolicy(
    optional_field_ref<const SecurityPolicy&> securityPolicy) {
  if (!THRIFT_FLAG(client_authwall_client_enabled) &&
      !THRIFT_FLAG(client_authwall_client_enforced)) {
    return true;
  }

  if (!securityPolicy.has_value()) {
    if (THRIFT_FLAG(client_authwall_client_enforced)) {
      transport::TTransportException ex(
          transport::TTransportException::SECURITY_POLICY_VIOLATION,
          "Connection terminated: server security policy not set");
      logger_->logException(ex);
      close(ex);
      return false;
    } else {
      // in case when client authWall is not enforced, just log the exception.
      logger_->logException(
          transport::TTransportException(
              transport::TTransportException::SECURITY_POLICY_MISSING,
              "Server security policy not set"));
      return true;
    }
  }

  folly::F14FastMap<std::string, std::string> policies;
  if (auto authorization = securityPolicy->authorization()) {
    policies[kAuthorization] =
        apache::thrift::util::enumNameSafe(*authorization);
  } else {
    policies[kAuthorization] = kUndefined;
  }

  if (auto authWall = securityPolicy->authWall()) {
    policies[kAuthWall] = apache::thrift::util::enumNameSafe(*authWall);
  } else {
    policies[kAuthWall] = kUndefined;
  }

  logger_->logSecurityPolicies(policies);

  return true;
}

} // namespace apache::thrift::rocket
