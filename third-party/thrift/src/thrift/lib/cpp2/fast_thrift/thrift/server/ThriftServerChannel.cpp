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

#include <thrift/lib/cpp2/fast_thrift/thrift/server/ThriftServerChannel.h>

#include <folly/ExceptionString.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <thrift/lib/cpp2/async/RpcTypes.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponseError.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponseMetadata.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/transport/core/RpcMetadataUtil.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

namespace {

/**
 * Build PayloadMetadata from request context.
 * For declared exceptions: reads uex/uexw/exMeta headers and delegates to
 * the shared buildDeclaredExceptionPayloadMetadata helper.
 * For normal responses: returns PayloadResponseMetadata.
 */
apache::thrift::PayloadMetadata buildPayloadMetadataFromContext(
    apache::thrift::Cpp2RequestContext& ctx) {
  if (!ctx.isException()) {
    apache::thrift::PayloadMetadata payloadMetadata;
    payloadMetadata.responseMetadata() =
        apache::thrift::PayloadResponseMetadata();
    return payloadMetadata;
  }

  std::string exName;
  std::string exWhat;
  std::optional<apache::thrift::ErrorClassification> classification;
  auto* writeHeaders = &ctx.getHeader()->mutableWriteHeaders();

  static const auto uex = std::string(apache::thrift::detail::kHeaderUex);
  if (auto uexPtr = folly::get_ptr(*writeHeaders, uex)) {
    exName = std::move(*uexPtr);
    writeHeaders->erase(uex);
  }
  static const auto uexw = std::string(apache::thrift::detail::kHeaderUexw);
  if (auto uexwPtr = folly::get_ptr(*writeHeaders, uexw)) {
    exWhat = std::move(*uexwPtr);
    writeHeaders->erase(uexw);
  }
  static const auto exMeta = std::string(apache::thrift::detail::kHeaderExMeta);
  if (auto metaPtr = folly::get_ptr(*writeHeaders, exMeta)) {
    classification =
        apache::thrift::detail::deserializeErrorClassification(*metaPtr);
    writeHeaders->erase(exMeta);
  }

  return buildDeclaredExceptionPayloadMetadata(
      std::move(exName), std::move(exWhat), classification);
}

/**
 * PipelineResponseChannelRequest - ResponseChannelRequest that sends
 * responses back through the fast_thrift pipeline.
 *
 * This is a lightweight implementation that does NOT inherit from
 * ThriftRequestCore. It skips ServerConfigs, RequestsRegistry, and
 * timeout scheduling for minimal overhead.
 *
 * Owns the Cpp2RequestContext for lifetime management - the context
 * must remain valid until the processor calls sendReply/sendErrorWrapped.
 */
class PipelineResponseChannelRequest
    : public apache::thrift::ResponseChannelRequest {
 public:
  PipelineResponseChannelRequest(
      uint32_t streamId,
      apache::thrift::fast_thrift::channel_pipeline::PipelineImpl* pipeline,
      std::shared_ptr<std::atomic<bool>> pipelineAlive,
      bool isOneway,
      apache::thrift::Cpp2ConnContext* connContext,
      std::string methodName)
      : streamId_(streamId),
        pipeline_(pipeline),
        pipelineAlive_(std::move(pipelineAlive)),
        isOneway_(isOneway),
        reqCtx_(connContext, &header_, std::move(methodName)) {}

  bool isActive() const override { return active_.load(); }

  bool isOneway() const override { return isOneway_; }

  bool includeEnvelope() const override { return false; }

  void sendReply(
      apache::thrift::ResponsePayload&& response,
      apache::thrift::MessageChannel::SendCallback* /*cb*/,
      folly::Optional<uint32_t> /*crc32*/) override {
    if (!active_.exchange(false)) {
      return;
    }

    apache::thrift::ResponseRpcMetadata responseMetadata;
    responseMetadata.payloadMetadata() =
        buildPayloadMetadataFromContext(reqCtx_);

    apache::thrift::fast_thrift::thrift::ThriftServerResponseMessage msg{
        .payload =
            {
                .data = std::move(response).buffer(),
                .metadata = detail::serializeResponseMetadata(responseMetadata),
                .complete = true,
            },
        .streamId = streamId_,
        .errorCode = 0};

    if (!pipelineAlive_->load()) {
      XLOG(WARN) << "Pipeline destroyed, cannot send reply for stream "
                 << streamId_;
      return;
    }
    auto result = pipeline_->fireWrite(
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(msg)));
    if (result !=
        apache::thrift::fast_thrift::channel_pipeline::Result::Success) {
      XLOG(WARN) << "Pipeline write failed for stream " << streamId_;
    }
  }

