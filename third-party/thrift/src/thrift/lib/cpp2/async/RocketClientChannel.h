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

#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/DelayedDestruction.h>

#include <thrift/lib/cpp2/async/RocketClientChannelBase.h>
#include <thrift/lib/cpp2/transport/rocket/client/RocketClient.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift {

/**
 * Thrift client channel using the Rocket protocol for RPC communication.
 */
class RocketClientChannel final : public RocketClientChannelBase,
                                  private rocket::RocketClient {
 public:
  using Ptr = std::
      unique_ptr<RocketClientChannel, folly::DelayedDestruction::Destructor>;

  // Resolve ambiguity between RocketClientChannelBase and RocketClient
  using RocketClientChannelBase::attachEventBase;
  using RocketClientChannelBase::closeNow;
  using RocketClientChannelBase::detachEventBase;
  using RocketClientChannelBase::encodeMetadataUsingBinary;
  using RocketClientChannelBase::getEventBase;
  using RocketClientChannelBase::getServerVersion;
  using RocketClientChannelBase::isDetachable;
  using RocketClientChannelBase::sendRequestBiDi;
  using RocketClientChannelBase::sendRequestResponse;
  using RocketClientChannelBase::sendRequestSink;
  using RocketClientChannelBase::sendRequestStream;
  using RocketClientChannelBase::setCloseCallback;
  using RocketClientChannelBase::setFlushList;
  using RocketClientChannelBase::setOnDetachable;
  using RocketClientChannelBase::terminateInteraction;

  static Ptr newChannel(folly::AsyncTransport::UniquePtr socket);
  static Ptr newChannelWithMetadata(
      folly::AsyncTransport::UniquePtr socket, RequestSetupMetadata meta);

 protected:
  rocket::RocketClient& getRocketClientImpl() override { return *this; }
  const rocket::RocketClient& getRocketClientImpl() const override {
    return *this;
  }

  bool hasCustomCompressor() const override {
    return customCompressor_ != nullptr;
  }
  bool encodeMetadataUsingBinary() const override {
    return rocket::RocketClient::encodeMetadataUsingBinary();
  }
  uint32_t getDestructorGuardCount() const override {
    return rocket::RocketClient::getDestructorGuardCount();
  }
  void closeNowImpl(
      apache::thrift::transport::TTransportException ex) noexcept override {
    rocket::RocketClient::closeNow(std::move(ex));
  }

 private:
  RocketClientChannel(
      folly::EventBase* evb,
      folly::AsyncTransport::UniquePtr socket,
      RequestSetupMetadata meta,
      int32_t keepAliveTimeoutMs = 0,
      std::shared_ptr<rocket::ParserAllocatorType> allocatorPtr = nullptr);

  RocketClientChannel(const RocketClientChannel&) = delete;
  RocketClientChannel& operator=(const RocketClientChannel&) = delete;
  RocketClientChannel(RocketClientChannel&&) = delete;
  RocketClientChannel& operator=(RocketClientChannel&&) = delete;

  ~RocketClientChannel() override;
};

} // namespace apache::thrift
