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

#include <folly/Portability.h>
#include <folly/coro/Baton.h>
#include <folly/coro/Task.h>

#include <thrift/lib/cpp2/async/StreamCallbacks.h>

namespace apache::thrift::detail {

class QueueConsumer {
 public:
  virtual ~QueueConsumer() = default;
  virtual void consume() = 0;
  virtual void canceled() = 0;
};

#if FOLLY_HAS_COROUTINES

class CoroConsumer final : public QueueConsumer {
 public:
  void consume() override { baton_.post(); }

  void canceled() override { baton_.post(); }

  folly::coro::Task<void> wait() { co_await baton_; }

 private:
  folly::coro::Baton baton_;
};
#endif

} // namespace apache::thrift::detail
