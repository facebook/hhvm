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

#include <stdint.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <utility>

#include <glog/logging.h>

#include <folly/Optional.h>
#include <folly/Portability.h>
#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp/protocol/TProtocolException.h>
#include <thrift/lib/cpp/protocol/TProtocolTypes.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/PluggableFunction.h>
#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <thrift/lib/cpp2/async/Sink.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>
#include <thrift/lib/cpp2/server/LoggingEvent.h>
#include <thrift/lib/cpp2/server/ServerConfigs.h>
#include <thrift/lib/cpp2/transport/core/RequestStateMachine.h>
#include <thrift/lib/cpp2/transport/core/ThriftChannelIf.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache {
namespace thrift {

namespace detail {
THRIFT_PLUGGABLE_FUNC_DECLARE(
    void, handleFrameworkMetadata, std::unique_ptr<folly::IOBuf>&&);
THRIFT_PLUGGABLE_FUNC_DECLARE(
    bool,
    handleFrameworkMetadataHeader,
    folly::F14NodeMap<std::string, std::string>&);
THRIFT_PLUGGABLE_FUNC_DECLARE(
    std::unique_ptr<folly::IOBuf>,
    makeThriftFrameworkMetadataOnResponse,
    apache::thrift::transport::THeader::StringToStringMap&);
} // namespace detail

/**
 * Manages per-RPC state.  There is one of these objects for each RPC.
 *
 * TODO: RSocket currently has a dependency to this class. We may want
 * to clean up our APIs to avoid the dependency to a ResponseChannel
 * object.
 */
class ThriftRequestCore : public ResponseChannelRequest {
 public:
  ThriftRequestCore(
      server::ServerConfigs& serverConfigs,
      RequestRpcMetadata&& metadata,
      Cpp2ConnContext& connContext);

  ~ThriftRequestCore() override { cancelTimeout(); }

  bool isActive() const final { return stateMachine_.isActive(); }

  bool tryCancel() { return stateMachine_.tryCancel(getEventBase()); }

  RpcKind kind() const { return kind_; }

  bool isOneway() const final {
    return kind_ == RpcKind::SINGLE_REQUEST_NO_RESPONSE;
  }

  bool includeInRecentRequests() const {
    return stateMachine_.includeInRecentRequests();
  }

  protocol::PROTOCOL_TYPES getProtoId() const {
    return static_cast<protocol::PROTOCOL_TYPES>(header_.getProtocolId());
  }

  Cpp2RequestContext* getRequestContext() { return &reqContext_; }
  const Cpp2RequestContext* getRequestContext() const { return &reqContext_; }

  const transport::THeader& getTHeader() const { return header_; }

  const std::string& getMethodName() const {
    return reqContext_.getMethodName();
  }

  const folly::Optional<CompressionConfig>& getCompressionConfig() {
    return compressionConfig_;
  }

  bool isStartedProcessing() const {
    return stateMachine_.getStartedProcessing();
  }

  // LogRequestSampleCallback is a wrapper for sampled requests
  class LogRequestSampleCallback : public MessageChannel::SendCallback {
   public:
    LogRequestSampleCallback(
        const ResponseRpcMetadata& metadata,
        const std::optional<ResponseRpcError>& responseRpcError,
        const server::TServerObserver::CallTimestamps& timestamps,
        const ThriftRequestCore& thriftRequest,
        server::TServerObserver* observer,
        MessageChannel::SendCallback* chainedCallback = nullptr);

    void sendQueued() override;
    void messageSent() override;
    void messageSendError(folly::exception_wrapper&& e) override;
    ~LogRequestSampleCallback() override;

   private:
    RequestLoggingContext buildRequestLoggingContext(
        const ResponseRpcMetadata& metadata,
        const std::optional<ResponseRpcError>& responseRpcError,
        const server::TServerObserver::CallTimestamps& timestamps,
        const ThriftRequestCore& thriftRequest);

    RequestLoggingContext requestLoggingContext_;
    server::TServerObserver* observer_;
    MessageChannel::SendCallback* chainedCallback_;
  };

  void sendReply(
      ResponsePayload&& response,
      MessageChannel::SendCallback* cb,
      folly::Optional<uint32_t> crc32c) override final;

