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

#include <folly/io/Cursor.h>
#include <folly/io/IOBufQueue.h>
#include <folly/lang/Hint.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Handler.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>

namespace apache::thrift::fast_thrift::frame::read::handler {

/**
 * FrameLengthParserHandler - Pipeline handler that parses RSocket frames.
 *
 * This handler implements the InboundHandler concept from Channel Pipeline.
 * It receives raw IOBuf bytes from the transport adapter, buffers them,
 * extracts complete frames based on the 3-byte length prefix, and fires
 * the frames (with length prefix stripped) downstream.
 *
 * Usage:
 *   auto pipeline = PipelineBuilder()
 *       .addNextInbound(frame_parser_tag,
 * std::make_unique<FrameLengthParserHandler>())
 * .addNextDuplex(frame_decoder_tag, std::make_unique<FrameDecoder>()) .build();
 *
 * Input: BytesPtr (raw bytes from transport)
 * Output: BytesPtr (complete RSocket frames, length prefix stripped)
 *
 * Backpressure:
 *   - If downstream returns Backpressure, this handler stops extracting frames
 *     and returns Backpressure to the transport adapter
 *   - Buffered bytes are preserved for the next onRead call
 */
class FrameLengthParserHandler {
 public:
  static constexpr size_t kDefaultMaxFrameSize = 16 * 1024 * 1024; // 16MB

  explicit FrameLengthParserHandler(
      size_t maxFrameSize = kDefaultMaxFrameSize) noexcept
      : maxFrameSize_(maxFrameSize) {}

  // === HandlerLifecycle ===

  template <typename Context>
  void handlerAdded(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {
    readBufQueue_.reset();
    size_ = 0;
    frameLength_ = 0;
    frameLengthAndFieldSize_ = 0;
  }

  // === InboundHandler ===
  template <typename Context>
  void onPipelineActivated(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onReadReady(Context& /*ctx*/) noexcept {}

  template <typename Context>
  FOLLY_ALWAYS_INLINE apache::thrift::fast_thrift::channel_pipeline::Result
  onRead(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto buf =
        msg.take<apache::thrift::fast_thrift::channel_pipeline::BytesPtr>();

    size_ += buf->computeChainDataLength();
    readBufQueue_.append(std::move(buf), true, true);

    return drainReadBufQueue(ctx);
  }

  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper e) noexcept {
    ctx.fireException(std::move(e));
  }

  // === Accessors for testing ===

  size_t frameLength() const noexcept { return frameLength_; }

  size_t frameLengthAndFieldSize() const noexcept {
    return frameLengthAndFieldSize_;
  }

  size_t size() const noexcept { return size_; }

 private:
  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result drainReadBufQueue(
      Context& ctx) noexcept {
    while (size_ >= kMetadataLengthSize) {
      if (!frameLength_) {
        computeFrameLength();

        // Reject frames that exceed the maximum allowed size
        if (FOLLY_UNLIKELY(frameLength_ > maxFrameSize_)) {
          ctx.fireException(
              folly::make_exception_wrapper<std::runtime_error>(
                  "Frame size exceeds maximum allowed size"));
          return apache::thrift::fast_thrift::channel_pipeline::Result::Error;
        }
      }

      if (size_ < frameLengthAndFieldSize_) {
        return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
      }

      // Strip the length prefix and extract the frame
      readBufQueue_.trimStart(kMetadataLengthSize);
      auto frame = readBufQueue_.split(frameLength_);

      // Update accounting before firing (in case of exception)
      size_ -= frameLengthAndFieldSize_;
      frameLength_ = 0;
      frameLengthAndFieldSize_ = 0;

      // Fire the complete frame downstream
      auto result = ctx.fireRead(
          apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
              std::move(frame)));

      if (result !=
          apache::thrift::fast_thrift::channel_pipeline::Result::Success) {
        // Backpressure: frame was accepted, but downstream wants us to pause.
        // Stop extracting more frames; remaining bytes stay buffered until
        // the next onRead() call.
        // Error: stop processing entirely.
        return result;
      }
    }

    return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
  }

  void computeFrameLength() noexcept {
    folly::io::Cursor cursor{readBufQueue_.front()};
    std::array<uint8_t, kMetadataLengthSize> bytes;
    cursor.pull(bytes.data(), bytes.size());
    frameLength_ = (static_cast<size_t>(bytes[0]) << 16) |
        (static_cast<size_t>(bytes[1]) << 8) | static_cast<size_t>(bytes[2]);
    frameLengthAndFieldSize_ = frameLength_ + kMetadataLengthSize;
  }

  size_t size_{0};
  size_t frameLength_{0};
  size_t frameLengthAndFieldSize_{0};
  size_t maxFrameSize_;

  folly::IOBufQueue readBufQueue_{folly::IOBufQueue::cacheChainLength()};
};

static_assert(
    apache::thrift::fast_thrift::channel_pipeline::InboundHandler<
        FrameLengthParserHandler,
        apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl>,
    "RocketClientRequestResponseFrameHandler must satisfy InboundHandler concept");

} // namespace apache::thrift::fast_thrift::frame::read::handler
