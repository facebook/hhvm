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

#pragma once

#include <memory>

#include <thrift/lib/cpp2/transport/rocket/client/RocketClient.h>
#include <thrift/lib/cpp2/transport/rocket/client/RocketClientLogger.h>

namespace apache::thrift::rocket {

/**
 * SecureRocketClient extends RocketClient with logging and custom termination
 * policy support.
 */
class SecureRocketClient : public RocketClient {
 public:
  using Ptr = std::
      unique_ptr<SecureRocketClient, folly::DelayedDestruction::Destructor>;

  static Ptr create(
      folly::EventBase& evb,
      folly::AsyncTransport::UniquePtr socket,
      RequestSetupMetadata&& setupMetadata,
      std::shared_ptr<RocketClientLogger> logger,
      int32_t keepAliveTimeoutMs = 0,
      std::shared_ptr<rocket::ParserAllocatorType> allocatorPtr = nullptr) {
    return Ptr(new SecureRocketClient(
        evb,
        std::move(socket),
        std::move(setupMetadata),
        std::move(logger),
        keepAliveTimeoutMs,
        std::move(allocatorPtr)));
  }

 protected:
  SecureRocketClient(
      folly::EventBase& evb,
      folly::AsyncTransport::UniquePtr socket,
      RequestSetupMetadata&& setupMetadata,
      std::shared_ptr<RocketClientLogger> logger,
      int32_t keepAliveTimeoutMs = 0,
      std::shared_ptr<rocket::ParserAllocatorType> allocatorPtr = nullptr)
      : RocketClient(
            evb,
            std::move(socket),
            std::move(setupMetadata),
            keepAliveTimeoutMs,
            std::move(allocatorPtr)),
        logger_(std::move(logger)) {}
  void handleSetupResponse(const ServerPushMetadata& serverMeta) override;

 private:
  bool validateSecurityPolicy(
      optional_field_ref<const SecurityPolicy&> securityPolicy);

  std::shared_ptr<RocketClientLogger> logger_;

  // For testing
  friend class SecureRocketClientTest;
};

} // namespace apache::thrift::rocket
