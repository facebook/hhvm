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
#include <thrift/lib/cpp2/transport/rocket/framing/parser/AlignedParserStrategy.h>
#include <thrift/lib/cpp2/transport/rocket/framing/parser/AllocatingParserStrategy.h>
#include <thrift/lib/cpp2/transport/rocket/framing/parser/FrameLengthParserStrategy.h>
#include <thrift/lib/cpp2/transport/rocket/framing/parser/ParserStrategy.h>

THRIFT_FLAG_DECLARE_string(rocket_frame_parser);

namespace apache::thrift::rocket {

namespace detail {
enum class ParserMode { STRATEGY, ALLOCATING, ALIGNED };
ParserMode stringToMode(const std::string& modeStr) noexcept;
ParserAllocatorType& getDefaultAllocator();
} // namespace detail

template <class T>
class Parser final : public folly::AsyncTransport::ReadCallback {
  using FrameLengthParser = ParserStrategy<T, FrameLengthParserStrategy>;
  using AllocatingParser =
      ParserStrategy<T, AllocatingParserStrategy, ParserAllocatorType>;
  using AlignedParser = ParserStrategy<T, AlignedParserStrategy>;
  using ParserVariant = std::variant<
      std::monostate,
      FrameLengthParser,
      AllocatingParser,
      AlignedParser>;

 public:
  explicit Parser(
      T& owner,
      std::string&& mode,
      std::shared_ptr<ParserAllocatorType> alloc = nullptr)
      : Parser(owner, mode, alloc) {}

  explicit Parser(
      T& owner,
      const std::string& mode,
      std::shared_ptr<ParserAllocatorType> alloc = nullptr)
      : owner_(owner), mode_(detail::stringToMode(mode)) {
    switch (mode_) {
      case detail::ParserMode::STRATEGY: {
        parser_.template emplace<FrameLengthParser>(owner_);
      } break;
      case detail::ParserMode::ALLOCATING: {
        parser_.template emplace<AllocatingParser>(
            owner_, alloc ? *alloc : detail::getDefaultAllocator());
      } break;
      case detail::ParserMode::ALIGNED:
        parser_.template emplace<AlignedParser>(owner_);
        break;
    }
  }

  FOLLY_NOINLINE void getReadBuffer(void** bufout, size_t* lenout) override;
  FOLLY_NOINLINE void readDataAvailable(size_t nbytes) noexcept override;
  FOLLY_NOINLINE void readEOF() noexcept override;
  FOLLY_NOINLINE void readErr(
      const folly::AsyncSocketException&) noexcept override;
  FOLLY_NOINLINE void readBufferAvailable(
      std::unique_ptr<folly::IOBuf> /*readBuf*/) noexcept override;

  bool isBufferMovable() noexcept override {
    return visit([](auto& parser) { return parser.isBufferMovable(); });
  }

  const folly::IOBuf& getReadBuffer() const;

 private:
  T& owner_;

  detail::ParserMode mode_;
  ParserVariant parser_;

  template <typename DelegateFunc>
  FOLLY_ALWAYS_INLINE decltype(auto) visit(DelegateFunc&& delegate);
};

template <class T>
template <typename DelegateFunc>
FOLLY_ALWAYS_INLINE decltype(auto) Parser<T>::visit(DelegateFunc&& delegate) {
  FOLLY_SAFE_DCHECK(
      std::holds_alternative<std::monostate>(parser_) == false,
      "parser variant must be set");
  switch (mode_) {
    case detail::ParserMode::STRATEGY:
      return std::invoke(
          std::forward<DelegateFunc>(delegate),
          std::get<FrameLengthParser>(parser_));
    case detail::ParserMode::ALLOCATING:
      return std::invoke(
          std::forward<DelegateFunc>(delegate),
          std::get<AllocatingParser>(parser_));
    case detail::ParserMode::ALIGNED:
      return std::invoke(
          std::forward<DelegateFunc>(delegate),
          std::get<AlignedParser>(parser_));
    default:
      LOG(FATAL) << "Unknown parser type";
  }
}

template <class T>
void Parser<T>::getReadBuffer(void** bufout, size_t* lenout) {
  visit([&](auto& parser) { parser.getReadBuffer(bufout, lenout); });
}

template <class T>
void Parser<T>::readDataAvailable(size_t nbytes) noexcept {
  folly::DelayedDestruction::DestructorGuard dg(&this->owner_);
  try {
    visit([&](auto& parser) { parser.readDataAvailable(nbytes); });
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

  owner_.close(
      transport::TTransportException(
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
    visit([&](auto& parser) { parser.readBufferAvailable(std::move(buf)); });
  } catch (...) {
    auto exceptionStr =
        folly::exceptionStr(folly::current_exception()).toStdString();
    LOG(ERROR) << "Bad frame received, closing connection: " << exceptionStr;
    owner_.close(transport::TTransportException(exceptionStr));
  }
}

} // namespace apache::thrift::rocket
