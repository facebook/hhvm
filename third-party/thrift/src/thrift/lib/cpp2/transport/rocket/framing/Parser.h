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

#include <chrono>
#include <utility>

#include <folly/ExceptionWrapper.h>
#include <folly/Function.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/AsyncTransport.h>

#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/RpcOptions.h>

THRIFT_FLAG_DECLARE_int64(rocket_parser_resize_period_seconds);
THRIFT_FLAG_DECLARE_bool(rocket_parser_dont_hold_buffer_enabled);
THRIFT_FLAG_DECLARE_bool(rocket_parser_hybrid_buffer_enabled);

namespace apache {
namespace thrift {
namespace rocket {

template <class T>
class Parser final : public folly::AsyncTransport::ReadCallback,
                     public folly::HHWheelTimer::Callback {
 public:
  explicit Parser(
      T& owner,
      std::chrono::milliseconds resizeBufferTimeout =
          kDefaultBufferResizeInterval)
      : newBufferLogicEnabled_(
            THRIFT_FLAG(rocket_parser_dont_hold_buffer_enabled)),
        hybridBufferLogicEnabled_(
            THRIFT_FLAG(rocket_parser_hybrid_buffer_enabled) &&
            !newBufferLogicEnabled_),
        owner_(owner),
        resizeBufferTimeout_(resizeBufferTimeout),
        periodicResizeBufferTimeout_(
            THRIFT_FLAG(rocket_parser_resize_period_seconds)),
        readBuffer_(
            folly::IOBuf::CreateOp(),
            hybridBufferLogicEnabled_ ? kStaticBufferSize : bufferSize_) {}

  ~Parser() override {
    if (currentFrameLength_) {
      owner_.decMemoryUsage(currentFrameLength_);
    }
  }

  // AsyncTransport::ReadCallback implementation
  FOLLY_NOINLINE void getReadBuffer(void** bufout, size_t* lenout) override;
  FOLLY_NOINLINE void readDataAvailable(size_t nbytes) noexcept override;
  FOLLY_NOINLINE void readEOF() noexcept override;
  FOLLY_NOINLINE void readErr(
      const folly::AsyncSocketException&) noexcept override;
  FOLLY_NOINLINE void readBufferAvailable(
      std::unique_ptr<folly::IOBuf> /*readBuf*/) noexcept override;

  bool isBufferMovable() noexcept override { return true; }

  // TODO: This should be removed once the new buffer logic controlled by
  // THRIFT_FLAG(rocket_parser_dont_hold_buffer_enabled) is stable.
  void timeoutExpired() noexcept override;

  // TODO: This should be removed once the new buffer logic controlled by
  // THRIFT_FLAG(rocket_parser_dont_hold_buffer_enabled) is stable.
  const folly::IOBuf& getReadBuffer() const { return readBuffer_; }

  // TODO: This should be removed once the new buffer logic controlled by
  // THRIFT_FLAG(rocket_parser_dont_hold_buffer_enabled) is stable.
  void setReadBuffer(folly::IOBuf&& buffer) { readBuffer_ = std::move(buffer); }

  size_t getReadBufferSize() const { return bufferSize_; }

  void setReadBufferSize(size_t size) { bufferSize_ = size; }

  // TODO: This should be removed once the new buffer logic controlled by
  // THRIFT_FLAG(rocket_parser_dont_hold_buffer_enabled) is stable.
  void resizeBuffer();

  size_t getReadBufLength() const {
    if (newBufferLogicEnabled_) {
      return readBufQueue_.chainLength();
    } else if (hybridBufferLogicEnabled_) {
      return dynamicBuffer_ ? dynamicBuffer_->length() : readBuffer_.length();
    }
    return readBuffer_.computeChainDataLength();
  }

  bool getNewBufferLogicEnabled() const { return newBufferLogicEnabled_; }

  static constexpr size_t kMinBufferSize{256};
  static constexpr size_t kMaxBufferSize{4096};

 private:
  bool customAlloc(folly::IOBuf& buffer, size_t startOffset, size_t frameSize);
  bool customAlloc(
      folly::IOBufQueue& bufQueue, size_t startOffset, size_t frameSize);
  std::unique_ptr<folly::IOBuf> getCustomAllocBuf(
      size_t numBytes, size_t startOffset, size_t trimLength);

  // "old" logic: maintain read buffer in Parser and resize as necessary, hand
  // out frames as IOBufs pointing to the buffer
  // TODO: remove once hybrid logic is stable
  void getReadBufferOld(void** bufout, size_t* lenout);
  void readDataAvailableOld(size_t nbytes);
  // "new" logic: allocate space for frames and transfer ownership to
  // application immediately
  // TODO: remove once hybrid logic is stable
  void getReadBufferNew(void** bufout, size_t* lenout);
  void readDataAvailableNew(size_t nbytes);
  // hybrid logic: maintain small, static read buffer in Parser (like "old"
  // logic), allocate space in dynamic buffer and immediately transfer ownership
  // if necessary (like "new" logic)
  void getReadBufferHybrid(void** bufout, size_t* lenout);
  void readDataAvailableHybrid(size_t nbytes);

  // Flag that controls if the parser should use the new buffer logic
  const bool newBufferLogicEnabled_;
  // Flag that controls if the parser should use the hybrid buffer logic
  const bool hybridBufferLogicEnabled_;

  // TODO: This should be removed once the new buffer logic controlled by
  // THRIFT_FLAG(rocket_parser_dont_hold_buffer_enabled) is stable.
  static constexpr std::chrono::milliseconds kDefaultBufferResizeInterval{
      std::chrono::seconds(3)};

  T& owner_;
  size_t bufferSize_{kMinBufferSize};
  // TODO: readBuffer_, lastResizeTime_, resizeBufferTimeout_ and
  // periodicResizeBufferTimeout_ should be removed once the new buffer logic
  // controlled by THRIFT_FLAG(rocket_parser_dont_hold_buffer_enabled) is
  // stable.
  std::chrono::steady_clock::time_point lastResizeTime_{
      std::chrono::steady_clock::now()};
  const std::chrono::milliseconds resizeBufferTimeout_;
  const int64_t periodicResizeBufferTimeout_;
  apache::thrift::RpcOptions::MemAllocType allocType_{
      apache::thrift::RpcOptions::MemAllocType::ALLOC_DEFAULT};
  size_t currentFrameLength_{0};
  uint8_t currentFrameType_{0};
  // used by readDataAvailable or readBufferAvailable API (only one will be
  // invoked for a given AsyncTransport)
  folly::IOBufQueue readBufQueue_{folly::IOBufQueue::cacheChainLength()};
  folly::IOBuf readBuffer_;

  // hybrid logic
  static constexpr size_t kStaticBufferSize = 1024;
  static constexpr size_t kReallocateThreshold = 64;
  std::unique_ptr<folly::IOBuf> dynamicBuffer_{nullptr};
  bool reallocateIfShared_{false};
  bool blockResize_{false};
};

} // namespace rocket
} // namespace thrift
} // namespace apache

#include <thrift/lib/cpp2/transport/rocket/framing/Parser-inl.h>