  void sendErrorWrapped(
      folly::exception_wrapper ex, std::string exCode) override {
    if (!active_.exchange(false)) {
      return;
    }

    // Connection closing — silently drop (legacy Rocket behavior).
    if (exCode == "-1") {
      XLOG(INFO) << "Connection closing for stream " << streamId_;
      return;
    }

    if (!pipelineAlive_->load()) {
      XLOG(WARN) << "Pipeline destroyed, cannot send error for stream "
                 << streamId_;
      return;
    }

    // Check if this is an infrastructure error that should be sent as
    // an ERROR frame (matching the legacy Rocket server's behavior).
    auto errorCode = mapExCodeToErrorCode(exCode);
    if (errorCode) {
      // Refine QUEUE_OVERLOADED → SHUTDOWN for LOADSHEDDING exceptions.
      *errorCode = refineErrorCode(*errorCode, ex);

      auto error =
          serializeResponseRpcError(*errorCode, ex.what().toStdString());
      ThriftServerResponseMessage msg{
          .payload =
              ThriftServerResponsePayload{
                  .data = std::move(error.data),
                  .metadata = nullptr,
                  .complete = true},
          .streamId = streamId_,
          .errorCode = static_cast<uint32_t>(error.rocketErrorCode)};

      auto result = pipeline_->fireWrite(
          apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
              std::move(msg)));
      if (result !=
          apache::thrift::fast_thrift::channel_pipeline::Result::Success) {
        XLOG(WARN) << "Pipeline write failed for error on stream " << streamId_;
      }
      return;
    }

    // App-level error: send as PAYLOAD frame with exception metadata.
    apache::thrift::ErrorBlame blame = (exCode == kAppClientErrorCode)
        ? apache::thrift::ErrorBlame::CLIENT
        : apache::thrift::ErrorBlame::SERVER;

    std::string exName;
    auto* writeHeaders = &reqCtx_.getHeader()->mutableWriteHeaders();
    static const auto uexHeader =
        std::string(apache::thrift::detail::kHeaderUex);
    if (auto uexPtr = folly::get_ptr(*writeHeaders, uexHeader)) {
      exName = std::move(*uexPtr);
      writeHeaders->erase(uexHeader);
    }
    static const auto uexwHeader =
        std::string(apache::thrift::detail::kHeaderUexw);
    writeHeaders->erase(uexwHeader);
    static const auto exHeader = std::string(apache::thrift::detail::kHeaderEx);
    writeHeaders->erase(exHeader);

    ThriftServerResponseMessage msg{
        .payload =
            ThriftServerResponsePayload{
                .data = nullptr,
                .metadata = makeAppErrorResponseMetadata(
                    std::move(exName), ex.what().toStdString(), blame),
                .complete = true},
        .streamId = streamId_,
        .errorCode = 0};

    auto result = pipeline_->fireWrite(
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(msg)));
    if (result !=
        apache::thrift::fast_thrift::channel_pipeline::Result::Success) {
      XLOG(WARN) << "Pipeline write failed for error on stream " << streamId_;
    }
  }

  apache::thrift::Cpp2RequestContext* requestContext() { return &reqCtx_; }

 protected:
  bool tryStartProcessing() override { return true; }

 private:
  uint32_t streamId_;
  apache::thrift::fast_thrift::channel_pipeline::PipelineImpl* pipeline_;
  std::shared_ptr<std::atomic<bool>> pipelineAlive_;
  bool isOneway_;
  std::atomic<bool> active_{true};
  apache::thrift::transport::THeader header_;
  apache::thrift::Cpp2RequestContext reqCtx_;
};

} // namespace

ThriftServerChannel::ThriftServerChannel(
    std::shared_ptr<apache::thrift::AsyncProcessorFactory> processorFactory)
    : processorFactory_(std::move(processorFactory)),
      processor_(processorFactory_->getProcessor()) {}

ThriftServerChannel::~ThriftServerChannel() {
  pipelineAlive_->store(false);
  auto cb = closeCallback_.withWLock([](auto& fn) { return std::move(fn); });
  if (cb) {
    cb();
  }
}

void ThriftServerChannel::setCloseCallback(std::function<void()> cb) {
  closeCallback_.withWLock([&](auto& fn) { fn = std::move(cb); });
}

void ThriftServerChannel::setPipeline(
    apache::thrift::fast_thrift::channel_pipeline::PipelineImpl::Ptr pipeline) {
  pipelineAlive_->store(false);
  pipeline_ = pipeline.get();
  ownedPipeline_ = std::move(pipeline);
  pipelineAlive_->store(true);
}

void ThriftServerChannel::setPipelineRef(
    apache::thrift::fast_thrift::channel_pipeline::PipelineImpl& pipeline) {
  pipelineAlive_->store(false);
  ownedPipeline_.reset();
  pipeline_ = &pipeline;
  pipelineAlive_->store(true);
}

void ThriftServerChannel::setWorker(
    std::shared_ptr<apache::thrift::Cpp2Worker> worker) {
  worker_ = std::move(worker);
  connContext_ = apache::thrift::Cpp2ConnContext(
      nullptr, nullptr, nullptr, nullptr, nullptr, worker_.get(), 0);
}

