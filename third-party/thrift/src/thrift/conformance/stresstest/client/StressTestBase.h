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

#include <folly/coro/Task.h>
#include <thrift/conformance/stresstest/client/StressTestClient.h>

namespace apache::thrift::stress {

/**
 * Base interface for stress tests
 */
class StressTestBase {
 public:
  virtual ~StressTestBase() = default;

  virtual folly::coro::Task<void> runWorkload(
      StressTestClient* client) const = 0;
};

} // namespace apache::thrift::stress
