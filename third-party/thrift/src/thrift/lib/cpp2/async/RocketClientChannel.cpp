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

#include <thrift/lib/cpp2/async/RocketClientChannel.h>

namespace apache::thrift {

constexpr auto kClientName = "RocketClientChannel.cpp";

RocketClientChannel::RocketClientChannel(
    folly::EventBase* evb,
    folly::AsyncTransport::UniquePtr socket,
    RequestSetupMetadata meta,
    int32_t keepAliveTimeoutMs,
    std::shared_ptr<rocket::ParserAllocatorType> allocatorPtr)
    : RocketClientChannelBase(evb),
      rocket::RocketClient(
          *evb,
          std::move(socket),
          populateSetupMetadata(std::move(meta), kClientName),
          keepAliveTimeoutMs,
          std::move(allocatorPtr)) {
  apache::thrift::detail::hookForClientTransport(getTransport());
}

RocketClientChannel::Ptr RocketClientChannel::newChannel(
    folly::AsyncTransport::UniquePtr socket) {
  auto evb = socket->getEventBase();
  return RocketClientChannel::Ptr(
      new RocketClientChannel(evb, std::move(socket), RequestSetupMetadata()));
}

RocketClientChannel::Ptr RocketClientChannel::newChannelWithMetadata(
    folly::AsyncTransport::UniquePtr socket, RequestSetupMetadata meta) {
  auto evb = socket->getEventBase();
  auto keepAliveTimeoutMs = getMetaKeepAliveTimeoutMs(meta);
  return RocketClientChannel::Ptr(new RocketClientChannel(
      evb, std::move(socket), std::move(meta), keepAliveTimeoutMs));
}

RocketClientChannel::~RocketClientChannel() {
  if (RocketClientChannelBase::evb_) {
    RocketClientChannelBase::evb_->dcheckIsInEventBaseThread();
  }
  unsetOnDetachable();
  RocketClientChannelBase::closeNow();
}

} // namespace apache::thrift
