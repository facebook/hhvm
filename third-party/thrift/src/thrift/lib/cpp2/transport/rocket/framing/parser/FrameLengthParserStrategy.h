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
#include <thrift/lib/cpp2/transport/rocket/framing/FrameType.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Serializer.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Util.h>

namespace apache::thrift::rocket {

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
  bool isBufferMovable();

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

template <class T>
FrameLengthParserStrategy<T>::~FrameLengthParserStrategy() {
  if (frameLengthAndFieldSize_) {
    owner_.decMemoryUsage(frameLengthAndFieldSize_);
  }
}

template <class T>
void FrameLengthParserStrategy<T>::getReadBuffer(
    void** bufReturn, size_t* lenReturn) {
  auto tail = readBufQueue_.tailroom();
  if (tail < Serializer::kBytesForFrameOrMetadataLength) {
    const auto ret = readBufQueue_.preallocate(minBufferSize_, maxBufferSize_);
    *bufReturn = ret.first;
    *lenReturn = ret.second;
  } else {
    *bufReturn = readBufQueue_.writableTail();
    *lenReturn = tail;
  }
}

template <class T>
void FrameLengthParserStrategy<T>::readDataAvailable(size_t len) {
  incrSize(len);
  readBufQueue_.postallocate(len);
  drainReadBufQueue<true>();
}

template <class T>
void FrameLengthParserStrategy<T>::readBufferAvailable(
    std::unique_ptr<folly::IOBuf> buf) {
  incrSize(buf->computeChainDataLength());
  readBufQueue_.append(std::move(buf), true, true);
  drainReadBufQueue<false>();
}

template <class T>
bool FrameLengthParserStrategy<T>::isBufferMovable() {
  return true;
}

template <class T>
template <bool resize>
void FrameLengthParserStrategy<T>::drainReadBufQueue() {
  while (size_ >= Serializer::kBytesForFrameOrMetadataLength) {
    if (!frameLength_) {
      computeFrameLength();

      if (UNLIKELY(!owner_.incMemoryUsage(frameLengthAndFieldSize_))) {
        frameLengthAndFieldSize_ = 0;
        return;
      }

      if (resize) {
        tryResize();
      }
    }

    if (size_ < frameLengthAndFieldSize_) {
      return;
    }

    // skip frame length field
    readBufQueue_.trimStart(Serializer::kBytesForFrameOrMetadataLength);

    // split out frame
    auto frame = readBufQueue_.split(frameLength_);

    SCOPE_EXIT {
      // reset the frame length fields
      resetFrameLength();
    };

    // hand frame off
    owner_.handleFrame(std::move(frame));
  }
}

template <class T>
void FrameLengthParserStrategy<T>::computeFrameLength() {
  cursor_.reset(readBufQueue_.front());
  frameLength_ = readFrameOrMetadataSize(cursor_);
  frameLengthAndFieldSize_ =
      frameLength_ + Serializer::kBytesForFrameOrMetadataLength;
}

template <class T>
void FrameLengthParserStrategy<T>::resetFrameLength() {
  owner_.decMemoryUsage(frameLengthAndFieldSize_);
  size_ -= frameLengthAndFieldSize_;
  frameLength_ = 0;
  frameLengthAndFieldSize_ = 0;
}

template <class T>
void FrameLengthParserStrategy<T>::tryResize() {
  if (readBufQueue_.tailroom() < frameLength_) {
    auto max = std::max(frameLengthAndFieldSize_, maxBufferSize_);
    readBufQueue_.preallocate(minBufferSize_, max, max);
  }
}

} // namespace apache::thrift::rocket
