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

#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrameVariant.h>

#include <folly/ExceptionWrapper.h>

#include <cstdint>

namespace apache::thrift::fast_thrift::rocket::server {

// ============================================================================
// Rocket Server Message Types
// ============================================================================

/**
 * RocketRequestMessage - Inbound request from client, delivered to App.
 *
 * Contains the client-assigned streamId and the parsed request frame.
 * App uses streamId to correlate responses back to this request.
 * Use FrameView to check frame type (REQUEST_RESPONSE, REQUEST_STREAM,
 * CANCEL, etc.).
 *
 * `streamType` is the originating REQUEST_* frame type that established
 * the stream (REQUEST_RESPONSE, REQUEST_STREAM, REQUEST_CHANNEL,
 * REQUEST_FNF). Stamped by RocketServerStreamStateHandler from its per-
 * stream map; downstream per-pattern handlers (RR/Stream/Channel/FNF)
 * use it as a stateless dispatch key.
 *
 * If error is set, the connection failed and frame may be empty.
 * App layer should check error first before processing frame.
 */
struct RocketRequestMessage {
  apache::thrift::fast_thrift::frame::read::ParsedFrame frame;
  folly::exception_wrapper error;
  uint32_t streamId{0};
  apache::thrift::fast_thrift::frame::FrameType streamType{
      apache::thrift::fast_thrift::frame::FrameType::RESERVED};
};

/**
 * RocketResponseMessage - Outbound response from App to StreamHandler.
 *
 * The `frame` field is a typed variant of per-frame payload structs from
 * `frame::ComposedFrame.h`. Handlers carry the typed payload all the way
 * down to the codec; the codec is the single point that serializes the
 * held payload into wire bytes via `frame.serialize()`. The 3-API surface
 * (streamId / frameType / serialize) dispatches inline with no
 * `std::visit` at any call site.
 *
 * `streamId` is exposed by `frame.streamId()`; "complete" semantics live
 * inside the held header (`ComposedPayloadFrame::header.complete`;
 * ERROR is implicitly terminal).
 *
 * `streamType` is the originating REQUEST_* frame type for the stream
 * this response belongs to. App must copy it from the inbound
 * `RocketRequestMessage` so per-pattern handlers (RR/Stream/Channel/FNF)
 * can dispatch statelessly on the outbound path.
 *
 * As STREAM/CHANNEL/FNF response frame types get wired up, their
 * corresponding `Composed*Frame` types will join the variant.
 */
struct RocketResponseMessage {
  apache::thrift::fast_thrift::frame::ComposedFrameVariant<
      apache::thrift::fast_thrift::frame::ComposedPayloadFrame,
      apache::thrift::fast_thrift::frame::ComposedErrorFrame>
      frame;
  apache::thrift::fast_thrift::frame::FrameType streamType{
      apache::thrift::fast_thrift::frame::FrameType::RESERVED};
};

} // namespace apache::thrift::fast_thrift::rocket::server
