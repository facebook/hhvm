/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/filters/CompressionFilter.h"

namespace {
using folly::coro::co_error;
}

#define CompressionFailedError \
  HTTPError(HTTPErrorCode::INTERNAL_ERROR, "CompressionFilter failed")

namespace proxygen::coro {
folly::coro::Task<HTTPHeaderEvent> CompressionFilter::readHeaderEvent() {
  // read from source
  auto headerEvent =
      co_await co_awaitTry(readHeaderEventImpl(/*deleteOnDone=*/false));
  auto guard = folly::makeGuard(lifetime(headerEvent));

  // check if compression should be skipped
  skipCompression_ = headerEvent.hasException() || headerEvent->eom ||
                     !params_->has_value() ||
                     !CompressionFilterUtils::shouldCompress(
                         *headerEvent->headers, params_->value());
  if (skipCompression_) {
    // passthru if skipping compression / has exception
    co_return headerEvent;
  }

  // try to create compressor
  compressor_ = params_->value().compressorFactory();
  if (!compressor_ || compressor_->hasError()) {
    co_yield co_error(CompressionFailedError);
  }

  // always set compressed req/res to chunked
  headerEvent->headers->getHeaders().remove(HTTP_HEADER_CONTENT_LENGTH);
  headerEvent->headers->getHeaders().set(HTTP_HEADER_CONTENT_ENCODING,
                                         params_->value().headerEncoding);
  headerEvent->headers->setIsChunked(true);
  co_return headerEvent;
}

folly::coro::Task<HTTPBodyEvent> CompressionFilter::readBodyEvent(
    uint32_t max) {
  // check if there's a pending event (i.e. trailer), deliver before reading
  // from source
  if (pendingBodyEvent_) {
    // wrap in folly::Try<> for lifetime() guard
    auto bodyEvent =
        folly::Try<HTTPBodyEvent>(std::move(pendingBodyEvent_.value()));
    auto guard = folly::makeGuard(lifetime(bodyEvent));
    co_return std::move(bodyEvent);
  }

  auto bodyEvent =
      co_await co_awaitTry(readBodyEventImpl(max, /*deleteOnDone=*/false));
  // note: guard is dismissed below if we need to buffer the trailers until next
  // ::readBodyEvent(). Dismissing guard prevents source being deleted if eom
  // was read in current bodyEvent
  auto guard = folly::makeGuard(lifetime(bodyEvent));

  // passthru on skipCompression flag or body exception
  if (skipCompression_ || bodyEvent.hasException()) {
    co_return bodyEvent;
  }

  // try to compress body, validate success
  if (bodyEvent->eventType == HTTPBodyEvent::BODY) {
    CHECK(compressor_);
    folly::IOBuf emptyBuf{};
    auto* pBuf = bodyEvent->event.body.empty() ? &emptyBuf
                                               : bodyEvent->event.body.front();
    auto compressed = compressor_->compress(pBuf, bodyEvent->eom);
    if (compressor_->hasError()) {
      co_yield co_error(CompressionFailedError);
    }
    // replace with compressed body
    bodyEvent->event.body = std::move(compressed);
    co_return bodyEvent;
  }

  // there may be remaining unflushed data in compressor, if so that data
  // should be inserted before non-body eom event (note that trailer flag is
  // identical to bodyEvent->eom)
  if (bodyEvent->eom) {
    // force flushing pending data in compressor and verify success
    folly::IOBuf emptyBuf{};
    auto compressed = compressor_->compress(&emptyBuf, /*trailer=*/true);
    if (compressor_->hasError()) {
      co_yield co_error(CompressionFailedError);
    }
    // if no remaining unflushed data, return bodyEvent as is
    if (compressed == nullptr || compressed->empty()) {
      co_return bodyEvent;
    }

    // need to buffer trailers, dismiss guard as mentioned above
    CHECK(!pendingBodyEvent_);
    pendingBodyEvent_.emplace(std::move(*bodyEvent));
    guard.dismiss();
    // eom=false; there's a pending event to be delivered via next
    // ::readBodyEvent
    co_return HTTPBodyEvent(std::move(compressed), /*trailer=*/false);
  }

  co_return bodyEvent;
}
} // namespace proxygen::coro
