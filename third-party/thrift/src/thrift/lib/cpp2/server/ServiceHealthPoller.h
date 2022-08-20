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

#include <chrono>
#include <vector>

#include <folly/experimental/coro/AsyncGenerator.h>
#include <folly/experimental/observer/Observer.h>

#include <thrift/lib/cpp2/server/PolledServiceHealth.h>

#if FOLLY_HAS_COROUTINES

namespace apache::thrift {

/**
 * A class that encapsulates logic to poll PolledServiceHealth handlers to
 * compute a combined ServiceHealth.
 *
 * The PolledServiceHealth instances will be polled at intervals in a loop.
 */
class ServiceHealthPoller final {
 public:
  ServiceHealthPoller() = delete;

  using ServiceHealth = PolledServiceHealth::ServiceHealth;

  /**
   * @param handlers all service handlers that should be polled for their health
   * @param pollingLiveness how long a computed result should be considered
   *                        "current" - i.e. how long to wait on a computed
   *                        result until polling again
   */
  static folly::coro::AsyncGenerator<ServiceHealth> poll(
      std::vector<PolledServiceHealth*> handlers,
      folly::observer::Observer<std::chrono::milliseconds> pollingLivenessMs);
};

} // namespace apache::thrift

#endif
