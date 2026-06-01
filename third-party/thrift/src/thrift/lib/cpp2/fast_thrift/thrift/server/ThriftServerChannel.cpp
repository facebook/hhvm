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
#include <folly/io/IOBufQueue.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <thrift/lib/cpp2/async/RpcTypes.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Messages.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

namespace {

// Headroom reserved for downstream frame header serialization.
// The framing layer needs up to 9 bytes (6B base header + 3B metadata
// length). 16 bytes provides alignment margin.
constexpr size_t kMetadataHeadroomBytes = 16;

/**
 * Serialize ResponseRpcMetadata into IOBuf using Binary protocol.
 *
 * Pre-computes the serialized size to allocate a right-sized buffer,
 * and reserves headroom for downstream frame header serialization
 * (enabling the zero-alloc fast path in FrameWriter).
 */
std::unique_ptr<folly::IOBuf> serializeResponseMetadata(
    const apache::thrift::ResponseRpcMetadata& metadata) {
  apache::thrift::BinaryProtocolWriter writer;
  auto serializedSize = metadata.serializedSizeZC(&writer);
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  auto buf = folly::IOBuf::create(kMetadataHeadroomBytes + serializedSize);
  buf->advance(kMetadataHeadroomBytes);
  queue.append(std::move(buf));
  writer.setOutput(&queue);
  metadata.write(&writer);
  return queue.move();
}

/**
 * Build a RocketResponseMessage containing a serialized
 * TApplicationException with PayloadExceptionMetadata.
 */
apache::thrift::fast_thrift::rocket::server::RocketResponseMessage
buildErrorResponseMessage(
    uint32_t streamId,
    apache::thrift::protocol::PROTOCOL_TYPES protocolId,
    std::string errorMessage) {
  apache::thrift::TApplicationException tae(
      apache::thrift::TApplicationException::INTERNAL_ERROR, errorMessage);
  auto buf = apache::thrift::serializeErrorStruct(protocolId, tae);

  apache::thrift::ResponseRpcMetadata responseMetadata;
  apache::thrift::PayloadMetadata payloadMetadata;
  apache::thrift::PayloadExceptionMetadataBase exBase;
  apache::thrift::PayloadExceptionMetadata exMeta;
  exMeta.appUnknownException() =
      apache::thrift::PayloadAppUnknownExceptionMetdata();
  exBase.metadata() = std::move(exMeta);
  exBase.what_utf8() = std::move(errorMessage);
  payloadMetadata.exceptionMetadata() = std::move(exBase);
  responseMetadata.payloadMetadata() = std::move(payloadMetadata);

  return apache::thrift::fast_thrift::rocket::server::RocketResponseMessage{
      .payload = std::move(buf),
      .metadata = serializeResponseMetadata(responseMetadata),
      .streamId = streamId,
      .errorCode = 0,
      .complete = true};
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
      apache::thrift::protocol::PROTOCOL_TYPES protocolId,
      apache::thrift::Cpp2ConnContext* connContext,
      std::string methodName)
      : streamId_(streamId),
        pipeline_(pipeline),
        pipelineAlive_(std::move(pipelineAlive)),
        isOneway_(isOneway),
        protocolId_(protocolId),
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
    apache::thrift::PayloadMetadata payloadMetadata;
    payloadMetadata.responseMetadata() =
        apache::thrift::PayloadResponseMetadata();
    responseMetadata.payloadMetadata() = std::move(payloadMetadata);

    apache::thrift::fast_thrift::rocket::server::RocketResponseMessage msg{
        .payload = std::move(response).buffer(),
        .metadata = serializeResponseMetadata(responseMetadata),
        .streamId = streamId_,
        .errorCode = 0,
        .complete = true};

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
      folly::exception_wrapper ex, std::string /*exCode*/) override {
    if (!active_.exchange(false)) {
      return;
    }

    auto msg = buildErrorResponseMessage(
        streamId_, protocolId_, ex.what().toStdString());

    if (!pipelineAlive_->load()) {
      XLOG(WARN) << "Pipeline destroyed, cannot send error for stream "
                 << streamId_;
      return;
    }
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
  apache::thrift::protocol::PROTOCOL_TYPES protocolId_;
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
ThriftServerChannel::onMessage(
    apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
        msg) noexcept {
  auto request = msg.take<
      apache::thrift::fast_thrift::rocket::server::RocketRequestMessage>();

  if (request.error) {
    XLOG(ERR) << "Request error: " << request.error.what();
    if (pipeline_ && request.streamId != 0) {
      sendErrorResponse(
          request.streamId,
          apache::thrift::protocol::T_COMPACT_PROTOCOL,
          request.error.what().toStdString());
    }
    return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
  }

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
      sendErrorResponse(
          request.streamId,
          apache::thrift::protocol::T_COMPACT_PROTOCOL,
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
      auto protocolId = apache::thrift::protocol::T_COMPACT_PROTOCOL;
      if (metadata.protocol().has_value()) {
        protocolId = static_cast<apache::thrift::protocol::PROTOCOL_TYPES>(
            *metadata.protocol());
      }
      sendErrorResponse(request.streamId, protocolId, "Unsupported RPC kind");
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
          protocolId,
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

void ThriftServerChannel::sendErrorResponse(
    uint32_t streamId,
    apache::thrift::protocol::PROTOCOL_TYPES protocolId,
    const std::string& errorMessage) {
  auto msg = buildErrorResponseMessage(streamId, protocolId, errorMessage);

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