apache::thrift::fast_thrift::channel_pipeline::Result
ThriftServerChannel::onRead(
    apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
        msg) noexcept {
  auto request = msg.take<ThriftServerRequestMessage>();

  if (!pipeline_) {
    XLOG(ERR) << "Pipeline not set";
    return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
  }

  // Deserialize request metadata from frame (on the stack, no heap alloc)
  apache::thrift::RequestRpcMetadata metadata;
  if (request.frame.hasMetadata() && request.frame.metadataSize() > 0) {
    try {
      apache::thrift::BinaryProtocolReader reader;
      reader.setInput(request.frame.metadataCursor());
      metadata.read(&reader);
    } catch (...) {
      XLOG(ERR) << "Failed to deserialize request metadata: "
                << folly::exceptionStr(std::current_exception());
      sendThriftError(
          request.streamId,
          apache::thrift::ResponseRpcErrorCode::REQUEST_PARSING_FAILURE,
          "Failed to deserialize request metadata");
      return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
    }
  }

  // Reject unsupported RPC kinds (streaming, sink, etc.)
  auto kindRef = metadata.kind();
  if (kindRef &&
      *kindRef != apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE &&
      *kindRef != apache::thrift::RpcKind::SINGLE_REQUEST_NO_RESPONSE) {
    XLOG(ERR) << "Unsupported RPC kind: " << static_cast<int>(*kindRef);
    if (request.streamId != 0) {
      sendThriftError(
          request.streamId,
          apache::thrift::ResponseRpcErrorCode::WRONG_RPC_KIND,
          "Unsupported RPC kind");
    }
    return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
  }

  // Extract method name from metadata
  std::string methodName;
  if (metadata.name().has_value()) {
    methodName = std::move(*metadata.name()).str();
  }

  // Extract protocol from metadata
  auto protocolId = apache::thrift::protocol::T_COMPACT_PROTOCOL;
  if (metadata.protocol().has_value()) {
    protocolId = static_cast<apache::thrift::protocol::PROTOCOL_TYPES>(
        *metadata.protocol());
  }

  // Extract data payload from frame (zero-copy)
  auto dataBuf = std::move(request.frame).extractData();
  if (!dataBuf) {
    dataBuf = pipeline_->allocate(0);
  }

  // Create SerializedCompressedRequest from data
  apache::thrift::SerializedCompressedRequest serializedRequest(
      apache::thrift::SerializedRequest(std::move(dataBuf)));

  // Detect oneway requests
  bool isOneway = kindRef &&
      *kindRef == apache::thrift::RpcKind::SINGLE_REQUEST_NO_RESPONSE;

  // Create ResponseChannelRequest (owns THeader and Cpp2RequestContext)
  auto channelRequest = std::unique_ptr<PipelineResponseChannelRequest>(
      new PipelineResponseChannelRequest(
          request.streamId,
          pipeline_,
          pipelineAlive_,
          isOneway,
          &connContext_,
          std::move(methodName)));

  auto* reqCtxPtr = channelRequest->requestContext();

  apache::thrift::ResponseChannelRequest::UniquePtr reqPtr(
      channelRequest.release());

  // Use WildcardMethodMetadata for dispatch
  apache::thrift::AsyncProcessorFactory::WildcardMethodMetadata
      wildcardMetadata;

  processor_->processSerializedCompressedRequestWithMetadata(
      std::move(reqPtr),
      std::move(serializedRequest),
      wildcardMetadata,
      protocolId,
      reqCtxPtr,
      pipeline_->eventBase(),
      nullptr /* tm */);

  return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
}

void ThriftServerChannel::sendThriftError(
    uint32_t streamId,
    apache::thrift::ResponseRpcErrorCode errorCode,
    const std::string& errorMessage) {
  auto error = serializeResponseRpcError(errorCode, std::string(errorMessage));
  ThriftServerResponseMessage msg{
      .payload =
          ThriftServerResponsePayload{
              .data = std::move(error.data),
              .metadata = nullptr,
              .complete = true},
      .streamId = streamId,
      .errorCode = static_cast<uint32_t>(error.rocketErrorCode)};

  auto result = pipeline_->fireWrite(
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(msg)));
  if (result !=
      apache::thrift::fast_thrift::channel_pipeline::Result::Success) {
    XLOG(WARN) << "Pipeline write failed for error on stream " << streamId;
  }
}

void ThriftServerChannel::onException(folly::exception_wrapper&& e) noexcept {
  XLOG(ERR) << "Pipeline exception: " << e.what();
  pipelineAlive_->store(false);
  auto cb = closeCallback_.withWLock([](auto& fn) { return std::move(fn); });
  if (cb && pipeline_) {
    // Defer the close callback so that onException fully completes before
    // the callback potentially removes the last shared_ptr to this channel
    // (from FastThriftServer::serverChannels_), destroying `this`.
    pipeline_->eventBase()->runInEventBaseThread(std::move(cb));
  }
}

} // namespace apache::thrift::fast_thrift::thrift
