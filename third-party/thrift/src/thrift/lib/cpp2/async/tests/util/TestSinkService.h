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

#include <thrift/lib/cpp2/async/Sink.h>
#include <thrift/lib/cpp2/async/tests/util/gen-cpp2/TestSinkService.h>

namespace testutil {
namespace testservice {

class TestSinkService : public apache::thrift::ServiceHandler<TestSinkService> {
 public:
  folly::coro::Task<::std::int32_t> co_test() override { co_return 42; }

  apache::thrift::SinkConsumer<int32_t, bool> range(
      int32_t from, int32_t to) override;

  apache::thrift::SinkConsumer<int32_t, bool> rangeThrow(
      int32_t from, int32_t to) override;

  apache::thrift::SinkConsumer<int32_t, bool> rangeFinalResponseThrow(
      int32_t from, int32_t to) override;

  apache::thrift::SinkConsumer<int32_t, int32_t> rangeEarlyResponse(
      int32_t from, int32_t to, int32_t early) override;

  apache::thrift::SinkConsumer<int32_t, bool> unSubscribedSink() override;

  folly::SemiFuture<apache::thrift::SinkConsumer<int32_t, bool>>
  semifuture_unSubscribedSinkSlowReturn() override;

  bool isSinkUnSubscribed() override;

  apache::thrift::ResponseAndSinkConsumer<bool, int32_t, bool> initialThrow()
      override;

  apache::thrift::SinkConsumer<int32_t, bool> rangeChunkTimeout() override;

  apache::thrift::SinkConsumer<int32_t, bool> sinkThrow() override;

  apache::thrift::SinkConsumer<int32_t, bool> sinkFinalThrow() override;

  apache::thrift::SinkConsumer<int32_t, bool> rangeCancelAt(
      int32_t from, int32_t to, int32_t cancelAt) override;

  apache::thrift::SinkConsumer<int32_t, bool> rangeSlowFinalResponse(
      int32_t from, int32_t to) override;

  void purge() override;

 private:
  std::atomic<int> activeSinks_{0};
};

} // namespace testservice
} // namespace testutil
