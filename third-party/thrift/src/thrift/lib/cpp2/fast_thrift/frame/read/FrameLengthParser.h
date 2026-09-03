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

#include <algorithm>
#include <array>
#include <cstdint>

#include <folly/io/Cursor.h>
#include <folly/io/IOBufQueue.h>
#include <folly/lang/Hint.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/transport/Parser.h>

namespace apache::thrift::fast_thrift::frame::read {

/**
 * Parser for RSocket's 3-byte big-endian length prefix. Emits complete frames
 * with the prefix stripped.
 *
 * The socket reads straight into the tail of the same queue frames are split
 * out of, so a frame that arrives whole is neither copied nor allocated for
 * beyond the queue's current buffer.
 *
 * Buffer growth is driven by the wire rather than fixed up front: a read
 * refills only once the tail cannot hold another length prefix, so consecutive
 * reads land on pages that are already faulted in.
 *
 * A parsed length prefix does NOT cause the frame to be preallocated. tryResize
 * defers to IOBufQueue::preallocate, which returns the existing tail whenever
 * it holds at least minBufferSize, so a large frame accumulates across reads as
 * its payload actually arrives. That is deliberate: preallocating on the
 * announced length would let a peer claim maxFrameSize and pin that much per
 * connection before sending anything.
 */
class FrameLengthParser {
 public:
  static constexpr size_t kDefaultMinBufferSize = 256;
  static constexpr size_t kDefaultMaxBufferSize = 4096;
  static constexpr size_t kDefaultMaxFrameSize = 16 * 1024 * 1024; // 16MB

  explicit FrameLengthParser(
      size_t minBufferSize = kDefaultMinBufferSize,
      size_t maxBufferSize = kDefaultMaxBufferSize,
      size_t maxFrameSize = kDefaultMaxFrameSize) noexcept
      : minBufferSize_(minBufferSize),
        maxBufferSize_(maxBufferSize),
        maxFrameSize_(maxFrameSize) {}

  void getReadBuffer(void** bufReturn, size_t* lenReturn) noexcept {
    // Refill only when the tail cannot hold another length prefix; everything
    // above that is usable read space and reusing it keeps consecutive reads
    // on the same already-faulted pages.
    auto tailroom = readBufQueue_.tailroom();
    if (tailroom < kMetadataLengthSize) {
      auto ret = readBufQueue_.preallocate(minBufferSize_, maxBufferSize_);
      *bufReturn = ret.first;
      *lenReturn = ret.second;
    } else {
      *bufReturn = readBufQueue_.writableTail();
      *lenReturn = tailroom;
    }
  }

  template <typename Sink>
  channel_pipeline::Result consume(size_t len, Sink&& sink) noexcept {
    size_ += len;
    readBufQueue_.postallocate(len);
    return drain</*mayResize=*/true>(sink);
  }

  template <typename Sink>
  channel_pipeline::Result consumeBuffer(
      channel_pipeline::BytesPtr buf, Sink&& sink) noexcept {
    DCHECK(buf);
    size_ += buf->computeChainDataLength();
    readBufQueue_.append(
        std::move(buf), /*pack=*/true, /*allowTailReuse=*/true);
    // The bytes came from elsewhere (io_uring, replayed data), so growing our
    // own tail for the pending frame would only allocate space nothing reads
    // into.
    return drain</*mayResize=*/false>(sink);
  }

  // Routes the queue's own allocations through the pipeline allocator.
  // preallocate consults the factory only when it has to grow, so neither the
  // tail-reuse path nor a frame that fits the current buffer pays for it.
  void setIOBufFactory(folly::IOBufFactory* factory) noexcept {
    readBufQueue_.setIOBufFactory(factory);
  }

  void reset() noexcept {
    readBufQueue_.reset();
    size_ = 0;
    frameLength_ = 0;
    frameLengthAndFieldSize_ = 0;
  }

  // === Accessors for testing ===

  size_t frameLength() const noexcept { return frameLength_; }

  size_t frameLengthAndFieldSize() const noexcept {
    return frameLengthAndFieldSize_;
  }

  size_t size() const noexcept { return size_; }

 private:
  template <bool mayResize, typename Sink>
  FOLLY_ALWAYS_INLINE channel_pipeline::Result drain(Sink& sink) noexcept {
    // size_ shadows readBufQueue_.chainLength(), which is not a plain load on
    // a length-caching queue. Kept in step by hand, so assert it in dev.
    DCHECK_EQ(size_, readBufQueue_.chainLength());
    while (size_ >= kMetadataLengthSize) {
      if (!frameLength_) {
        computeFrameLength();

        // Reject before growing: an oversized prefix would otherwise make us
        // allocate exactly the size we are about to refuse.
        if (FOLLY_UNLIKELY(frameLength_ > maxFrameSize_)) {
          return channel_pipeline::Result::Error;
        }

        if constexpr (mayResize) {
          tryResize();
        }
      }

      if (size_ < frameLengthAndFieldSize_) {
        return channel_pipeline::Result::Success;
      }

      readBufQueue_.trimStart(kMetadataLengthSize);
      auto frame = readBufQueue_.split(frameLength_);

      // Clear framing state before firing; the sink may re-enter.
      size_ -= frameLengthAndFieldSize_;
      frameLength_ = 0;
      frameLengthAndFieldSize_ = 0;

      auto result = sink(std::move(frame));
      if (result != channel_pipeline::Result::Success) {
        // Backpressure: the frame was accepted but downstream wants us to
        // stop. Error: give up. Either way the remaining bytes stay queued.
        return result;
      }
    }

    return channel_pipeline::Result::Success;
  }

  void computeFrameLength() noexcept {
    folly::io::Cursor cursor{readBufQueue_.front()};
    std::array<uint8_t, kMetadataLengthSize> bytes{};
    cursor.pull(bytes.data(), bytes.size());
    frameLength_ = (static_cast<size_t>(bytes[0]) << 16) |
        (static_cast<size_t>(bytes[1]) << 8) | static_cast<size_t>(bytes[2]);
    frameLengthAndFieldSize_ = frameLength_ + kMetadataLengthSize;
  }

  void tryResize() noexcept {
    if (readBufQueue_.tailroom() < frameLength_) {
      auto max = std::max(frameLengthAndFieldSize_, maxBufferSize_);
      readBufQueue_.preallocate(minBufferSize_, max, max);
    }
  }

  size_t size_{0};
  size_t frameLength_{0};
  size_t frameLengthAndFieldSize_{0};
  size_t minBufferSize_;
  size_t maxBufferSize_;
  size_t maxFrameSize_;

  folly::IOBufQueue readBufQueue_{folly::IOBufQueue::cacheChainLength()};
};

static_assert(transport::Parser<FrameLengthParser>);

} // namespace apache::thrift::fast_thrift::frame::read
