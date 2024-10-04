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

namespace apache::thrift::rocket {

namespace detail {
enum class ParserMode { STRATEGY, ALLOCATING };
ParserMode stringToMode(const std::string& modeStr) noexcept;
ParserAllocatorType& getDefaultAllocator();
} // namespace detail

// TODO (T160861572): deprecate most of logic in this class and replace with
// either AllocatingParserStrategy or FrameLengthParserStrategy
template <class T>
class Parser final : public folly::AsyncTransport::ReadCallback {
 public:
  explicit Parser(
      T& owner, std::shared_ptr<ParserAllocatorType> alloc = nullptr)
      : owner_(owner),
        mode_(detail::stringToMode(THRIFT_FLAG(rocket_frame_parser))) {
    if (mode_ == detail::ParserMode::STRATEGY) {
      frameLengthParser_ =
          std::make_unique<ParserStrategy<T, FrameLengthParserStrategy>>(
              owner_);
    }
    if (mode_ == detail::ParserMode::ALLOCATING) {
      allocatingParser_ = std::make_unique<
          ParserStrategy<T, AllocatingParserStrategy, ParserAllocatorType>>(
          owner_, alloc ? *alloc : detail::getDefaultAllocator());
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
    return mode_ != detail::ParserMode::ALLOCATING;
  }

  const folly::IOBuf& getReadBuffer() const;

 private:
  T& owner_;

  detail::ParserMode mode_;
  std::unique_ptr<ParserStrategy<T, FrameLengthParserStrategy>>
      frameLengthParser_;
  std::unique_ptr<
      ParserStrategy<T, AllocatingParserStrategy, ParserAllocatorType>>
      allocatingParser_;
};

} // namespace apache::thrift::rocket

#include <thrift/lib/cpp2/transport/rocket/framing/Parser-inl.h>
