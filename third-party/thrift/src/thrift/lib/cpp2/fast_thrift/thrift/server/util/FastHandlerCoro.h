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

#include <utility>

#include <folly/Executor.h>
#include <folly/Portability.h>

#if FOLLY_HAS_COROUTINES
#include <folly/coro/Task.h>
#endif

#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/FastHandlerCallback.h>

namespace apache::thrift::fast_thrift::thrift::detail {

#if FOLLY_HAS_COROUTINES

/**
 * Runs a co_<name> handler body and completes the callback with its outcome.
 *
 * Kept out of FastHandlerCallback.h so that only the generated sources that
 * actually dispatch a coroutine pull in <folly/coro/Task.h>.
 *
 * Scheduled on the callback's handler executor, which is the pool the calling
 * dispatcher is already running on. Falls back to the connection's EventBase
 * when the server has no CPU pool, so a coroutine handler still works on a
 * server that never configured one — at the cost of running on the data path,
 * which is the same trade every non-eb method makes in that configuration.
 */
template <typename T>
void fastRunCoro(
    FastHandlerCallbackPtr<T> callback, folly::coro::Task<T> task) {
  auto* executor = callback->getHandlerExecutor();
  folly::coro::co_withExecutor(
      executor != nullptr
          ? folly::Executor::KeepAlive<>(executor)
          : folly::Executor::KeepAlive<>(callback->getEventBase()),
      std::move(task))
      .start([cb = std::move(callback)](auto&& result) mutable noexcept {
        if (result.hasException()) {
          cb->exception(std::move(result.exception()));
        } else {
          cb->result(std::move(result.value()));
        }
      });
}

inline void fastRunCoro(
    FastHandlerCallbackPtr<void> callback, folly::coro::Task<void> task) {
  auto* executor = callback->getHandlerExecutor();
  folly::coro::co_withExecutor(
      executor != nullptr
          ? folly::Executor::KeepAlive<>(executor)
          : folly::Executor::KeepAlive<>(callback->getEventBase()),
      std::move(task))
      .start([cb = std::move(callback)](auto&& result) mutable noexcept {
        if (result.hasException()) {
          cb->exception(std::move(result.exception()));
        } else {
          cb->done();
        }
      });
}

#endif // FOLLY_HAS_COROUTINES

} // namespace apache::thrift::fast_thrift::thrift::detail
