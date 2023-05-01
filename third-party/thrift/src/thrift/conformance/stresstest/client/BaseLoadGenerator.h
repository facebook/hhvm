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

#include <folly/experimental/coro/AsyncGenerator.h>
#include <folly/experimental/coro/Coroutine.h>

namespace apache {
namespace thrift {
namespace stress {

/**
 * Generates a Signals that indicates the client should create a request.
 */
class BaseLoadGenerator {
 public:
  virtual ~BaseLoadGenerator() = default;
  using Count = int32_t;

  virtual folly::coro::AsyncGenerator<Count> getRequestCount() = 0;
  virtual void start() = 0;
};

} // namespace stress
} // namespace thrift
} // namespace apache