  bool sendStreamReply(
      ResponsePayload&& response,
      StreamServerCallbackPtr stream,
      folly::Optional<uint32_t> crc32c) override final {
    if (tryCancel()) {
      cancelTimeout();
      auto metadata = makeResponseRpcMetadata(
          header_.extractAllWriteHeaders(),
          header_.extractProxiedPayloadMetadata());
      if (crc32c) {
        metadata.crc32c() = *crc32c;
      }
      auto alive = sendReplyInternal(
          std::move(metadata), std::move(response).buffer(), std::move(stream));

      if (auto* observer = serverConfigs_.getObserver()) {
        observer->sentReply();
      }
      return alive;
    }
    return false;
  }

  void sendStreamReply(
      ResponsePayload&& response,
      apache::thrift::detail::ServerStreamFactory&& stream,
      folly::Optional<uint32_t> crc32c) override final {
    if (tryCancel()) {
      cancelTimeout();
      auto metadata = makeResponseRpcMetadata(
          header_.extractAllWriteHeaders(),
          header_.extractProxiedPayloadMetadata());
      if (crc32c) {
        metadata.crc32c() = *crc32c;
      }
      sendReplyInternal(
          std::move(metadata), std::move(response).buffer(), std::move(stream));

      if (auto* observer = serverConfigs_.getObserver()) {
        observer->sentReply();
      }
    }
  }

#if FOLLY_HAS_COROUTINES
  void sendSinkReply(
      ResponsePayload&& response,
      apache::thrift::detail::SinkConsumerImpl&& consumerImpl,
      folly::Optional<uint32_t> crc32c) override final {
    if (tryCancel()) {
      cancelTimeout();
      auto metadata = makeResponseRpcMetadata(
          header_.extractAllWriteHeaders(),
          header_.extractProxiedPayloadMetadata());
      if (crc32c) {
        metadata.crc32c() = *crc32c;
      }
      sendReplyInternal(
          std::move(metadata),
          std::move(response).buffer(),
          std::move(consumerImpl));

      if (auto* observer = serverConfigs_.getObserver()) {
        observer->sentReply();
      }
    }
  }

  bool sendSinkReply(
      ResponsePayload&& response,
      SinkServerCallbackPtr callback,
      folly::Optional<uint32_t> crc32c) override final {
    if (tryCancel()) {
      cancelTimeout();
      auto metadata = makeResponseRpcMetadata(
          header_.extractAllWriteHeaders(),
          header_.extractProxiedPayloadMetadata());
      if (crc32c) {
        metadata.crc32c() = *crc32c;
      }
      auto alive = sendReplyInternal(
          std::move(metadata),
          std::move(response).buffer(),
          std::move(callback));

      if (auto* observer = serverConfigs_.getObserver()) {
        observer->sentReply();
      }
      return alive;
    }
    return false;
  }
#endif

  void sendErrorWrapped(folly::exception_wrapper ew, std::string exCode) final {
    if (exCode == kConnectionClosingErrorCode) {
      closeConnection(std::move(ew));
    }

    if (tryCancel()) {
      cancelTimeout();
      sendErrorWrappedInternal(
          std::move(ew),
          exCode,
          header_.extractAllWriteHeaders(),
          header_.extractProxiedPayloadMetadata());
    }
  }

  void sendException(
      ResponsePayload&& response,
      MessageChannel::SendCallback* cb) override final;

  void sendQueueTimeoutResponse() final {
    if (tryCancel() && !isOneway()) {
      // once queue timeout is fired, there's no need for task timeout.
      // Also queue timeout is always <= task timeout,
      // so it makes sense to cancel both queue timeout and task timeout
      cancelTimeout();
      if (auto* observer = serverConfigs_.getObserver()) {
        observer->queueTimeout();
      }
      sendErrorWrappedInternal(
          TApplicationException(
              TApplicationException::TApplicationExceptionType::TIMEOUT,
              fmt::format(
                  "Load Shedding Due to Queue Timeout: {} ms",
                  queueTimeout_.value.count())),
          kServerQueueTimeoutErrorCode,
          {},
          {});
    }
  }

