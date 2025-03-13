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

template <class T>
void Parser<T>::getReadBuffer(void** bufout, size_t* lenout) {
  switch (mode_) {
    case (detail::ParserMode::STRATEGY):
      frameLengthParser_->getReadBuffer(bufout, lenout);
      break;
    case (detail::ParserMode::ALLOCATING):
      allocatingParser_->getReadBuffer(bufout, lenout);
      break;
  }
}

template <class T>
void Parser<T>::readDataAvailable(size_t nbytes) noexcept {
  folly::DelayedDestruction::DestructorGuard dg(&this->owner_);
  try {
    switch (mode_) {
      case (detail::ParserMode::STRATEGY):
        frameLengthParser_->readDataAvailable(nbytes);
        break;
      case (detail::ParserMode::ALLOCATING):
        allocatingParser_->readDataAvailable(nbytes);
        break;
    }
  } catch (...) {
    auto exceptionStr =
        folly::exceptionStr(folly::current_exception()).toStdString();
    LOG(ERROR) << "Bad frame received, closing connection: " << exceptionStr;
    owner_.close(transport::TTransportException(exceptionStr));
  }
}

template <class T>
void Parser<T>::readEOF() noexcept {
  folly::DelayedDestruction::DestructorGuard dg(&this->owner_);

  owner_.close(transport::TTransportException(
      transport::TTransportException::TTransportExceptionType::END_OF_FILE,
      "Channel got EOF. Check for server hitting connection limit, "
      "connection age timeout, server connection idle timeout, and server crashes."));
}

template <class T>
void Parser<T>::readErr(const folly::AsyncSocketException& ex) noexcept {
  folly::DelayedDestruction::DestructorGuard dg(&this->owner_);
  owner_.close(transport::TTransportException(ex));
}

template <class T>
void Parser<T>::readBufferAvailable(
    std::unique_ptr<folly::IOBuf> buf) noexcept {
  folly::DelayedDestruction::DestructorGuard dg(&this->owner_);
  try {
    switch (mode_) {
      case (detail::ParserMode::STRATEGY):
        frameLengthParser_->readBufferAvailable(std::move(buf));
        break;
      case (detail::ParserMode::ALLOCATING):
        // Will throw not implemented runtime exception
        allocatingParser_->readBufferAvailable(std::move(buf));
        break;
    }
  } catch (...) {
    auto exceptionStr =
        folly::exceptionStr(folly::current_exception()).toStdString();
    LOG(ERROR) << "Bad frame received, closing connection: " << exceptionStr;
    owner_.close(transport::TTransportException(exceptionStr));
  }
}

} // namespace apache::thrift::rocket
