/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/httpserver/Filters.h>
#include <proxygen/httpserver/RequestHandlerFactory.h>
#include <proxygen/lib/utils/StreamDecompressor.h>

namespace proxygen {

/**
 * A Server filter to perform decompression of incoming request bodies.
 * Supports zstd compression format.
 * If there are any errors it will abort the request.
 */
class DecompressionFilter : public Filter {
 public:
  explicit DecompressionFilter(RequestHandler* downstream)
      : Filter(downstream) {
  }

  ~DecompressionFilter() override = default;

  DecompressionFilter(const DecompressionFilter&) = delete;
  DecompressionFilter& operator=(const DecompressionFilter&) = delete;
  DecompressionFilter(DecompressionFilter&&) = delete;
  DecompressionFilter& operator=(DecompressionFilter&&) = delete;

  void onRequest(std::unique_ptr<HTTPMessage> msg) noexcept override;

  void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;

  void onEOM() noexcept override;

 private:
  std::unique_ptr<StreamDecompressor> decompressor_{nullptr};
};

/**
 * Factory for creating DecompressionFilter instances.
 */
class DecompressionFilterFactory : public RequestHandlerFactory {
 public:
  DecompressionFilterFactory() = default;

  ~DecompressionFilterFactory() override = default;

  DecompressionFilterFactory(const DecompressionFilterFactory&) = delete;
  DecompressionFilterFactory& operator=(const DecompressionFilterFactory&) =
      delete;
  DecompressionFilterFactory(DecompressionFilterFactory&&) = delete;
  DecompressionFilterFactory& operator=(DecompressionFilterFactory&&) = delete;

  void onServerStart(folly::EventBase* /*evb*/) noexcept override {
  }

  void onServerStop() noexcept override {
  }

  RequestHandler* onRequest(RequestHandler* h,
                            HTTPMessage* /*msg*/) noexcept override;
};

} // namespace proxygen
