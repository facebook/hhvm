/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/filters/DecompressionFilter.h"
#include <proxygen/lib/utils/ZstdStreamDecompressor.h>

namespace {

#define DecompressionFailedError \
  HTTPError(HTTPErrorCode::INTERNAL_ERROR, "DecompressionFilter failed")

using CompressionType = proxygen::CompressionType;
struct SupportedCompression {
  std::string_view name;
  CompressionType type;
};

// sorted in descending order of preference
constexpr std::array kSupportedCompressionTypes = {
    SupportedCompression{"gzip", CompressionType::GZIP},
    SupportedCompression{"deflate", CompressionType::DEFLATE},
    SupportedCompression{"zstd", CompressionType::ZSTD}};

using folly::coro::co_error;

using AcceptedEncodings =
    std::array<std::string_view, kSupportedCompressionTypes.size()>;
AcceptedEncodings getAcceptedEncodings() {
  AcceptedEncodings acceptedEncodings{};
  for (size_t idx = 0; idx < kSupportedCompressionTypes.size(); idx++) {
    acceptedEncodings[idx] = kSupportedCompressionTypes[idx].name;
  }
  return acceptedEncodings;
}

using namespace proxygen;
using namespace proxygen::coro;
void decompressionEgressHeaderEventHook(HTTPHeaderEvent& headerEvent) {
  // append the supported compression types if the header doesn't exist
  if (!headerEvent.headers->getHeaders().exists(HTTP_HEADER_ACCEPT_ENCODING)) {
    std::string acceptedEncodings = folly::join(", ", getAcceptedEncodings());
    headerEvent.headers->getHeaders().set(HTTP_HEADER_ACCEPT_ENCODING,
                                          acceptedEncodings);
  }
}

}; // namespace

namespace proxygen::coro {

DecompressionEgressFilter::DecompressionEgressFilter(HTTPSource* source)
    : MutateFilter(source,
                   /*headerHook=*/
                   decompressionEgressHeaderEventHook,
                   /*bodyHook=*/nullptr) {
}

folly::coro::Task<HTTPHeaderEvent>
DecompressionIngressFilter::readHeaderEvent() {
  auto headerEvent =
      co_await co_awaitTry(readHeaderEventImpl(/*deleteOnDone=*/false));
  auto guard = folly::makeGuard(lifetime(headerEvent));
  if (headerEvent.hasException() || headerEvent->eom) {
    co_return headerEvent;
  }
  initializeWithHTTPMessage(*headerEvent->headers);
  co_return headerEvent;
}

/* static */ bool DecompressionIngressFilter::compressionSupported(
    HTTPMessage& msg) {
  const auto& contentEncoding =
      msg.getHeaders().getSingleOrEmpty(HTTP_HEADER_CONTENT_ENCODING);
  return std::any_of(kSupportedCompressionTypes.begin(),
                     kSupportedCompressionTypes.end(),
                     [&contentEncoding](const auto& supportedCompression) {
                       return supportedCompression.name == contentEncoding;
                     });
}

void DecompressionIngressFilter::initializeWithHTTPMessage(HTTPMessage& msg) {
  auto& headers = msg.getHeaders();
  const auto& contentEncoding =
      headers.getSingleOrEmpty(HTTP_HEADER_CONTENT_ENCODING);

  // instantiate decompressor_ if supported compression type
  auto it = std::find_if(kSupportedCompressionTypes.begin(),
                         kSupportedCompressionTypes.end(),
                         [&contentEncoding](const auto& supportedCompression) {
                           return supportedCompression.name == contentEncoding;
                         });
  if (it != kSupportedCompressionTypes.end()) {
    headers.remove(HTTP_HEADER_CONTENT_LENGTH);
    headers.remove(HTTP_HEADER_CONTENT_ENCODING);
    headers.set(HTTP_HEADER_TRANSFER_ENCODING, "chunked");
    msg.setIsChunked(true);
    if (statsCallback_) {
      statsCallback_->onDecompressionAlgo(std::string(it->name));
    }
    if (it->type == CompressionType::ZSTD) {
      decompressor_ = std::make_unique<ZstdStreamDecompressor>();
    } else {
      decompressor_ = std::make_unique<ZlibStreamDecompressor>(it->type);
    }
  } else if (statsCallback_) {
    statsCallback_->onDecompressionAlgo("");
  }
}

folly::coro::Task<HTTPBodyEvent> DecompressionIngressFilter::readBodyEvent(
    uint32_t max) {
  auto bodyEvent =
      co_await co_awaitTry(readBodyEventImpl(max, /*deleteOnDone=*/false));
  auto guard = folly::makeGuard(lifetime(bodyEvent));
  if (!decompressor_ || bodyEvent.hasException()) {
    // if unsupported compression or error, return as is
    co_return bodyEvent;
  }

  if (bodyEvent->eventType == HTTPBodyEvent::EventType::BODY) {
    auto& body = bodyEvent->event.body;
    folly::IOBuf empty{};

    auto compressed = body.move();
    auto decompressed =
        decompressor_->decompress(compressed ? compressed.get() : &empty);
    if (!decompressed || decompressor_->hasError()) {
      if (statsCallback_) {
        statsCallback_->onDecompressionError();
      }
      co_yield co_error(DecompressionFailedError);
    }
    body.append(std::move(decompressed));
  }

  /**
   * At this point: if eom flag is true, we've attempted to decompress all the
   * body events consumed. If decompressor has not finished then yield a
   * decompression failed error.
   */
  if (bodyEvent->eom && !decompressor_->finished()) {
    if (statsCallback_) {
      statsCallback_->onDecompressionError();
    }
    co_yield co_error(DecompressionFailedError);
  }

  co_return bodyEvent;
}

}; // namespace proxygen::coro
