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
#include <folly/executors/GlobalExecutor.h>
#include <folly/futures/Future.h>

namespace folly {

template <class F>
auto async(F&& fn) {
  return folly::via<F>(
      getUnsafeMutableGlobalCPUExecutor().get(), std::forward<F>(fn));
}

} // namespace folly
