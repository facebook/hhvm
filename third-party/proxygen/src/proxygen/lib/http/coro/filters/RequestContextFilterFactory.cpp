/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/filters/RequestContextFilterFactory.h"
#include "folly/io/async/Request.h"

namespace proxygen::coro {

namespace {
class RequestContextFilter : public HTTPSourceFilter {
 public:
  explicit RequestContextFilter(
      std::shared_ptr<folly::RequestContext> context) noexcept
      : context_{std::move(context)} {
  }

  folly::coro::Task<HTTPHeaderEvent> readHeaderEvent() override {
    folly::RequestContextScopeGuard guard{context_};
    co_return co_await folly::coro::co_nothrow(
        HTTPSourceFilter::readHeaderEvent());
  }

  folly::coro::Task<HTTPBodyEvent> readBodyEvent(uint32_t max) override {
    folly::RequestContextScopeGuard guard{context_};
    co_return co_await folly::coro::co_nothrow(
        HTTPSourceFilter::readBodyEvent());
  }

 private:
  const std::shared_ptr<folly::RequestContext> context_;
};

} // namespace

std::pair<HTTPSourceFilter*, HTTPSourceFilter*>
RequestContextFilterFactory::makeFilters() {
  folly::RequestContextScopeGuard guard;
  auto context = folly::RequestContext::saveContext();
  auto requestFilter =
      std::make_unique<RequestContextFilter>(std::as_const(context));
  requestFilter->setHeapAllocated();
  auto responseFilter =
      std::make_unique<RequestContextFilter>(std::move(context));
  responseFilter->setHeapAllocated();
  return {requestFilter.release(), responseFilter.release()};
}

} // namespace proxygen::coro
