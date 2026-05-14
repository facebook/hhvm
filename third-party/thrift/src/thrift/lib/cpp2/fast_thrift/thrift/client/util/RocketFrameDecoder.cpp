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

#include <thrift/lib/cpp2/fast_thrift/thrift/client/util/RocketFrameDecoder.h>

#include <fmt/core.h>

#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameViews.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ResponseMetadata.h>

namespace apache::thrift::fast_thrift::thrift {

folly::Expected<ThriftClientInboundPayloadVariant, folly::exception_wrapper>
fromRocketFrame(
    apache::thrift::fast_thrift::frame::read::ParsedFrame&& frame,
    apache::thrift::RpcKind kind) {
  using apache::thrift::fast_thrift::frame::FrameType;

  const auto streamId = frame.streamId();
  const auto frameType = frame.type();

  switch (frameType) {
    case FrameType::PAYLOAD: {
      if (kind != apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE) {
        // Stream / Sink / Bidi PAYLOAD routing requires per-stream first-
        // vs-subsequent state, which doesn't flow through this function
        // yet. Will be plumbed when stream wiring lands.
        return folly::makeUnexpected(
            folly::make_exception_wrapper<
                apache::thrift::TApplicationException>(fmt::format(
                "fromRocketFrame: PAYLOAD on RpcKind={} not supported yet",
                static_cast<int>(kind))));
      }

      // RR / Sink invariants enforced upstream (kind check above): the wire
      // frame must be terminal + carry data. Drop the wire flag reads;
      // ThriftInitialResponsePayload bakes complete=next=true into
      // toRocketFrame.
      auto metadata = std::make_unique<apache::thrift::ResponseRpcMetadata>();
      if (auto ew = deserializeResponseMetadata(frame, *metadata)) {
        return folly::makeUnexpected(std::move(ew));
      }
      auto data = std::move(frame).extractData();

      return ThriftClientInboundPayloadVariant{
          ThriftInitialResponsePayload{
              .data = std::move(data),
              .metadata = std::move(metadata),
              .streamId = streamId},
          kind};
    }

    case FrameType::ERROR: {
      apache::thrift::fast_thrift::frame::read::ErrorView view(frame);
      const auto errorCode = view.errorCode();
      auto data = std::move(frame).extractData();

      return ThriftClientInboundPayloadVariant{
          ThriftErrorPayload{
              .data = std::move(data),
              .metadata = nullptr,
              .streamId = streamId,
              .errorCode = errorCode},
          kind};
    }

    case FrameType::CANCEL: {
      return ThriftClientInboundPayloadVariant{
          ThriftCancelPayload{.streamId = streamId}, kind};
    }

    case FrameType::REQUEST_N: {
      apache::thrift::fast_thrift::frame::read::RequestNView view(frame);
      return ThriftClientInboundPayloadVariant{
          ThriftRequestNPayload{
              .streamId = streamId, .requestN = view.requestN()},
          kind};
    }

    case FrameType::METADATA_PUSH: {
      // Connection-level frame (streamId=0). The entire payload IS the
      // metadata (no length prefix on the wire — see FrameParser
      // METADATA_PUSH branch). Trim the rocket header and surface the
      // remaining buffer chain as the metadata IOBuf.
      auto trimAmount = frame.metadata.payloadOffset;
      auto buf = std::move(frame).getUnderlyingBuffer();
      while (trimAmount > 0 && buf) {
        if (buf->length() > trimAmount) {
          buf->trimStart(trimAmount);
          trimAmount = 0;
        } else {
          trimAmount -= buf->length();
          buf = buf->pop();
        }
      }
      return ThriftClientInboundPayloadVariant{
          ThriftMetadataPushPayload{.metadata = std::move(buf)}, kind};
    }

    // Frame types that don't reach the thrift inbound bridge (handled
    // earlier in the rocket pipeline, or only valid on the request path).
    case FrameType::RESERVED:
    case FrameType::SETUP:
    case FrameType::LEASE:
    case FrameType::KEEPALIVE:
    case FrameType::REQUEST_RESPONSE:
    case FrameType::REQUEST_FNF:
    case FrameType::REQUEST_STREAM:
    case FrameType::REQUEST_CHANNEL:
    case FrameType::RESUME:
    case FrameType::RESUME_OK:
    case FrameType::EXT:
      break;
  }
  return folly::makeUnexpected(
      folly::make_exception_wrapper<apache::thrift::TApplicationException>(
          fmt::format(
              "fromRocketFrame: unsupported frame type {}",
              static_cast<int>(frameType))));
}

} // namespace apache::thrift::fast_thrift::thrift
