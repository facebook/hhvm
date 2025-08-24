/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/coro/HTTPSourceFilter.h"
#include "proxygen/lib/http/coro/filters/MutateFilter.h"
#include <proxygen/lib/utils/CompressionFilterUtils.h>

namespace proxygen::coro {

/**
 * Egress filter modifies the request headers to include the supported
 * compression types. More specifically, it adds an "accept-encoding: gzip,
 * deflate" request header; only if "accept-encoding" header isn't already
 * present.
 */
class DecompressionEgressFilter : public MutateFilter {
 public:
  DecompressionEgressFilter(HTTPSource* source = nullptr);
};

/**
 * Ingress filter that instantiates the type of StreamDecompressor based on
 * "content-encoding" header, if supported. If the "content-encoding"
 * value isn't supported, we passthru the header & body events unmodified.
 */
class DecompressionIngressFilter : public HTTPSourceFilter {
 public:
  class StatsCallback {
   public:
    virtual ~StatsCallback() = default;
    // algo might be empty string, if it's unsupported compression
    virtual void onDecompressionAlgo(const std::string& algo) = 0;
    virtual void onDecompressionError() = 0;
  };

  DecompressionIngressFilter(HTTPSource* source = nullptr,
                             std::shared_ptr<StatsCallback> statsCb = nullptr)
      : HTTPSourceFilter(source), statsCallback_(std::move(statsCb)) {
  }

  folly::coro::Task<HTTPHeaderEvent> readHeaderEvent() override;

  folly::coro::Task<HTTPBodyEvent> readBodyEvent(
      uint32_t max = std::numeric_limits<uint32_t>::max()) override;

  static bool compressionSupported(HTTPMessage& msg);

 protected:
  void initializeWithHTTPMessage(HTTPMessage& msg);

 private:
  std::unique_ptr<proxygen::StreamDecompressor> decompressor_;
  std::shared_ptr<StatsCallback> statsCallback_;
};

}; // namespace proxygen::coro
