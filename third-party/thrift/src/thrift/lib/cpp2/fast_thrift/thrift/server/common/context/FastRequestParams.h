/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <folly/Executor.h>
#include <folly/io/async/EventBase.h>

namespace apache::thrift::fast_thrift::thrift {

class ThriftRequestContext;

/**
 * Per-request handles a handler can reach without them appearing in its
 * signature: the request context, the connection's EventBase, and the
 * executor the body is running on.
 *
 * Reached two different ways, because neither covers both handler surfaces.
 * A sync_ body reads the thread-local published by FastRequestParamsGuard,
 * which is only valid for the synchronous span of the dispatcher — safe,
 * because a sync_ body cannot suspend. A co_ body outlives that span and may
 * resume on another thread, so it takes these by value as a parameter
 * instead.
 */
class FastRequestParams {
 public:
  FastRequestParams() = default;

  FastRequestParams(
      ThriftRequestContext* requestContext,
      folly::EventBase* eventBase,
      folly::Executor* handlerExecutor) noexcept
      : requestContext_(requestContext),
        eventBase_(eventBase),
        handlerExecutor_(handlerExecutor) {}

  ThriftRequestContext* getRequestContext() const noexcept {
    return requestContext_;
  }
  folly::EventBase* getEventBase() const noexcept { return eventBase_; }
  // Null when the server has no CPU pool, i.e. the body is running inline on
  // the EventBase.
  folly::Executor* getHandlerExecutor() const noexcept {
    return handlerExecutor_;
  }

 private:
  ThriftRequestContext* requestContext_{nullptr};
  folly::EventBase* eventBase_{nullptr};
  folly::Executor* handlerExecutor_{nullptr};
};

namespace detail {

// Function-local so the header stays definition-free; one instance per thread
// for the whole process, overwritten per dispatch.
inline FastRequestParams& tlRequestParams() {
  static thread_local FastRequestParams params;
  return params;
}

} // namespace detail

/**
 * Publishes params for the synchronous span of a generated dispatcher.
 *
 * Cleared on the way out rather than left stale: a sync_ body reached by any
 * route other than the dispatcher would otherwise read whichever request this
 * thread happened to serve last.
 */
class FastRequestParamsGuard {
 public:
  explicit FastRequestParamsGuard(FastRequestParams params) noexcept {
    detail::tlRequestParams() = params;
  }

  FastRequestParamsGuard(const FastRequestParamsGuard&) = delete;
  FastRequestParamsGuard& operator=(const FastRequestParamsGuard&) = delete;
  FastRequestParamsGuard(FastRequestParamsGuard&&) = delete;
  FastRequestParamsGuard& operator=(FastRequestParamsGuard&&) = delete;

  ~FastRequestParamsGuard() { detail::tlRequestParams() = FastRequestParams(); }
};

} // namespace apache::thrift::fast_thrift::thrift
