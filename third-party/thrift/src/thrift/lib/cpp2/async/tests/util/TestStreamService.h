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

#include <folly/io/async/ScopedEventBaseThread.h>
#include <thrift/lib/cpp2/async/ServerStreamMultiPublisher.h>
#include <thrift/lib/cpp2/async/tests/util/gen-cpp2/TestStreamService.h>

namespace testutil {
namespace testservice {

class TestStreamGeneratorService
    : public apache::thrift::ServiceHandler<TestStreamService> {
 public:
  apache::thrift::ServerStream<int32_t> range(
      int32_t from, int32_t to) override;

  apache::thrift::ServerStream<int32_t> rangeThrow(
      int32_t from, int32_t to) override;

  apache::thrift::ServerStream<int32_t> rangeThrowUDE(
      int32_t from, int32_t to) override;
};

class TestStreamPublisherService
    : public apache::thrift::ServiceHandler<TestStreamService> {
 public:
  apache::thrift::ServerStream<int32_t> range(
      int32_t from, int32_t to) override;

  apache::thrift::ServerStream<int32_t> rangeThrow(
      int32_t from, int32_t to) override;

  apache::thrift::ServerStream<int32_t> rangeThrowUDE(
      int32_t from, int32_t to) override;
};

class TestStreamGeneratorWithHeaderService
    : public apache::thrift::ServiceHandler<TestStreamService> {
 public:
  apache::thrift::ServerStream<int32_t> range(
      int32_t from, int32_t to) override;

  apache::thrift::ServerStream<int32_t> rangeThrow(
      int32_t from, int32_t to) override;

  apache::thrift::ServerStream<int32_t> rangeThrowUDE(
      int32_t from, int32_t to) override;
};

class TestStreamPublisherWithHeaderService
    : public apache::thrift::ServiceHandler<TestStreamService> {
 public:
  apache::thrift::ServerStream<int32_t> range(
      int32_t from, int32_t to) override;

  apache::thrift::ServerStream<int32_t> rangeThrow(
      int32_t from, int32_t to) override;

  apache::thrift::ServerStream<int32_t> rangeThrowUDE(
      int32_t from, int32_t to) override;
};

class TestStreamMultiPublisherService
    : public apache::thrift::ServiceHandler<TestStreamService> {
 public:
  apache::thrift::ServerStream<int32_t> range(
      int32_t from, int32_t to) override;

  apache::thrift::ServerStream<int32_t> rangeThrow(
      int32_t from, int32_t to) override;

  apache::thrift::ServerStream<int32_t> rangeThrowUDE(
      int32_t from, int32_t to) override;

  apache::thrift::ServerStream<int32_t> rangeWaitForCancellation(
      int32_t from, int32_t to) override;

  apache::thrift::ServerStream<int32_t> uncompletedPublisherDestructor()
      override;

  apache::thrift::ServerStream<int32_t> uncompletedPublisherMoveAssignment()
      override;

  folly::coro::Baton* waitForCancellation_;

 private:
  apache::thrift::ServerStream<int32_t> range(
      int32_t from, int32_t to, bool slow, folly::exception_wrapper ew);

  apache::thrift::ServerStreamMultiPublisher<int32_t> multipub_;
  std::atomic<size_t> activeStreams_{0};
};

class TestStreamMultiPublisherWithHeaderService
    : public apache::thrift::ServiceHandler<TestStreamService> {
 public:
  apache::thrift::ServerStream<int32_t> range(
      int32_t from, int32_t to) override;

  apache::thrift::ServerStream<int32_t> rangeThrow(
      int32_t from, int32_t to) override;

  apache::thrift::ServerStream<int32_t> rangeThrowUDE(
      int32_t from, int32_t to) override;

  apache::thrift::ServerStream<int32_t> rangeWaitForCancellation(
      int32_t from, int32_t to) override;

  apache::thrift::ServerStream<int32_t> uncompletedPublisherDestructor()
      override;

  apache::thrift::ServerStream<int32_t> uncompletedPublisherMoveAssignment()
      override;

  folly::coro::Baton* waitForCancellation_;

 private:
  apache::thrift::ServerStream<int32_t> range(
      int32_t from, int32_t to, bool slow, folly::exception_wrapper ew);

  apache::thrift::ServerStreamMultiPublisher<int32_t, true> multipub_;
  std::atomic<size_t> activeStreams_{0};
};

} // namespace testservice
} // namespace testutil
