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

#if FOLLY_HAS_MEMORY_RESOURCE
void setDefaultParserMemoryResource(std::pmr::memory_resource* resource);
#endif
} // namespace detail

// Install the std::pmr::memory_resource backing the default parser allocator
// returned by detail::getDefaultAllocator(). When set, the ALLOCATING parser
// (selected via the rocket_frame_parser flag) reads frame buffers from this
// resource on BOTH clients and servers that do not supply their own allocator
// -- the single process-wide interposition point for routing large rocket
// frames into custom (e.g. RDMA-registered) memory.
//
// Call this during process startup, before any RocketClient /
// RocketServerConnection is constructed: the default allocator captures the
// resource on first use, and any call after that point fatals (the late install
// would have no effect). Among calls made before capture the last one wins, so
// install exactly once to avoid ambiguity.
// The resource is held as a non-owning pointer captured by the process-wide
// default allocator (used for asynchronous frame/IOBuf deallocation); it MUST
// outlive all Rocket connections and buffers (i.e. have process lifetime).
// Passing a shorter-lived resource is a use-after-free.
// The resource must also be thread-safe: it is shared process-wide and its
// allocate()/deallocate() are invoked concurrently from multiple Rocket I/O
// threads (frame/IOBuf (de)allocation) with no external synchronization.
// Passing nullptr leaves the standard default resource in place.
//
// Declared only when std::pmr::memory_resource is available; callers must
// guard call sites with FOLLY_HAS_MEMORY_RESOURCE.
#if FOLLY_HAS_MEMORY_RESOURCE
void setDefaultParserMemoryResource(std::pmr::memory_resource* resource);
#endif

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
