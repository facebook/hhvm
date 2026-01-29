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

#include <chrono>
#include <limits>
#include <memory>
#include <optional>

#include <folly/ScopeGuard.h>
#include <folly/fibers/FiberManagerMap.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/DelayedDestruction.h>

#include <thrift/lib/cpp2/async/ClientChannel.h>
#include <thrift/lib/cpp2/transport/rocket/client/RocketClient.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace folly {
class EventBase;
class IOBuf;
} // namespace folly

namespace apache::thrift {

class ContextStack;
class RequestCallback;
class RpcOptions;
class StreamClientCallback;
class ThriftClientCallback;

namespace rocket {
class Payload;
} // namespace rocket

namespace transport {
class THeader;
} // namespace transport

class RocketClientChannelBase : public ClientChannel {
 public:
  using Ptr = std::unique_ptr<
      RocketClientChannelBase,
      folly::DelayedDestruction::Destructor>;

  using RequestChannel::sendRequestNoResponse;
  using RequestChannel::sendRequestResponse;
  using RequestChannel::sendRequestSink;
  using RequestChannel::sendRequestStream;

  void sendRequestResponse(
      const RpcOptions& rpcOptions,
      apache::thrift::MethodMetadata&& methodMetadata,
      SerializedRequest&& request,
      std::shared_ptr<transport::THeader> header,
      RequestClientCallback::Ptr cb,
      std::unique_ptr<folly::IOBuf> frameworkMetadata) override;

  void sendRequestNoResponse(
      const RpcOptions& rpcOptions,
      apache::thrift::MethodMetadata&& methodMetadata,
      SerializedRequest&& request,
      std::shared_ptr<transport::THeader> header,
      RequestClientCallback::Ptr cb,
      std::unique_ptr<folly::IOBuf> frameworkMetadata) override;

  void sendRequestStream(
      const RpcOptions& rpcOptions,
      apache::thrift::MethodMetadata&& methodMetadata,
      SerializedRequest&& request,
      std::shared_ptr<transport::THeader> header,
      StreamClientCallback* clientCallback,
      std::unique_ptr<folly::IOBuf> frameworkMetadata) override;

  void sendRequestSink(
      const RpcOptions& rpcOptions,
      apache::thrift::MethodMetadata&& methodMetadata,
      SerializedRequest&& request,
      std::shared_ptr<transport::THeader> header,
      SinkClientCallback* clientCallback,
      std::unique_ptr<folly::IOBuf> frameworkMetadata) override;

  void sendRequestBiDi(
      RpcOptions&& rpcOptions,
      MethodMetadata&& metadata,
      SerializedRequest&& request,
      std::shared_ptr<transport::THeader> header,
      BiDiClientCallback* clientCallback,
      std::unique_ptr<folly::IOBuf> frameworkMetadata) override;

  folly::EventBase* getEventBase() const override { return evb_; }

  uint16_t getProtocolId() override { return protocolId_; }

  void setProtocolId(uint16_t protocolId) { protocolId_ = protocolId; }

  folly::AsyncTransport* FOLLY_NULLABLE getTransport() override;
  bool good() override;

  size_t inflightRequestsAndStreams() const;

  void attachEventBase(folly::EventBase*) override;
  void detachEventBase() override;
  bool isDetachable() override;
  void setOnDetachable(folly::Function<void()> onDetachable) override;
  void unsetOnDetachable() override;

  uint32_t getTimeout() override { return timeout_.count(); }
  void setTimeout(uint32_t timeoutMs) override;

  CLIENT_TYPE getClientType() override { return THRIFT_ROCKET_CLIENT_TYPE; }

  void setMaxPendingRequests(uint32_t n) { maxInflightRequestsAndStreams_ = n; }
  SaturationStatus getSaturationStatus() override;

  void closeNow() override;
  void setCloseCallback(CloseCallback* closeCallback) override;

  using FlushList = boost::intrusive::list<
      folly::EventBase::LoopCallback,
      boost::intrusive::constant_time_size<false>>;

  void setFlushList(FlushList* flushList);

  // must be called from evb thread
  void terminateInteraction(InteractionId id) override;

  // supports nesting
  // must be called from evb thread
  InteractionId registerInteraction(
      apache::thrift::ManagedStringView&& name, int64_t id) override;

