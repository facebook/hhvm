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

#include <thrift/lib/cpp2/async/SecureRocketClientChannel.h>

namespace apache::thrift {

constexpr auto kClientName = "SecureRocketClientChannel.cpp";

SecureRocketClientChannel::SecureRocketClientChannel(
    folly::EventBase* evb,
    folly::AsyncTransport::UniquePtr socket,
    RequestSetupMetadata meta,
    std::shared_ptr<rocket::RocketClientLogger> logger,
    int32_t keepAliveTimeoutMs,
    std::shared_ptr<rocket::ParserAllocatorType> allocatorPtr)
    : RocketClientChannelBase(evb),
      rocket::SecureRocketClient(
          *evb,
          std::move(socket),
          populateSetupMetadata(std::move(meta), kClientName),
          std::move(logger),
          keepAliveTimeoutMs,
          std::move(allocatorPtr)) {
  apache::thrift::detail::hookForClientTransport(getTransport());
}

SecureRocketClientChannel::Ptr SecureRocketClientChannel::newChannel(
    folly::AsyncTransport::UniquePtr socket,
    std::shared_ptr<rocket::RocketClientLogger> logger) {
  auto evb = socket->getEventBase();
  return SecureRocketClientChannel::Ptr(new SecureRocketClientChannel(
      evb, std::move(socket), RequestSetupMetadata(), std::move(logger)));
}

SecureRocketClientChannel::Ptr
SecureRocketClientChannel::newChannelWithMetadata(
    folly::AsyncTransport::UniquePtr socket,
    RequestSetupMetadata meta,
    std::shared_ptr<rocket::RocketClientLogger> logger) {
  auto evb = socket->getEventBase();
  auto keepAliveTimeoutMs = getMetaKeepAliveTimeoutMs(meta);
  return SecureRocketClientChannel::Ptr(new SecureRocketClientChannel(
      evb,
      std::move(socket),
      std::move(meta),
      std::move(logger),
      keepAliveTimeoutMs));
}

SecureRocketClientChannel::~SecureRocketClientChannel() {
  if (RocketClientChannelBase::evb_) {
    RocketClientChannelBase::evb_->dcheckIsInEventBaseThread();
  }
  unsetOnDetachable();
  RocketClientChannelBase::closeNow();
}

} // namespace apache::thrift
