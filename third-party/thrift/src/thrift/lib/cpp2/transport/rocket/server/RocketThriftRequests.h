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

#include <folly/Portability.h>

#include <folly/Function.h>

#include <thrift/lib/cpp2/server/RequestsRegistry.h>
#include <thrift/lib/cpp2/transport/core/ThriftRequest.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerFrameContext.h>

namespace folly {
class EventBase;
class IOBuf;
} // namespace folly

namespace apache {
namespace thrift {

class AsyncProcessor;
class RocketSinkClientCallback;
class RocketStreamClientCallback;

namespace rocket {

class Payload;
class RocketServerFrameContext;

class RocketThriftRequest : public ThriftRequestCore {
 public:
  RocketThriftRequest(
      server::ServerConfigs& serverConfigs,
      RequestRpcMetadata&& metadata,
      Cpp2ConnContext& connContext,
      folly::EventBase& evb,
      RocketServerFrameContext&& context);

  folly::EventBase* getEventBase() noexcept final { return &evb_; }

 protected:
  folly::EventBase& evb_;
  RocketServerFrameContext context_;
};

// Object corresponding to rsocket REQUEST_RESPONSE request (single
// request-single response) handled by Thrift server
class ThriftServerRequestResponse final : public RocketThriftRequest {
 public:
  ThriftServerRequestResponse(
      RequestsRegistry::DebugStub* debugStubToInit,
      folly::EventBase& evb,
      server::ServerConfigs& serverConfigs,
      RequestRpcMetadata&& metadata,
      Cpp2ConnContext& connContext,
      std::shared_ptr<folly::RequestContext> rctx,
      RequestsRegistry& reqRegistry,
      rocket::Payload&& debugPayload,
      RocketServerFrameContext&& context,
      int32_t version,
      std::chrono::milliseconds maxResponseWriteTime);

  bool includeEnvelope() const override { return true; }

  void sendThriftResponse(
      ResponseRpcMetadata&&,
      std::unique_ptr<folly::IOBuf>,
      apache::thrift::MessageChannel::SendCallbackPtr) noexcept override;

  void sendSerializedError(
      ResponseRpcMetadata&& metadata,
      std::unique_ptr<folly::IOBuf> exbuf) noexcept override;

  void sendThriftException(
      ResponseRpcMetadata&& metadata,
      std::unique_ptr<folly::IOBuf> data,
      apache::thrift::MessageChannel::SendCallbackPtr) noexcept override;

  bool isStream() const override { return false; }

  void closeConnection(folly::exception_wrapper ew) noexcept override;

 private:
  const int32_t version_;
  std::chrono::milliseconds maxResponseWriteTime_;
};

// Object corresponding to rsocket REQUEST_FNF request (one-way request)
// handled by Thrift server
class ThriftServerRequestFnf final : public RocketThriftRequest {
 public:
  ThriftServerRequestFnf(
      RequestsRegistry::DebugStub* debugStubToInit,
      folly::EventBase& evb,
      server::ServerConfigs& serverConfigs,
      RequestRpcMetadata&& metadata,
      Cpp2ConnContext& connContext,
      std::shared_ptr<folly::RequestContext> rctx,
      RequestsRegistry& reqRegistry,
      rocket::Payload&& debugPayload,
      RocketServerFrameContext&& context,
      folly::Function<void()> onComplete);

  ~ThriftServerRequestFnf() override;

  bool includeEnvelope() const override { return true; }

  void sendThriftResponse(
      ResponseRpcMetadata&&,
      std::unique_ptr<folly::IOBuf>,
      apache::thrift::MessageChannel::SendCallbackPtr) noexcept override;

  void sendSerializedError(
      ResponseRpcMetadata&& metadata,
      std::unique_ptr<folly::IOBuf> exbuf) noexcept override;

  void sendThriftException(
      ResponseRpcMetadata&& metadata,
      std::unique_ptr<folly::IOBuf> data,
      apache::thrift::MessageChannel::SendCallbackPtr) noexcept override;

  void closeConnection(folly::exception_wrapper ew) noexcept override;