  bool isReplyChecksumNeeded() const override { return checksumRequested_; }

  virtual void closeConnection(folly::exception_wrapper) noexcept {
    LOG(FATAL) << "closeConnection not implemented";
  }

 protected:
  virtual void sendThriftResponse(
      ResponseRpcMetadata&& metadata,
      std::unique_ptr<folly::IOBuf> response,
      MessageChannel::SendCallbackPtr) noexcept = 0;

  virtual void sendSerializedError(
      ResponseRpcMetadata&& metadata,
      std::unique_ptr<folly::IOBuf> exbuf) noexcept = 0;

  virtual bool sendStreamThriftResponse(
      ResponseRpcMetadata&&,
      std::unique_ptr<folly::IOBuf>,
      StreamServerCallbackPtr) noexcept {
    folly::terminate_with<std::runtime_error>(
        "sendStreamThriftResponse not implemented");
  }

  virtual void sendStreamThriftResponse(
      ResponseRpcMetadata&&,
      std::unique_ptr<folly::IOBuf>,
      apache::thrift::detail::ServerStreamFactory&&) noexcept {
    LOG(FATAL) << "sendStreamThriftResponse not implemented";
  }

#if FOLLY_HAS_COROUTINES
  virtual void sendSinkThriftResponse(
      ResponseRpcMetadata&&,
      std::unique_ptr<folly::IOBuf>,
      apache::thrift::detail::SinkConsumerImpl&&) noexcept {
    LOG(FATAL) << "sendSinkThriftResponse not implemented";
  }

  virtual bool sendSinkThriftResponse(
      ResponseRpcMetadata&&,
      std::unique_ptr<folly::IOBuf>,
      SinkServerCallbackPtr) noexcept {
    LOG(FATAL) << "sendSinkThriftResponse not implemented";
  }
#endif

  virtual void sendThriftException(
      ResponseRpcMetadata&&,
      std::unique_ptr<folly::IOBuf>,
      MessageChannel::SendCallbackPtr) noexcept = 0;

  bool tryStartProcessing() final { return stateMachine_.tryStartProcessing(); }

  virtual folly::EventBase* getEventBase() noexcept = 0;

  void scheduleTimeouts() {
    auto differentTimeouts = serverConfigs_.getTaskExpireTimeForRequest(
        clientQueueTimeout_,
        clientTimeout_,
        queueTimeout_.value,
        taskTimeout_.value);

    auto reqContext = getRequestContext();
    if (clientTimeout_ > std::chrono::milliseconds::zero()) {
      reqContext->setRequestTimeout(clientTimeout_);
    } else {
      reqContext->setRequestTimeout(taskTimeout_.value);
    }

    if (differentTimeouts) {
      if (queueTimeout_.value > std::chrono::milliseconds(0)) {
        getEventBase()->timer().scheduleTimeout(
            &queueTimeout_, queueTimeout_.value);
      }
    }
    if (taskTimeout_.value > std::chrono::milliseconds(0)) {
      getEventBase()->timer().scheduleTimeout(
          &taskTimeout_, taskTimeout_.value);
    }
  }

  ResponseRpcMetadata makeResponseRpcMetadata(
      transport::THeader::StringToStringMap&& writeHeaders,
      std::optional<ProxiedPayloadMetadata> proxiedPayloadMetadata);

  MessageChannel::SendCallbackPtr createRequestLoggingCallback(
      MessageChannel::SendCallbackPtr&& sendCallback,
      const ResponseRpcMetadata& metadata,
      const std::optional<ResponseRpcError>& responseRpcError,
      server::TServerObserver* observer);

 private:
  static bool includeInRecentRequestsCount(const std::string_view);

  void sendReplyInternal(
      ResponseRpcMetadata&& metadata,
      std::unique_ptr<folly::IOBuf> buf,
      MessageChannel::SendCallbackPtr cb);

  bool sendReplyInternal(
      ResponseRpcMetadata&& metadata,
      std::unique_ptr<folly::IOBuf> buf,
      StreamServerCallbackPtr stream) {
    if (!checkResponseSize(*buf)) {
      sendResponseTooBigEx();
      return false;
    }
    return sendStreamThriftResponse(
        std::move(metadata), std::move(buf), std::move(stream));
  }