 protected:
  static int32_t getMetaKeepAliveTimeoutMs(RequestSetupMetadata& meta);
  static constexpr std::chrono::seconds kDefaultRpcTimeout{60};

  uint16_t protocolId_{apache::thrift::protocol::T_COMPACT_PROTOCOL};
  std::chrono::milliseconds timeout_{kDefaultRpcTimeout};
  uint32_t maxInflightRequestsAndStreams_{std::numeric_limits<uint32_t>::max()};
  folly::EventBase* evb_{nullptr};

  folly::F14FastMap<int64_t, ManagedStringView> pendingInteractions_;

  explicit RocketClientChannelBase(folly::EventBase* evb) : evb_(evb) {}
  ~RocketClientChannelBase() override;

  RocketClientChannelBase(const RocketClientChannelBase&) = delete;
  RocketClientChannelBase& operator=(const RocketClientChannelBase&) = delete;
  RocketClientChannelBase(RocketClientChannelBase&&) = delete;
  RocketClientChannelBase& operator=(RocketClientChannelBase&&) = delete;

  // Returns a reference to the underlying RocketClient implementation
  // (RocketClient or SecureRocketClient). Derived classes (RocketClientChannel,
  // SecureRocketClientChannel) implement these by returning *this.
  virtual rocket::RocketClient& getRocketClientImpl() = 0;
  virtual const rocket::RocketClient& getRocketClientImpl() const = 0;

  // Virtual methods for accessing protected RocketClient members.
  // Derived classes have direct access via private inheritance.
  virtual bool hasCustomCompressor() const = 0;
  virtual bool encodeMetadataUsingBinary() const = 0;
  virtual uint32_t getDestructorGuardCount() const = 0;
  virtual void closeNowImpl(
      apache::thrift::transport::TTransportException ex) noexcept = 0;

  template <typename Callback>
  void sendThriftRequest(
      const RpcOptions& rpcOptions,
      RpcKind kind,
      apache::thrift::MethodMetadata&& methodMetadata,
      SerializedRequest&& request,
      std::shared_ptr<apache::thrift::transport::THeader> header,
      Callback cb,
      std::unique_ptr<folly::IOBuf> frameworkMetadata);

  void sendSingleRequestNoResponse(
      const RpcOptions& rpcOptions,
      RequestRpcMetadata&& metadata,
      rocket::Payload requestPayload,
      RequestClientCallback::Ptr cb);

  void sendSingleRequestSingleResponse(
      const RpcOptions& rpcOptions,
      RequestRpcMetadata&& metadata,
      std::chrono::milliseconds timeout,
      size_t requestSerializedSize,
      rocket::Payload requestPayload,
      RequestClientCallback::Ptr cb);

  void sendStreamRequest(
      const RpcOptions& rpcOptions,
      RequestRpcMetadata&& metadata,
      std::chrono::milliseconds firstResponseTimeout,
      rocket::Payload requestPayload,
      uint16_t protocolId,
      StreamClientCallback* cb);

  void sendSinkRequest(
      const RpcOptions& rpcOptions,
      RequestRpcMetadata&& metadata,
      std::chrono::milliseconds firstResponseTimeout,
      rocket::Payload requestPayload,
      uint16_t protocolId,
      folly::Optional<CompressionConfig> compressionConfig,
      SinkClientCallback* cb);

  void sendBiDiRequest(
      const RpcOptions& rpcOptions,
      RequestRpcMetadata&& metadata,
      std::chrono::milliseconds firstResponseTimeout,
      rocket::Payload requestPayload,
      uint16_t protocolId,
      folly::Optional<CompressionConfig> compressionConfig,
      BiDiClientCallback* clientCallback);

  std::optional<std::chrono::milliseconds> getClientTimeout(
      const RpcOptions& rpcOptions) const;

  std::variant<InteractionCreate, int64_t, std::monostate> getInteractionHandle(
      const RpcOptions& rpcOptions);

  template <typename CallbackPtr>
  bool canHandleRequest(CallbackPtr& cb);

  RequestSetupMetadata populateSetupMetadata(RequestSetupMetadata&& meta);

  int32_t getServerVersion() const;

  class SingleRequestSingleResponseCallback;
  class SingleRequestNoResponseCallback;
};

} // namespace apache::thrift
