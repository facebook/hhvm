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

#include <thrift/lib/cpp2/transport/core/ThriftRequest.h>

#include <thrift/lib/cpp2/GeneratedCodeHelper.h>

THRIFT_FLAG_DEFINE_int64(queue_time_logging_threshold_ms, 5);

namespace apache {
namespace thrift {

namespace detail {
THRIFT_PLUGGABLE_FUNC_REGISTER(
    void, handleFrameworkMetadata, std::unique_ptr<folly::IOBuf>&&) {}
THRIFT_PLUGGABLE_FUNC_REGISTER(
    bool,
    handleFrameworkMetadataHeader,
    folly::F14NodeMap<std::string, std::string>&) {
  return false;
}
} // namespace detail

ThriftRequestCore::ThriftRequestCore(
    server::ServerConfigs& serverConfigs,
    RequestRpcMetadata&& metadata,
    Cpp2ConnContext& connContext)
    : serverConfigs_(serverConfigs),
      kind_(metadata.kind_ref().value_or(
          RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE)),
      checksumRequested_(metadata.crc32c_ref().has_value()),
      loadMetric_(
          metadata.loadMetric_ref()
              ? folly::make_optional(std::move(*metadata.loadMetric_ref()))
              : folly::none),
      reqContext_(
          &connContext,
          &header_,
          metadata.name_ref() ? std::move(*metadata.name_ref()).str()
                              : std::string{}),
      queueTimeout_(serverConfigs_),
      taskTimeout_(serverConfigs_),
      stateMachine_(
          includeInRecentRequestsCount(reqContext_.getMethodName()),
          serverConfigs_.getAdaptiveConcurrencyController(),
          serverConfigs_.getCPUConcurrencyController()) {
  // Note that method name, RPC kind, and serialization protocol are validated
  // outside the ThriftRequestCore constructor.
  header_.setProtocolId(static_cast<int16_t>(
      metadata.protocol_ref().value_or(ProtocolId::BINARY)));

  if (auto clientTimeoutMs = metadata.clientTimeoutMs_ref()) {
    clientTimeout_ = std::chrono::milliseconds(*clientTimeoutMs);
    header_.setClientTimeout(clientTimeout_);
  }
  if (auto queueTimeoutMs = metadata.queueTimeoutMs_ref()) {
    clientQueueTimeout_ = std::chrono::milliseconds(*queueTimeoutMs);
    header_.setClientQueueTimeout(clientQueueTimeout_);
  }
  if (auto priority = metadata.priority_ref()) {
    header_.setCallPriority(static_cast<concurrency::PRIORITY>(*priority));
  }
  auto otherMetadata = metadata.otherMetadata_ref();
  // When processing ThriftFrameworkMetadata, the header takes priority.
  if (!otherMetadata ||
      !detail::handleFrameworkMetadataHeader(*otherMetadata)) {
    if (auto frameworkMetadata = metadata.frameworkMetadata_ref()) {
      DCHECK(*frameworkMetadata && !(**frameworkMetadata).empty());
      detail::handleFrameworkMetadata(std::move(*frameworkMetadata));
    }
  }
  if (otherMetadata) {
    header_.setReadHeaders(std::move(*otherMetadata));
  }
  if (auto clientId = metadata.clientId_ref()) {
    header_.setClientId(*clientId);
    header_.setReadHeader(transport::THeader::kClientId, std::move(*clientId));
  }
  if (auto serviceTraceMeta = metadata.serviceTraceMeta_ref()) {
    header_.setServiceTraceMeta(*serviceTraceMeta);
    header_.setReadHeader(
        transport::THeader::kServiceTraceMeta, std::move(*serviceTraceMeta));
  }

  // Store client's compression configs (if client explicitly requested
  // compression codec and size limit, use these settings to compress
  // response)
  if (auto compressionConfig = metadata.compressionConfig_ref()) {
    compressionConfig_ = *compressionConfig;
  }

  if (auto* observer = serverConfigs_.getObserver()) {
    observer->receivedRequest(&reqContext_.getMethodName());
  }
}

bool ThriftRequestCore::includeInRecentRequestsCount(
    const std::string_view methodName) {
  return util::includeInRecentRequestsCount(methodName);
}

ThriftRequestCore::RequestTimestampSample::RequestTimestampSample(
    server::TServerObserver::CallTimestamps& timestamps,
    server::TServerObserver* observer,
    MessageChannel::SendCallback* chainedCallback)
    : timestamps_(timestamps),
      observer_(observer),
      chainedCallback_(chainedCallback) {
  DCHECK(observer != nullptr);
}

void ThriftRequestCore::RequestTimestampSample::sendQueued() {
  timestamps_.writeBegin = std::chrono::steady_clock::now();
  if (chainedCallback_ != nullptr) {
    chainedCallback_->sendQueued();
  }
}

void ThriftRequestCore::RequestTimestampSample::messageSent() {
  SCOPE_EXIT { delete this; };
  timestamps_.writeEnd = std::chrono::steady_clock::now();
  if (chainedCallback_ != nullptr) {
    chainedCallback_->messageSent();
  }
}

void ThriftRequestCore::RequestTimestampSample::messageSendError(
    folly::exception_wrapper&& e) {
  SCOPE_EXIT { delete this; };
  timestamps_.writeEnd = std::chrono::steady_clock::now();
  if (chainedCallback_ != nullptr) {
    chainedCallback_->messageSendError(std::move(e));
  }
}

ThriftRequestCore::RequestTimestampSample::~RequestTimestampSample() {
  if (observer_) {
    observer_->callCompleted(timestamps_);
  }
}

MessageChannel::SendCallbackPtr ThriftRequestCore::prepareSendCallback(
    MessageChannel::SendCallbackPtr&& cb, server::TServerObserver* observer) {
  auto cbPtr = std::move(cb);
  // If we are sampling this call, wrap it with a RequestTimestampSample,
  // which also implements MessageChannel::SendCallback. Callers of
  // sendReply/sendError are responsible for cleaning up their own callbacks.
  auto& timestamps = getTimestamps();
  if (stateMachine_.getStartedProcessing() &&
      timestamps.getSamplingStatus().isEnabledByServer()) {
    auto chainedCallback = cbPtr.release();
    return MessageChannel::SendCallbackPtr(
        new ThriftRequestCore::RequestTimestampSample(
            timestamps, observer, chainedCallback));
  }
  return cbPtr;
}

void ThriftRequestCore::sendReplyInternal(
    ResponseRpcMetadata&& metadata,
    std::unique_ptr<folly::IOBuf> buf,
    MessageChannel::SendCallbackPtr cb) {
  if (checkResponseSize(*buf)) {
    cb = prepareSendCallback(std::move(cb), serverConfigs_.getObserver());
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
    if (UNLIKELY(timestamps.getSamplingStatus().isEnabled())) {
      timestamps.processEnd = std::chrono::steady_clock::now();
    }
    auto* observer = serverConfigs_.getObserver();
    if (!isOneway()) {
      auto metadata = makeResponseRpcMetadata(
          header_.extractAllWriteHeaders(),
          header_.extractProxiedPayloadMetadata());
      if (crc32c) {
        metadata.crc32c_ref() = *crc32c;
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
    if (UNLIKELY(timestamps.getSamplingStatus().isEnabled())) {
      timestamps.processEnd = std::chrono::steady_clock::now();
    }
    auto* observer = serverConfigs_.getObserver();
    if (!isOneway()) {
      auto metadata = makeResponseRpcMetadata(
          header_.extractAllWriteHeaders(),
          header_.extractProxiedPayloadMetadata());
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
    std::optional<ProxiedPayloadMetadata> proxiedPayloadMetadata) {
  ResponseRpcMetadata metadata;

  metadata.proxiedPayloadMetadata_ref().from_optional(proxiedPayloadMetadata);

  if (loadMetric_) {
    metadata.load_ref() = serverConfigs_.getLoad(*loadMetric_);
  }

  if (!writeHeaders.empty()) {
    metadata.otherMetadata_ref() = std::move(writeHeaders);
  }

  auto queueTime = stateMachine_.queueingTime();
  auto& queueMetadata = metadata.queueMetadata_ref().ensure();
  if (queueTime.hasValue()) {
    queueMetadata.queueingTimeMs_ref() = queueTime.value().count();
  } else {
    queueMetadata.queueingTimeMs_ref() = -1;
  }

  queueMetadata.queueTimeoutMs_ref() = queueTimeoutUsed_.count();

  return metadata;
}

} // namespace thrift
} // namespace apache
