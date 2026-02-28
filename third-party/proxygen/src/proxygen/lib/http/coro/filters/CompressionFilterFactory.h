/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/coro/filters/CompressionFilter.h"
#include "proxygen/lib/http/coro/filters/ServerFilterFactory.h"
#include "proxygen/lib/http/coro/filters/VisitorFilter.h"

namespace {
using FilterParams = proxygen::CompressionFilterUtils::FilterParams;
using SharedCtx = folly::Optional<FilterParams>;
} // namespace

namespace proxygen::coro {

class ServerCompressionFilterFactory : public ServerFilterFactory {
 public:
  ServerCompressionFilterFactory(CompressionFilterUtils::FactoryOptions options)
      : options_(std::move(options)) {
  }

  // no-op
  void onServerStart(folly::EventBase* evb) noexcept override {
  }

  // no-op
  void onServerStop() noexcept override {
  }

  /**
   * The idea here is to pass a shared_ptr<folly::Optional<FilterParams>> as a
   * way for the ingress filter to publish FilterParams (created only when
   * request header is read) and the egress filter to consume the FilterParams
   * for creating the compressor (created only when response header/body is
   * sent). Since the FilterParams is only written to/read from after the
   * ingress&egress filters are created here, the shared_ptr is passed as a copy
   * to both filters allowing safe access since either filter may be deleted
   * arbitrarily via ::stopReading or reading error from source.
   */
  std::pair<HTTPSourceFilter*, HTTPSourceFilter*> makeFilters() override {
    auto ctx = std::make_shared<SharedCtx>(folly::none);
    return {makeIngressFilter(ctx), makeEgressFilter(ctx)};
  }

 private:
  HTTPSourceFilter* makeIngressFilter(std::shared_ptr<SharedCtx>& ctx) {
    // simple visitor to set filterParams upon rx'ing request headers
    auto visitor = std::make_unique<VisitorFilter>(
        /*source=*/
        nullptr,
        [this, ctx](auto&& headerEvent) {
          if (!headerEvent.hasException()) {
            ctx->assign(CompressionFilterUtils::getFilterParams(
                *headerEvent->headers, options_));
          }
        },
        /*bodyHook=*/nullptr);
    visitor->setHeapAllocated();
    return visitor.release();
  }

  HTTPSourceFilter* makeEgressFilter(std::shared_ptr<SharedCtx>& ctx) {
    auto compressionFilter = std::make_unique<CompressionFilter>(
        /*source=*/nullptr, /*params=*/ctx);
    compressionFilter->setHeapAllocated();
    return compressionFilter.release();
  }

  CompressionFilterUtils::FactoryOptions options_;
};
} // namespace proxygen::coro
