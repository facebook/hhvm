/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/httpserver/filters/DecompressionFilter.h>

#include <proxygen/lib/utils/ZstdStreamDecompressor.h>

namespace proxygen {

void DecompressionFilter::onRequest(std::unique_ptr<HTTPMessage> msg) noexcept {
  auto& headers = msg->getHeaders();
  auto contentEncoding =
      headers.getSingleOrNullptr(HTTP_HEADER_CONTENT_ENCODING);
  if (contentEncoding && *contentEncoding == "zstd") {
    decompressor_ = std::make_unique<ZstdStreamDecompressor>();
    headers.remove(HTTP_HEADER_CONTENT_ENCODING);
    headers.remove(HTTP_HEADER_CONTENT_LENGTH);
  }
  return Filter::onRequest(std::move(msg));
}

void DecompressionFilter::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {
  if (!decompressor_) {
    return Filter::onBody(std::move(body));
  }

  auto decompressed = decompressor_->decompress(body.get());

  if (decompressor_->hasError()) {

    return Filter::sendAbort(folly::none);
  }

  if (decompressed && !decompressed->empty()) {
    Filter::onBody(std::move(decompressed));
  }
}

void DecompressionFilter::onEOM() noexcept {
  if (!decompressor_) {
    return Filter::onEOM();
  }

  folly::IOBuf emptyBuf;
  auto decompressed = decompressor_->decompress(&emptyBuf);

  if (decompressor_->hasError()) {
    return Filter::sendAbort(folly::none);
  }

  if (decompressed && !decompressed->empty()) {
    Filter::onBody(std::move(decompressed));
  }

  return Filter::onEOM();
}

RequestHandler* DecompressionFilterFactory::onRequest(
    RequestHandler* h, HTTPMessage* /*msg*/) noexcept {
  return new DecompressionFilter(h);
}

} // namespace proxygen
