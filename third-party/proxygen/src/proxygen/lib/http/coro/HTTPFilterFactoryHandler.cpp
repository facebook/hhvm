/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/HTTPFilterFactoryHandler.h"
#include "proxygen/lib/http/coro/HTTPFixedSource.h"
#include "proxygen/lib/http/coro/HTTPSourceFilterChain.h"
#include "proxygen/lib/http/coro/filters/FilterFactory.h"

namespace proxygen::coro {

folly::coro::Task<HTTPSourceHolder> HTTPFilterFactoryHandler::handleRequest(
    folly::EventBase* evb,
    HTTPSessionContextPtr ctx,
    HTTPSourceHolder requestSource) {
  // construct request & response filter chains (in supplied order for request
  // chain and reverse order for response chain) maintaining the order in which
  // the filter factories were added
  proxygen::coro::FilterChain reqChain, resChain;
  reqChain.setSource(requestSource.release());
  for (auto& filterFactory : filterFactories_) {
    auto [reqFilter, resFilter] = filterFactory->makeFilters();
    if (reqFilter) {
      reqChain.insertFront(reqFilter);
    }
    if (resFilter) {
      resChain.insertEnd(resFilter);
    }
  }

  // hand off request to the next handler
  CHECK(getNextHandler());
  auto nextHandlerResult = co_await co_awaitTry(
      getNextHandler()->handleRequest(evb, std::move(ctx), reqChain.release()));

  if (nextHandlerResult.hasValue() && nextHandlerResult->readable()) {
    resChain.setSource(nextHandlerResult->release());
    co_return resChain.release();
  }

  /**
   * If either handleRequest yielded an error, or unreadable source (i.e
   * handleRequest returned nullptr), wrap in ErrorSource to execute response
   * filters. If the user returned unreadable source, we have to create a 500
   * status code response source (similar to HTTPCoroSession) here so we can
   * execute the response filters.
   */
  auto* errorSource = nextHandlerResult.hasException()
                          ? new HTTPErrorSource(getHTTPError(nextHandlerResult),
                                                /*heapAllocated=*/true)
                          : getErrorResponse(500);
  resChain.setSource(errorSource);

  co_return resChain.release();
}

HTTPFilterFactoryHandler&& HTTPFilterFactoryHandler::addFilterFactory(
    std::shared_ptr<FilterFactory>&& ff) {
  if (ff) {
    filterFactories_.push_back(std::move(ff));
  }
  return std::move(*this);
}

} // namespace proxygen::coro
