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
#include <thrift/lib/cpp2/transport/rocket/framing/parser/AllocatingParserStrategy.h>
#include <thrift/lib/cpp2/transport/rocket/framing/parser/FrameLengthParserStrategy.h>
#include <thrift/lib/cpp2/transport/rocket/framing/parser/ParserStrategy.h>

THRIFT_FLAG_DECLARE_string(rocket_frame_parser);

namespace apache {
namespace thrift {
namespace rocket {

// TODO (T160861572): deprecate most of logic in this class and replace with
// either AllocatingParserStrategy or FrameLengthParserStrategy
template <class T>
class Parser final : public folly::AsyncTransport::ReadCallback,
                     public folly::HHWheelTimer::Callback {
 public:
  explicit Parser(
      T& owner, std::shared_ptr<ParserAllocatorType> alloc = nullptr)
      : owner_(owner),
        readBuffer_(folly::IOBuf::CreateOp(), bufferSize_),
        mode_(stringToMode(THRIFT_FLAG(rocket_frame_parser))),
        allocator_(alloc ? alloc : std::make_shared<ParserAllocatorType>()) {
    if (mode_ == ParserMode::STRATEGY) {
      frameLengthParser_ =
          std::make_unique<ParserStrategy<T, FrameLengthParserStrategy>>(
              owner_);
    }
    if (mode_ == ParserMode::ALLOCATING) {
      allocatingParser_ = std::make_unique<
          ParserStrategy<T, AllocatingParserStrategy, ParserAllocatorType>>(
          owner_, *allocator_);
    }
  }

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

  bool isBufferMovable() noexcept override {
    return mode_ != ParserMode::ALLOCATING;
  }

  void timeoutExpired() noexcept override;

  const folly::IOBuf& getReadBuffer() const { return readBuffer_; }

  void setReadBuffer(folly::IOBuf&& buffer) { readBuffer_ = std::move(buffer); }

  size_t getReadBufferSize() const { return bufferSize_; }

  void setReadBufferSize(size_t size) { bufferSize_ = size; }

  void resizeBuffer();

  size_t getReadBufLength() const {
    return readBuffer_.computeChainDataLength();
  }

  bool isReadCallbackBased() const { return mode_ == ParserMode::LEGACY; }

  static constexpr size_t kMinBufferSize{256};
  static constexpr size_t kMaxBufferSize{4096};

 private:
  enum class ParserMode { LEGACY, STRATEGY, ALLOCATING };

  ParserMode stringToMode(const std::string& modeStr) noexcept {
    /* library-local */ const static std::map<std::string, ParserMode> modeMap =
        {
            {"legacy", ParserMode::LEGACY},
            {"strategy", ParserMode::STRATEGY},
            {"allocating", ParserMode::ALLOCATING},
        };
    auto it = modeMap.find(modeStr);
    if (it != modeMap.end()) {
      return it->second;
    } else {
      LOG(WARNING) << "Invalid parser mode: '" << modeStr
                   << ", default to ParserMode::LEGACY";
      return ParserMode::LEGACY;
    }
  }

  void getReadBufferOld(void** bufout, size_t* lenout);
  void readDataAvailableOld(size_t nbytes);
  static constexpr std::chrono::milliseconds kDefaultBufferResizeInterval{
      std::chrono::seconds(3)};

  T& owner_;
  size_t bufferSize_{kMinBufferSize};
  size_t currentFrameLength_{0};
  uint8_t currentFrameType_{0};
  folly::IOBufQueue readBufQueue_{folly::IOBufQueue::cacheChainLength()};
  folly::IOBuf readBuffer_;
  bool blockResize_{false};

  ParserMode mode_;
  std::unique_ptr<ParserStrategy<T, FrameLengthParserStrategy>>
      frameLengthParser_;

  std::shared_ptr<ParserAllocatorType> allocator_;
  std::unique_ptr<
      ParserStrategy<T, AllocatingParserStrategy, ParserAllocatorType>>
      allocatingParser_;
};

} // namespace rocket
} // namespace thrift
} // namespace apache

#include <thrift/lib/cpp2/transport/rocket/framing/Parser-inl.h>
