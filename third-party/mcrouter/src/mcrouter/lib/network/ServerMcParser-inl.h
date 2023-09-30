/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/lang/Bits.h>

#include "mcrouter/lib/debug/ConnectionFifo.h"

namespace facebook {
namespace memcache {

template <class Callback>
ServerMcParser<Callback>::ServerMcParser(
    Callback& cb,
    size_t minBufferSize,
    size_t maxBufferSize)
    : parser_(
          *this,
          minBufferSize,
          maxBufferSize,
          /* useJemallocNodumpAllocator */ false),
      asciiParser_(*this),
      callback_(cb) {}

template <class Callback>
ServerMcParser<Callback>::~ServerMcParser() {}

template <class Callback>
std::pair<void*, size_t> ServerMcParser<Callback>::getReadBuffer() {
  if (shouldReadToAsciiBuffer()) {
    return asciiParser_.getReadBuffer();
  } else {
    return parser_.getReadBuffer();
  }
}

template <class Callback>
bool ServerMcParser<Callback>::readDataAvailable(size_t len) {
  if (shouldReadToAsciiBuffer()) {
    asciiParser_.readDataAvailable(len);
    return true;
  } else {
    return parser_.readDataAvailable(len);
  }
}

template <class Callback>
bool ServerMcParser<Callback>::caretMessageReady(
    const CaretMessageInfo& headerInfo,
    const folly::IOBuf& buffer) {
  try {
    // Caret header and body are assumed to be in one coalesced IOBuf
    callback_.caretRequestReady(headerInfo, buffer);
  } catch (const std::exception& e) {
    std::string reason(std::string("Error parsing Caret message: ") + e.what());
    callback_.parseError(carbon::Result::REMOTE_ERROR, reason);
    return false;
  }
  return true;
}

template <class Callback>
void ServerMcParser<Callback>::handleAscii(folly::IOBuf& readBuffer) {
  if (FOLLY_UNLIKELY(parser_.protocol() != mc_ascii_protocol)) {
    std::string reason(folly::sformat(
        "Expected {} protocol, but received ASCII!",
        mc_protocol_to_string(parser_.protocol())));
    callback_.parseError(carbon::Result::LOCAL_ERROR, reason);
    return;
  }

  // Note: McParser never chains IOBufs.
  auto result = asciiParser_.consume(readBuffer);

  if (result == McAsciiParserBase::State::ERROR) {
    // Note: we could include actual parsing error instead of
    // "malformed request" (e.g. asciiParser_.getErrorDescription()).
    callback_.parseError(carbon::Result::CLIENT_ERROR, "malformed request");
  }
}

template <class Callback>
void ServerMcParser<Callback>::parseError(
    carbon::Result result,
    folly::StringPiece reason) {
  callback_.parseError(result, reason);
}

template <class Callback>
bool ServerMcParser<Callback>::shouldReadToAsciiBuffer() const {
  return parser_.protocol() == mc_ascii_protocol &&
      asciiParser_.hasReadBuffer();
}

template <class Callback>
template <class Request>
void ServerMcParser<Callback>::onRequest(Request&& req, bool noreply) {
  if (FOLLY_UNLIKELY(debugFifo_ && debugFifo_->isConnected())) {
    writeToPipe(req);
  }
  callback_.onRequest(std::move(req), noreply);
}

template <class Callback>
void ServerMcParser<Callback>::multiOpEnd() {
  callback_.multiOpEnd();
}

template <class Callback>
template <class Request>
void ServerMcParser<Callback>::writeToPipe(const Request& req) {
  assert(debugFifo_);
  AsciiSerializedRequest debugSerializedRequest;
  const struct iovec* iov;
  size_t iovLen;
  debugSerializedRequest.prepare(req, iov, iovLen);
  debugFifo_->startMessage(MessageDirection::Received, Request::typeId);
  debugFifo_->writeData(iov, iovLen);
}
} // namespace memcache
} // namespace facebook
