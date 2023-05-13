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

#include <thrift/lib/cpp2/async/tests/util/TestStreamService.h>

#include <folly/experimental/coro/AsyncScope.h>
#include <folly/portability/GTest.h>

namespace testutil {
namespace testservice {

apache::thrift::ServerStream<int32_t> TestStreamGeneratorService::range(
    int32_t from, int32_t to) {
  for (int i = from; i <= to; i++) {
    co_yield std::move(i);
  }
}

apache::thrift::ServerStream<int32_t> TestStreamGeneratorService::rangeThrow(
    int32_t from, int32_t to) {
  for (int i = from; i <= to; i++) {
    co_yield std::move(i);
  }
  throw std::runtime_error("I am a search bar");
}

apache::thrift::ServerStream<int32_t> TestStreamGeneratorService::rangeThrowUDE(
    int32_t from, int32_t to) {
  for (int i = from; i <= to; i++) {
    co_yield std::move(i);
  }
  throw UserDefinedException();
}

apache::thrift::ServerStream<int32_t> TestStreamPublisherService::range(
    int32_t from, int32_t to) {
  auto [stream, publisher] =
      apache::thrift::ServerStream<int32_t>::createPublisher([] {});

  for (int i = from; i <= to; i++) {
    publisher.next(i);
  }
  std::move(publisher).complete();

  return std::move(stream);
}

apache::thrift::ServerStream<int32_t> TestStreamPublisherService::rangeThrow(
    int32_t from, int32_t to) {
  auto [stream, publisher] =
      apache::thrift::ServerStream<int32_t>::createPublisher([] {});

  for (int i = from; i <= to; i++) {
    publisher.next(i);
  }
  std::move(publisher).complete(std::runtime_error("I am a search bar"));

  return std::move(stream);
}

apache::thrift::ServerStream<int32_t> TestStreamPublisherService::rangeThrowUDE(
    int32_t from, int32_t to) {
  auto [stream, publisher] =
      apache::thrift::ServerStream<int32_t>::createPublisher([] {});

  for (int i = from; i <= to; i++) {
    publisher.next(i);
  }
  std::move(publisher).complete(UserDefinedException());

  return std::move(stream);
}

using RichPayload = apache::thrift::detail::RichPayloadToSend<int32_t>;
using MessageVariant = apache::thrift::detail::MessageVariant<int32_t>;
using apache::thrift::detail::OrderedHeader;
using apache::thrift::detail::UnorderedHeader;

apache::thrift::ServerStream<int32_t>
TestStreamGeneratorWithHeaderService::range(int32_t from, int32_t to) {
  return folly::coro::co_invoke(
      [=]() -> folly::coro::AsyncGenerator<MessageVariant&&> {
        for (int i = from; i <= to; i++) {
          co_yield RichPayload{i, {{"val", std::to_string(i)}}};
          co_yield UnorderedHeader{{{"val", std::to_string(i)}}};
          co_yield OrderedHeader{{{"val", std::to_string(i)}}};
        }
      });
}

apache::thrift::ServerStream<int32_t>
TestStreamGeneratorWithHeaderService::rangeThrow(int32_t from, int32_t to) {
  return folly::coro::co_invoke(
      [=]() -> folly::coro::AsyncGenerator<MessageVariant&&> {
        for (int i = from; i <= to; i++) {
          co_yield RichPayload{i, {{"val", std::to_string(i)}}};
          co_yield UnorderedHeader{{{"val", std::to_string(i)}}};
          co_yield OrderedHeader{{{"val", std::to_string(i)}}};
        }
        throw std::runtime_error("I am a search bar");
      });
}

apache::thrift::ServerStream<int32_t>
TestStreamGeneratorWithHeaderService::rangeThrowUDE(int32_t from, int32_t to) {
  return folly::coro::co_invoke(
      [=]() -> folly::coro::AsyncGenerator<MessageVariant&&> {
        for (int i = from; i <= to; i++) {
          co_yield RichPayload{i, {{"val", std::to_string(i)}}};
          co_yield UnorderedHeader{{{"val", std::to_string(i)}}};
          co_yield OrderedHeader{{{"val", std::to_string(i)}}};
        }
        throw UserDefinedException();
      });
}

apache::thrift::ServerStream<int32_t>
TestStreamPublisherWithHeaderService::range(int32_t from, int32_t to) {
  auto [stream, publisher] =
      apache::thrift::ServerStream<int32_t>::createPublisherWithHeader([] {});

  for (int i = from; i <= to; i++) {
    publisher.next(RichPayload{i, {{"val", std::to_string(i)}}});
    publisher.next(UnorderedHeader{{{"val", std::to_string(i)}}});
    publisher.next(OrderedHeader{{{"val", std::to_string(i)}}});
  }
  std::move(publisher).complete();

  return std::move(stream);
}

apache::thrift::ServerStream<int32_t>
TestStreamPublisherWithHeaderService::rangeThrow(int32_t from, int32_t to) {
  auto [stream, publisher] =
      apache::thrift::ServerStream<int32_t>::createPublisherWithHeader([] {});

  for (int i = from; i <= to; i++) {
    publisher.next(RichPayload{i, {{"val", std::to_string(i)}}});
    publisher.next(UnorderedHeader{{{"val", std::to_string(i)}}});
    publisher.next(OrderedHeader{{{"val", std::to_string(i)}}});
  }
  std::move(publisher).complete(std::runtime_error("I am a search bar"));

  return std::move(stream);
}

apache::thrift::ServerStream<int32_t>
TestStreamPublisherWithHeaderService::rangeThrowUDE(int32_t from, int32_t to) {
  auto [stream, publisher] =
      apache::thrift::ServerStream<int32_t>::createPublisherWithHeader([] {});

  for (int i = from; i <= to; i++) {
    publisher.next(RichPayload{i, {{"val", std::to_string(i)}}});
    publisher.next(UnorderedHeader{{{"val", std::to_string(i)}}});
    publisher.next(OrderedHeader{{{"val", std::to_string(i)}}});
  }
  std::move(publisher).complete(UserDefinedException());

  return std::move(stream);
}

apache::thrift::ServerStream<int32_t> TestStreamMultiPublisherService::range(
    int32_t from, int32_t to) {
  return range(from, to, false, folly::exception_wrapper());
}

apache::thrift::ServerStream<int32_t>
TestStreamMultiPublisherService::rangeWaitForCancellation(
    int32_t from, int32_t to) {
  return range(from, to, true, folly::exception_wrapper());
}

apache::thrift::ServerStream<int32_t>
TestStreamMultiPublisherService::rangeThrow(int32_t from, int32_t to) {
  return range(from, to, false, std::runtime_error("oops"));
}

apache::thrift::ServerStream<int32_t>
TestStreamMultiPublisherService::rangeThrowUDE(int32_t from, int32_t to) {
  return range(from, to, false, UserDefinedException());
}

apache::thrift::ServerStream<int32_t>
TestStreamMultiPublisherService::uncompletedPublisherDestructor() {
  auto stream = multipub_.addStream();
  EXPECT_DEATH(
      [_ = std::move(multipub_)] {}(),
      "StreamMultiPublisher must be completed or all streams must be cancelled");
  return stream;
}

apache::thrift::ServerStream<int32_t>
TestStreamMultiPublisherService::uncompletedPublisherMoveAssignment() {
  auto stream = multipub_.addStream();
  EXPECT_DEATH(
      multipub_ = apache::thrift::ServerStreamMultiPublisher<int32_t>(),
      "StreamMultiPublisher must be completed or all streams must be cancelled");
  return stream;
}

apache::thrift::ServerStream<int32_t> TestStreamMultiPublisherService::range(
    int32_t from,
    int32_t to,
    bool waitForCancellation,
    folly::exception_wrapper ew) {
  auto stream = multipub_.addStream([&] { --activeStreams_; });
  if (++activeStreams_ == 5) {
    getAsyncScope()->add(
        folly::coro::co_invoke(
            [=, ew = std::move(ew)]() mutable -> folly::coro::Task<void> {
              for (int i = from; i <= to; i++) {
                if (waitForCancellation && i == from + 1) {
                  co_await *waitForCancellation_;
                }
                multipub_.next(i);
                co_await folly::coro::co_reschedule_on_current_executor;
              }
              if (ew) {
                std::move(multipub_).complete(std::move(ew));
              } else {
                std::move(multipub_).complete();
              }
              EXPECT_EQ(0, activeStreams_);
            })
            .scheduleOn(getEventBase()));
  }
  return stream;
}

apache::thrift::ServerStream<int32_t>
TestStreamMultiPublisherWithHeaderService::range(int32_t from, int32_t to) {
  return range(from, to, false, folly::exception_wrapper());
}

apache::thrift::ServerStream<int32_t>
TestStreamMultiPublisherWithHeaderService::rangeWaitForCancellation(
    int32_t from, int32_t to) {
  return range(from, to, true, folly::exception_wrapper());
}

apache::thrift::ServerStream<int32_t>
TestStreamMultiPublisherWithHeaderService::rangeThrow(
    int32_t from, int32_t to) {
  return range(from, to, false, std::runtime_error("oops"));
}

apache::thrift::ServerStream<int32_t>
TestStreamMultiPublisherWithHeaderService::rangeThrowUDE(
    int32_t from, int32_t to) {
  return range(from, to, false, UserDefinedException());
}

apache::thrift::ServerStream<int32_t>
TestStreamMultiPublisherWithHeaderService::uncompletedPublisherDestructor() {
  auto stream = multipub_.addStream();
  EXPECT_DEATH(
      [_ = std::move(multipub_)] {}(),
      "StreamMultiPublisher must be completed or all streams must be cancelled");
  return stream;
}
apache::thrift::ServerStream<int32_t>
TestStreamMultiPublisherWithHeaderService::
    uncompletedPublisherMoveAssignment() {
  auto stream = multipub_.addStream();
  auto multipub = apache::thrift::ServerStreamMultiPublisher<int32_t, true>();
  EXPECT_DEATH(
      multipub_ = std::move(multipub),
      "StreamMultiPublisher must be completed or all streams must be cancelled");
  return stream;
}

apache::thrift::ServerStream<int32_t>
TestStreamMultiPublisherWithHeaderService::range(
    int32_t from,
    int32_t to,
    bool waitForCancellation,
    folly::exception_wrapper ew) {
  auto stream = multipub_.addStream([&] { --activeStreams_; });
  if (++activeStreams_ == 5) {
    getAsyncScope()->add(
        folly::coro::co_invoke(
            [=, ew = std::move(ew)]() mutable -> folly::coro::Task<void> {
              for (int i = from; i <= to; i++) {
                if (waitForCancellation && i == from + 1) {
                  co_await *waitForCancellation_;
                }
                multipub_.next(RichPayload{i, {{"val", std::to_string(i)}}});
                multipub_.next(UnorderedHeader{{{"val", std::to_string(i)}}});
                multipub_.next(OrderedHeader{{{"val", std::to_string(i)}}});
                co_await folly::coro::co_reschedule_on_current_executor;
              }
              if (ew) {
                std::move(multipub_).complete(std::move(ew));
              } else {
                std::move(multipub_).complete();
              }
              EXPECT_EQ(0, activeStreams_);
            })
            .scheduleOn(getEventBase()));
  }
  return stream;
}

} // namespace testservice
} // namespace testutil
