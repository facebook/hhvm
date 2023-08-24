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

#include <memory>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Serializer.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Util.h>

THRIFT_FLAG_DECLARE_int64(rocket_allocating_parser_min_buffer_size);

namespace apache {
namespace thrift {
namespace rocket {

/**
 * An extension to ParserStrategy that allocates a buffer using a std::allocator
 * rather than the folly::IOBuf chain based on the size of the frame. Does not
 * support relocatable IOBuf.
 * Type parameter 'T' is currently either `RocketClient` or
 * `RocketServerConnection`. Their corresponding `handleFrame` methods take care
 * the rest of rocket frame parsing.
 */
template <class T, class Allocator = std::allocator<std::uint8_t>>
class AllocatingParserStrategy {
  using Traits = std::allocator_traits<Allocator>;
  using ElemType = typename Traits::value_type;
  static_assert(
      sizeof(ElemType) == sizeof(std::uint8_t),
      "Passed in Allocator doesn't operate on bytes");

 public:
  explicit AllocatingParserStrategy(T& owner, Allocator allocator = Allocator())
      : owner_(owner),
        allocator_(allocator),
        minBufferSize_(THRIFT_FLAG(rocket_allocating_parser_min_buffer_size)) {
    DCHECK_GE(minBufferSize_, Serializer::kBytesForFrameOrMetadataLength);
  }
  ~AllocatingParserStrategy() {
    if (buffer_) {
      Traits::deallocate(allocator_, buffer_, currentBufferSize_);
    }
  }

  void getReadBuffer(void** bufReturn, size_t* lenReturn) {
    if (!buffer_) {
      buffer_ = Traits::allocate(allocator_, minBufferSize_);
      currentBufferSize_ = minBufferSize_;
    }

    *bufReturn = buffer_ + size_;
    *lenReturn = currentBufferSize_ - size_;
  }

  // At the end of each readDataAvailable(...) call. All completed frames are
  // handed over to owner via handleFrame call.
  void readDataAvailable(size_t len) {
    size_ += len;

    while (size_ >= Serializer::kBytesForFrameOrMetadataLength) {
      if (!frameLength_) {
        frameLength_ = computeFrameLength(0);
        frameLengthAndFieldSize_ =
            frameLength_ + Serializer::kBytesForFrameOrMetadataLength;
        tryResize();
      }

      // not enough data available to parse frameSize;
      if (size_ < frameLengthAndFieldSize_) {
        return;
      }

      size_t excessByteSize = size_ - frameLengthAndFieldSize_;
      size_t nextFrameLength = 0;
      size_t nextFrameLengthAndFieldSize = 0;

      // if excessByteSize contains enough data to parse next frame length, we
      // allocate a new buffer big enough to hold the new frame. copy
      // excessBytes into the new frame and handle current frame to owner_
      if (excessByteSize >= Serializer::kBytesForFrameOrMetadataLength) {
        nextFrameLength = computeFrameLength(frameLengthAndFieldSize_);
        nextFrameLengthAndFieldSize =
            nextFrameLength + Serializer::kBytesForFrameOrMetadataLength;
        if (UNLIKELY(!owner_.incMemoryUsage(nextFrameLengthAndFieldSize))) {
          return;
        }
      }
      size_t newBufferSize = std::max(
          {excessByteSize, minBufferSize_, nextFrameLengthAndFieldSize});
      ElemType* buffer = Traits::allocate(allocator_, newBufferSize);
      if (excessByteSize > 0) {
        std::uninitialized_copy(
            buffer_ + frameLengthAndFieldSize_, buffer_ + size_, buffer);
      }

      // mark buffer_ as staging and reset the state
      ElemType* stagingBuffer = buffer_;
      size_t stagingBufferSize = currentBufferSize_;
      size_t stagingFrameLengthAndFieldSize = frameLengthAndFieldSize_;
      size_t trimEnd = currentBufferSize_ - frameLengthAndFieldSize_;
      currentBufferSize_ = newBufferSize;
      size_ = excessByteSize;
      frameLength_ = nextFrameLength;
      frameLengthAndFieldSize_ = nextFrameLengthAndFieldSize;
      buffer_ = buffer;

      // TODO: T161472735 figure out how to avoid allocation here.
      auto* allocAndSize =
          new std::pair<Allocator&, size_t>(allocator_, stagingBufferSize);
      auto frame = folly::IOBuf::takeOwnership(
          stagingBuffer,
          stagingBufferSize,
          &deallocate,
          folly::bit_cast<void*>(allocAndSize));
      frame->trimStart(Serializer::kBytesForFrameOrMetadataLength);
      frame->trimEnd(trimEnd);
      owner_.handleFrame(std::move(frame));
      owner_.decMemoryUsage(stagingFrameLengthAndFieldSize);
    }
  }

  void readBufferAvailable(std::unique_ptr<folly::IOBuf>) {
    throw std::runtime_error(
        "not implemented - AllocatingParserStrategy doesn't support readBufferAvailable");
  }

  // Functions for testing
  size_t getMinBufferSize() const { return minBufferSize_; }
  size_t getCurrentBufferSize() const { return currentBufferSize_; }
  size_t getFrameLength() const { return frameLength_; }
  size_t getSize() const { return size_; }

 private:
  T& owner_;
  FOLLY_ATTR_NO_UNIQUE_ADDRESS Allocator allocator_;

  // size_ is number of bytes already written into currently allocated buffer.
  // size_ will be incremented when readDataAvailable(nbytes) is called.
  size_t size_{0};

  // currentBufferSize_ is the actual size of current allocated buffer.
  size_t currentBufferSize_{0};

  // rsocket frame size takes 3 bytes, when enough bytes are available, we parse
  // rocket frame length.
  size_t frameLength_{0};

  // when frameLength_ is available, frameLengthAndFieldSize_ is the
  // frameLength_ + 3 bytes (rsocket frame size takes 3 bytes).
  size_t frameLengthAndFieldSize_{0};

  // "minimum of allocated buffer size, should be configurable."
  size_t minBufferSize_;
  ElemType* buffer_{nullptr};

  // Compute rocket frame length using
  // Serializer::kBytesForFrameOrMetadataLength bytes from offset.
  size_t computeFrameLength(size_t offset) {
    DCHECK_GE(
        currentBufferSize_ - offset,
        Serializer::kBytesForFrameOrMetadataLength);
    auto bytes = folly::bit_cast<
        std::array<std::uint8_t, Serializer::kBytesForFrameOrMetadataLength>*>(
        buffer_ + offset);
    size_t frameSize = readFrameOrMetadataSize(*bytes);
    return frameSize;
  }

  void tryResize() {
    if (UNLIKELY(!owner_.incMemoryUsage(frameLengthAndFieldSize_))) {
      return;
    }
    if (currentBufferSize_ < frameLengthAndFieldSize_) {
      ElemType* buffer = Traits::allocate(allocator_, frameLengthAndFieldSize_);
      std::uninitialized_copy(buffer_, buffer_ + currentBufferSize_, buffer);
      Traits::deallocate(allocator_, buffer_, currentBufferSize_);
      currentBufferSize_ = frameLengthAndFieldSize_;
      buffer_ = buffer;
    }
  }

  static void deallocate(void* buf, void* userData) {
    auto* bufferPtr = static_cast<ElemType*>(buf);
    auto* allocAndSize =
        folly::bit_cast<std::pair<Allocator&, size_t>*>(userData);
    Traits::deallocate(allocAndSize->first, bufferPtr, allocAndSize->second);
    delete allocAndSize;
  }
};

} // namespace rocket
} // namespace thrift
} // namespace apache
