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
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>

namespace apache {
namespace thrift {
namespace rocket {

template <class T>
class FrameLengthParserStrategy {
 public:
  explicit FrameLengthParserStrategy(
      T& owner, size_t minBufferSize = 256, size_t maxBufferSize = 4096)
      : owner_(owner),
        minBufferSize_(minBufferSize),
        maxBufferSize_(maxBufferSize) {}
  ~FrameLengthParserStrategy();

  void getReadBuffer(void** bufReturn, size_t* lenReturn);
  void readDataAvailable(size_t len);
  void readBufferAvailable(std::unique_ptr<folly::IOBuf> buf);

  // Functions for testing
  size_t getFrameLength() { return frameLength_; }
  size_t getFrameLengthAndFieldSize() { return frameLengthAndFieldSize_; }
  size_t getSize() { return size_; }

 private:
  template <bool resize>
  FOLLY_ALWAYS_INLINE void drainReadBufQueue();

  FOLLY_ALWAYS_INLINE void computeFrameLength();
  FOLLY_ALWAYS_INLINE void resetFrameLength();
  FOLLY_ALWAYS_INLINE void incrSize(size_t delta) { size_ += delta; }
  FOLLY_ALWAYS_INLINE void tryResize();

  T& owner_;
  size_t size_{0};
  size_t frameLength_{0};
  size_t frameLengthAndFieldSize_{0};
  size_t minBufferSize_;
  size_t maxBufferSize_;
  folly::IOBufQueue readBufQueue_{folly::IOBufQueue::cacheChainLength()};
  folly::io::Cursor cursor_{readBufQueue_.front()};
};

} // namespace rocket
} // namespace thrift
} // namespace apache

#include <thrift/lib/cpp2/transport/rocket/framing/parser/FrameLengthParserStrategy-inl.h>
