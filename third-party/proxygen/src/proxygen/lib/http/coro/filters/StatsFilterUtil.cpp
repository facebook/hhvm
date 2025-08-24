/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/filters/StatsFilterUtil.h"
#include "proxygen/lib/http/coro/HTTPSourceFilter.h"
#include "proxygen/lib/http/coro/filters/VisitorFilter.h"
#include "proxygen/lib/http/stats/HttpServerStats.h"

namespace {
// shared state between the proxygen::coro request and response stat filters
struct StatsState {
  ~StatsState() {
    // calculate latency and record request as complete
    const auto latency = proxygen::millisecondsSince(startTime);
    stats->recordRequestComplete(
        latency, error, requestBodyBytes, responseBodyBytes);
  }
  proxygen::TimePoint startTime{};
  size_t requestBodyBytes{0};
  size_t responseBodyBytes{0};
  proxygen::HttpServerStatsIf* stats{nullptr};
  // error observed in either req or res filters.
  proxygen::ProxygenError error{proxygen::ProxygenError::kErrorNone};
};

template <class T>
bool maybeSetError(StatsState& state, const folly::Try<T>& event) {
  if (event.hasException()) {
    state.error = HTTPErrorCode2ProxygenError(getHTTPError(event).code);
    return true;
  }
  return false;
}
} // namespace

namespace proxygen::coro {

// Returns a pair of filters that share StatsState to enable collecting stats
std::pair<HTTPSourceFilter*, HTTPSourceFilter*> StatsFilterUtil::makeFilters(
    HttpServerStatsIf* stats) {
  // shared state between the two filters
  auto state = std::make_shared<StatsState>();
  state->stats = stats;

  // Create req&res filters which is just a specialized visitor filter. Users
  // should invoke .setSource() on the returned filters.
  auto reqFilter = std::make_unique<VisitorFilter>(
      /*source=*/
      nullptr,
      [state](const folly::Try<HTTPHeaderEvent>& headerEvent) {
        constexpr auto zero = TimePoint::duration::zero();
        // request header is always final, startTime should only be set once
        CHECK(state->startTime.time_since_epoch() == zero);
        state->startTime = getCurrentTime();

        if (!maybeSetError(*state, headerEvent)) {
          state->stats->recordRequest(*headerEvent->headers);
        }
      },
      [state](const folly::Try<HTTPBodyEvent>& bodyEvent) {
        if (!maybeSetError(*state, bodyEvent) &&
            bodyEvent->eventType == HTTPBodyEvent::EventType::BODY) {
          state->requestBodyBytes += bodyEvent->event.body.chainLength();
        }
      });

  auto resFilter = std::make_unique<VisitorFilter>(
      /*source=*/
      nullptr,
      [state](const folly::Try<HTTPHeaderEvent>& headerEvent) {
        if (!maybeSetError(*state, headerEvent)) {
          state->stats->recordResponse(*headerEvent->headers);
        }
      },
      [state](const folly::Try<HTTPBodyEvent>& bodyEvent) {
        if (!maybeSetError(*state, bodyEvent) &&
            bodyEvent->eventType == HTTPBodyEvent::EventType::BODY) {
          state->responseBodyBytes += bodyEvent->event.body.chainLength();
        }
      });

  reqFilter->setHeapAllocated(), resFilter->setHeapAllocated();
  return {reqFilter.release(), resFilter.release()};
}

} // namespace proxygen::coro
