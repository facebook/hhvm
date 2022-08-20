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

#include <folly/fibers/FiberManager.h>
#include <folly/fibers/async/Async.h>
#include <folly/fibers/traits.h>

namespace folly {
namespace fibers {
namespace async {

/**
 * Async annotated wrapper around fibers::await
 */
template <typename F>
Async<typename FirstArgOf<F>::type::value_type> promiseWait(F&& func) {
  // Call into blocking API
  return fibers::await_async(std::forward<F>(func));
}

} // namespace async
} // namespace fibers
} // namespace folly