  void sendReplyInternal(
      ResponseRpcMetadata&& metadata,
      std::unique_ptr<folly::IOBuf> buf,
      apache::thrift::detail::ServerStreamFactory&& stream) {
    if (checkResponseSize(*buf)) {
      sendStreamThriftResponse(
          std::move(metadata), std::move(buf), std::move(stream));
    } else {
      sendResponseTooBigEx();
    }
  }

#if FOLLY_HAS_COROUTINES
  void sendReplyInternal(
      ResponseRpcMetadata&& metadata,
      std::unique_ptr<folly::IOBuf> buf,
      apache::thrift::detail::SinkConsumerImpl sink) {
    if (checkResponseSize(*buf)) {
      sendSinkThriftResponse(
          std::move(metadata), std::move(buf), std::move(sink));
    } else {
      sendResponseTooBigEx();
    }
  }

  bool sendReplyInternal(
      ResponseRpcMetadata&& metadata,
      std::unique_ptr<folly::IOBuf> buf,
      SinkServerCallbackPtr serverCb) {
    if (checkResponseSize(*buf)) {
      return sendSinkThriftResponse(
          std::move(metadata), std::move(buf), std::move(serverCb));
    } else {
      sendResponseTooBigEx();
      return false;
    }
  }
#endif

  void sendResponseTooBigEx() {
    sendErrorWrappedInternal(
        folly::make_exception_wrapper<TApplicationException>(
            TApplicationException::TApplicationExceptionType::INTERNAL_ERROR,
            "Response size too big"),
        kResponseTooBigErrorCode,
        header_.extractAllWriteHeaders(),
        header_.extractProxiedPayloadMetadata());
  }

  void sendErrorWrappedInternal(
      folly::exception_wrapper ew,
      const std::string& exCode,
      transport::THeader::StringToStringMap&& writeHeaders,
      std::optional<ProxiedPayloadMetadata> proxiedPayloadMetadata) {
    DCHECK(ew.is_compatible_with<TApplicationException>());
    writeHeaders["ex"] = exCode;
    ew.with_exception([&](TApplicationException& tae) {
      std::unique_ptr<folly::IOBuf> exbuf;
      auto proto = getProtoId();
      try {
        if (includeEnvelope()) {
          exbuf = serializeError</*includeEnvelope=*/true>(
              proto, tae, getMethodName(), 0);
        } else {
          exbuf = serializeError</*includeEnvelope=*/false>(
              proto, tae, getMethodName(), 0);
        }
      } catch (const protocol::TProtocolException& pe) {
        // Should never happen.  Log an error and return an empty
        // payload.
        LOG(ERROR) << "serializeError failed. type=" << pe.getType()
                   << " what()=" << pe.what();
      }

      if (tae.getType() ==
              TApplicationException::TApplicationExceptionType::UNKNOWN &&
          exbuf && !checkResponseSize(*exbuf)) {
        sendResponseTooBigEx();
        return;
      }

      sendSerializedError(
          makeResponseRpcMetadata(
              std::move(writeHeaders), proxiedPayloadMetadata),
          std::move(exbuf));
    });
  }

  void cancelTimeout() {
    queueTimeout_.cancelTimeout();
    taskTimeout_.cancelTimeout();
  }

  bool checkResponseSize(const folly::IOBuf& buf) {
    auto maxResponseSize = serverConfigs_.getMaxResponseSize();
    return maxResponseSize == 0 ||
        buf.computeChainDataLength() <= maxResponseSize;
  }

  struct QueueTimeout : public folly::HHWheelTimer::Callback {
    ThriftRequestCore& request;
    // final timeout value used
    std::chrono::milliseconds value;

    explicit QueueTimeout(ThriftRequestCore& requestP) : request(requestP) {}

    void timeoutExpired() noexcept override {
      if (request.stateMachine_.tryStopProcessing()) {
        request.sendQueueTimeoutResponse();
      }
    }
  };

  struct TaskTimeout : public folly::HHWheelTimer::Callback {
    ThriftRequestCore& request;
    // final timeout value used
    std::chrono::milliseconds value;
    const server::ServerConfigs& serverConfigs;

