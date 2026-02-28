/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/coro/filters/RateLimitFilter.h>

#include <folly/coro/Sleep.h>

using namespace std::literals::chrono_literals;
using folly::coro::co_error;

namespace proxygen::coro {

namespace {
constexpr uint32_t kApproximateMTU = 1400;
constexpr auto kRateLimitMaxDelay = 10s;
constexpr uint32_t kDefaultChunkSize = 4096;
} // namespace

RateLimitFilter::RateLimitFilter(HTTPSource* source,
                                 std::chrono::seconds maxDelay,
                                 uint32_t chunkBytes)
    : HTTPSourceFilter(source),
      maxDelay_(maxDelay),
      chunkBytes_(std::max(kApproximateMTU, chunkBytes)) {
}

RateLimitFilter::RateLimitFilter(HTTPSource* source)
    : RateLimitFilter(source, kRateLimitMaxDelay, kDefaultChunkSize) {
}

folly::coro::Task<HTTPBodyEvent> RateLimitFilter::readAndUpdateBytesCount(
    uint32_t maxRead) noexcept {
  auto readEvent =
      co_await co_awaitTry(HTTPSourceFilter::readBodyEvent(maxRead));
  if (readEvent.hasException() || readEvent->eom) {
    // The filter itself might have been deleted already at this point if it's
    // heap allocated. Either way, this is the end of body reads, so no need to
    // update bytes count in this case.
    co_return readEvent;
  }
  if (readEvent->eventType == HTTPBodyEvent::EventType::BODY) {
    readBytesCount_ += readEvent->event.body.chainLength();
  }
  co_return readEvent;
}

folly::coro::Task<HTTPBodyEvent> RateLimitFilter::readBodyEvent(uint32_t max) {
  if (bytesPerMs_ == 0 || readBytesCount_ == 0) {
    co_return co_await readAndUpdateBytesCount(max);
  }

  auto timeElapsedMs =
      millisecondsBetween(getCurrentTime(), startTime_).count();
  uint64_t bytesTarget = bytesPerMs_ * timeElapsedMs;
  // At the very minimal, we should read this amount, otherwise the caller
  // can make this into a tight loop with many small reads.
  auto minReadAmount = std::min(max, chunkBytes_);
  if (readBytesCount_ + minReadAmount <= bytesTarget) {
    auto amountToRead =
        std::min(max, folly::to<uint32_t>(bytesTarget - readBytesCount_));
    co_return co_await readAndUpdateBytesCount(amountToRead);
  }

  auto sleepTime = std::chrono::milliseconds(
      (readBytesCount_ + minReadAmount - bytesTarget) / bytesPerMs_);
  sleepTime = std::min(
      sleepTime,
      std::chrono::duration_cast<std::chrono::milliseconds>(maxDelay_));
  co_await folly::coro::sleepReturnEarlyOnCancel(sleepTime);
  const auto& cancelToken = co_await folly::coro::co_current_cancellation_token;
  if (cancelToken.isCancellationRequested()) {
    HTTPSourceFilter::stopReading();
    co_yield co_error(
        HTTPError(HTTPErrorCode::CORO_CANCELLED, "Read body cancelled"));
  }
  co_return co_await readAndUpdateBytesCount(minReadAmount);
}

void RateLimitFilter::setLimit(uint64_t bitsPerSecond) noexcept {
  bytesPerMs_ = bitsPerSecond / 1000 / 8;
  startTime_ = getCurrentTime();
  readBytesCount_ = 0;
}
} // namespace proxygen::coro
