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

#include <cstdint>

namespace apache::thrift::fast_thrift::rocket::server {

// ============================================================================
// Rocket Server Message Types
// ============================================================================

/**
 * RocketRequestMessage - Inbound message delivered to App.
 *
 * `frame` is the parsed wire frame from the codec.
 *
 * `streamType` is the originating REQUEST_* frame type that established
 * the stream (REQUEST_RESPONSE, REQUEST_STREAM, REQUEST_CHANNEL,
 * REQUEST_FNF). Stamped by RocketServerStreamStateHandler from its per-
 * stream map; downstream per-pattern handlers (RR/Stream/Channel/FNF)
 * use it as a stateless dispatch key.
 */
struct RocketRequestMessage {
  apache::thrift::fast_thrift::frame::read::ParsedFrame frame;
  uint32_t streamId{0};
  apache::thrift::fast_thrift::frame::FrameType streamType{
      apache::thrift::fast_thrift::frame::FrameType::RESERVED};
};

/**
 * RocketResponseMessage - Outbound response from App to StreamHandler.
 *
 * The `frame` field is a flat `ComposedFrame` that spans every RSocket
 * frame type via its `frameType` discriminator. Handlers carry it all
 * the way down to the codec; the codec is the single point that
 * serializes it into wire bytes via `frame.serialize()`.
 *
 * `streamType` is the originating REQUEST_* frame type for the stream
 * this response belongs to. App must copy it from the inbound
 * `RocketRequestMessage` so per-pattern handlers (RR/Stream/Channel/FNF)
 * can dispatch statelessly on the outbound path.
 */
struct RocketResponseMessage {
  apache::thrift::fast_thrift::frame::ComposedFrame frame;
  apache::thrift::fast_thrift::frame::FrameType streamType{
      apache::thrift::fast_thrift::frame::FrameType::RESERVED};
};

} // namespace apache::thrift::fast_thrift::rocket::server
