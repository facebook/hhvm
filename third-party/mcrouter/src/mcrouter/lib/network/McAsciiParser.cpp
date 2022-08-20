/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "McAsciiParser.h"

#include <folly/String.h>

#include "mcrouter/lib/fbi/cpp/LogFailure.h"

namespace facebook {
namespace memcache {

constexpr size_t kProtocolTailContextLength = 128;

McAsciiParserBase::State McClientAsciiParser::consume(folly::IOBuf& buffer) {
  assert(state_ == State::PARTIAL);
  assert(!hasReadBuffer());

  p_ = reinterpret_cast<const char*>(buffer.data());
  pe_ = p_ + buffer.length();

  (this->*consumer_)(buffer);

  if (savedCs_ == errorCs_) {
    handleError(buffer);
  }

  buffer.trimStart(p_ - reinterpret_cast<const char*>(buffer.data()));

  return state_;
}

bool McAsciiParserBase::hasReadBuffer() const noexcept {
  return state_ == State::PARTIAL && currentIOBuf_ != nullptr;
}

std::pair<void*, size_t> McAsciiParserBase::getReadBuffer() noexcept {
  assert(state_ == State::PARTIAL && currentIOBuf_ != nullptr);

  return std::make_pair(currentIOBuf_->writableTail(), remainingIOBufLength_);
}

void McAsciiParserBase::readDataAvailable(size_t length) {
  assert(state_ == State::PARTIAL && currentIOBuf_ != nullptr);
  assert(length <= remainingIOBufLength_);
  currentIOBuf_->append(length);
  remainingIOBufLength_ -= length;
  // We finished reading value.
  if (remainingIOBufLength_ == 0) {
    currentIOBuf_ = nullptr;
  }
}

void McAsciiParserBase::handleError(folly::IOBuf& buffer) {
  state_ = State::ERROR;
  // We've encoutered error we need to do proper logging.
  auto start = reinterpret_cast<const char*>(buffer.data());
  auto length =
      std::min(p_ - start + kProtocolTailContextLength, buffer.length());

  currentErrorDescription_ = folly::sformat(
      "Error parsing message '{}' at character {}!",
      folly::cEscape<std::string>(folly::StringPiece(start, start + length)),
      p_ - start);
}

folly::StringPiece McAsciiParserBase::getErrorDescription() const {
  return currentErrorDescription_;
}

bool McAsciiParserBase::readValue(folly::IOBuf& buffer, folly::IOBuf& to) {
  if (remainingIOBufLength_) {
    // Copy IOBuf for part of (or whole) value.
    size_t offset = p_ - reinterpret_cast<const char*>(buffer.data()) + 1;
    size_t toUse = std::min(buffer.length() - offset, remainingIOBufLength_);
    buffer.cloneOneInto(to);
    // Adjust buffer pointers.
    to.trimStart(offset);
    to.trimEnd(buffer.length() - offset - toUse);

    remainingIOBufLength_ -= toUse;
    // Move the state machine to the proper character.
    p_ += toUse;

    // Now if we don't have enough data, we need to preallocate second piece
    // for remaining buffer and signal partial read.
    if (remainingIOBufLength_) {
      auto secondPiece = folly::IOBuf::createCombined(remainingIOBufLength_);
      currentIOBuf_ = secondPiece.get();
      to.appendChain(std::move(secondPiece));
      return false;
    }
  }

  return true;
}
} // namespace memcache
} // namespace facebook
