/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/webtransport/WebTransport.h"

namespace proxygen {

WebTransport::Exception::Exception(uint32_t inError)
    : std::runtime_error(folly::to<std::string>(
          "Peer reset or abandoned stream with error=", inError)),
      error(inError) {
}

WebTransport::Exception::Exception(uint32_t inError, const std::string& msg)
    : std::runtime_error(folly::to<std::string>(msg, " with error=", inError)),
      error(inError) {
}

/*static*/ folly::Expected<uint32_t, WebTransport::ErrorCode>
WebTransport::toApplicationErrorCode(uint64_t h) {
  if (!isEncodedApplicationErrorCode(h)) {
    // This is not for us
    return folly::makeUnexpected(WebTransport::ErrorCode::GENERIC_ERROR);
  }
  uint64_t shifted = h - kFirstErrorCode;
  uint64_t appErrorCode = shifted - (shifted / 0x1f);
  DCHECK_LE(appErrorCode, std::numeric_limits<uint32_t>::max());
  return static_cast<uint32_t>(appErrorCode);
}

void WebTransport::StreamReadHandle::awaitNextRead(
    folly::Executor* exec,
    const ReadStreamDataFn& readCb,
    folly::Optional<std::chrono::milliseconds> timeout) {
  auto id = getID();
  auto fut = readStreamData();
  if (timeout) {
    fut = std::move(fut).within(*timeout);
  }
  std::move(fut).via(exec).thenTry([this, id, readCb](auto streamData) {
    readCb((streamData.hasException() || streamData->fin) ? nullptr : this,
           id,
           std::move(streamData));
  });
}

} // namespace proxygen
