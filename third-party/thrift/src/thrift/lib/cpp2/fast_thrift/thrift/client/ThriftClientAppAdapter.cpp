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

#include <thrift/lib/cpp2/fast_thrift/thrift/client/ThriftClientAppAdapter.h>

#include <fmt/core.h>
#include <folly/Expected.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/RequestMetadata.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/common/ThriftRequestContext.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/util/ErrorDecoding.h>
#include <thrift/lib/cpp2/util/ManagedStringView.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

void ThriftClientAppAdapter::sendRequestResponse(
    const apache::thrift::RpcOptions& rpcOptions,
    std::string_view methodName,
    apache::thrift::RpcKind rpcKind,
    std::unique_ptr<folly::IOBuf> data,
    RequestResponseHandler handler) noexcept {
  if (FOLLY_UNLIKELY(!pipeline_)) {
    handleMissingPipeline(std::move(handler));
    return;
  }

  auto metadata = makeRequestMetadata(
      rpcOptions,
      apache::thrift::ManagedStringView::from_static(methodName),
      rpcKind,
      static_cast<apache::thrift::ProtocolId>(protocolId_));

  ThriftRequestMessage msg{
      .payload =
          ThriftRequestResponsePayload{
              .data = std::move(data),
              .metadata = std::move(metadata),
          },
      .requestContext = apache::thrift::fast_thrift::rocket::from_unique_ptr(
          std::make_unique<apache::thrift::fast_thrift::thrift::client::
                               ThriftRequestContext>(std::move(handler))),
  };

  submitWrite(std::move(msg));
}

apache::thrift::fast_thrift::channel_pipeline::Result
ThriftClientAppAdapter::onRead(
    apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
        msg) noexcept {
  auto response = msg.take<ThriftResponseMessage>();
  auto ctx = response.requestContext.release_as<
      apache::thrift::fast_thrift::thrift::client::ThriftRequestContext>();
  if (FOLLY_UNLIKELY(!ctx)) {
    handleNullContext();
    return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
  }

  auto handler = std::move(ctx->handler);
  ctx.reset();

  // Per-request error from below (e.g., rocket in-process serialize
  // failure). Fail just this handler; channel stays Open for subsequent
  // requests.
  if (FOLLY_UNLIKELY(response.payload.is<ThriftClientResponseError>())) {
    handleResponseError(
        std::move(handler),
        std::move(response.payload.get<ThriftClientResponseError>().ew));
    return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
  }

  DCHECK(
      response.streamType ==
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE)
      << "Unsupported frame type: " << static_cast<int>(response.streamType);

  // Read stats before `response` is consumed by handleRequestResponse.
  const apache::thrift::RpcTransportStats rpcTransportStats =
      response.rpcTransportStats;
  handler(
      handleRequestResponse(std::move(response), protocolId_),
      rpcTransportStats);
  return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
}

void ThriftClientAppAdapter::onPipelineActive() noexcept {
  state_ = State::Open;
  lastError_ = {};
}

void ThriftClientAppAdapter::onPipelineInactive() noexcept {
  // Pipeline disconnect is the canonical "no longer accepting writes"
  // edge. Whether we got here via a graceful close or a hard error, the
  // state machine ends up Closed.
  state_ = State::Closed;
}

void ThriftClientAppAdapter::onException(
    folly::exception_wrapper&& e) noexcept {
  XLOG(ERR) << "Pipeline exception: " << e.what();
  if (state_ == State::Closed) {
    return;
  }
  if (state_ == State::Open) {
    lastError_ = e;
  }

  auto* tex = e.get_exception<apache::thrift::transport::TTransportException>();
  if (tex &&
      tex->getType() ==
          apache::thrift::transport::TTransportException::NOT_OPEN) {
    // CONNECTION_CLOSE: graceful drain — reject new writes, let inflight
    // responses complete. The eventual onPipelineInactive from the
    // transport side will drive us to Closed.
    state_ = State::Closing;
    return;
  }

  // Hard error: stop accepting new writes immediately. Transport will
  // drive the structural teardown via onPipelineInactive.
  state_ = State::Closed;
}

void ThriftClientAppAdapter::submitWrite(ThriftRequestMessage msg) noexcept {
  auto* eb = pipeline_->eventBase();
  if (eb->isInEventBaseThread()) {
    submitWriteOnEventBase(std::move(msg));
  } else {
    eb->runInEventBaseThread([this, msg = std::move(msg)]() mutable {
      submitWriteOnEventBase(std::move(msg));
    });
  }
}

void ThriftClientAppAdapter::submitWriteOnEventBase(
    ThriftRequestMessage msg) noexcept {
  if (FOLLY_UNLIKELY(state_ != State::Open)) {
    handleNotOpen(std::move(msg));
    return;
  }
  auto result = pipeline_->fireWrite(
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(msg)));
  if (FOLLY_UNLIKELY(
          result ==
          apache::thrift::fast_thrift::channel_pipeline::Result::Error)) {
    handleWriteError();
  }
}

void ThriftClientAppAdapter::handleMissingPipeline(
    RequestResponseHandler handler) noexcept {
  handler(
      folly::makeUnexpected(
          folly::make_exception_wrapper<apache::thrift::TApplicationException>(
              apache::thrift::TApplicationException::INTERNAL_ERROR,
              "Pipeline not set")),
      apache::thrift::RpcTransportStats{});
}

void ThriftClientAppAdapter::handleNullContext() noexcept {
  XLOG(WARN) << "Response for unknown stream (null requestContext)";
}

void ThriftClientAppAdapter::handleResponseError(
    RequestResponseHandler handler, folly::exception_wrapper ew) noexcept {
  handler(
      folly::makeUnexpected(std::move(ew)),
      apache::thrift::RpcTransportStats{});
}

void ThriftClientAppAdapter::handleNotOpen(ThriftRequestMessage msg) noexcept {
  // Recover the handler from the context so we can deliver the
  // not-open error directly without ever entering the pipeline.
  auto ctx = msg.requestContext.release_as<
      apache::thrift::fast_thrift::thrift::client::ThriftRequestContext>();
  if (FOLLY_LIKELY(ctx != nullptr)) {
    ctx->handler(
        folly::makeUnexpected(
            folly::make_exception_wrapper<
                apache::thrift::transport::TTransportException>(
                apache::thrift::transport::TTransportException::NOT_OPEN,
                lastError_ ? fmt::format(
                                 "Connection not open: {}",
                                 lastError_.what().toStdString())
                           : "Connection not open")),
        apache::thrift::RpcTransportStats{});
  }
}

void ThriftClientAppAdapter::handleWriteError() noexcept {
  // Close the channel. Subsequent sends will be rejected; pending
  // streams will be failed.
  onException(
      folly::make_exception_wrapper<
          apache::thrift::transport::TTransportException>(
          apache::thrift::transport::TTransportException::UNKNOWN,
          "Failed to write request to pipeline"));
}

} // namespace apache::thrift::fast_thrift::thrift
