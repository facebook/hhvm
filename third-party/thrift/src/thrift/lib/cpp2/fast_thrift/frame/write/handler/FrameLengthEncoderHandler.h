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

#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Handler.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>

namespace apache::thrift::fast_thrift::frame::write::handler {

namespace detail {

/**
 * Write a 3-byte big-endian length value to a buffer.
 */
inline void writeFrameLength(uint8_t* buf, size_t length) noexcept {
  buf[0] = static_cast<uint8_t>((length >> 16) & 0xFF);
  buf[1] = static_cast<uint8_t>((length >> 8) & 0xFF);
  buf[2] = static_cast<uint8_t>(length & 0xFF);
}

} // namespace detail

/**
 * FrameLengthEncoderHandler - Pipeline handler that prepends the 3-byte
 * frame length prefix to outbound frames.
 *
 * This is the outbound counterpart to FrameLengthParserHandler. While
 * FrameLengthParserHandler strips the length prefix from inbound frames,
 * this handler adds the length prefix to outbound frames before they
 * are written to the transport.
 *
 * Pipeline position (outbound):
 *   ... -> RocketClientRequestResponseFrameHandler -> FrameLengthEncoderHandler
 * -> Transport
 *
 * Message flow:
 *   Input:  std::unique_ptr<folly::IOBuf> (frame without length prefix)
 *   Output: std::unique_ptr<folly::IOBuf> (frame with 3-byte length prefix)
 *
 * Frame format:
 *   [length: 3 bytes BE][frame payload: length bytes]
 */
class FrameLengthEncoderHandler {
 public:
  FrameLengthEncoderHandler() = default;

  // === HandlerLifecycle ===

  template <typename Context>
  void handlerAdded(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {}

  // === OutboundHandler ===

  template <typename Context>
  void onPipelineActivated(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onPipelineDeactivated(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}

  /**
   * Handle outbound frames - prepend 3-byte length prefix.
   */
  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onWrite(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto frame =
        msg.take<apache::thrift::fast_thrift::channel_pipeline::BytesPtr>();

    // Calculate frame length
    size_t frameLength = frame->computeChainDataLength();

    // Optimization: if we have enough headroom, write length prefix in-place
    if (frame->headroom() >= kMetadataLengthSize && !frame->isSharedOne()) {
      frame->prepend(kMetadataLengthSize);
      detail::writeFrameLength(frame->writableData(), frameLength);
      return ctx.fireWrite(
          apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
              std::move(frame)));
    }

    // Fallback: create new buffer for length prefix
    auto lengthPrefix = ctx.allocate(kMetadataLengthSize);
    detail::writeFrameLength(lengthPrefix->writableData(), frameLength);
    lengthPrefix->append(kMetadataLengthSize);

    // Chain the frame after the length prefix
    lengthPrefix->appendChain(std::move(frame));

    return ctx.fireWrite(
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(lengthPrefix)));
  }
};

static_assert(
    apache::thrift::fast_thrift::channel_pipeline::OutboundHandler<
        FrameLengthEncoderHandler,
        apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl>,
    "FrameLengthEncoderHandler must satisfy OutboundHandler concept");

} // namespace apache::thrift::fast_thrift::frame::write::handler