    TaskTimeout(
        ThriftRequestCore& requestP,
        const server::ServerConfigs& serverConfigsP)
        : request(requestP), serverConfigs(serverConfigsP) {}

    void timeoutExpired() noexcept override {
      if (request.tryCancel() && !request.isOneway()) {
        if (auto* observer = serverConfigs.getObserver()) {
          observer->taskTimeout();
        }
        request.sendErrorWrappedInternal(
            TApplicationException(
                TApplicationException::TApplicationExceptionType::TIMEOUT,
                "Task expired"),
            kTaskExpiredErrorCode,
            {},
            {});
      }
    }
  };
  friend struct QueueTimeout;
  friend struct TaskTimeout;
  friend class ThriftProcessor;
  friend class LogRequestSampleCallback;

  server::TServerObserver::CallTimestamps& getTimestamps() {
    return static_cast<server::TServerObserver::CallTimestamps&>(
        reqContext_.getTimestamps());
  }

 protected:
  server::ServerConfigs& serverConfigs_;
  const RpcKind kind_;

 private:
  bool checksumRequested_{false};
  transport::THeader header_;
  folly::Optional<std::string> loadMetric_;
  Cpp2RequestContext reqContext_;
  folly::Optional<CompressionConfig> compressionConfig_;

  QueueTimeout queueTimeout_;
  TaskTimeout taskTimeout_;
  std::chrono::milliseconds clientQueueTimeout_{0};
  std::chrono::milliseconds clientTimeout_{0};

 protected:
  RequestStateMachine stateMachine_;
};

// HTTP2 uses this
class ThriftRequest final : public ThriftRequestCore {
 public:
  ThriftRequest(
      server::ServerConfigs& serverConfigs,
      std::shared_ptr<ThriftChannelIf> channel,
      RequestRpcMetadata&& metadata,
      std::unique_ptr<Cpp2ConnContext> connContext)
      : ThriftRequestCore(serverConfigs, std::move(metadata), *connContext),
        channel_(std::move(channel)),
        connContext_(std::move(connContext)) {
    serverConfigs_.incActiveRequests();
    scheduleTimeouts();
  }

  bool includeEnvelope() const override { return true; }

  ~ThriftRequest() override { serverConfigs_.decActiveRequests(); }

 private:
  void sendThriftResponse(
      ResponseRpcMetadata&& metadata,
      std::unique_ptr<folly::IOBuf> response,
      MessageChannel::SendCallbackPtr) noexcept override {
    channel_->sendThriftResponse(std::move(metadata), std::move(response));
  }

  void sendThriftException(
      ResponseRpcMetadata&& metadata,
      std::unique_ptr<folly::IOBuf> response,
      MessageChannel::SendCallbackPtr) noexcept override {
    channel_->sendThriftResponse(std::move(metadata), std::move(response));
  }

  void sendSerializedError(
      ResponseRpcMetadata&& metadata,
      std::unique_ptr<folly::IOBuf> exbuf) noexcept override {
    switch (kind_) {
      case RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE:
        sendThriftResponse(std::move(metadata), std::move(exbuf), nullptr);
        break;
      case RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE:
        sendStreamThriftResponse(
            std::move(metadata),
            std::move(exbuf),
            StreamServerCallbackPtr(nullptr));
        break;
#if FOLLY_HAS_COROUTINES
      case RpcKind::SINK:
        sendSinkThriftResponse(
            std::move(metadata),
            std::move(exbuf),
            SinkServerCallbackPtr(nullptr));
        break;
#endif
      default: // Don't send error back for one-way.
        LOG(ERROR) << "unknown rpckind " << static_cast<int32_t>(kind_);
        break;
    }
  }

  // Don't allow hiding of overloaded method.
#if FOLLY_HAS_COROUTINES
  using ThriftRequestCore::sendSinkThriftResponse;
#endif
  using ThriftRequestCore::sendStreamThriftResponse;

  folly::EventBase* getEventBase() noexcept override {
    return channel_->getEventBase();
  }

 private:
  std::shared_ptr<ThriftChannelIf> channel_;
  std::unique_ptr<Cpp2ConnContext> connContext_;
};

} // namespace thrift
} // namespace apache
