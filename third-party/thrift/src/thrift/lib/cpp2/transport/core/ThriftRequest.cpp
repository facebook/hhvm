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

#include <thrift/lib/cpp2/async/InterceptorFrameworkMetadata.h>
#include <thrift/lib/cpp2/server/CPUConcurrencyController.h>
#include <thrift/lib/cpp2/server/LoggingEvent.h>
#include <thrift/lib/cpp2/transport/core/ThriftRequest.h>

#include <thrift/lib/cpp2/GeneratedCodeHelper.h>

THRIFT_FLAG_DEFINE_int64(queue_time_logging_threshold_ms, 5);
THRIFT_FLAG_DEFINE_bool(enable_request_event_logging, true);

namespace apache::thrift {

namespace detail {
THRIFT_PLUGGABLE_FUNC_REGISTER(
    void,
    handleFrameworkMetadata,
    std::unique_ptr<folly::IOBuf>&&,
    Cpp2RequestContext*) {}
THRIFT_PLUGGABLE_FUNC_REGISTER(
    bool,
    handleFrameworkMetadataHeader,
    folly::F14NodeMap<std::string, std::string>&,
    Cpp2RequestContext*) {
  return false;
}
THRIFT_PLUGGABLE_FUNC_REGISTER(
    std::unique_ptr<folly::IOBuf>,
    makeThriftFrameworkMetadataOnResponse,
    apache::thrift::transport::THeader::StringToStringMap&) {
  return nullptr;
}
} // namespace detail

ThriftRequestCore::ThriftRequestCore(
    server::ServerConfigs& serverConfigs,
    RequestRpcMetadata&& metadata,
    Cpp2ConnContext& connContext,
    RequestsRegistry::DecoratorAndInterceptorStorage
        decoratorAndInterceptorStorage)
    : serverConfigs_(serverConfigs),
      kind_(metadata.kind().value_or(RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE)),
      checksumRequested_(metadata.crc32c().has_value()),
      loadMetric_(
          metadata.loadMetric()
              ? folly::make_optional(std::move(*metadata.loadMetric()))
              : folly::none),
      secondaryLoadMetric_(
          metadata.secondaryLoadMetric()
              ? folly::make_optional(std::move(*metadata.secondaryLoadMetric()))
              : folly::none),
      reqContext_(
          &connContext,
          &header_,
          metadata.name() ? std::move(*metadata.name()).str() : std::string{},
          std::move(decoratorAndInterceptorStorage.interceptorStorage),
          std::move(decoratorAndInterceptorStorage.decoratorDataStorage)),
      queueTimeout_(*this),
      taskTimeout_(*this, serverConfigs_),
      stateMachine_(
          includeInRecentRequestsCount(reqContext_.getMethodName()),
          serverConfigs_.getAdaptiveConcurrencyController(),
          serverConfigs_.getCPUConcurrencyController()) {
  // Note that method name, RPC kind, and serialization protocol are validated
  // outside the ThriftRequestCore constructor.
  header_.setProtocolId(
      static_cast<int16_t>(metadata.protocol().value_or(ProtocolId::BINARY)));

  if (auto clientTimeoutMs = metadata.clientTimeoutMs()) {
    clientTimeout_ = std::chrono::milliseconds(*clientTimeoutMs);
    header_.setClientTimeout(clientTimeout_);
  }
  if (auto queueTimeoutMs = metadata.queueTimeoutMs()) {
    clientQueueTimeout_ = std::chrono::milliseconds(*queueTimeoutMs);
    header_.setClientQueueTimeout(clientQueueTimeout_);
  }
  if (auto priority = metadata.priority()) {
    header_.setCallPriority(static_cast<concurrency::PRIORITY>(*priority));
  }
  auto otherMetadata = metadata.otherMetadata();

  // When processing ThriftFrameworkMetadata, the header takes priority.
  if (!otherMetadata ||
      !detail::handleFrameworkMetadataHeader(*otherMetadata, &reqContext_)) {
    if (auto frameworkMetadata = metadata.frameworkMetadata()) {
      DCHECK(*frameworkMetadata && !(**frameworkMetadata).empty());
      detail::handleFrameworkMetadata(
          std::move(*frameworkMetadata), &reqContext_);
    }
  }
  if (otherMetadata) {
    header_.setReadHeaders(std::move(*otherMetadata));
  }
  if (auto clientId = metadata.clientId()) {
    header_.setClientId(*clientId);
    header_.setReadHeader(transport::THeader::kClientId, std::move(*clientId));
  }
  if (auto serviceTraceMeta = metadata.serviceTraceMeta()) {
    header_.setServiceTraceMeta(*serviceTraceMeta);
    header_.setReadHeader(
        transport::THeader::kServiceTraceMeta, std::move(*serviceTraceMeta));
  }
  if (auto loggingContext = metadata.loggingContext()) {
    header_.loggingContext() = std::move(*loggingContext);
  }
  if (auto quotaReportConfig = metadata.quotaReportConfig()) {
    header_.quotaReportConfig() = std::move(*quotaReportConfig);
  }
  if (auto tenantId = metadata.tenantId()) {
    header_.setTenantId(*tenantId);
    header_.setReadHeader(transport::THeader::kTenantId, std::move(*tenantId));
  }

  // Store client's compression configs (if client explicitly requested
  // compression codec and size limit, use these settings to compress
  // response)
  if (auto compressionConfig = metadata.compressionConfig()) {
    compressionConfig_ = *compressionConfig;
  }

  header_.setChecksum(metadata.checksum().to_optional());

  if (auto* observer = serverConfigs_.getObserver()) {
    observer->receivedRequest(&reqContext_.getMethodName());
  }
}

bool ThriftRequestCore::includeInRecentRequestsCount(
    const std::string_view methodName) {
  return util::includeInRecentRequestsCount(methodName);
}

ThriftRequestCore::LogRequestSampleCallback::LogRequestSampleCallback(
    const ResponseRpcMetadata& metadata,
    const std::optional<ResponseRpcError>& responseRpcError,
    const server::TServerObserver::CallTimestamps& timestamps,
    const ThriftRequestCore& thriftRequest,
    MessageChannel::SendCallback* chainedCallback)
    : serverConfigs_(thriftRequest.serverConfigs_),
      requestLoggingContext_(buildRequestLoggingContext(
          metadata, responseRpcError, timestamps, thriftRequest)),
      chainedCallback_(chainedCallback) {}

void ThriftRequestCore::LogRequestSampleCallback::sendQueued() {
  requestLoggingContext_.timestamps.writeBegin =
      std::chrono::steady_clock::now();
  if (chainedCallback_ != nullptr) {
    chainedCallback_->sendQueued();
  }
}

void ThriftRequestCore::LogRequestSampleCallback::messageSent() {
  SCOPE_EXIT {
    delete this;
  };
  requestLoggingContext_.timestamps.writeEnd = std::chrono::steady_clock::now();
  if (chainedCallback_ != nullptr) {
    chainedCallback_->messageSent();
  }
}

void ThriftRequestCore::LogRequestSampleCallback::messageSendError(
    folly::exception_wrapper&& e) {
  SCOPE_EXIT {
    delete this;
  };
  requestLoggingContext_.timestamps.writeEnd = std::chrono::steady_clock::now();
  if (chainedCallback_ != nullptr) {
    chainedCallback_->messageSendError(std::move(e));
  }
}

ThriftRequestCore::LogRequestSampleCallback::~LogRequestSampleCallback() {
  const auto& samplingStatus =
      requestLoggingContext_.timestamps.getSamplingStatus();

  auto* observer = serverConfigs_.getObserver();
  if (samplingStatus.isServerSamplingEnabled() && observer &&
      requestLoggingContext_.requestStartedProcessing) {
    observer->callCompleted(requestLoggingContext_.timestamps);
  }

  if (THRIFT_FLAG(enable_request_event_logging) &&
      samplingStatus.isRequestLoggingEnabled()) {
    const bool error = requestLoggingContext_.exceptionMetaData.has_value() ||
        requestLoggingContext_.responseRpcError.has_value();
    if (auto samplingRatio = error ? samplingStatus.getLogErrorSampleRatio()
                                   : samplingStatus.getLogSampleRatio()) {
      auto& handler = getLoggingEventRegistry().getRequestEventHandler(
          error ? "error" : "success");
      handler.logSampled(samplingRatio, requestLoggingContext_);
    }
  }
}

RequestLoggingContext
ThriftRequestCore::LogRequestSampleCallback::buildRequestLoggingContext(
    const ResponseRpcMetadata& metadata,
    const std::optional<ResponseRpcError>& responseRpcError,
    const server::TServerObserver::CallTimestamps& timestamps,
    const ThriftRequestCore& thriftRequest) {
  RequestLoggingContext requestLoggingContext;
  requestLoggingContext.timestamps = timestamps;
  requestLoggingContext.responseRpcError = responseRpcError;
  if (auto payloadMetadata = metadata.payloadMetadata()) {
    if (auto exceptionMetadata = payloadMetadata->exceptionMetadata()) {
      requestLoggingContext.exceptionMetaData = *exceptionMetadata;
    }
  }

  const auto& reqContext = thriftRequest.getRequestContext();
  if (const auto* routingTarget = reqContext->getRoutingTarget()) {
    requestLoggingContext.routingTarget = *routingTarget;
  }
  if (const auto* clientId = reqContext->clientId()) {
    requestLoggingContext.clientId = *clientId;
  }
  requestLoggingContext.methodName = reqContext->getMethodName();
  if (const auto* requestId = reqContext->getClientRequestId()) {
    requestLoggingContext.requestId = *requestId;
  }
  requestLoggingContext.requestAttemptId = reqContext->getRequestAttemptId();

  requestLoggingContext.requestStartedProcessing =
      thriftRequest.isStartedProcessing();

  // final timeout
  requestLoggingContext.finalQueueTimeoutMs = thriftRequest.queueTimeout_.value;
  requestLoggingContext.finalTaskTimeoutMs = thriftRequest.taskTimeout_.value;
  // server timeout
  requestLoggingContext.serverQueueTimeoutMs = serverConfigs_.getQueueTimeout();
  requestLoggingContext.serverTaskTimeoutMs =
      serverConfigs_.getTaskExpireTime();
  requestLoggingContext.serverQueueTimeoutPct =
      serverConfigs_.getQueueTimeoutPct();
  requestLoggingContext.serverUseClientTimeout =
      serverConfigs_.getUseClientTimeout();
  // client timeout
  requestLoggingContext.clientQueueTimeoutMs =
      thriftRequest.clientQueueTimeout_;
  requestLoggingContext.clientTimeoutMs = thriftRequest.clientTimeout_;

  // CPUConcurrencyController mode
  if (serverConfigs_.getCPUConcurrencyController() != nullptr) {
    requestLoggingContext.cpuConcurrencyControllerMode = static_cast<uint8_t>(
        serverConfigs_.getCPUConcurrencyController()->config()->mode);
  } else {
    requestLoggingContext.cpuConcurrencyControllerMode =
        static_cast<uint8_t>(CPUConcurrencyController::Mode::DISABLED);
  }

  return requestLoggingContext;
}

MessageChannel::SendCallbackPtr ThriftRequestCore::createRequestLoggingCallback(
    MessageChannel::SendCallbackPtr&& cb,
    const ResponseRpcMetadata& metadata,
    const std::optional<ResponseRpcError>& responseRpcError) {
  auto cbPtr = std::move(cb);
  // If we are sampling this call, wrap it with a RequestTimestampSample,
  // which also implements MessageChannel::SendCallback. Callers of
  // sendReply/sendError are responsible for cleaning up their own callbacks.
  auto& timestamps = getTimestamps();
  if (timestamps.getSamplingStatus().isEnabled()) {
    auto chainedCallback = cbPtr.release();
    return MessageChannel::SendCallbackPtr(
        new ThriftRequestCore::LogRequestSampleCallback(
            metadata, responseRpcError, timestamps, *this, chainedCallback));
  }
  return cbPtr;
}

void ThriftRequestCore::sendReplyInternal(
    ResponseRpcMetadata&& metadata,
    std::unique_ptr<folly::IOBuf> buf,
    MessageChannel::SendCallbackPtr cb) {
  if (checkResponseSize(*buf)) {
    sendThriftResponse(std::move(metadata), std::move(buf), std::move(cb));
  } else {
    sendResponseTooBigEx();
  }
}

void ThriftRequestCore::sendReply(
    ResponsePayload&& response,
    MessageChannel::SendCallback* cb,
    folly::Optional<uint32_t> crc32c) {
  auto cbWrapper = MessageChannel::SendCallbackPtr(cb);
  if (tryCancel()) {
    cancelTimeout();
    // Mark processEnd for the request.
    // Note: this processEnd time unfortunately does not account for the time
    // to compress the response in rocket today (which happens in
    // ThriftServerRequestResponse::sendThriftResponse).
    // TODO: refactor to move response compression to CPU thread.
    auto& timestamps = getTimestamps();
    timestamps.processEnd = std::chrono::steady_clock::now();
    auto* observer = serverConfigs_.getObserver();
    if (!isOneway()) {
      auto metadata = makeResponseRpcMetadata(
          header_.extractAllWriteHeaders(),
          header_.extractProxiedPayloadMetadata(),
          header_.getChecksum());
      if (crc32c) {
        metadata.crc32c() = *crc32c;
      }
      sendReplyInternal(
          std::move(metadata),
          std::move(response).buffer(),
          std::move(cbWrapper));
      if (observer) {
        observer->sentReply();
      }
    }
    if (getRequestContext()->isException() && observer) {
      observer->declaredException();
    }
  }
}

void ThriftRequestCore::sendException(
    ResponsePayload&& response, MessageChannel::SendCallback* cb) {
  auto cbWrapper = MessageChannel::SendCallbackPtr(cb);
  if (tryCancel()) {
    cancelTimeout();
    // Mark processEnd for the request.
    // Note: this processEnd time unfortunately does not account for the time
    // to compress the response in rocket today (which happens in
    // ThriftServerRequestResponse::sendThriftResponse).
    // TODO: refactor to move response compression to CPU thread.
    auto& timestamps = getTimestamps();
    timestamps.processEnd = std::chrono::steady_clock::now();
    auto* observer = serverConfigs_.getObserver();
    if (!isOneway()) {
      auto metadata = makeResponseRpcMetadata(
          header_.extractAllWriteHeaders(),
          header_.extractProxiedPayloadMetadata(),
          std::nullopt /*checksum*/);
      if (checkResponseSize(*response.buffer())) {
        sendThriftException(
            std::move(metadata),
            std::move(response).buffer(),
            std::move(cbWrapper));
      } else {
        sendResponseTooBigEx();
      }
      if (observer) {
        observer->sentReply();
      }
    }
    if (getRequestContext()->isException() && observer) {
      observer->undeclaredException();
    }
  }
}

ResponseRpcMetadata ThriftRequestCore::makeResponseRpcMetadata(
    transport::THeader::StringToStringMap&& writeHeaders,
    std::optional<ProxiedPayloadMetadata> proxiedPayloadMetadata,
    std::optional<Checksum> checksum) {
  ResponseRpcMetadata metadata;

  if (auto tfmr = detail::makeThriftFrameworkMetadataOnResponse(writeHeaders)) {
    metadata.frameworkMetadata() = std::move(tfmr);
  }

  metadata.proxiedPayloadMetadata().from_optional(proxiedPayloadMetadata);

  if (loadMetric_) {
    metadata.load() = serverConfigs_.getLoad(*loadMetric_);
  }

  if (secondaryLoadMetric_) {
    metadata.secondaryLoad() = serverConfigs_.getLoad(*secondaryLoadMetric_);
  }

  if (!writeHeaders.empty()) {
    metadata.otherMetadata() = std::move(writeHeaders);
  }

  auto queueTime = stateMachine_.queueingTime();
  auto& queueMetadata = metadata.queueMetadata().ensure();
  if (queueTime.hasValue()) {
    queueMetadata.queueingTimeMs() = queueTime.value().count();
  } else {
    queueMetadata.queueingTimeMs() = -1;
  }

  queueMetadata.queueTimeoutMs() = queueTimeout_.value.count();

  if (checksum) {
    metadata.checksum() = *checksum;
  }

  return metadata;
}

} // namespace apache::thrift