 private:
  folly::Function<void()> onComplete_;
};

// Object corresponding to rsocket REQUEST_STREAM request (initial request to
// establish stream) handled by Thrift server
class ThriftServerRequestStream final : public RocketThriftRequest {
 public:
  ThriftServerRequestStream(
      RequestsRegistry::DebugStub* debugStubToInit,
      folly::EventBase& evb,
      server::ServerConfigs& serverConfigs,
      RequestRpcMetadata&& metadata,
      Cpp2ConnContext& connContext,
      std::shared_ptr<folly::RequestContext> rctx,
      RequestsRegistry& reqRegistry,
      rocket::Payload&& debugPayload,
      RocketServerFrameContext&& context,
      int32_t version,
      RocketStreamClientCallback* clientCallback,
      std::shared_ptr<AsyncProcessor> cpp2Processor);

  bool includeEnvelope() const override { return true; }

  void sendThriftResponse(
      ResponseRpcMetadata&&,
      std::unique_ptr<folly::IOBuf>,
      apache::thrift::MessageChannel::SendCallbackPtr) noexcept override;

  using ThriftRequestCore::sendStreamReply;

  void sendSerializedError(
      ResponseRpcMetadata&& metadata,
      std::unique_ptr<folly::IOBuf> exbuf) noexcept override;

  void sendThriftException(
      ResponseRpcMetadata&& metadata,
      std::unique_ptr<folly::IOBuf> data,
      apache::thrift::MessageChannel::SendCallbackPtr) noexcept override;

  bool sendStreamThriftResponse(
      ResponseRpcMetadata&&,
      std::unique_ptr<folly::IOBuf>,
      StreamServerCallbackPtr) noexcept override;

  virtual void sendStreamThriftResponse(
      ResponseRpcMetadata&&,
      std::unique_ptr<folly::IOBuf>,
      ::apache::thrift::detail::ServerStreamFactory&&) noexcept override;

  bool isStream() const override { return true; }

  void closeConnection(folly::exception_wrapper ew) noexcept override;

 private:
  const int32_t version_;
  RocketStreamClientCallback* clientCallback_;

  const std::shared_ptr<AsyncProcessor> cpp2Processor_;
};

// Object corresponding to rsocket sink (REQUEST_CHANNEL) request (initial
// request to establish stream) handled by Thrift server
class ThriftServerRequestSink final : public RocketThriftRequest {
 public:
  ThriftServerRequestSink(
      RequestsRegistry::DebugStub* debugStubToInit,
      folly::EventBase& evb,
      server::ServerConfigs& serverConfigs,
      RequestRpcMetadata&& metadata,
      Cpp2ConnContext& connContext,
      std::shared_ptr<folly::RequestContext> rctx,
      RequestsRegistry& reqRegistry,
      rocket::Payload&& debugPayload,
      RocketServerFrameContext&& context,
      int32_t version,
      RocketSinkClientCallback* clientCallback,
      std::shared_ptr<AsyncProcessor> cpp2Processor);

  bool includeEnvelope() const override { return true; }

  void sendThriftResponse(
      ResponseRpcMetadata&&,
      std::unique_ptr<folly::IOBuf>,
      apache::thrift::MessageChannel::SendCallbackPtr) noexcept override;

  void sendThriftException(
      ResponseRpcMetadata&& metadata,
      std::unique_ptr<folly::IOBuf> data,
      apache::thrift::MessageChannel::SendCallbackPtr) noexcept override;

  void sendSerializedError(
      ResponseRpcMetadata&& metadata,
      std::unique_ptr<folly::IOBuf> exbuf) noexcept override;

#if FOLLY_HAS_COROUTINES
  void sendSinkThriftResponse(
      ResponseRpcMetadata&&,
      std::unique_ptr<folly::IOBuf>,
      apache::thrift::detail::SinkConsumerImpl&&) noexcept override;

  bool sendSinkThriftResponse(
      ResponseRpcMetadata&&,
      std::unique_ptr<folly::IOBuf>,
      SinkServerCallbackPtr) noexcept override;
#endif

  bool isSink() const override { return true; }

  void closeConnection(folly::exception_wrapper ew) noexcept override;

 private:
  const int32_t version_;
  RocketSinkClientCallback* clientCallback_;

  const std::shared_ptr<AsyncProcessor> cpp2Processor_;
};

namespace detail {
THRIFT_PLUGGABLE_FUNC_DECLARE(
    void,
    onRocketThriftRequestReceived,
    const RocketServerConnection&,
    StreamId,
    RpcKind,
    const transport::THeader::StringToStringMap&);
} // namespace detail

} // namespace rocket
} // namespace thrift
} // namespace apache
